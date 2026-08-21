#include "../include/quickdmg_kit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonKeyDerivation.h>
#include <CommonCrypto/CommonHMAC.h>

#include "Common/MyWindows.h"
#define INITGUID
#include "Common/MyInitGuid.h"
#include "7zip/ICoder.h"
#include "7zip/IPassword.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamUtils.h"
#include "7zip/Common/RegisterArc.h"
#include "Common/StringConvert.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConv.h"

using namespace NWindows;
using namespace NFile;

extern "C" {
  UInt32 IsArc_Ext(const Byte *p, size_t size) { (void)p; (void)size; return 0; }
  UInt32 IsArc_Fat(const Byte *p, size_t size) { (void)p; (void)size; return 0; }
}

static const unsigned kNumArcsMax = 128;
static unsigned g_NumArcs = 0;
static const CArcInfo *g_Arcs[kNumArcsMax];

void RegisterArc(const CArcInfo *arcInfo) throw()
{
    if (g_NumArcs < kNumArcsMax) {
        g_Arcs[g_NumArcs++] = arcInfo;
    }
}

static uint32_t read_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint64_t read_u64_be(const uint8_t *p) {
    return ((uint64_t)read_u32_be(p)<<32) | read_u32_be(p+4);
}

// Stream for Encrypted Apple DMG
class CEncryptedDmgInStream :
    public IInStream,
    public CMyUnknownImp
{
    Z7_COM_UNKNOWN_IMP_1(IInStream)
public:
    CMyComPtr<IInStream> _baseStream;
    uint64_t _dataOffset;
    uint64_t _dataLen;
    uint32_t _blockSize;
    uint32_t _keyBits;
    uint8_t _aesKey[32];
    uint8_t _hmacKey[32];
    size_t _aesKeyLen;
    size_t _hmacKeyLen;
    uint64_t _curPos;

    CEncryptedDmgInStream(IInStream *baseStream, uint64_t dataOffset, uint64_t dataLen,
                          uint32_t blockSize, uint32_t keyBits,
                          const uint8_t *aesKey, size_t aesKeyLen,
                          const uint8_t *hmacKey, size_t hmacKeyLen)
        : _baseStream(baseStream), _dataOffset(dataOffset), _dataLen(dataLen),
          _blockSize(blockSize), _keyBits(keyBits),
          _aesKeyLen(aesKeyLen), _hmacKeyLen(hmacKeyLen), _curPos(0)
    {
        memcpy(_aesKey, aesKey, aesKeyLen);
        memcpy(_hmacKey, hmacKey, hmacKeyLen);
    }

    Z7_COM7F_IMF(Read(void *data, UInt32 size, UInt32 *processedSize)) {
        if (processedSize) *processedSize = 0;
        if (_curPos >= _dataLen || size == 0) return S_OK;

        UInt64 rem = _dataLen - _curPos;
        UInt32 toRead = (size > rem) ? (UInt32)rem : size;
        UInt32 totalRead = 0;

        while (totalRead < toRead) {
            uint64_t blockIdx = (_curPos + totalRead) / _blockSize;
            uint32_t blockOffset = (uint32_t)((_curPos + totalRead) % _blockSize);
            uint32_t chunkAvail = _blockSize - blockOffset;
            uint32_t chunkToCopy = toRead - totalRead;
            if (chunkToCopy > chunkAvail) chunkToCopy = chunkAvail;

            uint64_t rawFilePos = _dataOffset + blockIdx * _blockSize;
            RINOK(_baseStream->Seek(rawFilePos, STREAM_SEEK_SET, NULL));

            uint8_t rawBlock[4096];
            UInt32 rawRead = 0;
            RINOK(_baseStream->Read(rawBlock, _blockSize, &rawRead));
            if (rawRead < _blockSize) {
                memset(rawBlock + rawRead, 0, _blockSize - rawRead);
            }

            uint8_t b_be[4];
            b_be[0] = (blockIdx >> 24) & 0xFF;
            b_be[1] = (blockIdx >> 16) & 0xFF;
            b_be[2] = (blockIdx >> 8) & 0xFF;
            b_be[3] = (blockIdx) & 0xFF;

            uint8_t blockHmac[20];
            CCHmac(kCCHmacAlgSHA1, _hmacKey, _hmacKeyLen, b_be, 4, blockHmac);

            uint8_t decBlock[4096];
            size_t decMoved = 0;
            CCCrypt(kCCDecrypt, kCCAlgorithmAES, 0, _aesKey, _aesKeyLen, blockHmac,
                    rawBlock, _blockSize, decBlock, sizeof(decBlock), &decMoved);

            memcpy((uint8_t *)data + totalRead, decBlock + blockOffset, chunkToCopy);
            totalRead += chunkToCopy;
        }

        _curPos += totalRead;
        if (processedSize) *processedSize = totalRead;
        return S_OK;
    }

    Z7_COM7F_IMF(Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)) {
        switch (seekOrigin) {
            case STREAM_SEEK_SET: _curPos = offset; break;
            case STREAM_SEEK_CUR: _curPos += offset; break;
            case STREAM_SEEK_END: _curPos = _dataLen + offset; break;
            default: return E_INVALIDARG;
        }
        if (newPosition) *newPosition = _curPos;
        return S_OK;
    }
};

