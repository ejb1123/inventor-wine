"""Check aggregate lowering using self-authored matrix cases (no Autodesk input)."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--assembler', required=True, type=Path)
parser.add_argument('--baseline', required=True, type=Path)
parser.add_argument('--patched', required=True, type=Path)
parser.add_argument('--baseline-library-dir', type=Path)
parser.add_argument('--patched-library-dir', type=Path)
args = parser.parse_args()
source = (Path(__file__).parent / 'matrix-reinterpret.ll').read_text()
report = []
with tempfile.TemporaryDirectory(prefix='aggregate-lowering-') as folder:
    work = Path(folder)
    for rows, columns in [(2, 3), (3, 3), (3, 4), (4, 4)]:
        count = rows * columns
        text = source.replace('[3 x <4 x float>]', f'[{rows} x <{columns} x float>]')
        text = text.replace('<12 x float>', f'<{count} x float>')
        ll = work / f'matrix-{rows}x{columns}.ll'
        dxil = ll.with_suffix('.dxil')
        ll.write_text(text)
        subprocess.run([str(args.assembler.resolve()), str(ll), str(dxil)], check=True)
        row = {'shape': f'{rows}x{columns}'}
        for name in ('baseline', 'patched'):
            env = os.environ.copy()
            lib = getattr(args, name + '_library_dir')
            if lib:
                env['LD_LIBRARY_PATH'] = str(lib.resolve()) + (':' + env['LD_LIBRARY_PATH'] if env.get('LD_LIBRARY_PATH') else '')
            result = subprocess.run([str(getattr(args, name).resolve()), str(dxil), '--validate',
                                     '--output', str(work / (name + '.spv'))],
                                    env=env, capture_output=True, text=True)
            row[name] = result.returncode
            if name == 'baseline':
                assert result.returncode != 0 and 'Bitcast' in result.stderr, result.stderr
            else:
                assert result.returncode == 0, result.stderr
        report.append(row)
    # A load that reinterprets an eight-lane allocation as twelve scalar lanes
    # must fail explicitly rather than generating an invalid aggregate cast.
    ll = work / 'unequal-lanes.ll'
    ll.write_text(source.replace('<12 x float>', '<8 x float>'))
    dxil = ll.with_suffix('.dxil')
    subprocess.run([str(args.assembler.resolve()), str(ll), str(dxil)], check=True)
    env = os.environ.copy()
    if args.patched_library_dir:
        env['LD_LIBRARY_PATH'] = str(args.patched_library_dir.resolve())
    result = subprocess.run([str(args.patched.resolve()), str(dxil), '--validate',
                             '--output', str(work / 'unequal.spv')],
                            env=env, capture_output=True, text=True)
    assert result.returncode != 0 and 'equal scalar lane counts' in result.stderr, result.stderr
    report.append({'shape': '8-to-12 lanes', 'patched_rejects': True})
print(json.dumps(report, indent=2))
