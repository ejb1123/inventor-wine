# Falcon-reference camera head and solar mount — F0

A printable dimensional/appearance prototype, reconstructed from publicly available Flock-authored drawings and photos. **Not an exact replica, supplier CAD, weatherproof product or load-rated mount.** This is a separate design study from the Hasion camera line. No Flock logo is reproduced.

## Open / print

- `outputs/Falcon-F0-Inventor-parts.zip`: 26 verified native Inventor 2027 parts. Extract and open individual `.ipt` files in `parts/`.
- `outputs/falcon-camera-assembly.step`: camera housing assembly for Inventor STEP import.
- `outputs/falcon-camera-exploded.step`: separated camera components for inspection.
- `outputs/falcon-solar-site-assembly.step`: complete camera, pole, solar-panel and mount study.
- Individual numbered STEP parts and 20 STL files in `outputs/`.
- `outputs/drawing-notes.pdf`: geometry views, dimensions and fabrication limitations.
- `outputs/Falcon-F0-print-study.zip`: packaged deliverables.

In Inventor use Open, select STEP, choose millimetres and import as assembly/solids where available. The STEP files are neutral solids. Native Inventor 2027 IPTs are now available in `outputs/inventor/parts/`: all 26 were saved, reopened and exported back to STEP for independent solid-count, validity, volume and bounding-box checks. These contain imported solids, not a reconstructed parametric feature history. Native IAM assemblies are withheld: import tests lost geometry and a later retry lost the Inventor process. See `NATIVE-IMPORT-NOTES.md`. Edit the reproducible CadQuery `build.py` to change geometry. `parameters.json` records generated values; **edit `PARAMS` in the script**, because JSON is output, not input.

## What matches the reference

Flock's dimensioned 2023 preliminary installation drawing, reproduced in Oakland's July 9, 2024 public packet, page 172 (PDF index171), specifies camera **277.8125 mm high × 138.1125 mm wide × 119.0625 mm deep**. Our head including the hood follows those three dimensions. The newly designed pole saddle protrudes beyond the head envelope. The front has a tapered, rounded lower body, broad curved shoulder/hood, an upper optical cluster, six ring apertures and the small lower sensor opening. The internal walls, splice joints and mounting holes are our engineering choices, not measured original parts.

The same drawing specifies a side-mounted panel **709.6125 mm wide**, with **539.75 mm** panel-side length. A 30 mm frame and 45° tilt are our assumptions. The panel is represented as a single simplified nonprinted envelope; original drawing shows two adjacent sections. Exact solar-cell count and frame channels are not reproduced. Pole diameter defaults to 73.025 mm / 2.875 inches, configurable in code (saddle cutters must also be updated if diameter changes).

## Print and camera assembly

STLs use millimetres, with minimum coordinate zero. Small front plates/inserts and solar pivot are reoriented flat; STEP parts retain assembly coordinates. Target printer volume 220×220×220 mm; see STL bounds in `mesh-checks.json` before slicing.

Suggested fit prototype: PETG, 0.2 mm layers, 0.4 mm nozzle, 4–5 perimeters, 30–40% infill. These are starting settings, not tested material specifications. Inspect slicer supports, especially the curved hood and yoke. Print the optics insert and splice first to check dimensional shrinkage. Pilot holes are 2.8 mm for suitable M3 plastic thread-forming screws; do not force standard machine screws into unthreaded plastic. Drill pilot/clearance sizes to match the actual hardware.

1. Fit body halves over internal splice sleeve; 0.35 mm external seam and approximately 0.35 mm mating clearance are intentional. Four rear M3 screws retain the splice. Adhesive may reinforce this cosmetic seam.
2. Bond front backing strip to **lower cover only**. The upper removable cover overlaps the strip. Eight front M3 screws attach covers to bosses.
3. Fit optical insert and a real optical window behind it. Window is a 80 mm diameter × 2 mm reference; retain with an appropriate bonded/sealed joint or redesign for the selected optic. Six 12 mm IR openings, 24 mm lens opening and 11 mm small sensor opening are visual/fit placeholders, not selected electronics.
4. Mount hood via four M4-compatible 3.6 mm pilot bores; the screws enter through body rear. Pilot holes require the selected thread-forming screw or machining/tapping to a validated size.
5. Fit four 32 mm spacers and rear saddle using four M5 through bolts, washers and nuts. Nut/bolt orientation must allow access through open front. No bolts are modeled. Blank the rear 16 mm cable opening until a real sealed gland is selected.

The head has no seal grooves, certified gasket compression, EMI coating, lens-focus mount or approved heat path. A flat Forlinx-sized board can be placed inside the central envelope, but full carrier, lens, modem, antennas, connector bend radii and cooling fit **has not been established**. The camera front split/optics apertures must be sealed or redesigned before an outdoor functional build.

## Solar mount: bench assembly only

The printed yoke and long pivot arm reproduce the side-mount relationship using our own joints. Use purchased metal band clamps around the pole, M8 pivot bolt and M6 index bolt, washers and locknuts. The index hole is fixed for the modeled configuration; it is not a continuously adjustable lock. The geometry includes two purchased 20×20×540 mm rail references, a metal 50×420×10 mm crossplate and four printable rail clamps. Panel/rail positioning is adjustable in the source.

**The adapter-to-crossplate fastening pattern and panel frame fastening pattern are not finalized.** The crossplate intersects the arm as an explicit reserved attachment region; do not treat that interface as a bolted production assembly. Measure the selected panel, add matching through holes/bolts and verify attachment before mounting a real panel. A foamboard/cardboard panel is appropriate for this first appearance/fit print. The 540×710 mm real panel presents a large wind area; printed mount strength, creep, UV life and pole attachment have not been tested. This is not ready to support a real panel overhead/outdoors. Use a rated purchased metal mount or engineer/test a metal replacement for deployment.

## BOM / hardware to source

- 20 printed pieces: shell halves, sleeve, cover halves, backing strip, optics insert, cable blank, hood, camera saddle, four spacers, solar yoke, pivot arm, four rail clamps.
- One optical window (reference 80×2 mm; actual coating/transmission TBD).
- Approx. 12 M3 thread-forming screws (8 cover +4 splice); select engagement after print.
- Four M4-compatible hood screws; four M5 camera through bolts + nuts/washers (length selected from real stack).
- M8 pivot hardware and M6 index hardware; four metal pole band clamps (two camera, two panel).
- Two 20×20×540 mm aluminum rails, one crossplate, compatible T-nuts/bolts, real panel/frame hardware after selection.
- Pole, camera electronics, panel, battery/charge controller, cables and seals not supplied.

## Evidence and uncertainty

See `SOURCES.md`. Source dimensions are not fabrication tolerances. Photo-derived proportions are visibly approximate; rear hardware and unseen geometry are original design. No weather/wind/thermal/structural simulation, live camera operation or physical print has been completed. Valid B-rep / closed STL does not prove fit or strength.

## Rebuild

From repository root:

```sh
LD_LIBRARY_PATH="$(cat fixtures/mobius-library-path):/nix/store/86f8ip7shrdrrhwc12gqkspzn9vsd58m-expat-2.7.3/lib" fixtures/mobius-venv/bin/python research/cctv-camera/flock-housing/build.py
fixtures/mobius-venv/bin/python research/cctv-camera/flock-housing/package.py
```

`package.py` requires ImageMagick and ReportLab. All source geometry is newly authored; vendor images/PDFs are linked rather than redistributed.
