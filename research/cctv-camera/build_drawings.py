"""Dimensioned concept/RFQ sheets. ReportLab output, not Inventor IDW or released drawings."""
from pathlib import Path
import csv,json,math,textwrap
from reportlab.pdfgen import canvas
from reportlab.lib.colors import HexColor,Color,black,white
from reportlab.lib.units import mm
P=Path(__file__).parent;O=P/'outputs'
W,H=420*mm,297*mm
c=canvas.Canvas(str(O/'CCTV-A0-drawing-sheets.pdf'),pagesize=(W,H))
c.setTitle('CCTV A0 - PCBWay feasibility/RFQ drawing sheets - NOT FOR FABRICATION')
INK=HexColor('#142d40'); TEAL=HexColor('#168582'); ORANGE=HexColor('#b35917')
def txt(x,y,s,size=9,color=INK):
 c.setFillColor(color);c.setFont('Helvetica',size);c.drawString(x*mm,y*mm,s)
def line(x1,y1,x2,y2):c.setStrokeColor(INK);c.setLineWidth(.55);c.line(x1*mm,y1*mm,x2*mm,y2*mm)
def rect(x,y,w,h,fill=None,r=0):
 c.setStrokeColor(INK);c.setLineWidth(.7);c.setFillColor(fill or white)
 c.roundRect(x*mm,y*mm,w*mm,h*mm,r*mm,stroke=1,fill=bool(fill))
def circle(x,y,r):c.setStrokeColor(INK);c.circle(x*mm,y*mm,r*mm,stroke=1,fill=0)
def dim(x1,y1,x2,y2,label,vertical=False):
 line(x1,y1,x2,y2)
 if vertical:
  for y in (y1,y2):line(x1-2,y,x1+2,y)
  txt(x1+3,(y1+y2)/2,label,8)
 else:
  for x in (x1,x2):line(x,y1-2,x,y1+2)
  txt((x1+x2)/2-len(label)*.75,y1+2,label,8)
def notes(x,y,lines,width=85):
 for text in lines:
  for ln in textwrap.wrap(text,width):txt(x,y,ln,9);y-=4.5
  y-=2
 return y
def sheet(n,title):
 c.setFillColor(white);c.rect(0,0,W,H,fill=1,stroke=0)
 rect(10,10,400,277)
 txt(17,276,'CCTV / OUTDOOR CAMERA',11,TEAL);txt(17,264,title,20)
 txt(17,254,'A0 CONCEPT / RFQ ONLY - NOT FOR FABRICATION OR ASSEMBLY',11,ORANGE)
 line(10,31,410,31);txt(15,23,'Units: mm | Dimensions govern | Supplier envelopes are approximate',9)
 txt(15,16,'2026-09-24 | IP66 is a target, not a rating | Generated outside Inventor',8)
 txt(290,23,f'CCTV-A0-{n:03d} | Sheet {n} of 7',11)
 txt(290,16,'Revision A0 | Scale: schematic unless stated',8)
