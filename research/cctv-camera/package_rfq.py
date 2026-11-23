"""Verify artifacts and package ONLY authored A0 RFQ materials, never raw logs."""
from pathlib import Path
import hashlib,json,zipfile
import cadquery as cq
from pypdf import PdfReader
P=Path(__file__).parent;O=P/'outputs'
# R12 ASCII DXF geometry uses millimetre coordinates; R12 has no reliable INSUNITS.
lines=['0','SECTION','2','HEADER','9','$ACADVER','1','AC1009','0','ENDSEC','0','SECTION','2','ENTITIES']
for a,b in [((0,0),(140,0)),((140,0),(140,86)),((140,86),(0,86)),((0,86),(0,0))]:
 lines += ['0','LINE','8','BOARD_OUTLINE','10',str(a[0]),'20',str(a[1]),'30','0','11',str(b[0]),'21',str(b[1]),'31','0']
for x in (5,135):
 for y in (5,81):lines += ['0','CIRCLE','8','NPTH_REFERENCE','10',str(x),'20',str(y),'30','0','40','1.6']
lines += ['0','ENDSEC','0','EOF']
(O/'pcb-outline.dxf').write_text('\n'.join(lines)+'\n')
checks=json.loads((O/'geometry-checks.json').read_text())
assert checks['solid_validity']
assert checks['pcb_thickness_mm']==1.6
assert len(checks['checks'])==18
assert all(r['intersection_mm3']<1e-5 for r in checks['checks'])
steps={}
for f in sorted(O.glob('*.step')):
 s=cq.importers.importStep(str(f)).val()
 assert s.isValid(),f.name
 steps[f.name]={'valid_after_import':True,'solids':len(s.Solids())}
assert steps['seven-copper-layer-board.step']['solids']==13
pages=len(PdfReader(str(O/'CCTV-A0-drawing-sheets.pdf')).pages)
assert pages==7
(O/'artifact-checks.json').write_text(json.dumps({'step_roundtrips':steps,'drawing_pages':pages,'scope':'File integrity/geometry only; no manufacturing approval'},indent=2)+'\n')
# Explicit root documents and known output extensions: no fixtures or confidential material.
files=[P/n for n in ['README.md','PCBWAY-HANDOFF.md','VALIDATION.md','requirements.json','bom.csv']]
files += sorted(f for f in O.iterdir() if f.suffix.lower() in {'.step','.pdf','.svg','.dxf','.json'})
manifest='\n'.join(hashlib.sha256(f.read_bytes()).hexdigest()+'  '+str(f.relative_to(P)) for f in files)+'\n'
(O/'SHA256SUMS.txt').write_text(manifest)
with zipfile.ZipFile(O/'CCTV-A0-PCBWAY-RFQ-ONLY.zip','w',zipfile.ZIP_DEFLATED) as z:
 for f in files:z.write(f,str(f.relative_to(P)))
 z.write(O/'SHA256SUMS.txt','SHA256SUMS.txt')
print(f'Validated {len(steps)} STEP files, {pages} drawing sheets; packaged {len(files)} authored files. RFQ ONLY.')
