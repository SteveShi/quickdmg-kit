#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
SEVENZ_DIR="${ROOT_DIR}/7zip"
BUILD_DIR="${ROOT_DIR}/build"
OUTPUT_DIR="${ROOT_DIR}/output"

rm -rf "${BUILD_DIR}" "${OUTPUT_DIR}"
mkdir -p "${BUILD_DIR}/arm64" "${BUILD_DIR}/x86_64" "${OUTPUT_DIR}"

CFLAGS="-O3 -fPIC -DUNICODE -D_UNICODE -DZ7_EXTRACT_ONLY -DZ7_PROG_VARIANT_Z -DZ7_EXTERNAL_CODECS -DZ7_AFFINITY_DISABLE -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -I${SEVENZ_DIR}/C -I${SEVENZ_DIR}/CPP -I${ROOT_DIR}/Bridge/include"
CXXFLAGS="${CFLAGS} -std=c++17 -Wno-inconsistent-missing-override"

# Collect C sources
C_SRCS=(
  "${SEVENZ_DIR}/C/7zCrc.c"
  "${SEVENZ_DIR}/C/7zCrcOpt.c"
  "${SEVENZ_DIR}/C/7zStream.c"
  "${SEVENZ_DIR}/C/Alloc.c"
  "${SEVENZ_DIR}/C/Bcj2.c"
  "${SEVENZ_DIR}/C/Bra.c"
  "${SEVENZ_DIR}/C/Bra86.c"
  "${SEVENZ_DIR}/C/BraIA64.c"
  "${SEVENZ_DIR}/C/CpuArch.c"
  "${SEVENZ_DIR}/C/Delta.c"
  "${SEVENZ_DIR}/C/LzFind.c"
  "${SEVENZ_DIR}/C/LzFindOpt.c"
  "${SEVENZ_DIR}/C/LzmaDec.c"
  "${SEVENZ_DIR}/C/Lzma2Dec.c"
  "${SEVENZ_DIR}/C/Lzma2DecMt.c"
  "${SEVENZ_DIR}/C/MtDec.c"
  "${SEVENZ_DIR}/C/Sha1.c"
  "${SEVENZ_DIR}/C/Sha1Opt.c"
  "${SEVENZ_DIR}/C/Sha256.c"
  "${SEVENZ_DIR}/C/Sha256Opt.c"
  "${SEVENZ_DIR}/C/Sha512.c"
  "${SEVENZ_DIR}/C/Sha512Opt.c"
  "${SEVENZ_DIR}/C/Sort.c"
  "${SEVENZ_DIR}/C/SwapBytes.c"
  "${SEVENZ_DIR}/C/Xz.c"
  "${SEVENZ_DIR}/C/XzDec.c"
  "${SEVENZ_DIR}/C/XzIn.c"
  "${SEVENZ_DIR}/C/XzCrc64.c"
  "${SEVENZ_DIR}/C/XzCrc64Opt.c"
  "${SEVENZ_DIR}/C/Aes.c"
  "${SEVENZ_DIR}/C/AesOpt.c"
  "${SEVENZ_DIR}/C/Threads.c"
)

# Collect CPP sources
CPP_SRCS=(
  "${ROOT_DIR}/Bridge/src/quickdmg_kit.cpp"
  
  # Common
  "${SEVENZ_DIR}/CPP/Common/CRC.cpp"
  "${SEVENZ_DIR}/CPP/Common/IntToString.cpp"
  "${SEVENZ_DIR}/CPP/Common/MyString.cpp"
  "${SEVENZ_DIR}/CPP/Common/MyVector.cpp"
  "${SEVENZ_DIR}/CPP/Common/MyWindows.cpp"
  "${SEVENZ_DIR}/CPP/Common/MyXml.cpp"
  "${SEVENZ_DIR}/CPP/Common/StringConvert.cpp"
  "${SEVENZ_DIR}/CPP/Common/StringToInt.cpp"
  "${SEVENZ_DIR}/CPP/Common/UTFConvert.cpp"
  "${SEVENZ_DIR}/CPP/Common/Wildcard.cpp"
  "${SEVENZ_DIR}/CPP/Common/NewHandler.cpp"

  # Windows emulation
  "${SEVENZ_DIR}/CPP/Windows/FileDir.cpp"
  "${SEVENZ_DIR}/CPP/Windows/FileFind.cpp"
  "${SEVENZ_DIR}/CPP/Windows/FileIO.cpp"
  "${SEVENZ_DIR}/CPP/Windows/FileName.cpp"
  "${SEVENZ_DIR}/CPP/Windows/PropVariant.cpp"
  "${SEVENZ_DIR}/CPP/Windows/PropVariantConv.cpp"
  "${SEVENZ_DIR}/CPP/Windows/PropVariantUtils.cpp"
  "${SEVENZ_DIR}/CPP/Windows/Synchronization.cpp"
  "${SEVENZ_DIR}/CPP/Windows/System.cpp"
  "${SEVENZ_DIR}/CPP/Windows/TimeUtils.cpp"

  # 7zip Common
  "${SEVENZ_DIR}/CPP/7zip/Common/CreateCoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/CWrappers.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/FileStreams.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/FilterCoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/InBuffer.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/InOutTempBuffer.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/LimitedStreams.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/LockedStream.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/MemBlocks.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/MethodId.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/MethodProps.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/OffsetStream.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/OutBuffer.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/ProgressUtils.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/PropId.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/StreamBinder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/StreamObjects.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/StreamUtils.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/UniqBlocks.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Common/VirtThread.cpp"

  # Compressors / Decoders
  "${SEVENZ_DIR}/CPP/7zip/Compress/BZip2Crc.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/BitlDecoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/BZip2Decoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/Bcj2Coder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/Bcj2Register.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/BcjCoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/BcjRegister.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/BranchMisc.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/BranchRegister.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/ByteSwap.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/CopyCoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/CopyRegister.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/DeltaFilter.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/DeflateDecoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/DeflateRegister.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/LzOutWindow.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/LzmaDecoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/LzmaRegister.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/Lzma2Decoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/Lzma2Register.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/LzfseDecoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/XzDecoder.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Compress/ZlibDecoder.cpp"

  # Crypto
  "${SEVENZ_DIR}/CPP/7zip/Crypto/7zAes.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Crypto/7zAesRegister.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Crypto/MyAes.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Crypto/MyAesReg.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Crypto/RandGen.cpp"

  # Archive Handlers (DMG, HFS, APFS, GPT, APM, MBR, XAR, EXT, FAT)
  "${SEVENZ_DIR}/CPP/7zip/Archive/Base64Handler.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/HandlerCont.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/DmgHandler.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/HfsHandler.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/ApfsHandler.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/GptHandler.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/ApmHandler.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/MbrHandler.cpp"
  
  
  "${SEVENZ_DIR}/CPP/7zip/Archive/XarHandler.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/Common/CoderMixer2.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/Common/FindSignature.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/Common/InStreamWithCRC.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/Common/ItemNameUtils.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp"
  "${SEVENZ_DIR}/CPP/7zip/Archive/Common/OutStreamWithSha1.cpp"
)

