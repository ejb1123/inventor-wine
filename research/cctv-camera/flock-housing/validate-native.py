"""Compare Inventor-exported STEP geometry with original source models."""
import argparse
import json
from pathlib import Path
import cadquery as cq

p = argparse.ArgumentParser()
p.add_argument("roundtrip", type=Path, help="Directory of STEP exports from saved native files")
p.add_argument("--parts-only", action="store_true")
p.add_argument("--output", type=Path)
args = p.parse_args()
base = Path(__file__).parent / "outputs"
pairs = [(f.stem, f) for f in sorted(base.glob("[0-9]*.step"))]
if not args.parts_only:
    pairs += [("camera", base / "falcon-camera-assembly.step"),
              ("solar", base / "falcon-solar-site-assembly.step")]
rows = []
for name, source in pairs:
    a = cq.importers.importStep(str(source)).solids().vals()
    b = cq.importers.importStep(str(args.roundtrip / (name + ".step"))).solids().vals()
    av, bv = sum(x.Volume() for x in a), sum(x.Volume() for x in b)
    rel = abs(av - bv) / av
    valid = bool(b) and all(x.isValid() for x in b)
    # Whole-model bounds check also catches misplaced components.
    def bounds(solids):
        bb = cq.Compound.makeCompound(solids).BoundingBox()
        return [bb.xmin, bb.ymin, bb.zmin, bb.xmax, bb.ymax, bb.zmax]
    delta = max(abs(x-y) for x,y in zip(bounds(a), bounds(b))) if b else None
    row = dict(name=name, source_solids=len(a), native_solids=len(b),
               source_volume=av, native_volume=bv, relative_volume_error=rel,
               max_bounds_error_mm=delta, valid=valid,
               **{"pass": valid and len(a)==len(b) and rel<1e-5 and delta<0.01})
    rows.append(row)
    print(name, "PASS" if row["pass"] else "FAIL", len(b), rel, delta)
if args.output:
    args.output.write_text(json.dumps(rows, indent=2) + "\n")
raise SystemExit(0 if all(r["pass"] for r in rows) else 1)
