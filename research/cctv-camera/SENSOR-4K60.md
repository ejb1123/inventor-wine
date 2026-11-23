# 4K60 sensor decision — 2026-09-24

Requirement: real 3840 x 2160 at 60 frames/s; Sony preferred; one-inch optical
format no longer required. Sensor recommendation: **IMX585 STARVIS 2**, a
1/1.2-type CMOS rolling-shutter sensor. Exact module is not procurement-released.

[Sony](https://www.sony-semicon.com/en/news/2021/2021062901.html) specifies
3856 x 2180 effective pixels, 12-bit 60 fps / 10-bit 90 fps, 2.9 um pixels and
MIPI output. This establishes sensor capability, not a supported 4K60 HDR mode
or a validated camera pipeline. Exposure must remain compatible with 60 fps;
night-time exposure cannot silently slow the stream.

[FRAMOS FSM-IMX585](https://docs.framos.com/en/latest/FSMEcosystem/ProductDocumentation/FSM/index.html)
is a module candidate: its table lists 90.1 fps maximum with four lanes versus
44 fps with two. Resolve the exact 12-bit/60 mode, MIPI lane rate, connector,
voltages, driver and ISP tuning on the chosen processor before selecting it.
No drop-in RK3588 support has been established.

Smaller alternative: IMX678 (1/1.8 type), with a module vendor explicitly listing
3840 x 2160 at 60 fps:
[SENSING SG8-IMX678C-MIPI](https://wiki.sensing-world.com/docs/1_2_MIPI_CSI-2_Camera/SG8-IMX678C-MIPI).
Host compatibility, mode details and temperature limits still need confirmation.

Do not buy by sensor name alone. The
[Arducam IMX585 USB module](https://www.arducam.com/presalesarducam-8-3mp-imx585-manual-focus-usb-3-0-camera-module-with-16mm-c-mount-lens.html)
reviewed lists only 4K15. The prior B0477 in A0 lists 4K20 and is rejected for this
new target. Sensor and packaged-camera frame rates are different requirements.

Calculated active-pixel payload at 4K60 RAW12 is 5.972 Gbit/s before transport
and blanking; four lanes average 1.493 Gbit/s each before overhead. Select actual
lane rate from sensor timing tables, not this lower bound. Use MIPI to the ISP
and hardware encoder for the revised architecture; USB-C remains power/service.
An external ISP or tuned processor ISP, H.264/H.265 encoding, power/thermal budget
and sustained network streaming must all be validated at 60 fps. Do not infer
these from the sensor's maximum frame rate.

The A0 ZIP, BOM and drawing sheets are historical concepts and have NOT been
regenerated into a 4K60 design. Carrier wiring, camera supply, optics, module
mounting, CSI connector and power estimates must be revised before a new release.
