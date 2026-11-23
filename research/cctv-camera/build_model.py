"""A0 envelope model, not supplier geometry or production tooling. CadQuery 2.8."""
import json
from pathlib import Path
import cadquery as cq
P=Path(__file__).parent
p=json.loads((P/'requirements-a0.json').read_text())['provisional']
O=P/'outputs'; O.mkdir(exist_ok=True)
parts=[]
def box(x,y,z,dx,dy,dz):
    return cq.Workplane('XY').box(dx,dy,dz,centered=(False,False,False)).translate((x,y,z)).val()
def cyl(x,y,z,r,l):
    return cq.Workplane('YZ',origin=(x,y,z)).circle(r).extrude(l).val()
def rr(x,w,h,r,l):
    return cq.Workplane('YZ',origin=(x,0,0)).rect(w,h).extrude(l).edges('|X').fillet(r).val()
def ring(x,w,h,r,iw,ih,ir,l):
    return rr(x,w,h,r,l).cut(rr(x-.01,iw,ih,ir,l+.02))
def add(name,s,c,material='envelope',exploded=(0,0,0)):
    assert s.isValid(),name
    parts.append(dict(name=name,shape=s,color=c,material=material,exploded=exploded))
L,W,H=p['body_length'],p['body_width'],p['body_height']
body=rr(0,W,H,12,L).cut(rr(-1,W-16,H-16,12,L+2))
# Constant through cavity: removes inaccessible internal shoulder/undercut.
for x in (0,L-1.5):
    body=body.cut(ring(x,107.6,83.6,17.8,102,78,15,1.5))
for x in (0,L-10):
    for y in (-51,51):
        for z in (-39,39): body=body.cut(cyl(x-.1,y,z,1.65,10.2))
add('01_body',body,(.72,.75,.77),'aluminum')
front=rr(-6,W,H,12,6).cut(cyl(-7,0,0,28,8))
front=front.cut(cyl(-3,0,0,32.2,3.1)).cut(cyl(-4.5,0,0,32,1.5))
rear=rr(L,W,H,12,6)
for y,z,r in [(-25,-10,10),(18,-15,11),(30,20,6),(-30,20,3.25)]: rear=rear.cut(cyl(L-.1,y,z,r,6.2))
for y in (-51,51):
    for z in (-39,39):
        front=front.cut(cyl(-6.1,y,z,2.2,6.2));rear=rear.cut(cyl(L-.1,y,z,2.2,6.2))
add('02_front_plate',front,(.82,.84,.85),'aluminum',(-45,0,0))
add('03_rear_plate_PORTS_PROVISIONAL',rear,(.82,.84,.85),'aluminum',(45,0,0))
for name,x,offset in [('04_front_seal',0,-20),('05_rear_seal',L-1.5,25)]:
    # Compressed rectangular gasket envelope, not a circular O-ring.
    add(name,ring(x,107.2,83.2,17.6,102.4,78.4,15.2,1.5),(.12,.13,.14),'silicone',(offset,0,0))
add('06_window',cyl(-3,0,0,32,3),(.32,.65,.8,.4),'glass',(-30,0,0))
add('07_window_gasket',cyl(-4.5,0,0,32,1.5).cut(cyl(-4.6,0,0,28,1.7)),(.15,.15,.15),'silicone',(-32,0,0))
add('08_window_retainer',cyl(0,0,0,36,2).cut(cyl(-.1,0,0,28,2.2)),(.2,.2,.2),'aluminum',(-12,0,0))
# Sunshield: open-bottom folded U section with 8 mm top air gap.
shield=box(-18,-65,53,238,130,2).fuse(box(-18,-65,25,238,2,28)).fuse(box(-18,63,25,238,2,28))
add('09_sunshield',shield,(.94,.94,.92),'aluminum',(0,0,45))
for x in (35,175):
    add(f'shield_spacer_{x}',box(x,-5,45,10,10,8),(.5,.5,.5),'aluminum')
# PCB: full-area layer envelopes, NOT routed copper planes.
x,y,z=p['pcb_origin']; dx,dy=p['pcb_length'],p['pcb_width']; boardparts=[]
for i in range(13):
    cu=i%2==0;t=p['copper_thickness'] if cu else p['dielectric_thickness']
    s=box(x,y,z,dx,dy,t)
    for hx in (x+5,x+dx-5):
        for hy in (y+5,y+dy-5):
            hole=cq.Workplane('XY',origin=(hx,hy,z-.01)).circle(1.6).extrude(t+.02).val();s=s.cut(hole)
    name=f'PCB_Cu_{i//2+1}_ENVELOPE' if cu else f'PCB_dielectric_{i//2+1}'
    add(name,s,(.72,.39,.12) if cu else (.1,.35,.22),'copper' if cu else 'fr4',(0,0,(i-6)*2))
    boardparts.append(s);z+=t
for hx in (x+5,x+dx-5):
    for hy in (y+5,y+dy-5):
        s=cq.Workplane('XY',origin=(hx,hy,-37)).circle(3).circle(1.6).extrude(8).val()
        add(f'standoff_{hx}_{hy}',s,(.6,.6,.6),'aluminum')
