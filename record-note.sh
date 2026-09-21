#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh

run_dir=${OBS_RUN_DIR:-}
if [[ -z "$run_dir" && -L "$PROJECT_DIR/logs/latest" ]]; then
  run_dir=$(readlink -f -- "$PROJECT_DIR/logs/latest")
fi
if [[ -z "$run_dir" || ! -d "$run_dir" ]]; then
  printf 'No recorded run found. Start an installer or run ./diagnose.sh first.\n' >&2
  exit 1
fi
if (($#)); then note="$*"; else IFS= read -r note; fi
printf '%s\t%s\n' "$(date --iso-8601=seconds)" "$note" >>"$run_dir/notes.tsv"
printf 'Note recorded in %s\n' "$run_dir"

