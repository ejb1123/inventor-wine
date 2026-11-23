# PTZ supplier backup research — 2026-09-24

Status: sourcing candidates, not approved suppliers or a validated assembly.
Hasion X6Z7 remains the fixed-camera candidate. No inquiries or orders sent.

## Recommended mechanical fallback: BIT-CCTV / Blue Icon (Tianjin)

BIT-CCTV explicitly offers OEM/custom products, motorized pan/tilt positioners,
and camera housings. BIT-PT503 is a concrete starting point for a modular
prototype: manufacturer specifications list 3.5 kg payload, 3.5 kg unit weight,
0–360° pan, ±60° tilt, 30°/s maximum pan, 15°/s maximum tilt, ±0.1° preset
accuracy, IP66 and RS485/RS422 Pelco P/D control. Camera housings are available.
The weight makes it an industrial fallback, not a compact dome recommendation.
Do not infer unlimited rotation or an included Ethernet slip ring from 0–360°.

Contact published by manufacturer: sales@bit-cctv.com.

Sources:
- https://www.bit-cctv.com/products/
- https://www.bit-cctv.com/products/outdoor-mini-pan-tilt-positioner-head.html
- https://www.bit-cctv.com/uploads/file/20250628/BIT-PT503_Mini_Size_Outdoor_3.5kg_Top_Load_Worm_Gear_Pan_Tilt_Unit.pdf

## Compact OEM alternative: Hangzhou Soar Security

SOAR908 is a compact outdoor PTZ family advertised with OEM/ODM customization,
IP66, optional PoE, API/SDK and continuous pan. The reviewed page lists a
160 mm diameter × 270 mm envelope. It is only a mechanical/ODM inquiry lead:
published 2MP/4MP variants do not meet 4K60. The page also mixes 4x and 33x lens
descriptions; require a dated model-specific drawing and datasheet before use.
Sale of a bare mechanism and acceptance of our own electronics are unconfirmed.

Contact published by manufacturer: sales@hzsoar.com.

Sources:
- https://www.hzsoar.com/advanced-ethernet-output-camera-module-hzsoar-s-active-deterrence-mini-ptz-camera/
- https://www.hzsoar.com/ptz-housing/

## Verified optical candidate: Sony FCB-ER9500

Sony specifies 3840×2160 at 60p, a 1/1.8-type STARVIS 2 sensor and 25x optical
zoom. This is a candidate for the PTZ branch, not a replacement of the fixed
camera's IMX585 candidate. Video uses HDMI-compatible TMDS rather than the
assumed raw MIPI interface; a suitable 4K60 network encoder is still required.
No fit, power, thermal or controller compatibility with either mechanism has
been validated. Do not substitute FCB-EV9500L: that model is FHD.

Sources:
- https://pro.sony/ue_US/products/zoom-camera-blocks/fcb-er9500
- https://pro.sony/s3/2025/01/13153252/FCBER9500_DS_HR-2024-11-20-14_34_56.pdf

## Complete-camera benchmark: Bolin EXU420F

Bolin advertises an outdoor IP67 Sony-based 4K60 PTZ with 20x optical zoom,
4K60 HEVC IP streaming and PoE++. This establishes a complete-product option,
but public specifications do not establish OEM mechanism sales, custom-board
integration, Wi-Fi, Bluetooth or USB-C power/service for our product.

Source: https://bolintechnology.com/product/exu420f-4k60-live-production-outdoor-ptz

## Inquiry scope — draft only, not sent

Ask BIT-CCTV and Soar for a mechanism/housing/controller kit suitable for a
Sony FCB-ER9500 and separate encoder, plus the following:

1. Confirm kit contents: motors, gears, bearings, encoders, home sensors,
   control board, seals, cables, mounting hardware and spares. Can they supply
   an unpopulated housing and support independently designed electronics?
2. Provide internal STEP and dimensioned drawings, payload/center-of-gravity
   limits, optical clearance, mounting patterns and thermal attachment points.
3. Specify continuous versus limited pan, tilt travel, backlash, repeatability,
   duty cycle, lifetime, wind loading and test evidence for the exact kit.
4. Specify cable routing and slip-ring channels, Ethernet speed and integrity,
   power current limits, and location of the encoder relative to the rotating
   joint. Do not assume raw video can cross an unspecified slip ring.
5. Provide controller protocol/SDK, command and position-feedback examples,
   firmware update method, licensing terms and long-term availability.
6. Provide motor startup/stall and simultaneous motion power, plus heater/IR
   loads separately. Determine PoE class and USB-C PD requirements from the
   complete budget; neither has been selected for the PTZ branch.
7. Confirm sealed USB-C service access, Wi-Fi/Bluetooth antenna provisions and
   the ingress rating in each connector-cover state. Target ambient is
   -20 to +50°C; provide thermal/cold-start evidence, including optics.
8. Quote 1–2 evaluation kits and indicative 100/1,000 quantities, MOQ, lead
   time, NRE/tooling, warranty and documentation availability. Quote quantities
   are not production commitments. Do not start tooling or production.

Acceptance requires a sample demonstrating sustained 4K60 IP video while
panning/tilting/zooming, measured worst-case power, environmental qualification,
and verified service/wireless interfaces. No supplier currently qualifies for
the entire requirement set from the evidence reviewed. Pricing is unquoted.
