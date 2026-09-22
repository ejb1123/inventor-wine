#!/usr/bin/env bash
set -euo pipefail
trap '"$WINESERVER" -k || true' EXIT
"$WINESERVER" -p60
wineboot -u > /work/wineboot.log 2>&1
cd /work
export WINEDLLOVERRIDES='d3d12,d3d12core,dxgi=n;winewayland.drv=d;mscoree,mshtml=d'
wine ./probe.exe > probe.log 2>&1