add('10_SOM_ENVELOPE_82x53',box(102,-36,-21,82,53,12),(.18,.35,.2))
add('11_PoE_ENVELOPE',box(68,22,-27.4,58,18,18),(.2,.2,.25))
add('12_RADIO_M2_ENVELOPE',box(155,20,-23,30,22,4),(.15,.3,.2))
add('13_camera_PCB_ENVELOPE',box(42,-22,-22,2,44,44),(.12,.3,.2))
add('14_lens_ENVELOPE',cyl(6,0,0,22,34),(.12,.12,.13))
add('15_thermal_spreader_RESERVATION',box(102,-36,-9,82,53,3),(.7,.45,.2),'envelope')
# External connector envelopes; actual component drawings must set cutouts.
add('16_Ethernet_gland_ENVELOPE',cyl(216,-25,-10,13,20).cut(cyl(215,-25,-10,4,22)),(.15,.15,.16))
add('17_USB_service_cap_ENVELOPE',cyl(216,18,-15,14,12),(.15,.15,.16))
add('18_vent_ENVELOPE',cyl(216,30,20,8,8),(.15,.15,.16))
add('19_antenna_bulkhead_ENVELOPE',cyl(216,-30,20,5,9),(.3,.3,.3))
add('20_RF_antenna_ENVELOPE',cq.Workplane('XY',origin=(230,-30,20)).circle(4).extrude(60).val(),(.1,.1,.1))
add('21_mount_boss',box(120,-15,-51,30,30,6),(.7,.7,.7),'aluminum')
add('22_bracket_arm_CONCEPT',box(125,-15,-105,20,30,54),(.7,.7,.7),'aluminum')
base=box(100,-40,-111,70,80,6)
for hx in (110,160):
    for hy in (-30,30): base=base.cut(cq.Workplane('XY',origin=(hx,hy,-112)).circle(3.3).extrude(8).val())
add('23_mount_base',base,(.7,.7,.7),'aluminum')
assembly=cq.Assembly(name='CCTV_A0_CONCEPT')
exploded=cq.Assembly(name='CCTV_A0_EXPLODED_CONCEPT')
for a in parts:
    assembly.add(a['shape'],name=a['name'],color=cq.Color(*a['color']))
    exploded.add(a['shape'].translate(a['exploded']),name=a['name'],color=cq.Color(*a['color']))
assembly.save(str(O/'cctv-assembly.step'))
exploded.save(str(O/'cctv-exploded.step'))
cq.exporters.export(cq.Compound.makeCompound(boardparts),str(O/'seven-copper-layer-board.step'))
# Export individual authored housing components for CAD reuse.
for a in parts:
    if a['material']!='envelope' and not a['name'].startswith('PCB'):
        cq.exporters.export(a['shape'],str(O/(a['name']+'.step')))
visible=[a['shape'] for a in parts if not a['name'].startswith('PCB')]
cq.exporters.export(cq.Compound.makeCompound(visible),str(O/'assembly-preview.svg'),opt={'width':1000,'height':700,'projectionDir':(1,-1,0.7),'showHidden':False,'strokeWidth':.6})
cq.exporters.export(cq.Compound.makeCompound([a['shape'].translate(a['exploded']) for a in parts]),str(O/'exploded-preview.svg'),opt={'width':1200,'height':800,'projectionDir':(1,-1,.7),'showHidden':False,'strokeWidth':.5})
# Pairwise checks restricted to critical *envelopes*: these are not final fit checks.
checks=[]
by={a['name']:a['shape'] for a in parts}
for name in ['10_SOM_ENVELOPE_82x53','11_PoE_ENVELOPE','12_RADIO_M2_ENVELOPE','13_camera_PCB_ENVELOPE','14_lens_ENVELOPE']:
    checks.append({'check':name+' vs body','intersection_mm3':round(by[name].intersect(body).Volume(),6)})
for a,b in [('10_SOM_ENVELOPE_82x53','11_PoE_ENVELOPE'),('10_SOM_ENVELOPE_82x53','12_RADIO_M2_ENVELOPE'),('14_lens_ENVELOPE','13_camera_PCB_ENVELOPE')]:
    checks.append({'check':a+' vs '+b,'intersection_mm3':round(by[a].intersect(by[b]).Volume(),6)})
density={'aluminum':2.70e-3,'copper':8.96e-3,'fr4':1.85e-3,'silicone':1.15e-3,'glass':2.5e-3}
for x in (0,L-1.5):
    seal=ring(x,107.2,83.2,17.6,102.4,78.4,15.2,1.5)
    checks.append({'check':f'seal at x={x} vs body','intersection_mm3':round(seal.intersect(body).Volume(),6)})
    for y in (-51,51):
        for zz in (-39,39):
            checks.append({'check':f'seal vs bolt clearance {x},{y},{zz}','intersection_mm3':round(seal.intersect(cyl(x,y,zz,2.2,1.5)).Volume(),6)})
report={'status':'CONCEPT: not production or IP qualification','solid_validity':all(a['shape'].isValid() for a in parts),'part_count':len(parts),'copper_layer_count':7,'pcb_thickness_mm':round(z-p['pcb_origin'][2],8),'checks':checks,'nominal_gasket_squeeze_percent':25,'gasket_squeeze_tolerance_percent':[18.421,30.952],'mass_g_included_geometry_only':round(sum(a['shape'].Volume()*density.get(a['material'],0) for a in parts),1),'excluded_from_mass':'supplier envelopes, cables, fasteners; copper modeled as full sheets so PCB mass is an upper estimate','unvalidated':['temperature and solar gain','IP66','radio antenna and EMC','optics and image quality','module fasteners and pinouts','seal tolerances and compression force','mount strength and adjustment','thermal contact to outer case','PCB routing and impedance','Inventor native features/drawings']}
(O/'geometry-checks.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
