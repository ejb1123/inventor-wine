#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh

# Keep the baseline useful without enabling the extremely noisy module and SEH
# trace channels. Focused traces can be enabled after a failing component is
# isolated.
obs_set_trace_mode
obs_init bootstrap "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
log="$OBS_RUN_DIR/wine.log"
nix --extra-experimental-features 'nix-command flakes' develop --command \
  wine "$INVENTOR_BASE_INSTALLER" 2>&1 | tee "$log"
