#!/usr/bin/env bash
# Bounded integration test against an already-running isolated Inventor.
set -euo pipefail
research_pid=${1:?host Inventor PID required}
research_label=${2:?log label required}
research_wine=/nix/store/nhwk03hw57qgq72npd8ny0ff4pb1d40q-wine-wow64-staging-inventor-11.8/bin/wine
research_xdotool=/nix/store/nwz0i5qzplymm81n1yc82r23swbf9yj8-xdotool-4.20260303.1/bin/xdotool
research_window=$($research_xdotool search --name '^Autodesk Inventor Professional 2027$')
[[ $research_window =~ ^[0-9]+$ ]] || { echo 'Expected exactly one Inventor window' >&2; exit 2; }
for research_cycle in 1 2 3; do
  for research_helper in layout-restore layout-maximize; do
    sudo -n nsenter -t "$research_pid" -a -S 1000 -G 100 -e -w "$research_wine" 'C:\ResearchUI\'"$research_helper"'.exe' > "logs/raytracing/$research_label-$research_cycle-$research_helper.log" 2>&1
    sleep 2
  done
  /nix/store/87sb6kd14i3fmkwfignqfvp4sp2nv0mp-xwininfo-1.1.6/bin/xwininfo -id "$research_window" -shape > "logs/raytracing/$research_label-$research_cycle-shape.log"
  /run/current-system/sw/bin/spectacle -b -n -f -o "$PWD/logs/raytracing/$research_label-$research_cycle-desktop.png" > /dev/null 2>&1
 done
sudo -n nsenter -t "$research_pid" -a -S 1000 -G 100 -e -w "$research_wine" 'C:\ResearchUI\view-status.exe' > "logs/raytracing/$research_label-status.log" 2>&1 || true
rg 'Width:|Height:|shape' "logs/raytracing/$research_label-"*-shape.log
