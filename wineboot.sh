#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
exec nix --extra-experimental-features 'nix-command flakes' develop --command wineboot --init
