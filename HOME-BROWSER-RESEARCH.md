# Inventor Home browser investigation — 2026-09-22

The blank Home panel is a display/presentation failure in WebView2's newer-Windows
compatibility path under Wine. Setting **only `msedgewebview2.exe` to `win8`**
fixes both the standalone native WebView2 test and Inventor Home. Keep `--disable-gpu`; Inventor itself
retains its Windows 10 setting. `lib/research-session.sh` now applies this
per-executable setting before launch. See verification below.

Verified in a normal launch (no debugger or injected probes), run
`logs/research-20260922-045821`: the Inventor window displays Home's recent-file
list, project selector, Open/New buttons and help links. Screenshot:
`logs/home-debug/inventor-home-fixed.png`. License checkout also succeeded.
Clicking the rendered Home **New** button opened Inventor's Create New File
dialog (`logs/home-debug/home-new-dialog.png`); the test dialog was dismissed
without creating a document.

## Evidence

- WebView2 103.0.1264.77 launches its browser and renderer processes.
- The Home page comes from local `C:\ProgramData\Autodesk\Inventor 2027\WebBrowser\index.html`,
  not a remote Autodesk website. The isolated session remains offline.
- Browser diagnostics report `document.readyState = complete`, a visible document,
  and a 2560×1200 viewport. The DOM contains Open, New, and the recent-file list.
- Console messages include `initializationComplete()`, `inventorIdle()`, and
  recent-view rendering completion. Communication with Inventor is functioning.
- A DevTools `Page.captureScreenshot` shows the populated Home page, while a
  simultaneous capture of the Inventor window shows an empty panel.
- Win32 enumeration finds the browser child windows visible and correctly sized.
  The baseline final `Intermediate D3D Window` has layered/transparent extended
  styles and belongs to the browser's GPU process.

This rules out a missing page, wholly failed JavaScript startup, a zero-sized
browser control, or network access being required for the local Home contents.
It narrows the fault to presenting/compositing the browser into its host window;
it does not yet prove a particular graphics API or source-code defect.

## Captures and diagnostics

- `logs/home-debug/browser-before.png`: browser-rendered, populated Home page.
- `logs/home-debug/window.png`: blank Inventor host window in the same test.
- `logs/home-debug/dom-baseline.json`: URL, state, page text, size, visibility.
- `logs/home-debug/windows.txt`: native child-window hierarchy and styles.
- `logs/home-debug/webview-before.log`: Chromium startup and page console output.
- `fixtures/home-debug/cdp_probe.py`: local DevTools inspection helper.
- `fixtures/home-debug/windows.c`: native window-enumeration helper.

The debugger uses port 9222 inside the private network namespace, accessed with
`nsenter -n`. It is not exposed to the host network. Diagnostic mode is explicitly
selected with `./run-kde-app.sh --debug-home`.

## Tests

- Baseline `--disable-gpu`: page renders internally; embedded panel remains blank.
- Adding `--in-process-gpu`: panel becomes black, still no visible Home UI.
- ANGLE OpenGL plus `--disable-direct-composition`: still blank, with a GPU
  command-buffer creation failure and EGL/pixel-format warnings.
- ANGLE D3D11 with app-local DXVK 3.1.1 plus `--disable-direct-composition`:
  DXVK created a feature-level 11_1 device on the RTX 3080, but Home remained
  blank. Chromium logged a transient GPU command-buffer failure. See
  `logs/home-debug/dxvk.png`, `webview-dxvk.log`, and run
  `logs/research-20260922-043104`.

Experimental browser flags, DXVK DLLs beside WebView2, and its per-app native DLL
overrides were removed. Normal launches keep `--disable-gpu` plus the confirmed
browser-only `win8` compatibility setting. Debugging is opt-in; normal launches
do not open the debug port.

## Follow-up investigation and workaround

- Reparenting the browser, temporarily clearing layered-window styles, disabling
  occlusion throttling, and disabling SwiftShader did not fix presentation.
- A Chromium frame trace captured 29 `SoftwareRenderer::SwapBuffers` events
  during a short animation despite the blank window. The compositor was active.
- Temporary in-process import probes were diagnostic only and were unloaded on
  restart. They are not part of the launcher.
- A minimal native C WebView2 host with a fresh profile and plain HTML reproduced
  the black screen independently of Inventor: `fixtures/home-debug/minimal.c`.
  SDK: Microsoft.Web.WebView2 1.0.1245.22 from Microsoft's NuGet package.
- With the same runtime and test page, changing only the browser's Wine version
  to Windows 8 restored visible content: `logs/home-debug/minimal-win8.png`.
- This matches Wine bugs 59370 and 58921. The practical cause is the version-
  selected presentation path. The exact Wine source defect is not independently
  established by these experiments.

The override is at
`HKCU\Software\Wine\AppDefaults\msedgewebview2.exe`, string `Version=win8`.
Rollback requires removing that value and its launcher command, then restarting
the application. The pre-test query is `logs/home-debug/browser-version-before.log`.

Restart checks refuse to quit with dirty documents. The first restart found a
saved Part1; subsequent tests had no dirty documents.

## References

- [Microsoft WebView2 debugging arguments](https://learn.microsoft.com/en-us/microsoft-edge/webview2/how-to/debug-visual-studio-code)
- [Chromium DirectComposition switch definition](https://chromium.googlesource.com/chromium/src/+/49144fdaf4673478c0fdcc3da926ec246fb1ccb5/ui/gl/gl_switches.cc)
- [Wine bug 59370 and maintainer workaround](https://list.winehq.org/hyperkitty/list/wine-bugs%40list.winehq.org/thread/UDZKZTMP5WYISOAUDGEOMFOSU7X73JND/)
- [Wine bug 58921, newer-Windows version failure](https://list.winehq.org/hyperkitty/list/wine-bugs%40list.winehq.org/message/N5MS6ABMYAAG7XZRU62F7XIF4Z4GK3VX/)
