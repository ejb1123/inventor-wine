#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/../.."
project=$PWD
wine=$(readlink -f result)
server=$(readlink -f result-server)
bash=$(readlink -f "$(command -v bash)")
core=$(dirname "$(readlink -f "$(command -v cat)")")
mkdir -p fixtures/dx12-research/prefix fixtures/dx12-research/home
args=(--unshare-all --die-with-parent --new-session --clearenv
 --ro-bind /nix/store /nix/store --ro-bind /etc /etc --ro-bind /sys /sys --proc /proc --dev /dev --tmpfs /tmp
 --ro-bind /run/opengl-driver /run/opengl-driver --ro-bind /tmp/.X11-unix /tmp/.X11-unix
 --bind "$project/fixtures/dx12-research" /work --bind "$project/fixtures/dx12-research/home" /home/probe
 --setenv HOME /home/probe --setenv USER probe --setenv WINEPREFIX /work/prefix
 --setenv WINESERVER "$server/bin/wineserver" --setenv PATH "$wine/bin:$core:$(dirname "$bash")"
 --setenv DISPLAY "${DISPLAY:-:0}" --setenv WINEDEBUG '-all,err+all' --setenv WINEDLLOVERRIDES 'winewayland.drv=d;mscoree,mshtml=d'
 --setenv VK_DRIVER_FILES /run/opengl-driver/share/vulkan/icd.d/nvidia_icd.json
 --setenv DXVK_FILTER_DEVICE_NAME 'NVIDIA GeForce RTX 3080' --setenv VKD3D_FILTER_DEVICE_NAME 'NVIDIA GeForce RTX 3080'
 --setenv VKD3D_DEBUG info --setenv DXVK_LOG_LEVEL info --setenv WINE_SERVICE_SESSION_ZERO 1)
if [[ -n "${XAUTHORITY:-}" ]]; then
 args+=(--ro-bind "$XAUTHORITY" "$XAUTHORITY" --setenv XAUTHORITY "$XAUTHORITY")
fi
for d in /dev/dri /dev/nvidia0 /dev/nvidiactl /dev/nvidia-modeset /dev/nvidia-uvm /dev/nvidia-uvm-tools; do
 [[ ! -e "$d" ]] || args+=(--dev-bind "$d" "$d")
done
exec bwrap "${args[@]}" "$bash" /work/probe-session.sh
