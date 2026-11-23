from pathlib import Path
import cadquery as cq,json
from OCP.STEPControl import STEPControl_Writer,STEPControl_AsIs
from OCP.Interface import Interface_Static
from OCP.IGESControl import IGESControl_Writer,IGESControl_Reader
P=Path('fixtures/flock-import-alternatives');P.mkdir(parents=True,exist_ok=True);S=Path(__file__).resolve().parent.parent/'outputs'
report=[]
for name in ['23_rail_clamp_-220_180','24_cross_rail_REFERENCE']:
 original=cq.importers.importStep(str(S/(name+'.step'))).val()
 variants={'imported':original}
 def box(x,y,z,a,b,c):return cq.Workplane('XY').box(a,b,c,centered=(False,False,False)).translate((x,y,z)).val()
 def pose(s):return s.rotate((0,0,0),(0,1,0),-45).translate((-120,0,560))
 if name.startswith('24'):
  rebuilt=pose(box(-25,-210,-30,50,420,10))
 else:
  # Rebuild with a single extruded U profile instead of three box fuses.
  rebuilt=(cq.Workplane('YZ',origin=(-245,0,0)).polyline([(160,-27),(200,-27),(200,4),(193,4),(193,-20),(167,-20),(167,4),(160,4)]).close().extrude(50).val())
  for x in (-235,-205): rebuilt=rebuilt.cut(cq.Workplane('XY',origin=(x,180,-28)).circle(2.75).extrude(10).val())
  rebuilt=pose(rebuilt)
 variants['rebuilt']=rebuilt
 for variant,shape in variants.items():
  for schema in ['AP203','AP214IS']:
   path=P/(name+'_'+variant+'_'+schema+'.step')
   w=STEPControl_Writer();Interface_Static.SetCVal_s('write.step.schema',schema);Interface_Static.SetIVal_s('write.surfacecurve.mode',0);Interface_Static.SetCVal_s('write.step.unit','MM')
   w.Transfer(shape.wrapped,STEPControl_AsIs);w.Write(str(path))
   check=cq.importers.importStep(str(path)).val()
   diff=check.cut(original).Volume()+original.cut(check).Volume()
   report.append({'file':path.name,'valid':check.isValid(),'solid_count':len(check.Solids()),'volume_mm3':check.Volume(),'symmetric_difference_mm3':diff,'relative_volume_error':abs(check.Volume()-original.Volume())/original.Volume()})
  path=P/(name+'_'+variant+'.igs');w=IGESControl_Writer('MM',1);w.AddShape(shape.wrapped);w.ComputeModel();w.Write(str(path))
  r=IGESControl_Reader();r.ReadFile(str(path));r.TransferRoots();check=cq.Shape.cast(r.OneShape())
  report.append({'file':path.name,'valid':check.isValid(),'solid_count':len(check.Solids()),'volume_mm3':check.Volume(),'relative_volume_error':abs(check.Volume()-original.Volume())/original.Volume()})
(P/'validation.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
