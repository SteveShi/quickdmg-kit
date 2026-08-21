import struct
import binascii

def u32(x): return struct.pack('>I', x)

def make_record(filename, type_code, data_type, data):
    fn_encoded = filename.encode('utf-16be')
    res = u32(len(fn_encoded) // 2)
    res += fn_encoded
    res += type_code.encode('ascii')
    res += data_type.encode('ascii')
    if data_type == 'blob':
        res += u32(len(data))
        res += data
    elif data_type in ('long', 'shor', 'bool'):
        res += data
    elif data_type == 'comp':
        res += data
    elif data_type == 'type':
        res += data
    else:
        raise ValueError(data_type)
    return res

bwsp = bytes.fromhex("62706c6973743030d80102030405060708090909090909090a0b5f1014436f6e7461696e657253686f77536964656261725f10155072657669657750616e655669736962696c6974795b53686f77506174686261725d53686f775374617475734261725b53686f77546162566965775b53686f77546f6f6c6261725c5369646562617257696474685c57696e646f77426f756e64730810005f10147b7b302c20307d2c207b3430302c203330307d7d0819304854626e7a8794959700000000000001010000000000000000000c00000000000000000000000000000000000000ae")
icvo = bytes.fromhex("62706c6973743030dd0102030405060708090a0b0c0d0e0f0f0f10111112131414151659617272616e676542795f10136261636b67726f756e64436f6c6f72426c75655f10146261636b67726f756e64436f6c6f72477265656e5f10126261636b67726f756e64436f6c6f725265645e6261636b67726f756e64547970655b677269644f6666736574585b677269644f6666736574595b6772696453706163696e675869636f6e53697a655d6c6162656c4f6e426f74746f6d5f100f73686f7749636f6e507265766965775c73686f774974656d496e666f587465787453697a65546e6f6e65233ff000000000000010022300000000000000002340590000000000002340600000000000000908234028000000000000080023002d0043005a006f007e008a009600a200ab00b900cb00d800e100e600ef00f100fa0103010c010d010e000000000000020100000000000000170000000000000000000000000000000117")

recs = []
recs.append(make_record('.', 'BKGD', 'blob', b'PctB' + u32(100) + u32(0)))
recs.append(make_record('.', 'bwsp', 'blob', bwsp))
recs.append(make_record('.', 'icvo', 'blob', icvo))
recs.append(make_record('App.app', 'Iloc', 'blob', u32(100) + u32(100) + u32(0xFFFF) + u32(0xFFFF)))
recs.append(make_record('Applications', 'Iloc', 'blob', u32(300) + u32(100) + u32(0xFFFF) + u32(0xFFFF)))

# Sort records: length of filename, then filename string, then record type
def sort_key(r):
    # record length is 4 (fn length) + len * 2 + 4 + 4 + data...
    fn_len = struct.unpack('>I', r[:4])[0]
    fn = r[4:4+fn_len*2]
    type_code = r[4+fn_len*2:4+fn_len*2+4]
    return (fn.decode('utf-16be'), type_code)

recs.sort(key=sort_key)

# DSDB (B-tree master block)
dsdb = u32(1) + u32(1) + u32(len(recs)) + u32(1) + u32(1)
dsdb = dsdb.ljust(32, b'\x00')

# Root block (B-tree node)
root = u32(0) + u32(len(recs)) # next node: 0
for r in recs: root += r
root = root.ljust(4096, b'\x00')

# Allocator Info
alloc_info = u32(2) + u32(0)
offsets = [32, 64] + [0]*254
for o in offsets: alloc_info += u32(o)
for i in range(32): alloc_info += u32(0)
# TOC
alloc_info += u32(1) # 1 entry
alloc_info += bytes([4]) + b"DSDB" + u32(0)

# Build file
header = u32(1) + b'Bud1' + u32(64+4096) + u32(len(alloc_info)) + u32(64+4096) + bytes(12)

file_data = header + dsdb + root + alloc_info
with open("test.ds_store", "wb") as f: f.write(file_data)
