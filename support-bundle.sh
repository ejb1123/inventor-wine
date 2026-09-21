#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh

run_dir=${1:-}
if [[ -z "$run_dir" && -L "$PROJECT_DIR/logs/latest" ]]; then
  run_dir=$(readlink -f -- "$PROJECT_DIR/logs/latest")
fi
if [[ -z "$run_dir" || ! -d "$run_dir" ]]; then
  printf 'Usage: %s [logs/runs/RUN_ID]\n' "$0" >&2
  exit 2
fi

bundle_root=$(mktemp -d)
trap 'rm -rf -- "$bundle_root"' EXIT
bundle_name="inventor-wine-$(basename -- "$run_dir")-support"
mkdir -p "$bundle_root/$bundle_name"

# Only include text diagnostics. Raw prefixes, installers, registry hives,
# binaries, dumps, and strace payload buffers are intentionally excluded.
find "$run_dir" -type f \
  ! -name '*.dmp' ! -name '*.reg' ! -name 'strace*' ! -name 'file-index.tsv' \
  -size -20M -print0 |
while IFS= read -r -d '' file; do
  rel=${file#"$run_dir"/}
  mkdir -p "$bundle_root/$bundle_name/$(dirname -- "$rel")"
  if grep -Iq . "$file" 2>/dev/null; then
    sed -E \
      -e 's#([A-Za-z]:)?[/\\](users|home)[/\\][^/\\[:space:]]+#<HOME>#gI' \
      -e 's#(token|password|passwd|secret|authorization|cookie|license[_ -]?key)([=: ]+)[^,;[:space:]]+#\1\2<REDACTED>#gI' \
      -e 's#(Bearer[[:space:]]+)[A-Za-z0-9._~+/-]+=*#\1<REDACTED>#gI' \
      "$file" >"$bundle_root/$bundle_name/$rel"
  fi
done

cat >"$bundle_root/$bundle_name/README.txt" <<'EOF'
This is an automatically sanitized diagnostic bundle. Autodesk installers,
Wine prefixes, binaries, registry hives, crash dumps, and raw strace captures
are excluded. Review the archive manually before sharing it publicly.
EOF

output="$PROJECT_DIR/logs/$bundle_name.tar.gz"
tar -C "$bundle_root" -czf "$output" "$bundle_name"
printf 'Created %s\nReview it before sharing.\n' "$output"

