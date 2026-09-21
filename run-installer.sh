#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh

timestamp="$(date +%Y%m%d-%H%M%S)"
log="$PROJECT_DIR/logs/bootstrap-$timestamp.log"

# Keep the baseline useful without enabling the extremely noisy module and SEH
# trace channels. Focused traces can be enabled after a failing component is
# isolated.
export WINEDEBUG="+timestamp,+pid,+tid,warn+all,err+all,fixme+all"
nix --extra-experimental-features 'nix-command flakes' develop --command \
  wine "$INVENTOR_BASE_INSTALLER" 2>&1 | tee "$log"