build_arch() {
  local ARCH=$1
  local TARGET_DIR="${BUILD_DIR}/${ARCH}"
  local OBJS=()

  echo "===> Compiling for ${ARCH}..."

  local ARCH_FLAGS="-arch ${ARCH} -mmacosx-version-min=14.0"

  for src in "${C_SRCS[@]}"; do
    local base=$(basename "${src}")
    local obj="${TARGET_DIR}/${base}.o"
    clang ${CFLAGS} ${ARCH_FLAGS} -I$(dirname "${src}") -c "${src}" -o "${obj}"
    OBJS+=("${obj}")
  done

  for src in "${CPP_SRCS[@]}"; do
    local base=$(basename "${src}")
    local dir_hash=$(echo "${src}" | md5 -q | cut -c 1-8)
    local obj="${TARGET_DIR}/${base}_${dir_hash}.o"
    clang++ ${CXXFLAGS} ${ARCH_FLAGS} -I$(dirname "${src}") -c "${src}" -o "${obj}"
    OBJS+=("${obj}")
  done

  echo "===> Linking dylib for ${ARCH}..."
  clang++ -shared -dynamiclib ${ARCH_FLAGS} \
    -install_name "@rpath/QuickdmgKit.framework/Versions/A/QuickdmgKit" \
    -compatibility_version 1.0.0 -current_version 1.0.0 \
    -lpthread \
    -o "${TARGET_DIR}/QuickdmgKit.dylib" "${OBJS[@]}"
}

build_arch "arm64"
build_arch "x86_64"

echo "===> Creating Universal Binary..."
lipo -create "${BUILD_DIR}/arm64/QuickdmgKit.dylib" "${BUILD_DIR}/x86_64/QuickdmgKit.dylib" -output "${BUILD_DIR}/QuickdmgKit_universal"

echo "===> Creating Framework Structure..."
FW_DIR="${OUTPUT_DIR}/QuickdmgKit.framework"
mkdir -p "${FW_DIR}/Versions/A/Headers"
mkdir -p "${FW_DIR}/Versions/A/Modules"
mkdir -p "${FW_DIR}/Versions/A/Resources"

cp "${BUILD_DIR}/QuickdmgKit_universal" "${FW_DIR}/Versions/A/QuickdmgKit"
cp "${ROOT_DIR}/Bridge/include/quickdmg_kit.h" "${FW_DIR}/Versions/A/Headers/"
cp "${ROOT_DIR}/Bridge/include/module.modulemap" "${FW_DIR}/Versions/A/Modules/"

cat << PLIST_EOF > "${FW_DIR}/Versions/A/Resources/Info.plist"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>QuickdmgKit</string>
    <key>CFBundleIdentifier</key>
    <string>com.steveshi.QuickdmgKit</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>QuickdmgKit</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
</dict>
</plist>
PLIST_EOF

ln -sf A "${FW_DIR}/Versions/Current"
ln -sf Versions/Current/QuickdmgKit "${FW_DIR}/QuickdmgKit"
ln -sf Versions/Current/Headers "${FW_DIR}/Headers"
ln -sf Versions/Current/Modules "${FW_DIR}/Modules"
ln -sf Versions/Current/Resources "${FW_DIR}/Resources"

echo "===> Packaging QuickdmgKit.xcframework..."
rm -rf "${OUTPUT_DIR}/QuickdmgKit.xcframework"
xcodebuild -create-xcframework \
  -framework "${FW_DIR}" \
  -output "${OUTPUT_DIR}/QuickdmgKit.xcframework"

echo "===> Successfully created ${OUTPUT_DIR}/QuickdmgKit.xcframework!"
