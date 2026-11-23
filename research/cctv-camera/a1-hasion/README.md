# A1 — Hasion enclosure integration

Hasion is the user-selected enclosure supplier. X6Z7 is the lead candidate,
not a fit-qualified or ordered variant. Current requirements live in
[`../requirements.json`](../requirements.json).

- [Planning BOM](bom-planning.csv): replaces the A0 cost/part assumptions for
  current decisions; all supplier prices await quotes. This is not a PCBA BOM.
- [Supplier inquiry draft](HASION-RFQ-DRAFT.md): ready for review, not sent.
- [Sensor research](../SENSOR-4K60.md): IMX585 recommendation; module, ISP,
  encoder/host support and thermal performance remain unverified.

## Evidence

The [Hasion product catalog](https://www.cctvhasion.com/ProductInfoCategory)
lists X6Z7 with 38/42 mm board support and optional antenna/service features.
The [Hasion 2025 exhibitor catalog](https://oemfile.informamarkets-info.com/FileUpload/2025SEC_39523/20250228212738965.pdf#page=22)
identifies those as sensor-board sizes. Page 22 shows photos and an external
outline dimensioned 180.5 mm overall length; it is not an internal clearance
or mounting drawing. Manufacturer IP67 claims need checking for our actual
configured assembly. Catalog and page screenshot are held in ignored local
fixtures/logs; supplier artwork is not copied into the public source tree.

## Mechanical/electrical integration sequence

1. Obtain internal STEP/PDF and exact options, quote and a physical sample.
2. Freeze lens/module envelope and optical axis; assess focus/service access.
3. Select a compact ISP/encoder platform with verified 4K60 mode support and
   power consumption; size PoE and USB-C inputs around actual worst-case demand.
4. Lay out sensor/processor/power/radio boards within supplier keepouts. Sensor
   board support is not proof that all boards must or can be 42 mm square.
5. Design the thermal bridge and internal tray, then check interference, cable
   bends, sealing, mounting and service access with the real housing model.
6. Produce a new PCBWay release after schematic, routing, stackup and drawings
   are complete. Qualify the finished product's thermal/ingress performance.

A0 STEP models and PDFs remain historical references only. The old custom outer
shell, 140 x 86 carrier, large SOM, USB camera module and total budget are not
carried forward as approved parts. No new enclosure CAD has been guessed from
photographs, and there is no fabrication package for A1 yet.
