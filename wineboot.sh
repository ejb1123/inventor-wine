#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh
obs_set_trace_mode
obs_init wineboot "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
nix --extra-experimental-features 'nix-command flakes' develop --command wineboot --init \
  2>&1 | tee "$OBS_RUN_DIR/wine.log"
