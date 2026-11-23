"""Falcon-reference F0 printable fit prototype; mm. Run from repo root with CAD venv."""
import cadquery as cq
import json, math, struct, hashlib, zipfile
from pathlib import Path
P=Path(__file__).parent; O=P/'outputs';O.mkdir(exist_ok=True)
PARAMS={'head_height':277.8125,'hood_width':138.1125,'overall_depth':119.0625,'body_width':112.,'body_depth':68.,'wall':3.2,'panel_width':709.6125,'panel_height':539.75,'panel_frame_depth':30.,'panel_tilt_deg':45.,'pole_diameter':73.025,'print_clearance':0.35}
(P/'parameters.json').write_text(json.dumps(PARAMS,indent=2)+'\n')
parts=[]
def box(x,y,z,dx,dy,dz):return cq.Workplane('XY').box(dx,dy,dz,centered=(False,False,False)).translate((x,y,z)).val()
def cx(x,y,z,r,l):return cq.Workplane('YZ',origin=(x,y,z)).circle(r).extrude(l).val()
def cz(x,y,z,r,l):return cq.Workplane('XY',origin=(x,y,z)).circle(r).extrude(l).val()
def rr(x,w,h,r,l,z=0):return cq.Workplane('YZ',origin=(x,0,z)).rect(w,h).extrude(l).edges('|X').fillet(r).val()
def pill(x,l,shrink=0):
 w=56-shrink;top=130-shrink; bot=-138.90625+shrink;r=43-shrink
 # Narrow lower end and broad rounded shoulders, based on drawing silhouette.
 return (cq.Workplane('YZ',origin=(x,0,0)).moveTo(0,bot)
 .threePointArc((r*.707,bot+r*.293),(r,bot+r))
 .lineTo(w,top-45).threePointArc((w*.707,top-12),(0,top))
 .threePointArc((-w*.707,top-12),(-w,top-45)).lineTo(-r,bot+r)
 .threePointArc((-r*.707,bot+r*.293),(0,bot)).close().extrude(l).val())
def add(n,s,color=(.16,.17,.18),printable=True,group='camera',move=(0,0,0)):
 s=s.clean();assert s.isValid(),n;assert len(s.Solids())==1,(n,len(s.Solids()))
 parts.append(dict(name=n,shape=s,color=color,printable=printable,group=group,move=move))
# Hollow front-access tub, split at middle for small print beds.
body=pill(4,64).cut(pill(3.9,60.9,3.2))
# Front face screw bosses, pilot holes for M3 thread-forming screws.
fasteners=[(-49,95),(49,95),(-48,30),(48,30),(-42,-35),(42,-35),(-36,-108),(36,-108)]
for y,z in fasteners: body=body.fuse(cx(4,y,z,5,13)).cut(cx(3.9,y,z,1.4,14))
# Back mounting holes M5, accessible from open enclosure; rear screw heads outside.
for y in (-20,20):
 for z in (-35,35):body=body.cut(cx(63,y,z,2.75,7))
# Cable exit rear, blanking plug supplied for prototype.
body=body.cut(cx(63,0,-72,8,7))
add('01_body_upper',body.intersect(box(-25,-100,.175,160,200,160)),move=(25,0,25))
add('02_body_lower',body.intersect(box(-25,-100,-160,160,200,159.825)),move=(25,0,-25))
# Separate internal splice sleeve, butt seam 0.35mm gap; M3 seam fasteners from rear.
sleeve=pill(8,55,3.55).cut(pill(7.9,52,6.55)).intersect(box(0,-100,-12,100,200,24))
# Four matching rear pilot/clearance holes into sleeve.
for z in (-7,7):
 for y in (-25,25):
  sleeve=sleeve.cut(cx(59,y,z,1.4,8))
  for p in parts[:2]:p['shape']=p['shape'].cut(cx(63,y,z,1.7,7))
add('03_body_splice',sleeve,move=(60,0,0))
lid=pill(0,3.65)
for y,z in fasteners:lid=lid.cut(cx(-.1,y,z,1.7,4))
# Optical opening and removable patterned insert. Dimensions estimated, not OEM lens data.
lid=lid.cut(cx(-.1,0,72,38.2,4)).cut(cx(-.1,0,17,5.5,4))
# Teardrop surround shallow relief.
outline=cq.Workplane('YZ',origin=(-.1,0,0)).moveTo(0,5).threePointArc((35,38),(42,73)).threePointArc((0,116),(-42,73)).threePointArc((-35,38),(0,5)).close().extrude(1).val()
lid=lid.cut(outline.cut(cx(-.2,0,72,38.2,2)))
add('04_front_upper',lid.intersect(box(-1,-100,.175,10,200,150)),move=(-30,0,15))
add('05_front_lower',lid.intersect(box(-1,-100,-150,10,200,149.825)),move=(-30,0,-15))
# Front seam backing strip, glue to lower half; captures upper cover with small overlap.
strip=box(3.65,-40,-10,2.5,80,20)
add('06_front_splice_strip',strip,move=(-10,0,0))
optics=cx(-1,0,72,40,1).fuse(cx(0,0,72,37.85,3.3)).cut(cx(-2,0,72,12,7))
for k in range(6):
 a=2*math.pi*k/6;optics=optics.cut(cx(-2,26*math.sin(a),72+26*math.cos(a),6,7))
