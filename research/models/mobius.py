"""Generate a finite-thickness Mobius ribbon as a sewn, validated STEP solid.
Dimensions are millimetres. Requires cadquery 2.8 / OCP 7.9.
"""
import math
from pathlib import Path
import cadquery as cq
from OCP.BRepOffsetAPI import BRepOffsetAPI_ThruSections
from OCP.BRepBuilderAPI import BRepBuilderAPI_Sewing, BRepBuilderAPI_MakeSolid
from OCP.TopoDS import TopoDS
from OCP.BRepCheck import BRepCheck_Analyzer
from OCP.ShapeFix import ShapeFix_Solid

RADIUS = 60.0
WIDTH = 24.0
THICKNESS = 3.0
SECTIONS = 97

def build():
    # Local radial/vertical frame rotates half a turn around the circular spine.
    # Disable automatic wire matching: it can silently remove the intended twist.
    loft = BRepOffsetAPI_ThruSections(False, False, 1e-6)
    loft.CheckCompatibility(False)
    for i in range(SECTIONS):
        u = 2 * math.pi * i / (SECTIONS - 1)
        cu, su = math.cos(u), math.sin(u)
        ch, sh = math.cos(u / 2), math.sin(u / 2)
        points = []
        for a, b in [(-WIDTH/2, -THICKNESS/2), (WIDTH/2, -THICKNESS/2),
                     (WIDTH/2, THICKNESS/2), (-WIDTH/2, THICKNESS/2)]:
            radial = RADIUS + a * ch - b * sh
            points.append(cq.Vector(radial * cu, radial * su, a * sh + b * ch))
        loft.AddWire(cq.Wire.makePolygon(points, close=True).wrapped)
    loft.Build()
    if not loft.IsDone():
        raise RuntimeError('Loft failed')
    sewing = BRepBuilderAPI_Sewing(1e-5)
    sewing.Add(loft.Shape())
    sewing.Perform()
    shell = cq.Shape.cast(sewing.SewedShape())
    shells = shell.Shells()
    if len(shells) != 1 or not shells[0].wrapped.Closed():
        raise RuntimeError(f'Expected one closed shell, got {len(shells)}')
    solid = BRepBuilderAPI_MakeSolid(TopoDS.Shell_s(shells[0].wrapped)).Solid()
    fixer = ShapeFix_Solid(solid)
    fixer.Perform()
    result = cq.Shape.cast(fixer.Solid())
    if not BRepCheck_Analyzer(result.wrapped).IsValid() or len(result.Solids()) != 1:
        raise RuntimeError('Invalid solid')
    if result.Volume() <= 0:
        raise RuntimeError('Nonpositive volume')
    return result

if __name__ == '__main__':
    shape = build()
    output = Path(__file__).with_name('mobius.step')
    cq.exporters.export(shape, str(output))
    readback = cq.importers.importStep(str(output)).val()
    assert readback.isValid() and len(readback.Solids()) == 1
    print(f'{output}: valid solid, volume={readback.Volume():.3f} mm^3, faces={len(readback.Faces())}')