class CDmgPasswordCallback :
    public ICryptoGetTextPassword,
    public IArchiveOpenCallback,
    public CMyUnknownImp
{
    Z7_COM_UNKNOWN_IMP_2(ICryptoGetTextPassword, IArchiveOpenCallback)
public:
    quickdmg_password_cb _cb;
    void *_userData;

    CDmgPasswordCallback(quickdmg_password_cb cb, void *userData)
        : _cb(cb), _userData(userData) {}

    Z7_COM7F_IMF(CryptoGetTextPassword(BSTR *password)) {
        if (!password) return E_INVALIDARG;
        *password = NULL;
        if (!_cb) return E_ABORT;

        char pwd[512] = {0};
        if (!_cb(pwd, sizeof(pwd), _userData)) {
            return E_ABORT;
        }

        UString ustr = MultiByteToUnicodeString(pwd);
        *password = ::SysAllocString(ustr.Ptr());
        return S_OK;
    }

    Z7_COM7F_IMF(SetTotal(const UInt64 *files, const UInt64 *bytes)) { return S_OK; }
    Z7_COM7F_IMF(SetCompleted(const UInt64 *files, const UInt64 *bytes)) { return S_OK; }
};

struct quickdmg_archive {
    CMyComPtr<IInArchive> archive;
    CMyComPtr<IInStream> inStream;
    char *filePath;
    uint32_t numItems;
    AString lastPath;
    AString lastSymLink;
};

static bool create_directories_recursively(const char *dir_path) {
    char tmp[1024];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir_path);
    len = strlen(tmp);
    if (len == 0) return true;
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) return false;
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) return false;
    return true;
}

