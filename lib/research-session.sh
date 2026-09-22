#!/usr/bin/env bash
set -euo pipefail
research_cleanup() {
  local status=$?
  printf '%s\n' "$status" > /research-logs/session-exit-status.txt
  for artifact in corehost.log dotnet-host.log Inventor_dxgi.log Inventor_d3d11.log webview-debug.log raytracing-compat.log; do
    if [[ -f "$WINEPREFIX/drive_c/ResearchLicense/$artifact" ]]; then
      cp "$WINEPREFIX/drive_c/ResearchLicense/$artifact" "/research-logs/$artifact" || true
    fi
  done
  "$WINESERVER" -k 2>/dev/null || true
}
trap research_cleanup EXIT
export DOTNET_ROOT='C:\Program Files\dotnet'
export WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS='--disable-gpu'
research_raytracing_debug=0
if [[ "${1:-}" == --debug-raytracing ]]; then
  research_raytracing_debug=1
  shift
fi
if [[ "${1:-}" == --debug-home ]]; then
  shift
  export WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS='--disable-gpu --remote-debugging-port=9222 --enable-logging --v=1 --log-file=C:\ResearchLicense\webview-debug.log'
fi
export COREHOST_TRACE=1 DOTNET_HOST_TRACE=1
export COREHOST_TRACEFILE='C:\ResearchLicense\corehost.log'
export DOTNET_HOST_TRACEFILE='C:\ResearchLicense\dotnet-host.log'
"$RESEARCH_IP" addr show > /research-logs/network.txt
"$RESEARCH_IP" route show >> /research-logs/network.txt
"$WINESERVER" -p60 > /research-logs/wineserver.log 2>&1
# WebView2's newer-Windows composition path renders blank under Wine.
# Restrict the compatibility version to the browser; Inventor stays on Windows 10.
wine reg add 'HKCU\Software\Wine\AppDefaults\msedgewebview2.exe' /v Version /t REG_SZ /d win8 /f > /research-logs/webview-compat.txt 2>&1
if [[ "${1:-}" == --configure-dialogs ]]; then
  wine 'C:\ResearchUI\configure.exe' > /research-logs/dialog-style.log 2>&1
  exit 0
fi
for graphics_dll in d3d12 d3d12core dxgi d3d11; do
  wine reg add 'HKCU\Software\Wine\AppDefaults\Inventor.exe\DllOverrides' /v "$graphics_dll" /t REG_SZ /d native,builtin /f >> /research-logs/override.txt 2>&1
done
export DXVK_LOG_PATH='C:\ResearchLicense'
# Keep the actual NVIDIA adapter identity available to Inventor's capability checks.
export DXVK_CONFIG='dxgi.hideNvidiaGpu = False'
wine 'C:\Program Files\dotnet\dotnet.exe' --info > /research-logs/dotnet-info.txt 2>&1 || true
cd "$WINEPREFIX/drive_c/ResearchLicense"
if (( research_raytracing_debug )); then
  wine 'C:\ResearchUI\raytracing-loader.exe' > /research-logs/raytracing-loader.log 2>&1 &
fi
if [[ "${1:-}" == --shortcut ]]; then
  if [[ "${2:-}" == access ]]; then
    shortcut="$WINEPREFIX/drive_c/ProgramData/Microsoft/Windows/Start Menu/Programs/Autodesk/Autodesk Access/Autodesk Access.lnk"
  else
    shortcut="$WINEPREFIX/drive_c/users/$USER/AppData/Roaming/Microsoft/Windows/Start Menu/Programs/Autodesk Inventor 2027/${2:?Missing shortcut name}"
  fi
  [[ -f "$shortcut" ]] || { echo "Shortcut not found: $shortcut" >&2; exit 2; }
  wine start /wait /unix "$shortcut" > /research-logs/application.log 2>&1
  exit 0
fi
wine 'C:\Program Files\Autodesk\Inventor 2027\Bin\Inventor.exe' /language=ENU "$@" > /research-logs/inventor.log 2>&1
