# PCBWay handoff — A0 feasibility / RFQ only

**Do not order fabrication or assembly from this revision.** This package has
STEP solids and PDF drawings in formats PCBWay documents accepting for CNC
quoting. Format compatibility is not PCBWay design approval. No file has been
uploaded or sent to PCBWay; no quote or engineering acceptance has been obtained.

The user requested a seven-copper-layer camera PCB and matching outdoor case.
PCBWay's published standard stackup page lists even layer counts including eight.
Exactly seven remains a custom feasibility request; eight is the recommended
alternative pending the user's choice. No claim that seven is impossible.

## What the supplied files can be used for

| Files | Intended use | Release status |
|---|---|---|
| `outputs/cctv-assembly.step`, `cctv-exploded.step` | Overall packaging and fit discussion | Envelope concept; supplier parts approximate |
| Individual `outputs/*.step` housing parts | CNC manufacturing feasibility and budget quotation | RFQ only; attachments and connector selections unfinished |
| `outputs/CCTV-A0-drawing-sheets.pdf` | Seven sheets: arrangement, body, end plates, PCB, assembly, electrical architecture, validation | Drawings made outside Inventor; not native IDW/DWG |
| `outputs/seven-copper-layer-board.step` | Requested board thickness/layer visualization | Full-sheet copper envelopes, not electrical artwork |
| `outputs/pcb-outline.dxf` | Board outline and four mounting-hole centers in mm | Mechanical reference only; not NC drill data |
| `bom.csv`, `outputs/CCTV-A0-system-BOM.pdf` | System-level sourcing and cost planning | Candidates/TBDs; NOT the PCBA BOM |
| `outputs/geometry-checks.json` | Reproducible geometry check results | Restricted checks only, not product validation |
| `requirements.json`, `VALIDATION.md` | Requirements, assumptions and release gaps | Engineering review |

## PCBWay requirements checked 2026-09-24

1. [Assembly file requirements](https://www.pcbway.com/assembly-file-requirements.html):
   supply RS-274X Gerbers, BOM and centroid data. Include copper, silkscreen and
   solder paste. Assembly documentation must clarify orientation and polarity.
2. [Assembly ordering help](https://www.pcbway.com/helpcenter/pcb_assembly_ordering/What_files_are_requested_for_assembly_production_.html):
   placement data carries references, X/Y, rotation and mm units; it is required
   for SMT. Coordinate/designator correspondence is essential.
3. [Assembly process](https://www.pcbway.com/assembly-process.html): NC drill and
   fabrication information are part of the handoff, alongside board/placement
   data. Glue data only if the chosen process needs adhesive; confirm with factory.
4. [Standard stackups](https://www.pcbway.com/pcb_prototype/What_is_layer_stack_up.html):
   published standard families include 4, 6, 8 and other even layer counts.
5. [PCB tolerances](https://www.pcbway.com/pcb_prototype/PCB_Manufacturing_tolerances.html):
   the published nominal thickness tolerance for boards >=1 mm is +/-10%; the
   cited general minimum annular-ring width is 0.15 mm. This is not an approved
   stackup or permission to use every process limit simultaneously.
6. [CNC ordering process](https://www.pcbway.com/blog/22/CNC_Ordering_Process_PCBWay_Website_Exploration_07_59da1665.html):
   STEP is supported along with other solid-CAD formats. CAD and selections in
   the quote form take priority over technical drawings: keep them consistent.
7. [CNC quote form](https://www.pcbway.com/rapid-prototyping/manufacture/?type=2):
   technical drawings and material/finish specifications accompany the 3D files.
   Internal corners and assembly requirements need explicit review.

## Adjustments already made for machining review

- Removed the first concept's internal shoulder/undercut. The body now has a
  constant 98 x 74 mm, R12 through cavity with access from both ends.
- Enlarged the corner radii and moved screw locations to keep the end seals
  continuous. Selected seal-to-body and seal-to-screw intersections are zero.
- Retained separate front/rear plates and a separately formed sunshield.
- Exported individual authored parts as STEP, in mm, with revision-matched PDF
  views. Connector holes remain clearly marked reservations.
- Requested 6061-T6 for machined body/plates and 5052-H32 for formed shield.
  Final finish and masking of seals/threads require supplier review.

The deep through cavity still needs a tool-reach and cost review. This is a heavy
machined prototype concept, not an optimized production die casting. Ask whether
machining, an available extrusion plus secondary machining, or a redesigned
housing gives the best cost. Do not substitute an extrusion without matching
geometry and sealing interfaces.

## Proposed PCB manufacturing constraints (not a laminate release)

- 140 x 86 mm, nominal finished thickness 1.60 mm; fit must accommodate 1.44–1.76 mm.
- High-Tg FR-4 candidate, ENIG, green mask, white top reference designators.
- Seven requested copper layers: S / GND / S / PWR / S / GND / S.
- Eight-layer alternative for engineering review: S / GND / S / PWR / GND / S / GND / S.
- Factory to provide available copper weights, cores/prepreg, pressed thickness,
  impedance calculation/coupon proposal and stack symmetry before routing.
- Start general routing at 0.20 mm width/clearance where feasible; exceptions for
  selected connector fanout require DRC and supplier agreement.
- Candidate through vias: 0.30 mm finished hole / 0.65 mm pad (0.175 mm nominal ring).
  No blind vias or via-in-pad assumed. Actual drill allowance comes from factory.
- USB differential target 90 ohms; Ethernet MDI differential target 100 ohms;
  single-ended controls typically 50 ohms where required. Exact routing and
  length rules must come from the selected component reference designs.
- RF 50-ohm routing only if on-board RF traces are needed; coax path is preferred.
- No trace widths are assigned to controlled-impedance nets until stackup approval.
- Separation across PoE isolation, transient protection and shield bonding need
  an electrical design review. Generic signal clearance is NOT an isolation rule.

## Missing before a PCB/PCBA order

- Complete pin-level schematic and ERC review, including the SOM connector,
  supported Ethernet PHY, PoE bridges, USB power switching, service data routing,
  regulators, sequencing, protection, all passives and programming headers.
- Exact orderable MPN, package and footprint for every fitted component;
  verified supplier mating connector drawings and camera/processor BSP support.
- Routed PCB with DRC and electrical review, reference planes and power budgets.
- RS-274X Gerbers for every physical copper layer, board outline, top/bottom mask,
  silkscreen and paste as applicable; PTH and NPTH drill files and drill map.
- Complete PCBWay-style assembly BOM with line/quantity/reference/MPN/description/
  package/mounting type/manufacturer, substitutions and DNP flags.
- Placement file with matching references, X/Y mm, rotation and board side,
  common origin, polarity/pin-1 assembly drawings and fit checks.
- Firmware programming/test procedure, expected current and functional criteria.

No dummy Gerbers or invented component placements have been generated. System
BOM line IDs E01/M01 etc. are item IDs, not actual circuit reference designators.

## Questions for a supplier feasibility request

- Can you support exactly seven physical copper layers at this thickness? If not,
  quote an eight-layer alternative with the proposed reference-plane roles.
- Provide a manufacturable stack and 90/100-ohm routing proposal before layout.
- Can the body through cavity be machined to the supplied model economically?
- Review end sealing lands, gland seats, finish growth and achievable flatness.
- Quote components separately from custom mechanical parts; identify MOQ,
  lead time and suggested substitutions, but do not purchase substitutions.
- Treat all A0 models as feasibility geometry, not authority to manufacture.
