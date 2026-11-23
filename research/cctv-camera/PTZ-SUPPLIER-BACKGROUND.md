# PTZ background supplier research — 2026-09-24

Research only. No supplier contacted, samples ordered, or supplier qualified.
This supplements `PTZ-SUPPLIER-SHORTLIST.md`; Hasion remains the fixed-camera
lead. Published specifications below are vendor claims, not our test results.

## Recommendation

**KEVIS is the strongest new lead for a modular OEM PTZ assembly.** Its explicit
assembly-level supply and SDK offering is a better match to our own electronics
than a catalog of finished cameras. Request its smallest suitable mechanism,
with KSD-9500 / KSD-2000 family housings as starting references, before selecting
an exact SKU. SOAR908 remains the compact alternate; BIT-PT503 remains the
documented but heavier independent motion-platform fallback.

**No reviewed supplier is verified to deliver the complete requested product:**
outdoor 4K60 optical PTZ, shared non-Android Linux compute, AI acceleration,
PoE, Wi-Fi, Bluetooth, eventual LTE/5G, and USB-C power/service. OEM capability
does not establish acceptance of our board or fulfillment of those interfaces.

## 1. KEVIS — modular dome mechanism/housing lead

The [manufacturer PTZ page](https://www.kevis.co.kr/ptz) explicitly offers
assemblies separately, SDK support and OEM/ODM customization. It lists
selectable camera, IP electronics, IR and wiper options. Its newer header
advertises Sony IMX678 8.48MP or IMX664 4.22MP, 38x zoom, IP67 and PoE++.
Those are family claims, not a confirmed 4K60 SKU.

The [manufacturer catalog, pages 3–4](https://www.kevis.co.kr/ko/_files/ugd/e9f8d1_349e11ab408643fe90b09bb4b0dd4181.pdf)
shows KSD-2000, KSD-3000, KSD-8000, KSD-9500 and KSD-9500V2 shells plus
ODF-series interchangeable front assemblies. Its KSD-9500 IoT table instead
lists IMX664, approximately 4.22MP, 40x optics, and a 1920×1080 video row.
This discrepancy prevents treating the website header as an orderable 4K60
configuration. Catalog pages were visually inspected because text extraction
was corrupted.

- Controller: assembly-level SDK promised; electrical interface, command set,
  position feedback and rights to control it from our Linux board unverified.
- Dimensions/internal STEP: not established from reviewed sources. Do not size
  our PCB from product photos.
- Sample/MOQ/price: not publicly established.
- Published contact: **yjhong@kevis.co.kr**, +82-31-427-9801.
- Next evidence needed: dated mechanical-kit BOM, internal CAD, and confirmation
  it accepts our optics and separate SoM/carrier rather than their IP board.

## 2. Hangzhou Soar — SOAR908-2133 compact ODM alternative

The [manufacturer SOAR908-2133 listing](https://www.hzsoar.com/es/poe-ir-speed-network-dome-ptz-camera-built-in-red-blue-light-and-two-way-audio-product/)
specifies Ø160×270 mm, continuous 360° pan, −18° to +90° tilt, 33x zoom,
IP66, DC12V/30W maximum and optional PoE. It offers OEM/ODM and custom molds.
The exact listed camera is 1920×1080; general family text mentions 2MP/4MP.
**Neither is 4K60.** Published network control includes ONVIF/CGI and 10/100
Ethernet, which does not establish an accessible bare motor-controller protocol.
The linked Spanish manufacturer page was available; the corresponding English
URL failed during this check.

- Internal CAD and spare interior volume: unverified.
- Bare mechanism/controller sale: unverified despite ODM advertising.
- Sample/MOQ/price: unverified.
- Published contact: **sales@hzsoar.com**, +86-15858277138.
- Inquiry target: retain compact mechanics while replacing sensor/encoder with
  a 4K60 pipeline and our Linux carrier; establish thermal and antenna room.

## 3. BIT-CCTV / Blue Icon — BIT-PT503 independent motion fallback

The [exact product page](https://www.bit-cctv.com/products/outdoor-mini-pan-tilt-positioner-head.html)
specifies **141×110×199 mm**, 3.5 kg unit mass and 3.5 kg payload; a camera
housing adds size and mass. It lists DC24V, ≤30W, IP66, −30 to +65°C,
±60° tilt, up to 30°/s pan and 15°/s tilt. RS485 Pelco-D/P, optional
RS232/RS422, position feedback and ±0.1° preset accuracy are specified.

Unlike the earlier summary's conservative reading, the current technical
table explicitly says **continuous 360° pan**, with customized limit switch.
It still does not establish Ethernet slip-ring channels or signal integrity.
Camera housings are offered separately. The page has dimension/wiring images
but no verified downloadable STEP. FAQ text incorrectly names BIT-703A;
request a dated BIT-PT503 drawing and quotation to avoid revision ambiguity.

- Sample/MOQ/price: not established.
- Published contact: **sales@bit-cctv.com**, +86-139-2068-8922.
- Assessment: easiest documented external-controller approach for our Linux
  platform; less suitable for an attractive compact dome product.

## 4. Fengtaida — IRM compact-family ODM inquiry only

The [IRM product section](https://www.fengtaida.com/products/irm-mini-outdoor-ptz-camera)
lists an aluminum outdoor PTZ with IP66/IK10, Ø226×363 mm, approximately 4 kg,
AC24V/36W, 10/100 Ethernet and ONVIF Profile S. Its page also embeds other
families' specifications, so the later Ø223×372 mm/8 kg IRS table must not be
treated as IRM data. Its [PTZ catalog](https://www.fengtaida.com/collections/ptz-camera)
offers OEM/ODM, while the specific FTD-IRS-E2082 4K model is **25/30fps**, not
60fps. A finished 4K30 camera is not an acceptable substitution.

- Controller access, bare mechanism sale and internal CAD: unverified.
- MOQ is mentioned without a number; sample terms and price unverified.
- Published contact: **sales6@fengtaida.com**.
- Assessment: reserve ODM lead; physically larger than SOAR908 and no proven
  4K60 offering. Ask whether an IRM mechanical kit can accept our electronics.

## 5. Ruilong / Lensolve — complementary optics/encoder integration lead

This is **not a verified outdoor housing/mechanism supplier**. Its
[PTZ integration page](https://www.lensolve.com/PTZ-Camera-pl46381697.html)
offers Sony FCB camera blocks, interface boards, custom integration and SDKs,
and mentions matched pan/tilt mechanisms without an exact mechanical SKU.
Its [VRS-UD410 product](https://www.lensolve.com/VRS-UD410-4KP60-AI-Capable-Addable-IP-Interface-Board-pd593053838.html)
explicitly lists Sony FCB-ER9500 at 2160P60 over IP, H.264/H.265, Gigabit
Ethernet, TTL/VISCA, USB host/device and a roughly 48×75×5 mm board excluding
heatsink. It quotes 12V input and 2W consumption; worst-case measured power
still needs confirmation. The page contains placeholder fields, so require a
controlled datasheet and sample test.

**The board is described as RTOS-based, not Linux.** Treat it only as a possible
auxiliary encoder, never as fulfillment of our shared Linux/AI computer.
It also does not establish USB-C PD, Bluetooth, LTE/5G, outdoor sealing or
unrestricted custom AI deployment. Contact through the site's published
Contact link; no verified numerical MOQ or sample pricing was found.

## Common Linux platform: mechanical procurement gates

These are our requirements for a future inquiry, not vendor commitments:

1. Supply housing, bearings, motors, gears, home/position sensors and control
   board independently of a sealed proprietary camera-compute stack. Document
   firmware/protocol access, fault reporting and update/recovery procedures.
2. Provide internal STEP, dimensioned mounting drawings, rotation envelopes,
   payload/center-of-gravity limits and lens-window clearance for a Sony 4K60
   block plus separate Linux SoM, carrier and AI hardware. No board outline is
   fixed until those clearances are verified.
3. Reserve modem mounting, SIM/service access and antenna provision for eventual
   LTE/5G plus Wi-Fi/Bluetooth. Establish which antennas remain stationary,
   their RF paths, clearance and sealing. Do not assume RF works through a
   metal dome or that arbitrary coax can cross a rotating joint.
4. State stationary versus rotating electronics arrangement and slip-ring
   channels/current/data ratings. Prefer a validated compressed-data path
   across the joint; do not assume raw 4K60 video can traverse any slip ring.
5. Provide thermal attachment surfaces and worst-case temperature limits for
   compute, AI, modem, encoder and optics under sunlight and simultaneous PTZ
   motion. Budget motor startup/stall, zoom, IR and heater loads separately.
6. Confirm sealed USB-C power/service implementation, diagnostic connection and
   monitor output routing if required by the compute architecture. The eventual
   PoE class and USB-C PD contract follow the measured complete power budget.
7. Quote 1–2 evaluation kits plus indicative 100/1,000-unit pricing, MOQ,
   lead times, tooling/NRE, lifetime supply and spare motors/seals/controllers.

Before approving production, demonstrate sustained 3840×2160 at 60fps while
panning, tilting and zooming, with our Linux software, measured power/thermal
behavior and the intended wireless/service interfaces. A vendor IP rating on
a stock camera does not qualify the modified product.