class CDmgExtractCallback :
    public IArchiveExtractCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp
{
    Z7_COM_UNKNOWN_IMP_2(IArchiveExtractCallback, ICryptoGetTextPassword)
public:
    quickdmg_archive *_ar;
    const char *_destDir;
    quickdmg_progress_cb _progressCb;
    void *_userData;

    uint64_t _totalBytes;
    uint64_t _completedBytes;
    AString _currentOutPath;
    CMyComPtr<ISequentialOutStream> _outFileStream;
    bool _isSymLink;
    AString _symLinkTarget;
    uint32_t _posixAttrib;

    CDmgExtractCallback(quickdmg_archive *ar, const char *destDir,
                        quickdmg_progress_cb progressCb, void *userData)
        : _ar(ar), _destDir(destDir), _progressCb(progressCb), _userData(userData),
          _totalBytes(0), _completedBytes(0), _isSymLink(false), _posixAttrib(0) {}

    Z7_COM7F_IMF(SetTotal(UInt64 total)) {
        _totalBytes = total;
        return S_OK;
    }

    Z7_COM7F_IMF(SetCompleted(const UInt64 *completeValue)) {
        if (completeValue) {
            _completedBytes = *completeValue;
            if (_progressCb) {
                _progressCb(_completedBytes, _totalBytes, _currentOutPath.Ptr(), _userData);
            }
        }
        return S_OK;
    }

    Z7_COM7F_IMF(GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)) {
        *outStream = NULL;
        _outFileStream.Release();
        _isSymLink = false;
        _symLinkTarget.Empty();
        _posixAttrib = 0;

        if (askExtractMode != NArchive::NExtract::NAskMode::kExtract) {
            return S_OK;
        }

        NWindows::NCOM::CPropVariant propPath;
        RINOK(_ar->archive->GetProperty(index, kpidPath, &propPath));
        if (propPath.vt != VT_BSTR) return S_OK;

        AString relPath = UnicodeStringToMultiByte(propPath.bstrVal, CP_UTF8);
        _currentOutPath = _destDir;
        _currentOutPath += "/";
        _currentOutPath += relPath;

        NWindows::NCOM::CPropVariant propIsDir;
        bool isDir = false;
        if (_ar->archive->GetProperty(index, kpidIsDir, &propIsDir) == S_OK && propIsDir.vt == VT_BOOL) {
            isDir = (propIsDir.boolVal != VARIANT_FALSE);
        }

        NWindows::NCOM::CPropVariant propSymLink;
        if (_ar->archive->GetProperty(index, kpidSymLink, &propSymLink) == S_OK && propSymLink.vt == VT_BSTR) {
            _isSymLink = true;
            _symLinkTarget = UnicodeStringToMultiByte(propSymLink.bstrVal, CP_UTF8);
        }

        NWindows::NCOM::CPropVariant propAttrib;
        if (_ar->archive->GetProperty(index, kpidPosixAttrib, &propAttrib) == S_OK && propAttrib.vt == VT_UI4) {
            _posixAttrib = propAttrib.ulVal;
        }

        char parentDir[1024];
        snprintf(parentDir, sizeof(parentDir), "%s", _currentOutPath.Ptr());
        char *lastSlash = strrchr(parentDir, '/');
        if (lastSlash) {
            *lastSlash = 0;
            create_directories_recursively(parentDir);
        }

        if (_isSymLink) {
            unlink(_currentOutPath.Ptr());
            symlink(_symLinkTarget.Ptr(), _currentOutPath.Ptr());
            return S_OK;
        }

        if (isDir) {
            create_directories_recursively(_currentOutPath.Ptr());
            if (_posixAttrib != 0) {
                chmod(_currentOutPath.Ptr(), _posixAttrib & 07777);
            }
            return S_OK;
        }

        COutFileStream *fileStreamSpec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> outStreamLoc(fileStreamSpec);
        if (!fileStreamSpec->Create_ALWAYS(us2fs(propPath.bstrVal))) {
            if (!fileStreamSpec->Create_ALWAYS(us2fs(MultiByteToUnicodeString(_currentOutPath.Ptr(), CP_UTF8)))) {
                return E_FAIL;
            }
        }

        _outFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
        return S_OK;
    }

    Z7_COM7F_IMF(PrepareOperation(Int32 askExtractMode)) { return S_OK; }

    Z7_COM7F_IMF(SetOperationResult(Int32 resultEOperationResult)) {
        if (_outFileStream) {
            _outFileStream.Release();
        }
        if (!_isSymLink && _posixAttrib != 0 && !_currentOutPath.IsEmpty()) {
            chmod(_currentOutPath.Ptr(), _posixAttrib & 07777);
        }
        return S_OK;
    }

    Z7_COM7F_IMF(CryptoGetTextPassword(BSTR *password)) {
        *password = NULL;
        return S_OK;
    }
};

