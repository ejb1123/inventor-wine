from pathlib import Path
import struct
p=Path('/home/ej/.local/share/wineprefixes/inventor-2027-research/drive_c/Program Files/Autodesk/Inventor 2027/Bin/agp_hydra_bridge.dll')
b=p.read_bytes();u=lambda o:struct.unpack_from('<I',b,o)[0];q=lambda o:struct.unpack_from('<Q',b,o)[0];pe=u(60);ns=struct.unpack_from('<H',b,pe+6)[0];opt=pe+24;sh=opt+struct.unpack_from('<H',b,pe+20)[0]
sections=[struct.unpack_from('<IIII',b,sh+40*i+8) for i in range(ns)]
def off(r):
 for vs,va,rs,ptr in sections:
  if va<=r<va+max(vs,rs):return ptr+r-va
 raise ValueError(hex(r))
def st(r):o=off(r);return b[o:b.index(0,o)].decode(errors='replace')
i=off(u(opt+120))
while u(i+12):
 orig,_,_,name,thunk=struct.unpack_from('<IIIII',b,i);dll=st(name);j=0
 while q(off(orig)+8*j):
  n=q(off(orig)+8*j)
  if n<2**63:print(hex(0x180000000+thunk+8*j),dll,st(n+2))
  j+=1
 i+=20
for r in [0x124b68,0x124b50]:print(hex(r),st(r))
