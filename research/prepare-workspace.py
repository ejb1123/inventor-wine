"""Link published historical sources into ignored fixture paths, without overwriting."""
from pathlib import Path
import json
import os

root = Path(__file__).resolve().parent.parent
manifest = json.loads((root / 'research/source-manifest.json').read_text())
for entry in manifest:
    source = root / entry['path']
    target = root / entry['original_path']
    if target.exists() or target.is_symlink():
        print(f'Keeping existing {target.relative_to(root)}')
        continue
    target.parent.mkdir(parents=True, exist_ok=True)
    target.symlink_to(os.path.relpath(source, target.parent))
    print(f'Linked {target.relative_to(root)}')
