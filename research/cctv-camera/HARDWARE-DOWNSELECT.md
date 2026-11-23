# Hardware downselection — 2026-09-24

Research recommendation only. No purchased samples, lab tests or supplier
confirmation. No schematic, fabrication BOM or housing-fit approval implied.
Software implementation remains deferred per user direction.

## Decision

Prioritize **Forlinx FET3588J-C**, with the matching industrial evaluation kit,
for one common Linux/AI platform. Evaluate 8 GB RAM / 64 GB eMMC as a starting
configuration, not a measured requirement. Use the existing 6 TOPS NPU first.
Keep Geniatech SOM3588 industrial variant as a second-source design option
(not pin-compatible). Retain Radxa CM5 only for smaller bench experiments.

| Candidate | Published module size | Camera interfaces | Temperature evidence | Role |
| --- | --- | --- | --- | --- |
| Forlinx FET3588J-C family | 68×50 mm | MIPI and HDMI RX | Industrial variant advertised −40 to +85°C | Lead evaluation |
| Geniatech SOM3588 | 82×53 mm | MIPI and 4K60 HDMI RX | Wide-temperature option advertised −40 to +85°C | Larger alternate |
| Radxa CM5 RK3588S2 | 56×41 mm current page | MIPI; no HDMI RX listed | Recommended 0 to +50°C | Bench alternative |

Forlinx publishes Linux BSPs, hardware codec examples and an HDMI capture node.
Its evaluation board also provides cellular and Wi-Fi/BT module connectors.
This is stronger integration evidence than a bare SoC feature list, but does
not prove our combined workload or exact camera compatibility.

Sources:
- [Forlinx module](https://www.forlinx.net/product/rk3588-som-134.html)
- [Forlinx evaluation board](https://www.forlinx.net/product/135.html)
- [Linux HDMI/camera documentation](https://docs.forlinx.net/rockchip/ok3588-c/OK3588-C_Forlinx_Desktop22_04_User_Manual.html)
- [Geniatech module](https://www.geniatech.com/product/som3588/)
- [Radxa module](https://radxa.com/products/cm/cm5/)

## Draft interface allocation — not pinmux-validated

| Function | Proposed connection | Closure needed |
| --- | --- | --- |
| Fixed camera IMX585 module | Four-lane MIPI CSI + I2C | Exact module/driver/ISP tuning, D-PHY timing, 4K60 |
| PTZ Sony FCB-ER9500 | Native HDMI RX + UART VISCA | TMDS adapter/pinout, voltage levels, EDID/color formats, sustained capture |
| PoE Ethernet | GMAC + PHY + magnetics + PoE PD stage | Electrical/reference design and measured power class |
| Quectel RM520N cellular | Dedicated USB3 host path, SIM and reset | Exact regional SKU, modem firmware, transient power and Linux driver |
| Connectivity Wi-Fi | Dedicated PCIe x1 module | Pinmux and exact Linux driver/firmware |
| Bluetooth connectivity | Module USB2 or UART, per selected module | Select one transport and reserve it explicitly |
| USB-C service | Separate USB OTG controller + PD sink | Gadget/recovery support, muxing, ESD and input power arbitration |
| Optional observation Wi-Fi | Second independent PCIe/USB radio | User scope pending; no capture-capable module selected |
| Optional BLE receiver | USB2/UART | Scope/receiver pending; not a general Classic Bluetooth sniffer |
| Supervisory MCU | UART or I2C + dedicated reset/power signals | Fail-safe outputs and watchdog independence |
| PTZ motion controller | UART with appropriate RS485/RS422 transceiver | Supplier protocol, feedback and safe startup states |
| Local storage | On-module eMMC; optional microSD | Endurance, capacity and recovery; do not consume modem port |

Reserve separate controllers for cellular and USB-C service. Do not count USB,
PCIe and SATA alternate functions as simultaneously available ports. Disable
unused display functions where they compete with required signals. Reference
kit connector availability is not proof of a conflict-free custom carrier.

Reference pinmux/hardware:
https://docs.forlinx.net/rockchip/ok3588-c/OK3588-C_User_Hardware_Manual.html

## Mechanical and power decisions

Use one compute module with two carrier outlines, fixed and PTZ. For full
cellular/radio builds, permit a separate sealed electronics base or junction
box if the optical head is too small. Housing internal drawings must decide
this; do not force the complete stack into a 38/42 mm sensor-board mount.

The industrial J variant has lower published CPU clocks than the commercial
version. Benchmark the exact SKU; do not transfer commercial-board throughput
results to it. Confirm module operating-temperature conditions, heat-spreader
requirements and component grades in writing. A module temperature rating is
not an outdoor camera qualification or permission to omit cooling.

Power documentation needs reconciliation: product page lists 12 V input,
current brief lists 5–13 V, and some manual pin rows list 5–12 V. Select the
exact hardware revision before regulator design. Do not reuse the historical
Firefly A0 module's 4 V rail for this module.

Keep the final PoE class and USB-C PD contract open until measured simultaneous
encode + AI + cellular transmit + motion/illumination/heater power is available.
The old A0 PoE budget is superseded. USB-C cannot be assumed to run all functions
from an arbitrary phone charger; later software must expose supported modes
for the negotiated power budget.

Source for variant clocks, supply and multiplexing:
https://www.forlinx.net/download/FET3588-C-and-OK3588-C-product-brief.pdf

## Qualification sequence and acceptance evidence

1. Obtain dated industrial module SKU, price, lifecycle, schematic/pinmux,
   mechanical STEP, BSP license/update policy and measured power information.
2. Confirm Linux camera paths on the evaluation kit: IMX585 RAW input with ISP;
   FCB-ER9500 HDMI input with the correct adapter. Record format and timestamps.
3. Sustain 3840×2160 at 60 fps encoding while running the chosen AI model on
   a scaled stream; record dropped frames, latency, memory, power and thermal
   throttling. AI frame rate remains a separate target to define.
4. Add modem and wireless. Test simultaneous upload, service connection,
   reconnects and power transients. Verify optional monitoring independently.
5. Freeze port/pin allocation, then make complete CAD stack and cooling design.
   Only then freeze carrier outline and PCB stackup.
6. Verify cold start, hot ambient/solar exposure, RF coexistence, environmental
   sealing and sustained workload on the assembled product.

Unresolved supplier questions are procurement inputs, not reasons to claim
the platform is proven. No quotes requested or messages sent in this research.
