#!/usr/bin/env bash
set -euo pipefail

cd /home/ej/Projects/inventor-wine
source ./env.sh
exec nix-shell ./shell.nix --run 'wineboot --init'
