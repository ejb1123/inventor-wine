# Validation record — A0

## Executed checks

`build_model.py` checks every modeled shape with Open CASCADE validity and runs
18 selected solid intersections: major supplier envelopes against the enclosure,
selected module pairs, end seals against the body, and seals against screw
clearance cylinders. These are explicit limited checks, not an all-part assembly
interference audit. The exported stack contains seven copper envelopes and six
dielectric envelopes totaling 1.600 mm. No electrical features are modeled.

See `outputs/geometry-checks.json` for measured results. Model mass excludes
supplier envelope masses and hardware, and includes unetched copper sheets, so
it is not finished product mass. No Inventor mass/FEA/thermal result is claimed.

## Hand calculations and design risks

### Seal compression

For a proposed 2.00 mm free gasket seated in a 1.50 mm groove, nominal squeeze is
25%. Assuming (not sourcing) free-thickness tolerance +/-0.10 mm and groove
+/-0.05 mm gives a range of 18.42–30.95%. Compression force, compound, thermal
expansion, relaxation, groove fill and fastening preload remain unknown. This
range needs supplier approval or a changed groove depth. The model represents
the compressed rectangular gasket, not a standard circular O-ring.
The [Parker handbook](https://www.parker.com/content/dam/Parker-com/Literature/O-Ring-Division-Literature/ORD-5700.pdf)
is a reference for static sealing principles; its O-ring tables are not a
qualification of this custom rectangular gasket.

### Power and heat

Assumed steady allocations: SOM 10 W, camera 1.5 W, radio 1.5 W, PHY/aux 2 W.
This is 15 W load or 16.67 W before a 90%-efficient downstream converter. These
are engineering allocations, NOT measured maxima or module guarantees.
[Ag5300](https://silvertel.com/images/datasheets/Ag5300-datasheet-smallest-30W-Power-Over-Ethernet-Plus-Module-PoEplusPD.pdf)
lists 24 W continuous under specified nominal conditions; 30 W is peak. Cable
loss, bridges, PD conversion, elevated-temperature derating and startup need
separate accounting. Do not infer 30 W usable load from a PoE+ label.

A crude body area is 2*(.210*.114 + .210*.090 + .114*.090) = 0.1062 m².
At assumed effective h = 5 W/m²K, convection-only resistance is about 1.88 K/W.
At 18 W this implies ~34 C case rise before internal thermal resistances or sun.
A 50 C ambient could therefore produce an ~84 C case in this simplified model.
Solar absorption at 1000 W/m² and assumed absorptivity .3 across .024 m² adds
~7.2 W before shade/airflow effects. These rough bounds FLAG a thermal risk;
they do not predict a validated operating temperature. The sunshield helps but
cannot be credited without testing. A conduction bridge to the shell remains
unfinished. No heater or IR illumination power is allocated.

### Processor rail and peak-power release blocker

The [Firefly product specification](https://www.t-firefly.com/products/core-3588j-8k-ai-core-board-rockchip-rk3588-new-gen-8-core-64-bit-processor)
lists a 4 V +/-5% module supply. The concept now allocates separate buck
regulators for that rail and the camera 5 V rail. The same page lists about
20 W maximum in a 12 V test context; the carrier/test setup and transients need
supplier clarification. Using 20 W plus 5 W other loads gives 27.8 W before
PoE conversion at assumed 90% downstream efficiency, exceeding the 24 W PD
candidate's nominal continuous output. The 10 W SOM allocation is therefore
not a verified worst-case budget. A measured/enforced power limit, lower-power
processor or higher-power 802.3bt PD redesign is required. No unrestricted PoE+
operation is claimed. USB PD 15 V / 2 A also needs margin review at this peak.

### Optics and environment

The [B0477 datasheet](https://www.arducam.com/downloads/datasheet/B0477_20MP_IMX283_USB3.0_Camera_Datasheet.pdf)
identifies an IMX283 and a 4K20 mode, 34 x 34 mm board and maximum stated camera
power of 1.38 W. Its board/lens envelope is deliberately oversized in the CAD.
No extended outdoor operating-temperature guarantee was established from the
reviewed module data. No IR sensitivity/filter, wide-angle requirement, full
sensor-width 4K readout, low-light performance or frame-rate acceptance is assumed.
The Sony Type 1 description is an optical format, not a literal 25.4 mm diagonal.

A membrane vent is proposed for pressure/humidity management; it does not itself
qualify the enclosure. [Gore's camera guidance](https://www.gore.com/sites/default/files/2016-04/Gore-PTV-WhitePaper-Condensation-Reduction-in-Security-Cameras-US.pdf)
provides relevant condensation design context. UV, corrosion, rain, thermal
cycling and a capped service interface are separate design concerns.

## Physical test plan — not yet executed

| Test | Evidence needed |
|---|---|
| Bench image chain | Sensor mode list; stable 3840x2160 capture/encode/stream; frame loss/latency and long run |
| Power modes | PoE classification; startup; PD negotiation; source swaps; no backfeed; current/thermal logging |
| Service | USB enumeration with PoE present, recovery and signed/update rollback procedure |
| Electrical | ERC/DRC; rail sequencing; impedance/coupon; Ethernet/USB signal integrity; ESD and surge |
| RF | Antenna clearance, sensitivity/throughput, coexistence and final host compliance |
| Thermal | Soak at target ambient and solar load; sensor/SOM/PD temperatures; throttling and shutdown |
| Cold/condensation | Cold start, humidity cycling, glass fogging; seals after aging |
| Ingress | Agreed IP66 dust/water tests on final assembled hardware, cap and cable fitted |
| Mechanical | Mount preload and adjustment, wind/vibration, gasket compression, fastener retention |
| Optics | MTF/focus across field, window ghosts/reflections, night performance if required |

Targets and acceptance limits need a design review before qualification.

## Inventor feature plan and blocker

Use the relevant Inventor features after the Wine automation/import issue is
resolved: parameter-driven parts, extrudes/shells/fillets, hole/thread features,
assembly constraints, interference/section/mass checks, sheet-metal sunshield,
exploded presentation, parts lists/balloons and associative drawing views.
FEA of mount/load cases needs agreed loads/material data; thermal/flow simulation
requires a capable solver and boundary conditions. Rendering is visualization,
not proof of outdoor durability. It would be misleading to say every Inventor
feature has been exercised.

Current STEP export and PDF drawings were generated with CadQuery/ReportLab.
Inventor previously failed importing both the Möbius STEP and a plain cube; native
ComponentDefinition automation returned an integer instead of an object.
No native IPT/IAM/IDW for this camera has been completed or validated.