def finish():c.showPage()
# 1 General arrangement - side and front, nominal geometric positions.
sheet(1,'General arrangement and interfaces')
s=.8; ox,oy=30,132
rect(ox,oy,210*s,90*s,r=8*s);rect(ox-6*s,oy,6*s,90*s);rect(ox+210*s,oy,6*s,90*s)
rect(ox-18*s,oy+98*s,238*s,2*s)
rect(ox+120*s,oy-6*s,30*s,6*s);rect(ox+125*s,oy-60*s,20*s,54*s);rect(ox+100*s,oy-66*s,70*s,6*s)
dim(ox-6*s,oy-74*s,ox+216*s,oy-74*s,'222 plate-to-plate')
dim(ox-18*s,oy+111*s,ox+220*s,oy+111*s,'238 sunshield')
txt(ox,oy+45*s,'SIDE VIEW - body 210 x 114 x 90',10)
fx,fy=302,177
rect(fx-57*.75,fy-45*.75,114*.75,90*.75,r=12*.75);circle(fx,fy,32*.75);circle(fx,fy,28*.75)
dim(fx-57*.75,fy-53*.75,fx+57*.75,fy-53*.75,'114')
notes(254,121,['1-inch-type Sony sensor; candidate 4K mode is 20 fps.','PoE Ethernet: cable gland to protected internal termination.','USB-C: capped power/service port. Service opening is not weather sealed.','External antenna avoids a metal enclosure shielding the radio.','Bullet form, environment, lens and budget are provisional.'],61)
finish()
# 2 Body cross-section and machining.
sheet(2,'Main body - CNC feasibility')
fx,fy=85,176;s=1
rect(fx-57,fy-45,114,90,r=12);rect(fx-49,fy-37,98,74,r=12)
rect(fx-53.8,fy-41.8,107.6,83.6,r=17.8);rect(fx-51,fy-39,102,78,r=15)
for y in (-51,51):
 for z in (-39,39):circle(fx+y,fy+z,1.65)
dim(fx-57,fy-57,fx+57,fy-57,'114 outside')
dim(fx+66,fy-45,fx+66,fy+45,'90',True)
txt(30,233,'END VIEW / sealing land',11)
rect(207,154,168,72);line(207,160.4,375,160.4);line(207,219.6,375,219.6)
dim(207,141,375,141,'210 length (view scale 0.8)')
notes(200,124,['6061-T6 aluminum candidate. Constant 98 x 74 through cavity; internal R12.','Both ends open for tool access; deep bore reach requires PCBWay review.','8 x M4 x 0.7 proposed end fasteners; minor bore shown at 3.3; 8 mm useful thread within 10 mm bore.','Seal grooves: 1.50 deep; rounded rectangular ring 107.6 x 83.6 R17.8 outside / 102 x 78 R15 inside.','Blind attachment features for mount, standoffs and shield remain to be detailed.'],92)
notes(25,89,['No internal undercut in revised model. End screw holes do not cross the seal envelopes.','Proposed seal-land finish: Ra <= 1.6 um; flatness 0.10 mm for supplier review.','Critical dimensions are post-finish; masking/coating build must be agreed. No blanket precision tolerance claimed.'],90)
finish()
# 3 Plates.
sheet(3,'End plates, window and service reservations')
for fx,label in [(88,'FRONT / optical window'),(280,'REAR / ports provisional')]:
 fy=178;rect(fx-57,fy-45,114,90,r=12)
 for y in (-51,51):
  for z in (-39,39):circle(fx+y,fy+z,2.2)
 txt(fx-57,fy+53,label,11);dim(fx-57,fy-55,fx+57,fy-55,'114 x 90 x 6 plate')
 if fx==88:
  for r in (28,32,32.2):circle(fx,fy,r)
 else:
  for y,z,r in [(-25,-10,10),(18,-15,11),(30,20,6),(-30,20,3.25)]:circle(fx+y,fy+z,r)
notes(25,108,['Front: clear aperture diameter 56; glass 64 x 3; seat diameter 64.4, depth 3.','Window gasket seat diameter 64 to diameter 56, depth 1.5 below glass seat.','4 x 4.4 clearance holes at (Y,Z) = (+/-51,+/-39). Coordinates about optical axis.','Glass edge finish, optical coating, gasket tolerance and retainer screws remain open.'],85)
notes(220,108,['Rear port centers (Y,Z), about body axis:','Ethernet (-25,-10), diameter 20; USB (18,-15), diameter 22.','Vent (30,20), diameter 12; RF (-30,20), diameter 6.5.','These are envelope reservations, NOT selected connector cutouts.','Do not machine rear plate before supplier parts are frozen.'],77)
finish()
# 4 PCB floorplan and stack.
sheet(4,'PCB mechanical floorplan and requested stack')
bx,by=30,104;s=1.15;rect(bx,by,140*s,86*s,r=0)
for x in (5,135):
 for y in (5,81):circle(bx+x*s,by+y*s,1.6*s)
