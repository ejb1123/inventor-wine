"""Diagnostic-only scalar-preserving repair of this captured Aurora shader.
The translator flattened a 3x4 matrix into an illegal vec12 then bitcast it to
and from a struct containing float4[3]. Keep scalar order with explicit copies.
"""
from pathlib import Path
import sys
s=Path(sys.argv[1] if len(sys.argv)>1 else 'fixtures/raytracing/raygen.spvasm').read_text()
assert s.count('%v12float = OpTypeVector %float 12') == 1
s=s.replace('%v12float = OpTypeVector %float 12','%v12float = OpTypeArray %float %uint_12')
old='%1575 = OpBitcast %__1 %1487'
assert s.count(old)==1
lines=[]
for i in range(12): lines.append(f'%repair_s{i} = OpCompositeExtract %float %1487 {i}')
for i in range(3): lines.append(f'%repair_v{i} = OpCompositeConstruct %v4float ' + ' '.join(f'%repair_s{j}' for j in range(4*i,4*i+4)))
lines += ['%repair_a = OpCompositeConstruct %_arr_v4float_uint_3 %repair_v0 %repair_v1 %repair_v2','%1575 = OpCompositeConstruct %__1 %repair_a']
s=s.replace(old,'\n'.join(lines))
old='%2200 = OpBitcast %v12float %2199'
assert s.count(old)==1
lines=[f'%repair_r{i} = OpCompositeExtract %float %2199 0 {i//4} {i%4}' for i in range(12)]
lines.append('%2200 = OpCompositeConstruct %v12float ' + ' '.join(f'%repair_r{i}' for i in range(12)))
s=s.replace(old,'\n'.join(lines))
Path('fixtures/raytracing/raygen-fixed.spvasm').write_text(s)
