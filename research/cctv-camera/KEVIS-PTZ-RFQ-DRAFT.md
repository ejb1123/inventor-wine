# Draft only — not sent

To: KEVIS OEM/PTZ engineering and sales
Published contact: yjhong@kevis.co.kr
Subject: Modular outdoor PTZ kit for Sony 4K60 block and our Linux/AI electronics

Hello KEVIS team,

We are developing a camera product line with a shared non-Android Linux/AI
platform. Your PTZ page advertises modular assemblies, SDK support and OEM/ODM
customization. We would like to evaluate your smallest suitable outdoor
housing, pan/tilt mechanism and controller, independently of your IP camera
electronics. KSD-2000/KSD-9500 families are references, not selected SKUs.

Proposed payload:
- Sony FCB-ER9500 camera block: 3840 x 2160 at 60 fps, 25x optical zoom;
  nominal block envelope 56 x 64 x 125 mm from Sony's published datasheet.
  Final lens-window clearances and mounting CAD must be checked.
- Forlinx FET3588J-C Linux/AI SoM: nominal 68 x 50 mm, plus custom carrier,
  connectors and cooling. It would capture the Sony block's HDMI-compatible
  video and command zoom/focus separately from the pan/tilt controller.
- PoE Ethernet, Wi-Fi, Bluetooth, future LTE/5G modem (candidate RM520N,
  52 x 30 x 2.3 mm bare module), and USB-C power/service.

Please respond to the following technical and commercial questions:

1. Can you supply housing, motors/gears/bearings, home/position sensors, motor
   controller and wiring as a kit without your sensor/encoder board? Identify
   exact SKUs, included parts and which functions require your proprietary IP
   electronics. Please do not substitute a finished 4K30 camera.
2. Provide internal STEP and dimensioned drawings for the smallest suitable
   kit: payload bay, mounting patterns, camera optical axis, dome/window clear
   aperture, service access, cable paths and rotating/stationary volumes.
   Can our camera block and separate compute/carrier fit through the full
   travel range without clipping the image or contacting the housing?
3. Specify payload mass and center-of-gravity limits, continuous/limited pan,
   tilt limits, speed/acceleration, backlash, preset repeatability and motion
   duty/lifetime. Include swept envelopes and any restrictions at maximum zoom.
4. Provide controller electrical interface, voltage levels, SDK/protocol,
   position feedback, homing/limit behavior, faults, update/recovery procedure
   and licensing terms. Can our ARM64 Linux software operate it directly?
5. Describe the slip ring or cable management. Which power/data channels are
   provided, with current and Ethernet speed ratings? We prefer keeping the
   camera and encoder together and passing compressed Ethernet across the
   rotating joint where practical. Do not assume raw HDMI will cross a
   generic slip ring. Please recommend the supported arrangement.
6. Identify stationary antenna locations, sealed bulkhead options, cellular
   SIM access and any RF-transparent housing sections. Can USB-C be accessed
   for service and external power? State ingress protection with covers
   closed and with service/power cables attached.
7. Supply idle, motion, startup/stall, heater, wiper and IR power separately,
   required rails and protection behavior. PoE class and USB-C PD budget will
   follow the complete measured load; neither is finalized.
8. Target ambient is -20 to +50 C. Provide heat-transfer attachment surfaces,
   available thermal test data, cold-start/heater behavior, sun exposure
   assumptions, seals/vent design and ingress reports for the quoted kit.
   Specify which tests must be repeated with our modified electronics.
9. Your web page and catalog appear to describe different camera resolutions
   and zoom options. Please send a dated, configuration-specific datasheet
   and mechanical BOM. Our imaging target is real 4K60; we are not assuming
   your stock camera electronics provide it.
10. Quote 1 and 2 evaluation kits, indicative 100/1,000-unit pricing, MOQ,
    lead time, tooling/NRE, customization limits, warranty, lifecycle and
    spare controller/motor/seal availability. Quote quantities are exploratory.

Please identify any unsupported requirements and the smallest alternative
assembly you recommend. Do not begin tooling, customization or production.

Thank you.

---
Draft references:
- https://www.kevis.co.kr/ptz
- https://www.kevis.co.kr/ko/about
- https://pro.sony/s3/2025/01/13153252/FCBER9500_DS_HR-2024-11-20-14_34_56.pdf
No contact, order or commitment has been made.
