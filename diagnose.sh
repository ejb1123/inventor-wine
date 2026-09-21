#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh

obs_init diagnose "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

{
  printf '## CPU\n'
  lscpu 2>&1 || true
  printf '\n## Memory\n'
  free -h 2>&1 || true
  printf '\n## PCI display devices\n'
  lspci -nnk 2>&1 | grep -A4 -Ei 'vga|3d|display' || true
  printf '\n## Vulkan\n'
  timeout 30s vulkaninfo --summary 2>&1 || true
  printf '\n## Processes\n'
  ps -ef 2>&1 || true
} >"$OBS_RUN_DIR/system-diagnostics.txt"

nix --extra-experimental-features 'nix-command flakes' flake metadata --json \
  >"$OBS_RUN_DIR/flake-metadata.json" 2>"$OBS_RUN_DIR/flake-metadata.err" || true
nix --extra-experimental-features 'nix-command flakes' eval --raw .#wine.outPath \
  >"$OBS_RUN_DIR/wine-store-path.txt" 2>&1 || true