# Geometric positions consistent with assembly x=58, y=-43.
for x,y,w,h,label in [(44,7,82,53,'SOM 82 x 53'),(10,65,58,18,'PoE envelope'),(97,63,30,22,'RADIO')]:
 rect(bx+x*s,by+y*s,w*s,h*s,HexColor('#e5efed'));txt(bx+x*s+2,by+(y+h/2)*s,label,8)
dim(bx,by-10,bx+140*s,by-10,'140')
dim(bx-8,by,bx-8,by+86*s,'86',True)
notes(25,79,['4 x diameter 3.2 NPTH at (5,5), (135,5), (5,81), (135,81).','Local datum: lower-left of TOP view; dimensions in mm.','No traces, pads, vias or connections released. This is a placement study.'],92)
roles=['L1 signal/components','L2 ground','L3 signal','L4 power','L5 signal','L6 ground','L7 signal/components']
for i,label in enumerate(roles):
 yy=225-i*17;rect(258,yy,113,2,HexColor('#b86e32'));txt(260,yy+4,label,8)
 if i<6:rect(258,yy-10,113,7,HexColor('#d9e4d2'))
notes(241,96,['Requested: 7 copper x 0.035 + 6 dielectric x 0.225833 = 1.600 nominal.','This is only a CAD stack. It is NOT a PCBWay-approved laminate schedule or impedance solution.','PCBWay publishes 8-layer standard options. Layer-count decision pending.','Factory stackup determines actual copper, resin and trace geometry.'],71)
finish()
# 5 exploded assembly routing.
sheet(5,'Assembly sequence and service design')
steps=[('01 BODY','CNC body'),('04 / 05 SEALS','End gaskets'),('06 / 07 / 08','Window, seal, retainer'),('PCB + MODULES','Carrier, SOM, radio, PD'),('02 / 03 PLATES','End plates and seals'),('09 SHIELD','Sunshield + spacers'),('21 / 22 / 23','Mounting assembly')]
for i,(tag,label) in enumerate(steps):
 x=22+(i%4)*96;y=202-(i//4)*64
 rect(x,y,87,36,HexColor('#edf3f5'),3);txt(x+4,y+24,tag,11);txt(x+4,y+12,label,9)
notes(25,99,['Assembly order proposal: seat window seal and retainer; populate and bench-test PCB; secure board to standoffs; route camera/RF/data cables; fit dry gaskets and plates; attach shield and mount.','Do not trap wiring across sealing lands. Use internal strain relief at Ethernet entry and secure USB3 cable against vibration.','Keep hot processor and PD converter thermally separated from sensor. A spreader reservation is modeled, but its connection to the outer case is not designed yet.','Missing for release: captive/tethered service cap, fastener length/torque schedule, lens lock, PCB/camera retention, mount pivots and range-of-motion check.'],160)
finish()
# 6 power functional diagram, not circuit.
sheet(6,'Electrical architecture - schematic pending')
blocks=[(25,210,88,22,'PoE+ / magnetics / PD'),(25,166,88,22,'USB-C PD sink / protection'),(144,192,85,25,'Priority mux: PoE first'),(262,192,112,25,'4 V buck -> SOM (separate rail)'),(262,144,112,25,'5 V camera + aux radio / PHY'),(144,99,230,25,'Camera USB3 -> SOM encoder -> GbE / Wi-Fi')]
for x,y,w,h,t in blocks:rect(x,y,w,h,HexColor('#e8f1f0'),2);txt(x+3,y+9,t,10)
line(113,221,144,208);line(113,177,144,200);line(229,204,262,204);line(249,204,249,156);line(249,156,262,156)
notes(25,142,['PoE nominal output candidate: 12 V.','USB PD proposal: 15 V, >= 2 A.','USB 5 V attach: full camera held off.','With PoE running, USB service data still works.','USB2 service uses separate device port.'],77)
notes(25,76,['Design allocation: SOM 10 W + camera 1.5 W + radio 1.5 W + PHY/aux 2 W = 15 W load.','At 90% downstream conversion: 16.67 W before PoE conversion; allowances are not measured maxima.','Published SOM max ~20 W makes unrestricted peak system demand exceed this PoE candidate. Limit/measure or redesign PD.','No end-to-end 4K encode, USB gadget, ONVIF or network recording has been validated.'],155)
finish()
# 7 verification record.
sheet(7,'Validation status and manufacturing release gates')
r=json.loads((O/'geometry-checks.json').read_text())
checks=[('CAD solid validity',str(r['solid_validity'])),('Copper envelopes / thickness','7 / 1.600 mm'),('Selected envelope and seal collisions','See geometry-checks.json; limited scope'),('Included geometry mass',str(r['mass_g_included_geometry_only'])+' g (not full product mass)'),('Seal nominal squeeze','25%; tolerance corner ~18.4 to 31.0%, compound approval pending'),('CNC producibility','Quote review pending; no factory approval'),('PCB electrical verification','NOT RUN: no completed schematic or routed layout'),('IP66 / outdoor rating','NOT TESTED'),('Thermal / solar / cold start','NOT VALIDATED'),('Mount strength / vibration','NOT VALIDATED'),('Inventor native model / drawing validation','BLOCKED: Wine import / automation issue')]
y=232
for label,result in checks:
 txt(22,y,label,10);txt(143,y,result,9);line(20,y-4,397,y-4);y-=12
notes(22,89,['Required closure: module pinouts/SDK; PCBWay stackup; schematic/ERC; layout/DRC and impedance; supplier connector geometry; gasket/fastener tolerances; chassis thermal path; radio and ESD/surge; optical/IR behavior; outdoor qualification.','Industrial-design checks included: separate service access, external RF path, weather hood, drainage-oriented entries, seal continuity, material/finish intent and selected internal clearance checks. These do not constitute FEA, CFD, certification or factory DFM sign-off.'],170)
finish();c.save()
# Separate all-lines system BOM PDF. Not an assembly placement BOM.
from reportlab.platypus import SimpleDocTemplate,Table,TableStyle,Paragraph,Spacer
from reportlab.lib.styles import getSampleStyleSheet
styles=getSampleStyleSheet(); rows=list(csv.DictReader((P/'bom.csv').open()))
data=[[Paragraph(h,styles['BodyText']) for h in ['Item / Qty','Description / Candidate','Status','USD allowance']]]
for r in rows:
 data.append([r['item']+' / '+r['quantity'],Paragraph(r['description']+'<br/>'+r['manufacturer']+' / '+r['candidate_mpn'],styles['BodyText']),Paragraph(r['status'],styles['BodyText']),r['unit_allowance_low_USD']+' - '+r['unit_allowance_high_USD']])
t=Table(data,colWidths=[65,330,220,90],repeatRows=1)
t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),HexColor('#dce8ed')),('VALIGN',(0,0),(-1,-1),'TOP'),('GRID',(0,0),(-1,-1),.3,HexColor('#aab8bf')),('BOTTOMPADDING',(0,0),(-1,-1),7)]))
doc=SimpleDocTemplate(str(O/'CCTV-A0-system-BOM.pdf'),pagesize=(W,H),rightMargin=35,leftMargin=35,topMargin=35,bottomMargin=35)
doc.build([Paragraph('CCTV A0 system BOM - RFQ planning only',styles['Title']),Paragraph('Unquoted engineering allowances; not distributor prices. Total USD 733-1843 excluding tax, shipping, design and qualification. Candidate parts and allowance sets are not a released component-level PCBA BOM. See bom.csv for sources and notes.',styles['BodyText']),Spacer(1,15),t])
print('Created seven drawing sheets and system BOM PDF')
