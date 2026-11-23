# Outdoor Sony camera — research and superseded A0 concept

**Current direction:** true 4K60, sensor size flexible, **Hasion selected**.
See the [A1 Hasion integration package](a1-hasion/README.md) and its current
planning BOM and supplier inquiry draft. The user
rejected the A0 exterior. See [sensor selection](SENSOR-4K60.md) and
[housing shortlist](ENCLOSURE-SOURCING.md). A0 drawings/BOM/ZIP below are historical
and must not be used as the current product specification.

The separate proposed PTZ line has a [supplier backup shortlist](PTZ-SUPPLIER-SHORTLIST.md):
BIT-CCTV for OEM mechanics, Soar for compact customization inquiry, and a Sony
4K60 optical-zoom block candidate. Full-system compatibility is unverified.

See [common compute platform](COMPUTE-PLATFORM.md) for the proposed non-Android
Linux/AI architecture, modular cellular/wireless options and monitoring limits.

The [fixed-camera fit screen](a1-hasion/FIXED-CAMERA-FIT.md) does not establish
X6Z7 fit for the complete electronics. Technical inquiries are prepared for
[Hasion](a1-hasion/HASION-RFQ-DRAFT.md), [KEVIS PTZ](KEVIS-PTZ-RFQ-DRAFT.md),
and [Forlinx compute](FORLINX-TECHNICAL-QUERY-DRAFT.md); all remain unsent.

The requested product is a Sony 1-inch-type 4K outdoor camera with PoE Ethernet,
Wi-Fi, Bluetooth and USB-C power/service, plus a multilayer PCB, case, drawings
and BOM. This directory is the first concrete concept and PCBWay RFQ package.
**It is not a completed camera or a fabrication-ready PCB.**

Start with [PCBWay handoff](PCBWAY-HANDOFF.md),
[seven drawing sheets](outputs/CCTV-A0-drawing-sheets.pdf),
[system BOM](bom.csv), and [validation record](VALIDATION.md).
The [assembly STEP](outputs/cctv-assembly.step) and
[exploded STEP](outputs/cctv-exploded.step) include approximate supplier envelopes.
The [seven-layer board model](outputs/seven-copper-layer-board.step) represents
copper/dielectric thicknesses; it contains no circuit routing.

## Status

- Generated and validated CAD solids; ran 18 selected interference checks.
- Created seven A3 PDF drawing sheets, a system BOM PDF/CSV, DXF outline and
  standalone STEP parts in PCBWay-documented file formats for quoting.
- Revised internal cavity tool access and screw/seal clearance for CNC review.
- Confirmed standard PCBWay documentation includes eight-layer stacks; seven
  remains a custom feasibility question, not an approved standard stackup.
- Camera candidate B0477 uses Sony IMX283 and advertises 4K at 20 fps. Frame rate,
  viewing distance, night vision, environment, form and budget are not frozen.
- Supplier review corrected the SOM rail to 4 V and identified a peak-power
  shortfall for the provisional PoE PD; separate 4 V/5 V regulators are budgeted.
- Manufacturing remains blocked on component selection/reference designs,
  schematic/layout completion, factory stackup and mechanical-detail closure.
- No Inventor-native model/drawing or simulation has been completed: the existing
  Wine import/automation failure remains unresolved. PDFs and STEP were generated
  using CadQuery and ReportLab. No files have been uploaded or sent to PCBWay.

## Reproduction

Tested with Python 3.12, CadQuery 2.8.0 / OCP 7.9.3.1.1 and ReportLab 5.0.1.
`pypdf` is used to verify drawing page counts. Native libraries needed by
CadQuery must be available on the host (the Nix test environment used an ignored
local virtual environment and explicit library paths).

```sh
python -m pip install cadquery==2.8.0 reportlab==5.0.1 pypdf
python research/cctv-camera/build_model.py
python research/cctv-camera/build_bom.py
python research/cctv-camera/build_drawings.py
python research/cctv-camera/package_rfq.py
```

The code defines an authored concept; it is not supplier CAD, a PCB library,
qualified circuit or final CAM source. Most enclosure dimensions are preliminary.
Only keys actually read by the model are driving parameters; several reference
values in `requirements-a0.json` are descriptive and remain explicit constants in
`build_model.py`. Review both before changing dimensions.

## Supplier references

- [Sony IMX283 Type 1 flyer](https://www.sony-semicon.com/files/62/pdf/p-13_IMX283CQJ_Flyer.pdf)
- [Arducam B0477 camera data](https://www.arducam.com/downloads/datasheet/B0477_20MP_IMX283_USB3.0_Camera_Datasheet.pdf)
- [Firefly Core-3588J](https://www.t-firefly.com/products/core-3588j-8k-ai-core-board-rockchip-rk3588-new-gen-8-core-64-bit-processor)
- [Ezurio radio variant data](https://www.ezurio.com/documentation/datasheet-sterling-lwb5-m2)
- Power-component links and selection caveats are in `bom.csv`.

No manufacturer acceptance, outdoor rating or successful end-to-end prototype
operation is implied by inclusion of any candidate part.