add('07_optics_insert',optics,(.08,.08,.09),move=(-50,0,0))
# Actual transparent material, not printable plastic, required for optical quality.
add('08_window_REFERENCE',cx(4.2,0,72,40,2),(.25,.48,.6,.45),False,move=(-15,0,0))
plug=cx(65,0,-72,7.65,5).fuse(cx(68,0,-72,10,2))
add('09_rear_cable_blank',plug,move=(30,0,0))
# Hood upper arch with long side cheeks; top of full envelope exactly source dimension.
hood=rr(-19.0625,138.1125,230,62,119.0625,z=23.90625)
hood=hood.cut(rr(-20,131.7125,223.6,58.8,116.8,z=23.90625))
hood=hood.intersect(box(-25,-80,-48,140,160,200))
# Open central rear area below upper arch, preserving only a thin rear web.
# Attach hood to backplate with 4 M4 fasteners through newly designed standoffs.
for y in (-32,32):
 for z in (35,95):
  hood=hood.fuse(cx(68,y,z,5.5,32)).cut(cx(67,y,z,1.8,34))
  for p in parts[:2]:p['shape']=p['shape'].cut(cx(63,y,z,2.2,7))
for y in (-20,20):
 for z in (-35,35):hood=hood.cut(cx(67,y,z,5.35,34))
add('10_hood',hood,move=(0,0,40))
# Camera rear pole saddle: generic replacement prototype, not original hardware.
saddle=box(100,-37,-45,30,74,90).cut(cz(153.5,0,-46,36.8625,92))
for y in (-20,20):
 for z in (-35,35):saddle=saddle.cut(cx(99,y,z,2.75,33))
# Strap passages across width for two stainless bands.
for z in (-25,25):saddle=saddle.cut(box(109,-38,z-7.5,4,76,15))
add('11_camera_pole_saddle',saddle,move=(50,0,0))
# Four compression tubes carry M5 rear mounting bolts across hood clearance.
for i,(y,z) in enumerate([(y,z) for y in (-20,20) for z in (-35,35)]):
 add(f'12_camera_spacer_{i+1}',cx(68,y,z,5,32).cut(cx(67,y,z,2.75,34)))
# Solar mount is independent prototype at z500; panel above camera, tilts outward.
# Pole saddle with band slots and twin trunnion cheeks, M8 pivot axis Y.
solar_saddle=box(100,-45,370,35,90,150).cut(cz(153.5,0,369,36.8625,152))
for z in (397,493):solar_saddle=solar_saddle.cut(box(108,-46,z-7.5,4,92,15))
# Cheeks project toward camera-side of pole, pivot x60,z475.
for y in (-29,21):
 cheek=box(50,y,450,65,8,50).fuse(cq.Workplane('XZ',origin=(60,y+8,475)).circle(25).extrude(8).val())
 solar_saddle=solar_saddle.fuse(cheek)
solar_saddle=solar_saddle.cut(cq.Workplane('XZ',origin=(60,50,475)).circle(4.3).extrude(100).val())
add('20_solar_pole_yoke',solar_saddle,group='solar',move=(40,0,0))
# Panel adapter on two purchased 20x20 aluminum rails; custom print fits rails.
adapter=cq.Workplane('XZ',origin=(0,20,0)).polyline([(60,450),(80,475),(60,500),(-103,575),(-138,538)]).close().extrude(40).val()
adapter=adapter.cut(cq.Workplane('XZ',origin=(60,25,475)).circle(4.3).extrude(50).val())
# Fixed second M6 pin prevents relying on friction alone; 45deg geometry engineered, not OEM.
adapter=adapter.cut(cq.Workplane('XZ',origin=(80,25,490)).circle(3.3).extrude(50).val())
parts[-1]['shape']=parts[-1]['shape'].cut(cq.Workplane('XZ',origin=(80,50,490)).circle(3.3).extrude(100).val())
# Adapter anchors a reference panel frame via cross rail, 2 M5 holes.
for y in (-12,12):adapter=adapter.cut(cz(60,y,510,2.75,45))
add('21_solar_pivot_block',adapter,group='solar',move=(-35,0,0))
# Panel local coordinates: X along slope, Y width, Z normal. transform about Y.
def panel_pose(s):return s.rotate((0,0,0),(0,1,0),-45).translate((-120,0,560))
# Rails length540 bought metal; four configurable clamps attach printed fit assembly.
for y in (-180,180):
 add('22_rail_'+str(y)+'_REFERENCE',panel_pose(box(-270,y-10,-20,540,20,20)),(.55,.57,.6),False,'solar')
 for x in (-220,220):
  clamp=box(x-25,y-20,-27,50,40,7).fuse(box(x-25,y-20,-20,50,7,24)).fuse(box(x-25,y+13,-20,50,7,24))
  # M5 holes through bottom connect rail T nuts. Clamp top tabs support panel frame.
  for xx in (x-15,x+15):clamp=clamp.cut(cz(xx,y,-28,2.75,10))
  add(f'23_rail_clamp_{x}_{y}',panel_pose(clamp),group='solar')
