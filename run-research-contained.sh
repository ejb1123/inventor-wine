#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
research_project="$PWD"
research_bwrap="$research_project/fixtures/desktop-tools/bwrap"
if [[ ! -x "$research_bwrap" ]]; then
  research_bwrap=$(command -v bwrap) || {
    echo 'Bubblewrap is missing; restore fixtures/desktop-tools/bwrap before launching.' >&2
    exit 1
  }
fi
research_prefix="${XDG_DATA_HOME:-$HOME/.local/share}/wineprefixes/inventor-2027-research"
[[ -d "$research_prefix" ]] || { echo "Research prefix is missing." >&2; exit 1; }
research_logs="$research_project/logs/research-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$research_logs"
for research_folder in Desktop Documents Downloads Music Pictures Videos; do
  mkdir -p "$research_prefix/research-home/$research_folder"
done
research_wine=$(readlink -f result)
research_server=$(readlink -f result-server)
research_bash=$(readlink -f "$(command -v bash)")
research_core=$(dirname -- "$(readlink -f "$(command -v cat)")")
research_ip=$(readlink -f "$(command -v ip)")
research_session="$research_project/lib/research-session.sh"
if [[ -f "$research_project/confidential/lib/research-session.sh" ]]; then
  research_session="$research_project/confidential/lib/research-session.sh"
fi
args=(--unshare-all --die-with-parent --new-session --clearenv
  --ro-bind /nix/store /nix/store --ro-bind /etc /etc --ro-bind /sys /sys --proc /proc --dev /dev
  --tmpfs /tmp --ro-bind /tmp/.X11-unix /tmp/.X11-unix
  --dir "$HOME" --dir "$HOME/Desktop" --dir "$HOME/Documents"
  --dir "$HOME/Downloads" --dir "$HOME/.cache" --dir "$HOME/.wineserver"
  --bind "$research_prefix" "$research_prefix" --bind "$research_logs" /research-logs
  --ro-bind "$research_session" /session.sh
  --ro-bind /run/opengl-driver /run/opengl-driver
  --setenv HOME "$HOME" --setenv USER "$USER" --setenv LOGNAME "$USER"
  --setenv PATH "$research_wine/bin:$research_server/bin:$research_core:$(dirname "$research_bash")"
  --setenv WINEPREFIX "$research_prefix" --setenv WINEARCH win64
  --setenv WINESERVER "$research_server/bin/wineserver"
  --setenv WINE_SERVICE_SESSION_ZERO 1
  --setenv WINEDEBUG '-all,+timestamp,+pid,+tid,err+all'
  --setenv WINEDLLOVERRIDES 'winewayland.drv=d;msxml6=n,b'
  --setenv DISPLAY "${DISPLAY:-:0}" --setenv LC_ALL C.UTF-8
  --setenv RESEARCH_IP "$research_ip")
# Built against the same Wine derivation; fixes stale X11 shapes after resize.
research_x11="$research_project/result-winex11/lib/wine/x86_64-unix/winex11.so"
if [[ -f "$research_x11" ]]; then
  args+=(--ro-bind "$research_x11" "$research_wine/lib/wine/x86_64-unix/winex11.so")
  readlink -f "$research_x11" > "$research_logs/x11-driver.txt"
fi
for research_folder in Desktop Documents Downloads Music Pictures Videos; do
  args+=(--bind "$research_prefix/research-home/$research_folder" "$HOME/$research_folder")
done
if [[ -n "${XAUTHORITY:-}" ]]; then
  args+=(--ro-bind "$XAUTHORITY" "$XAUTHORITY" --setenv XAUTHORITY "$XAUTHORITY")
fi
for research_device in /dev/dri /dev/nvidia0 /dev/nvidiactl /dev/nvidia-modeset /dev/nvidia-uvm /dev/nvidia-uvm-tools; do
  [[ ! -e "$research_device" ]] || args+=(--dev-bind "$research_device" "$research_device")
done
echo "Research logs: $research_logs"
exec flock --nonblock --conflict-exit-code 73 "$research_prefix/.research-launch.lock" \
  "$research_bwrap" "${args[@]}" -- "$research_bash" /session.sh "$@" > "$research_logs/console.log" 2>&1