int quickdmg_open(const char *file_path,
                  quickdmg_password_cb pass_cb,
                  void *user_data,
                  quickdmg_archive **out_archive)
{
    if (!file_path || !out_archive) return QUICKDMG_ERROR_INVALID_ARG;
    *out_archive = NULL;

    CInFileStream *rawFileStream = new CInFileStream;
    CMyComPtr<IInStream> inStream(rawFileStream);
    if (!rawFileStream->Open(file_path)) {
        return QUICKDMG_ERROR_OPEN;
    }

    // Check for Apple Encrypted DMG (encrcdsa)
    uint8_t hdr[1024];
    UInt32 readHdr = 0;
    if (inStream->Read(hdr, sizeof(hdr), &readHdr) == S_OK && readHdr >= 0x60 && memcmp(hdr, "encrcdsa", 8) == 0) {
        uint32_t keyBits = read_u32_be(hdr + 0x18);
        uint32_t bytesPerBlock = read_u32_be(hdr + 0x34);
        uint64_t dataLen = read_u64_be(hdr + 0x38);
        uint64_t offsetToDataStart = read_u64_be(hdr + 0x40);
        uint64_t itemOffset = read_u64_be(hdr + 0x50);

        if (itemOffset + 0x68 > readHdr) {
            return QUICKDMG_ERROR_CORRUPT;
        }

        const uint8_t *wp = hdr + itemOffset;
        uint64_t kdfIter = read_u64_be(wp + 0x04);
        uint32_t kdfSaltLen = read_u32_be(wp + 0x0c);
        const uint8_t *kdfSalt = wp + 0x10;
        const uint8_t *blobEncIv = wp + 0x34;
        uint32_t blobEncKeyBits = read_u32_be(wp + 0x54);
        uint32_t encryptedKeyblobLen = read_u32_be(wp + 0x64);
        const uint8_t *encryptedKeyblob = wp + 0x68;

        if (!pass_cb) {
            return QUICKDMG_ERROR_PASSWORD;
        }

        char password[512] = {0};
        if (!pass_cb(password, sizeof(password), user_data)) {
            return QUICKDMG_ERROR_PASSWORD;
        }

        uint8_t derived_key[32];
        CCKeyDerivationPBKDF(kCCPBKDF2, password, strlen(password), kdfSalt, kdfSaltLen,
                             kCCPRFHmacAlgSHA1, (unsigned)kdfIter, derived_key, 32);

        uint8_t unwrapped[128];
        size_t unwrapped_len = 0;
        CCCryptorStatus status = CCCrypt(kCCDecrypt, kCCAlgorithm3DES, kCCOptionPKCS7Padding,
                                         derived_key, blobEncKeyBits / 8, blobEncIv,
                                         encryptedKeyblob, encryptedKeyblobLen,
                                         unwrapped, sizeof(unwrapped), &unwrapped_len);

        if (status != kCCSuccess) {
            return QUICKDMG_ERROR_PASSWORD;
        }

        size_t aes_key_len = keyBits / 8;
        size_t hmac_key_len = 20;
        const uint8_t *aes_key = unwrapped;
        const uint8_t *hmac_key = unwrapped + aes_key_len;

        // Verify password correctness by decrypting trailer
        uint64_t lastBlockIdx = (dataLen - 1) / bytesPerBlock;
        uint64_t rawFilePos = offsetToDataStart + lastBlockIdx * bytesPerBlock;
        inStream->Seek(rawFilePos, STREAM_SEEK_SET, NULL);

        uint8_t testRawBlock[4096];
        UInt32 testRawRead = 0;
        inStream->Read(testRawBlock, bytesPerBlock, &testRawRead);

        uint8_t testBBe[4];
        testBBe[0] = (lastBlockIdx >> 24) & 0xFF;
        testBBe[1] = (lastBlockIdx >> 16) & 0xFF;
        testBBe[2] = (lastBlockIdx >> 8) & 0xFF;
        testBBe[3] = (lastBlockIdx) & 0xFF;

        uint8_t testHmac[20];
        CCHmac(kCCHmacAlgSHA1, hmac_key, hmac_key_len, testBBe, 4, testHmac);

        uint8_t testDecBlock[4096];
        size_t testDecMoved = 0;
        CCCrypt(kCCDecrypt, kCCAlgorithmAES, 0, aes_key, aes_key_len, testHmac,
                testRawBlock, bytesPerBlock, testDecBlock, sizeof(testDecBlock), &testDecMoved);

        size_t trailerOffsetInBlock = (size_t)(dataLen % bytesPerBlock);
        if (trailerOffsetInBlock == 0) trailerOffsetInBlock = bytesPerBlock;
        if (trailerOffsetInBlock < 512) {
            // trailer spans across or fits in block
        } else {
            if (memcmp(testDecBlock + trailerOffsetInBlock - 512, "koly", 4) != 0) {
                return QUICKDMG_ERROR_PASSWORD;
            }
        }

        CEncryptedDmgInStream *encStream = new CEncryptedDmgInStream(
            inStream, offsetToDataStart, dataLen, bytesPerBlock, keyBits,
            aes_key, aes_key_len, hmac_key, hmac_key_len);
        inStream = encStream;
    }

    inStream->Seek(0, STREAM_SEEK_SET, NULL);

    CMyComPtr<IArchiveOpenCallback> openCallback(new CDmgPasswordCallback(pass_cb, user_data));

    // Scan registered format handlers
    for (unsigned i = 0; i < g_NumArcs; i++) {
        const CArcInfo &arc = *g_Arcs[i];
        if (!arc.CreateInArchive) continue;

        CMyComPtr<IInArchive> archive = arc.CreateInArchive();
        if (!archive) continue;

        inStream->Seek(0, STREAM_SEEK_SET, NULL);
        const UInt64 maxCheckStartPosition = 1024 * 1024;
        HRESULT hr = archive->Open(inStream, &maxCheckStartPosition, openCallback);

        if (hr == S_OK) {
            UInt32 count = 0;
            archive->GetNumberOfItems(&count);

            // Check if archive contains nested filesystem partitions (e.g. DMG -> HFS+/APFS)
            bool containsAppOrFiles = false;
            for (UInt32 idx = 0; idx < count; idx++) {
                NWindows::NCOM::CPropVariant pPath;
                if (archive->GetProperty(idx, kpidPath, &pPath) == S_OK && pPath.vt == VT_BSTR) {
                    AString path = UnicodeStringToMultiByte(pPath.bstrVal, CP_UTF8);
                    if (path.Find(".app") >= 0 || path.Find(".pkg") >= 0) {
                        containsAppOrFiles = true;
                        break;
                    }
                }
            }

            if (!containsAppOrFiles && count > 0) {
                CMyComPtr<IInArchiveGetStream> getStream;
                archive->QueryInterface(IID_IInArchiveGetStream, (void **)&getStream);
                if (getStream) {
                    for (UInt32 idx = 0; idx < count; idx++) {
                        CMyComPtr<ISequentialInStream> subSeqStream;
                        if (getStream->GetStream(idx, &subSeqStream) == S_OK && subSeqStream) {
                            CMyComPtr<IInStream> subInStream;
                            subSeqStream->QueryInterface(IID_IInStream, (void **)&subInStream);
                            if (subInStream) {
                                for (unsigned j = 0; j < g_NumArcs; j++) {
                                    const CArcInfo &subArc = *g_Arcs[j];
                                    if (!subArc.CreateInArchive) continue;
                                    CMyComPtr<IInArchive> innerArchive = subArc.CreateInArchive();
                                    if (!innerArchive) continue;
                                    subInStream->Seek(0, STREAM_SEEK_SET, NULL);
                                    const UInt64 maxCheck = 1024 * 1024;
                                    if (innerArchive->Open(subInStream, &maxCheck, openCallback) == S_OK) {
                                        UInt32 innerCount = 0;
                                        innerArchive->GetNumberOfItems(&innerCount);
                                        if (innerCount > 0) {
                                            archive = innerArchive;
                                            inStream = subInStream;
                                            count = innerCount;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            quickdmg_archive *res = (quickdmg_archive *)calloc(1, sizeof(quickdmg_archive));
            res->archive = archive;
            res->inStream = inStream;
            res->filePath = strdup(file_path);
            res->numItems = count;

            *out_archive = res;
            return QUICKDMG_OK;
        } else if (hr == E_ABORT) {
            return QUICKDMG_ERROR_PASSWORD;
        }
    }

    return QUICKDMG_ERROR_UNSUPPORTED;
}

int quickdmg_get_item_count(quickdmg_archive *archive, uint32_t *out_count) {
    if (!archive || !out_count) return QUICKDMG_ERROR_INVALID_ARG;
    *out_count = archive->numItems;
    return QUICKDMG_OK;
}

int quickdmg_get_item_info(quickdmg_archive *archive, uint32_t index, quickdmg_item_info *out_info) {
    if (!archive || !out_info || index >= archive->numItems) return QUICKDMG_ERROR_INVALID_ARG;
    memset(out_info, 0, sizeof(quickdmg_item_info));

    NWindows::NCOM::CPropVariant prop;

    if (archive->archive->GetProperty(index, kpidPath, &prop) == S_OK && prop.vt == VT_BSTR) {
        archive->lastPath = UnicodeStringToMultiByte(prop.bstrVal, CP_UTF8);
        out_info->path = archive->lastPath.Ptr();
    }

    if (archive->archive->GetProperty(index, kpidSize, &prop) == S_OK && prop.vt == VT_UI8) {
        out_info->size = prop.uhVal.QuadPart;
    }

    if (archive->archive->GetProperty(index, kpidPackSize, &prop) == S_OK && prop.vt == VT_UI8) {
        out_info->pack_size = prop.uhVal.QuadPart;
    }

    if (archive->archive->GetProperty(index, kpidIsDir, &prop) == S_OK && prop.vt == VT_BOOL) {
        out_info->is_dir = (prop.boolVal != VARIANT_FALSE);
    }

    if (archive->archive->GetProperty(index, kpidSymLink, &prop) == S_OK && prop.vt == VT_BSTR) {
        out_info->is_symlink = true;
        archive->lastSymLink = UnicodeStringToMultiByte(prop.bstrVal, CP_UTF8);
        out_info->symlink_target = archive->lastSymLink.Ptr();
    }

    if (archive->archive->GetProperty(index, kpidPosixAttrib, &prop) == S_OK && prop.vt == VT_UI4) {
        out_info->posix_attrib = prop.ulVal;
    }

    return QUICKDMG_OK;
}

int quickdmg_extract_all(quickdmg_archive *archive,
                         const char *destination_dir,
                         quickdmg_progress_cb progress_cb,
                         void *user_data)
{
    if (!archive || !destination_dir) return QUICKDMG_ERROR_INVALID_ARG;
    create_directories_recursively(destination_dir);

    CMyComPtr<IArchiveExtractCallback> extractCb(new CDmgExtractCallback(archive, destination_dir, progress_cb, user_data));
    HRESULT hr = archive->archive->Extract(NULL, (UInt32)(Int32)-1, NArchive::NExtract::NAskMode::kExtract, extractCb);

    if (hr == S_OK) return QUICKDMG_OK;
    if (hr == E_ABORT) return QUICKDMG_ERROR_PASSWORD;
    return QUICKDMG_ERROR_EXTRACT;
}

int quickdmg_extract_items(quickdmg_archive *archive,
                           const uint32_t *indices,
                           uint32_t num_indices,
                           const char *destination_dir,
                           quickdmg_progress_cb progress_cb,
                           void *user_data)
{
    if (!archive || !indices || num_indices == 0 || !destination_dir) return QUICKDMG_ERROR_INVALID_ARG;
    create_directories_recursively(destination_dir);

    CMyComPtr<IArchiveExtractCallback> extractCb(new CDmgExtractCallback(archive, destination_dir, progress_cb, user_data));
    HRESULT hr = archive->archive->Extract(indices, num_indices, NArchive::NExtract::NAskMode::kExtract, extractCb);

    if (hr == S_OK) return QUICKDMG_OK;
    if (hr == E_ABORT) return QUICKDMG_ERROR_PASSWORD;
    return QUICKDMG_ERROR_EXTRACT;
}

void quickdmg_close(quickdmg_archive *archive) {
    if (archive) {
        if (archive->filePath) free(archive->filePath);
        archive->archive.Release();
        archive->inStream.Release();
        free(archive);
    }
}
