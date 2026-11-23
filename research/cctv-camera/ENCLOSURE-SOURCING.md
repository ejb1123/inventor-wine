# Sourced camera housings — 2026-09-24

**Decision:** Hasion selected by user; X6Z7 remains the lead model pending fit.
See [A1 integration](a1-hasion/README.md).

The user rejected the A0 custom housing appearance. Stop refining that exterior.
Select an existing camera housing, then design our electronics and internal tray
around its supplier drawings. No supplier has been contacted and no sample,
price, minimum order or available stock has been confirmed directly.

## Shortlist

| Candidate | Published evidence | Fit and sourcing assessment |
|---|---|---|
| Hasion X6Z7 | Manufacturer lists aluminum/plastic construction, ~310 g, IP67 claim, 38/42 mm PCB support, optional Wi-Fi and service features | Best compact OEM direction to investigate; requires much smaller boards/processor solution. Sample availability, price, MOQ and internal CAD unknown. |
| Hinmade H71X7 | Catalog lists ~310 g, 38/42 mm PCB support, IP67 claim and optional Wi-Fi opening | Another compact bullet-shell candidate; do not assume separate factory or independent tooling from similar Hasion designs. |
| Pelco/Videotec HOV | Published 100 x 70 x 250 mm usable interior, aluminum, side opening; appropriate variants rated IP66/IP67 | Most plausible roomy development housing for 86-mm-wide carrier aligned along length. Bounding dimensions alone do not establish fit. Orderable variants through surveillance distribution; request exact SKU/stock/quote. |
| AXIS T93F10 (5900-271) | Polymer housing, published IP66/IP67/NEMA 4X and IK10; dimension resources available | Finished commercial option, but larger box-camera form and intended for specified Axis cameras. Custom tray, thermal load and proprietary internals need inspection. T93F10 is AC/DC powered, not the PoE model. |
| Videotec PUNTO | Legacy datasheet: 70 x 70 x 250 mm usable interior, 0.8 kg, two M16 entries, IP66 | Narrower and simpler alternative but current 86-mm carrier will not lie flat inside. Current variant availability requires confirmation. |

A compact OEM bullet/turret housing best fits the requested appearance. The
current 82 x 53 mm processor SOM plus 140 x 86 mm carrier cannot simply move into
a shell designed around 38/42 mm camera boards. We should evaluate a compact
4K60-capable camera processor/encoder board together with the IMX585 sensor
module, power and radio, before fixing a housing model.

For a near-term large-board prototype, HOV is the more credible mechanical
candidate. It remains a sizeable industrial box; it is not presented as a sleek
compact product. The factory listing for Mingxin 14 PK was screened out for a
small prototype run because it states MOQ 2,000 and 35 working days.

## Integration requirements to obtain before selection

- Exact internal envelope, PCB hole pattern, module-stack height, lens/window
  clearances and removable end-cap drawings; preferably STEP and 2D dimensioned
  PDF. Obtain a physical sample before PCB outline freeze.
- Glass/IR window specification, lens holder options, vent and gasket details,
  installed mount range, cable exit and sealed service access.
- External antenna bulkhead or a suitable polymer RF window; radio performance
  must be checked in the finished housing. A Wi-Fi hole alone is insufficient.
- USB-C inside a gasketed service compartment or behind a specified sealed cap.
  Avoid drilling a new exposed port and assuming the original ingress claim holds.
- PoE power and heat dissipation compatible with 4K60, plus optional heater/IR
  loads. Housing heater supplies are not automatically suitable camera DC rails.
- Itemized sample/bulk price, MOQ, tooling/modification fees, shipping and lead
  time. Supplier IP claims/test reports must apply to the exact assembled variant.

PCBWay can make the carrier PCB and custom mounting tray/heat spreader after
mechanical selection. The outer shell, glass, gaskets and mount can be sourced
as an OEM housing assembly, reducing custom cosmetic and seal-design work.
No unsupported housing procurement price is included in the BOM yet.

## Sources and product photos

- [Hasion product catalog / X6Z7](https://www.cctvhasion.com/ProductInfoCategory)
- [Hasion sourcing FAQ](https://www.cctvhasion.com/faq): small orders are discussed,
  but the site does not provide a numeric MOQ for this part. Direct page retrieval
  was rate limited during this review; use the indexed manufacturer listing as
  a lead, not proof of present sample stock.
- [Hinmade housing catalog / H71X7](https://www.hinmade.com/ProductInfoCategory?PageInfoId=0&categoryId=138540)
- [Current Pelco housing catalog](https://www.pelco.com/products/accessories/camera-enclosures)
- [Pelco HOV specification](https://www.pelco.com/fs/documents/pelco-hov-aluminium-camera-housing-specification-sheet-en.pdf)
- [Axis T93F10 product and photos](https://www.axis.com/products/axis-t93f10-outdoor-housing)
- [Axis T93F10 specification](https://www.axis.com/dam/public/4d/bc/42/datasheet-axis-t93f10-outdoor-housing-en-US-524773.pdf)
- [Axis internal dimension resources](https://www.axis.com/products/axis-t93f10-outdoor-housing/support)
- [Videotec PUNTO legacy specification](https://legacy.videotec.com/dep/ekr/Schede_Tecniche/EN/PUNTO_Datasheet.pdf)
- [Mingxin 14 PK supplier / MOQ](https://www.nbmxdq.com/Industrial-camera-enclosures/aluminum-bullet-camera-housing-14-pk-assembly-for-industrial-vision-systems)

The old A0 RFQ ZIP, case model and BOM are superseded concept records. They
should not be sent as the desired enclosure design or ordered. New supplier
selection and the 4K60 sensor/interface change require a new drawing/BOM revision.
