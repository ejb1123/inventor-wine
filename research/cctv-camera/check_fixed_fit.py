"""Necessary envelope checks only; external dimensions are NOT cavity dimensions."""
import json
from pathlib import Path

OUT = Path(__file__).parent / 'a1-hasion'
BODY_L, BODY_W = 107.5, 75.2
SOM_L, SOM_W = 68.0, 50.0
MODEM_L, MODEM_W = 52.0, 30.0
GAP = 3.0  # Proposed screening clearance, not a supplier requirement.
cases = [
    ('som_only', SOM_L, SOM_W),
    ('side_by_side_same_orientation', max(SOM_L, MODEM_L), SOM_W + MODEM_W),
    ('end_to_end_same_orientation', SOM_L + MODEM_L, max(SOM_W, MODEM_W)),
    ('stacked_bare_boards_plan_only', max(SOM_L, MODEM_L), max(SOM_W, MODEM_W)),
    ('stacked_with_assumed_3mm_perimeter', SOM_L + 2*GAP, SOM_W + 2*GAP),
]
rows = []
for name, length, width in cases:
    rows.append(dict(layout=name, length_mm=length, width_mm=width,
                     external_length_margin_mm=round(BODY_L-length, 2),
                     external_width_margin_mm=round(BODY_W-width, 2),
                     within_external_rectangle=length <= BODY_L and width <= BODY_W,
                     fit_status='UNVERIFIED' if length <= BODY_L and width <= BODY_W
                     else 'REJECT_THIS_ORIENTATION'))
result = dict(status='FIT_NOT_ESTABLISHED',
              method='2D necessary-condition screening, no enclosure CAD or collision test',
              housing_external_reference_mm=dict(length=BODY_L, width=BODY_W),
              assumed_perimeter_mm=GAP, layouts=rows,
              unknowns=['internal cross-sections', 'wall/boss intrusion', 'stack height',
                        'carrier and power circuitry', 'lens and optical axis',
                        'heatsink', 'sockets and cable bends', 'RF keepouts'])
OUT.mkdir(exist_ok=True)
(OUT/'fixed-fit-check.json').write_text(json.dumps(result, indent=2)+'\n')

# Authored schematic; no reproduction of supplier artwork.
svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="1100" height="550" viewBox="0 0 1100 550">',
       '<rect width="1100" height="550" fill="#f8fafc"/>',
       '<g font-family="sans-serif" fill="#152238">',
       '<text x="30" y="35" font-size="24">X6Z7 fixed-camera envelope screening</text>',
       '<text x="30" y="65" font-size="16">Dashed boundary: 107.5 × 75.2 mm EXTERNAL reference — not usable interior.</text>']
scale=3.1
for ox, title, stacked in [(35,'Flat side-by-side',False),(600,'Stacked: plan view only',True)]:
    oy=115
    svg.append(f'<text x="{ox}" y="98" font-size="18">{title}</text>')
    svg.append(f'<rect x="{ox}" y="{oy}" width="{BODY_L*scale}" height="{BODY_W*scale}" fill="none" stroke="#64748b" stroke-width="2" stroke-dasharray="7 5"/>')
    svg.append(f'<rect x="{ox}" y="{oy}" width="{SOM_L*scale}" height="{SOM_W*scale}" fill="#93c5fd" fill-opacity="0.7" stroke="#1d4ed8"/>')
    my=oy if stacked else oy+SOM_W*scale
    svg.append(f'<rect x="{ox}" y="{my}" width="{MODEM_L*scale}" height="{MODEM_W*scale}" fill="#fbbf24" fill-opacity="0.5" stroke="#b45309"/>')
    svg.append(f'<text x="{ox}" y="390" font-size="16">SoM: 68 × 50 mm; modem: 52 × 30 mm</text>')
    caption='Height, carrier, optics and cooling unknown.' if stacked else '80 mm combined width exceeds 75.2 mm exterior.'
    svg.append(f'<text x="{ox}" y="418" font-size="16">{caption}</text>')
svg += ['<text x="30" y="475" font-size="17" font-weight="bold">FIT NOT ESTABLISHED. A rectangle inside this boundary is not a fit approval.</text>',
        '<text x="30" y="505" font-size="15">Boards shown bare, without sockets, carrier, lens, power stage, heat spreaders or cable clearance.</text>', '</g></svg>']
(OUT/'fixed-fit-screen.svg').write_text('\n'.join(svg)+'\n')
print(json.dumps(result, indent=2))
