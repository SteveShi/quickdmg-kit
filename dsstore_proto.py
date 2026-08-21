import struct

def u32(x): return struct.pack('>I', x)

class Node:
    def __init__(self):
        self.records = []
    
    def to_bytes(self):
        # leaf node
        res = u32(0) # next node id
        res += u32(len(self.records))
        for r in self.records:
            res += r
        return res

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

bwsp = b"bplist00..." # dummy
icvo = b"bplist00..." # dummy

recs = []
recs.append(make_record('.', 'BKGD', 'blob', b'PctB' + u32(100) + u32(0)))
recs.append(make_record('.', 'bwsp', 'blob', bwsp))
recs.append(make_record('.', 'icvo', 'blob', icvo))
recs.append(make_record('App.app', 'Iloc', 'blob', u32(100) + u32(100) + u32(0) + u32(0) + u32(0) + u32(0)))

n = Node()
n.records = recs
block1 = n.to_bytes()

# Master node (Block 0)
master = u32(1) # root block id
master += u32(1) # levels
master += u32(len(recs))
master += u32(1) # nodes
master += u32(1) # is leaf

print(f"Master size: {len(master)}, Block1 size: {len(block1)}")
