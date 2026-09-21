#!/usr/bin/env bash
set -euo pipefail

cd /home/ej/Projects/inventor-wine
source ./env.sh

timestamp="$(date +%Y%m%d-%H%M%S)"
log="/home/ej/Projects/inventor-wine/logs/bootstrap-$timestamp.log"

# Keep the baseline useful without enabling the extremely noisy module and SEH
# trace channels. Focused traces can be enabled after a failing component is
# isolated.
export WINEDEBUG="+timestamp,+pid,+tid,warn+all,err+all,fixme+all"
nix-shell ./shell.nix --run 'wine "$INVENTOR_BASE_INSTALLER"' 2>&1 | tee "$log"
