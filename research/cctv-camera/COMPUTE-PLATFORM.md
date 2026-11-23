# Common camera compute platform — preliminary, 2026-09-24

User requirements: no Android, shared software across fixed/PTZ products,
hardware AI, eventual LTE/5G, Wi-Fi, Bluetooth and selectable radio monitoring.
Software development is deferred. No production part, housing fit or power
budget is approved. Monitoring scope is awaiting clarification.

**Updated shortlist:** prioritize an industrial **Forlinx FET3588J-C** evaluation
for the shared fixed/PTZ platform. Native HDMI RX and MIPI avoid the CM5's PTZ
input gap. See [hardware downselection](HARDWARE-DOWNSELECT.md) for evidence,
provisional interface allocation and qualification gates. CM5 remains a smaller
bench alternative, not the lead outdoor candidate.

## Proposed architecture

Use one ARM64 Linux application platform and, where feasible, the same SoM.
Change the carrier outline, power stage, camera adapter and radio population
for each enclosure. An MCU is a companion for watchdogs, power sequencing,
temperature/fan/heater control and PTZ interlocks, not the 4K video computer.
Keep independent supervision available if Linux hangs. Exact MCU is TBD.

Shared software interfaces should separate capture/ISP/encoding, AI inference,
network management, PTZ control, radio diagnostics and updates. Same application
code does not imply identical device trees or accelerator binaries across SoCs.
Debian is a prototype option; a reproducible embedded Linux distribution is a
later production decision. Preserve board support package and firmware versions.

## Smaller bench alternative: Radxa CM5 / RK3588S2

Current product page lists eight CPU cores, integrated 6 TOPS NPU, H.264/H.265
hardware encoding, Debian, Gigabit Ethernet, MIPI camera inputs, USB3 and PCIe.
Size is listed as 56×41 mm; older product brief says 55×40 mm. Use the exact
revision STEP/drawing rather than either nominal dimension for placement.
Public schematic, carrier notes and STEP are available.

Critical limitation: recommended ambient is only 0–50°C, so this is NOT a
qualified -20–50°C outdoor choice. A supported industrial-temperature module
or qualified thermal design is a production gate. The USB3/PCIe/SATA multiplexing
also requires pin-level allocation before promising simultaneous modem, Wi-Fi,
capture and service interfaces. No HDMI input is listed; HDMI output is not input.

Sources:
- https://radxa.com/products/cm/cm5/
- https://dl.radxa.com/cm5/radxa_cm5_product_brief_Revision_1.3.pdf
- https://docs.radxa.com/en/som/cm/cm5/radxa-os/app-dev/ai

Treat TOPS as a vendor peak, not application throughput. Benchmark intended
models alongside continuous 4K60 capture/encode; AI can use a scaled stream
and need not infer on all 60 full-resolution frames. Start with the integrated
NPU, and add another accelerator only if benchmarks justify space and power.
RAM/storage capacities remain workload-dependent, not finalized BOM choices.

## Camera compatibility gate

Fixed camera: IMX585 MIPI candidate needs the exact sensor driver, lane rate,
ISP tuning and verified sustained 4K60 encoder path on our chosen Linux BSP.
PTZ: Sony FCB-ER9500 supplies HDMI-compatible TMDS. It needs a verified 4K60
capture bridge or auxiliary encoder; an arbitrary HDMI-to-CSI adapter is not
automatically adequate. Prefer changing the camera front-end over maintaining
two unrelated application platforms. If native HDMI RX is decisive, revisit
full RK3588 modules and their exposed inputs before committing to RK3588S2.

Sony source: https://pro.sony/ue_US/products/zoom-camera-blocks/fcb-er9500

## Radios as replaceable subsystems

Cellular candidate: Quectel RM520N family, M.2 30×52×2.3 mm, with LTE/5G and
vendor Linux USB documentation. Exact region/carrier SKU, bands, firmware,
host interface and certification must be selected. Reserve SIM/eSIM provisions,
antenna connectors/clearance, burst-current capacity, reset/load switch and
thermal path. M.2 dimensions omit socket, standoff and cable bends.

Sources:
- https://www.quectel.com/product/5g-rm520n-series/
- https://quectel.com/content/uploads/2024/04/Quectel_UMTS_LTE_5G_Linux_USB_Driver_User_Guide_V3.2.pdf

Wi-Fi: choose exact chipset, module, firmware and Linux driver based on actual
monitor-mode tests, not a Wi-Fi generation label. Linux documents concurrent
managed/monitor operation as hardware-dependent and best effort. Proposed
option: independent connectivity and observation radios if monitoring must
continue while streaming. One tuned radio does not observe every channel at
once; channel hopping leaves gaps. No Wi-Fi module is selected yet.

Source: https://wireless.docs.kernel.org/en/latest/en/users/documentation/modes.html

Bluetooth: distinguish advertisement scanning, logging our own controller
traffic, and over-the-air packet capture. Nordic's BLE sniffer is a candidate
evaluation tool for the last task, not a promise of arbitrary Bluetooth Classic
or encrypted-session capture. A separate receiver may be needed for concurrent
connectivity and observation. No final Bluetooth hardware selected.

Source: https://www.nordicsemi.com/Products/Development-tools/nRF-Sniffer-for-Bluetooth-LE

LTE/5G: do not promise Wi-Fi-like monitor mode from a normal modem. Initially
scope to its own link/cell diagnostics and supported vendor logs. General RF
observation would be a separate receiver/system requirement, not a software
toggle. No subscriber identification or third-party traffic capture capability
is assumed by this architecture.

Vendor diagnostics reference:
https://quectel.com/content/uploads/2024/04/Quectel_QLog_Linux_Android_User_Guide_V2.0.pdf

## Mechanical and validation gates

- Hasion 38/42 mm sensor-board support is not a usable compute/radio envelope.
  Obtain internal CAD and place SoM, carrier, modem, shields, connectors, flexes,
  antenna cables and heat spreaders as a complete stack.
- Budget simultaneous encode, AI, cellular transmit, motion startup and optional
  IR/heater loads. Determine PoE class and USB-C PD input from measurements.
- Validate RF performance with the actual metal housing and motor electronics.
  External sealed antennas or deliberate RF windows may be necessary.
- Confirm all interfaces can operate simultaneously and survive modem resets,
  link changes and power loss without corrupting recording or firmware.
- Require cold start, hot ambient plus solar load, sustained streaming, inference
  latency and frame-loss tests before claiming outdoor 4K60 operation.
- Do not select an alternative from a “4K” label: capture, decode, display and
  encoding limits are different. For example Ambarella's CV75S announcement
  specifies 4K30 encoding, so that part alone does not satisfy our 4K60 target.

Alternative-screening source:
https://www.ambarella.com/news/ambarellas-latest-5nm-ai-soc-family-runs-vision-language-models-and-ai-based-image-processing-with-industrys-lowest-power-consumption/
