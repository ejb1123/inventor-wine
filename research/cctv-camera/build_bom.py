"""Engineering allowances, not distributor quotes or a released purchasing BOM."""
import csv,json
from pathlib import Path
P=Path(__file__).parent
rows=[
('E01',1,'Camera + bundled 16 mm lens','Arducam','B0477','CANDIDATE: 4K20; outdoor temperature unknown',150,250,'https://www.arducam.com/downloads/datasheet/B0477_20MP_IMX283_USB3.0_Camera_Datasheet.pdf','Sony IMX283 Type 1; 34x34 mm PCB; mechanical model reserves larger envelope. Lens FOV/IR-cut not frozen.'),
('E02',1,'Processor SOM, RAM and eMMC','Firefly','Core-3588J, memory/temperature SKU TBD','CANDIDATE: vendor pinout and SDK review needed',120,260,'https://www.t-firefly.com/products/core-3588j-8k-ai-core-board-rockchip-rk3588-new-gen-8-core-64-bit-processor','82x53 mm; hardware encode candidate; no end-to-end 4K stream test.'),
('E03',1,'SOM carrier socket','TBD','MXM3 314-pin matching vendor drawing','TBD',10,25,'','Do not buy before supplier reference design confirms key, height and mating part.'),
('E04',1,'Wi-Fi/Bluetooth module','Ezurio','453-00049','CANDIDATE',20,45,'https://www.ezurio.com/documentation/datasheet-sterling-lwb5-m2','USB/USB variant; M.2 interface must be wired for this variant; Linux driver integration pending.'),
('E05',1,'M.2 socket + retention hardware','TBD','Matching 2230 E-key implementation','TBD',3,8,'','Confirm key and USB pin mapping with Ezurio.'),
('E06',1,'Dual-band outdoor antenna + bulkhead + coax','TBD','Approved antenna from radio vendor list','TBD',12,35,'','Single shared RF path candidate; final antenna and ground-plane testing required.'),
('E07',1,'Isolated PoE+ PD converter 12 V','Silvertel','Ag5300','CANDIDATE',20,40,'https://silvertel.com/ag5300/','24 W continuous nominal; 30 W peak. Candidate cannot cover unrestricted SOM max plus other loads; revise PD or verify hard power limit.'),
('E08',2,'PoE input bridge rectifiers','TBD','Rated for 802.3at input and thermal load','TBD',1,3,'','Include input TVS and surge coordination; no polarity wiring released.'),
('E09',1,'Gigabit Ethernet PHY','TBD','Vendor-supported RGMII PHY','TBD - check SOM onboard PHY first',4,12,'','Do not double populate if chosen SOM already includes PHY.'),
('E10',1,'Gigabit PoE magnetics + internal RJ45','TBD','802.3at center-tap arrangement','TBD',6,18,'','Ethernet enters through cable gland and terminates inside; shield/chassis bonding review needed.'),
('E11',1,'USB-C sink PD controller','STMicroelectronics','STUSB4500QTR','CANDIDATE',3,7,'https://www.st.com/en/interfaces-and-transceivers/stusb4500.html','Request 15 V supply; 5 V attach state cannot enable full camera. External switching/protection required.'),
('E12',1,'Input priority power mux','Texas Instruments','TPS2121','CANDIDATE',3,7,'https://www.ti.com/product/TPS2121','12 V PoE versus protected 15 V USB; prioritize PoE; validate backfeed in all unpowered cases.'),
('E13',2,'Separate SOM 4 V and camera 5 V buck modules','Texas Instruments','TPSM63606','CANDIDATE',8,18,'https://www.ti.com/product/TPSM63606','One regulator per rail: SOM 4.0 V +/-5%, camera 5 V. No shared 5 V SOM rail. Peak budget unresolved.'),
('E14',1,'3.3 V auxiliary regulator','TBD','Sized after SOM/radio interface review','TBD',1,4,'','1.8 V only if selected interfaces require it; sequencing TBD.'),
('E15',1,'Protection and power switching set','TBD','eFuse/FETs, CC/VBUS/data TVS, common mode parts','TBD',8,25,'','Channel counts and values pending electrical schematic.'),
('E16',1,'USB-C service receptacle and capped panel system','TBD','Sealed assembly, USB2 data + PD','TBD',8,30,'','Model cutout is only reservation; sealing applies with cap fitted.'),
('E17',1,'Internal locking USB3 camera harness','TBD','5 Gbit/s qualified harness','TBD',8,20,'','Separate internal camera host port from service USB2 device port.'),
('E18',1,'Carrier PCB bare fabrication allowance','Custom','CCTV-PCB-A0 140x86 mm; 7 copper requested','NOT RELEASED',35,100,'','Fabricator must approve odd stack; no routed schematic/layout or Gerbers yet.'),
('E19',1,'PCB passives/connectors/test points set','TBD','Values and count pending schematic','TBD',15,45,'','This is a cost reserve, not a component-level BOM.'),
('E20',1,'PCBA setup and assembly allocation','Service','Prototype small-batch allowance','ESTIMATE',60,180,'','Per-unit allowance is quantity dependent; no quote obtained.'),
('M01',1,'Main body','Custom','CCTV-01 aluminum 6061-T6 concept','CONCEPT',80,220,'','Machining plus corrosion-resistant finish; inspect seal lands after finishing.'),
('M02',1,'Front plate','Custom','CCTV-02 aluminum 6061-T6','CONCEPT',25,70,'','Lens window seat, gasket and retention still require tolerance closure.'),
('M03',1,'Rear plate','Custom','CCTV-03 aluminum 6061-T6','CONCEPT',25,70,'','Port cutouts cannot be machined until connector selection.'),
('M04',1,'Sunshield','Custom','CCTV-09 formed aluminum 5052-H32 2 mm','CONCEPT',15,45,'','Light finish; 8 mm air gap; bend radii and developed blank not released.'),
('M05',2,'End gasket','Custom elastomer supplier','Molded rectangular silicone 2 mm free thickness','CONCEPT',5,18,'','25% nominal squeeze; supplier compound and dimensional tolerances pending.'),
('M06',1,'Optical window','Custom optics supplier','64 mm diameter x 3 mm, AR coating TBD','CONCEPT',15,55,'','Clear aperture 56 mm; flatness, coating, impact strength and ghosting tests pending.'),
('M07',1,'Window gasket + retainer set','Custom','CCTV-07/08','CONCEPT',8,25,'','Retainer fixing and compression stops not detailed.'),
('M08',1,'Pressure-equalization membrane vent','Gore candidate','Exact PolyVent variant TBD','TBD',4,15,'https://www.gore.com/categories/vents/outdoor-electronics','Needs water/airflow/temperature selection; does not alone qualify enclosure.'),
('M09',1,'Ethernet cable gland','TBD','IP66-or-better part for selected cable OD','TBD',4,14,'','Model uses 20 mm reservation; confirm thread, panel seal and cable range.'),
('M10',1,'Adjustable mounting bracket + base','Custom','CCTV-21/22/23 concept','CONCEPT',20,70,'','Current model is envelope only; pivots, serrations and lock geometry not detailed.'),
('M11',1,'Hardware and isolation washers set','TBD','Stainless fasteners, isolating washers, spacers','TBD',10,30,'','Coated aluminum/stainless galvanic pairing; all lengths and torques pending.'),
('M12',1,'Thermal interfaces and chassis bridge','TBD','Pad + conduction bridge to body','TBD',8,25,'','Model shows spreader reservation only: thermal route NOT complete.'),
('M13',1,'Labels, tether, seals, cable retention set','Custom','Service and assembly consumables','TBD',5,15,'','Ingress-critical cap must remain tethered; serial and MAC labels.'),
]
fields=['item','quantity','description','manufacturer','candidate_mpn','status','unit_allowance_low_USD','unit_allowance_high_USD','source_url','notes']
with (P/'bom.csv').open('w',newline='') as f:
 w=csv.writer(f);w.writerow(fields);w.writerows(rows)
totals={key:sum(r[1]*r[idx] for r in rows) for key,idx in [('low_USD',6),('high_USD',7)]}
totals.update({'basis':'unquoted engineering allowances for one small-batch prototype; not current selling prices','excluded':'tax, shipping, engineering labor, qualification testing, tooling/NRE beyond listed allowances, PoE injector/switch, USB PD supply, optional IR/heater/microSD','procurement_release':False})
(P/'outputs'/'bom-budget.json').write_text(json.dumps(totals,indent=2)+'\n')
print(totals)