# Cross rail and simple bolted adapter bridge are metal references, not structural-print substitutes.
add('24_cross_rail_REFERENCE',panel_pose(box(-25,-210,-30,50,420,10)),(.5,.52,.55),False,'solar')
# two-piece bought panel envelope / exact panel mounting positions pending real panel drawing
pw=PARAMS['panel_width'];ph=PARAMS['panel_height']
add('25_solar_panel_REFERENCE',panel_pose(box(-ph/2,-pw/2,4,ph,pw,30)),(.12,.19,.29),False,'solar')
add('26_pole_REFERENCE',cz(153.5,0,-250,36.5125,1000),(.28,.29,.30),False,'site')
# Export assemblies, individual printable parts aligned to their bed bounding boxes.
interferences=[]
cam=[p for p in parts if p['group']=='camera']
for i,a in enumerate(cam):
 for b in cam[i+1:]:
  v=a['shape'].intersect(b['shape']).Volume()
  if v>0.05:interferences.append({'a':a['name'],'b':b['name'],'volume_mm3':v})
(O/'camera-interferences.json').write_text(json.dumps(interferences,indent=2)+'\n')
checks=[]
for p in parts:
 n,s=p['name'],p['shape'];assert s.isValid(),n
 cq.exporters.export(s,str(O/(n+'.step')))
 bb=s.BoundingBox()
 if p['printable']:
  bed=s
  if n.startswith(('04_','05_','06_','07_')):bed=bed.rotate((0,0,0),(0,1,0),90)
  if n.startswith('21_'):bed=bed.rotate((0,0,0),(1,0,0),90)
  # panel clamps inverse rotation permits flat-bed printing, even though STEP is assembled.
  if n.startswith('23_'):bed=s.translate((120,0,-560)).rotate((0,0,0),(0,1,0),45)
  b=bed.BoundingBox();bed=bed.translate((-b.xmin,-b.ymin,-b.zmin))
  cq.exporters.export(bed,str(O/(n+'.stl')),tolerance=.12,angularTolerance=.12)
 imported=cq.importers.importStep(str(O/(n+'.step'))).val()
 checks.append({'part':n,'printable':p['printable'],'valid':s.isValid(),'solids':len(s.Solids()),'dimensions_mm':[round(bb.xlen,3),round(bb.ylen,3),round(bb.zlen,3)],'volume_mm3':round(s.Volume(),3),'step_roundtrip_valid':imported.isValid(),'step_relative_volume_error':abs(imported.Volume()-s.Volume())/s.Volume()})
for name,subset,explode in [('falcon-camera-assembly',[p for p in parts if p['group']=='camera'],False),('falcon-camera-exploded',[p for p in parts if p['group']=='camera'],True),('falcon-solar-site-assembly',parts,False)]:
 a=cq.Assembly(name=name)
 for p in subset:a.add(p['shape'].translate(p['move'] if explode else (0,0,0)),name=p['name'],color=cq.Color(*p['color']))
 a.save(str(O/(name+'.step')))
# SVG engineering previews contain geometry rendered from real CAD, not imagined output.
for name,subset,view in [('camera-preview',[p for p in parts if p['group']=='camera'],(-1,-1,.6)),('site-preview',parts,(-1,-1,.6)),('camera-front',[p for p in parts if p['group']=='camera'],(-1,0,0))]:
 shape=cq.Compound.makeCompound([p['shape'] for p in subset])
 cq.exporters.export(shape,str(O/(name+'.svg')),opt={'width':1000,'height':850,'projectionDir':view,'showAxes':False,'showHidden':False,'strokeWidth':.8})
# OCC chooses an inverted image-up direction for these negative-X views.
for n in ('camera-preview','site-preview','camera-front'):
 f=O/(n+'.svg');t=f.read_text();t=t.replace('    <g transform=', '    <g transform="rotate(180 500 425)"><g transform=',1).replace('</svg>','</g></svg>');f.write_text(t)
(O/'validation.json').write_text(json.dumps(checks,indent=2)+'\n')
print(json.dumps({'parts':len(parts),'print_parts':sum(p['printable'] for p in parts),'all_valid':all(c['valid'] and c['step_roundtrip_valid'] for c in checks),'max_step_volume_error':max(c['step_relative_volume_error'] for c in checks)},indent=2))
