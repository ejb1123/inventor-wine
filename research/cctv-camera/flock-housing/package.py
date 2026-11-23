"""Inspect closed STL meshes, prepare preview/drawing sheet and deliverable archive."""
from pathlib import Path
import struct,json,collections,subprocess,zipfile,hashlib
from reportlab.pdfgen import canvas
from reportlab.lib.pagesizes import A3,landscape
P=Path(__file__).parent;O=P/'outputs';checks=[]
for f in sorted(O.glob('*.stl')):
 data=f.read_bytes();n=struct.unpack_from('<I',data,80)[0];assert len(data)==84+50*n,f
 edges=collections.Counter();verts=[];volume=0
 for i in range(n):
  raw=struct.unpack_from('<12fH',data,84+50*i);v=[tuple(round(c,4) for c in raw[j:j+3]) for j in (3,6,9)];verts+=v
  for j in range(3):edges[tuple(sorted((v[j],v[(j+1)%3])))]+=1
  a,b,c=v;volume+=(a[0]*(b[1]*c[2]-b[2]*c[1])+a[1]*(b[2]*c[0]-b[0]*c[2])+a[2]*(b[0]*c[1]-b[1]*c[0]))/6
 bounds=[max(v[j] for v in verts)-min(v[j] for v in verts) for j in range(3)]
 ok=all(count==2 for count in edges.values());assert ok,(f,'nonmanifold')
 assert volume>0,(f,'orientation')
 checks.append({'file':f.name,'triangles':n,'all_edges_have_two_faces':ok,'signed_volume_mm3':round(volume,2),'print_bounds_mm':[round(b,3) for b in bounds],'fits_220mm_cube':max(bounds)<=220})
(O/'mesh-checks.json').write_text(json.dumps(checks,indent=2)+'\n')
for n in ('camera-preview','site-preview','camera-front'):
 subprocess.run(['magick','-background','white',str(O/(n+'.svg')),str(O/(n+'.png'))],check=True)
c=canvas.Canvas(str(O/'drawing-notes.pdf'),pagesize=landscape(A3));W,H=landscape(A3)
for title,img,notes in [
 ('Falcon-reference F0: camera housing', 'camera-preview.png', ['Head envelope H277.8125 x W138.1125 x D119.0625 mm; pole saddle extends behind.', 'Body and covers split for 220 mm printer. 3.2 mm shell wall; 0.35 mm seam.', '3D-print fit prototype. Not exact OEM geometry. Dimensions from public drawing.', 'STEP: falcon-camera-assembly.step; individual STEP/STL also included.']),
 ('Falcon-reference F0: solar / pole arrangement','site-preview.png',['Panel envelope 709.6125 x 539.75 x 30 mm, 45 deg assumption.', 'Solar panel and rails are nonprinted references. Printed brackets are bench-fit only.', 'Crossplate/arm and panel fastening need actual hardware drawings and redesign.', 'Do not support real overhead/outdoor panel with unqualified printed brackets.']),
 ('Falcon-reference F0: front and assembly','camera-front.png',['8 front M3 screws; 4 rear M3 splice screws; 4 M4-compatible hood screws.', 'Four 32 mm spacers + M5 through bolts attach camera saddle. Bolts not modeled.', 'Optics apertures visual placeholders; real lens/IR/window/PCB fit not established.', 'Review README.md, SOURCES.md and validation.json before printing.'])]:
 c.setFont('Helvetica-Bold',19);c.drawString(35,H-40,title)
 c.drawImage(str(O/img),35,105,width=W-400,height=H-160,preserveAspectRatio=True,anchor='c')
 c.setFont('Helvetica',11)
 for i,line in enumerate(notes):c.drawString(35,80-i*16,line)
 c.setFont('Helvetica',10);c.drawString(W-355,H-100,'Units: mm | Revision F0 | 2026-09-24')
 c.drawString(W-355,H-120,'Appearance / dimensional study; not for fabrication release.')
 c.showPage()
c.save()
files=[p for p in P.rglob('*') if p.is_file() and p.suffix not in ('.zip',) and p.name!='SHA256SUMS.txt']
(O/'SHA256SUMS.txt').write_text(''.join(hashlib.sha256(f.read_bytes()).hexdigest()+'  '+str(f.relative_to(P))+'\n' for f in sorted(files)))
with zipfile.ZipFile(O/'Falcon-F0-print-study.zip','w',zipfile.ZIP_DEFLATED) as z:
 for f in files+[O/'SHA256SUMS.txt']:z.write(f,f.relative_to(P))
print(json.dumps({'meshes':len(checks),'closed_positive_meshes':True,'all_fit_220mm_cube':all(v['fits_220mm_cube'] for v in checks),'archive':str(O/'Falcon-F0-print-study.zip')},indent=2))
