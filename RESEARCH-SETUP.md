# Contained research installation

Created September 22, 2026. Original installation remains at
`~/.local/share/wineprefixes/inventor-2027`. The separate research clone is at
`~/.local/share/wineprefixes/inventor-2027-research`.

Run from this project:

```sh
./run-research-contained.sh
```

The launcher starts Inventor in a bubblewrap
namespace with only loopback networking. It mounts the research prefix and its
private Documents/Desktop folders, not the normal home directory. X11 and GPU
access are shared, so this is containment, not a complete malware sandbox.
No global hosts-file changes were made. Exit cleanup stops the contained Wine
server. Each run has a timestamped directory under `logs/research-*`.

Files saved in the research Windows Documents folder persist at
`~/.local/share/wineprefixes/inventor-2027-research/research-home/Documents`.
The normal `./run-inventor.sh` still launches the original installation.

## Application compatibility findings

Microsoft .NET 10.0.0 Core, Desktop, and ASP.NET runtimes are present and reported
by `dotnet --info`. The contained session explicitly sets `DOTNET_ROOT` and
records host tracing. Wine Mono nevertheless tries to load some managed add-ins
and fails resolving `SupportedOSPlatformAttribute` from System.Runtime 10.0.
Shared Views and Interactive Tutorial startup loading have been disabled in
the research prefix; original manifests are in `logs/research-setup/`.
Full add-in compatibility remains unverified.

## Working graphics configuration

The current configuration uses **DirectX 12 through vkd3d-proton 3.0.1 to Vulkan**.
Native `d3d12.dll`, `d3d12core.dll`, and DXVK 3.1.1 `dxgi.dll`/`d3d11.dll`
are beside research Inventor.exe with application-specific Wine overrides.

An earlier DX12 setter failed. With all documents closed, setting
`HardwareOptions.GraphicsDriverType=61449` succeeded and survived restart.
Diagnostics with the bevel gear open confirm `VirtualDeviceDx12`, Hardware
Renderer, and `UseSoftwareGraphics=0`; persisted Hardware Driver Type is `0x208`.
The gear rendered, camera changes succeeded, and a new saved copy reopened.
Evidence and limitations: `DX12-RESEARCH.md` and `logs/dx12-integration/`.
Large assemblies and long-session stability remain untested.

The sandbox now exposes `/sys` read-only for graphics-device discovery.
WebView2 runs with `--disable-gpu`, a troubleshooting option documented by
[Microsoft](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/webview-features-flags).
Home now renders its recent-file list and Open/New controls. WebView2 requires a
browser-only Wine `Version=win8` override plus `--disable-gpu`; the launcher sets
these automatically. Inventor itself retains Windows 10 compatibility. See
`HOME-BROWSER-RESEARCH.md` for diagnosis and verification.

Other recorded limitations: two earlier runs exited unexpectedly; the immediate
cause was not established. No OOM evidence was found. An experimental cube
automation helper crashed in its own COM handling; Inventor remained running.
That helper is not part of the launcher. Successful sample open/save and screenshots
are separate evidence from that failed helper. Do not infer feature completeness.

## Dialog typography

After user feedback, the research prefix uses a separate `balanced.msstyles`
theme under `C:\windows\resources\themes\research`, derived from Wine's Aero
theme with larger static DejaVu Sans fonts. Nonclient/message text uses a
14-pixel font height and ClearType smoothing. Theme control/list fonts use 10
points, since theme units differ from LOGFONT pixels. Shell-dialog and Segoe UI fallbacks
map to DejaVu Sans. The initial Noto preview was too small; a 16-pixel preview
was too large, so the final preview was reduced to 14 pixels.

Original registry snapshots are in `logs/dialog-style-backup/`. Preview evidence:
`logs/research-20260922-035159/preview.png`. No model files were modified for these
appearance changes. The dialogs retain light backgrounds; this is a typography
improvement, not a complete dark-theme replacement. Full Inventor dialog layout
was rechecked after restarting the application. The open, unsaved Part1 was saved
as `Recovered-Part1-20260922-040418.ipt` in the private Documents folder before
normal application shutdown, then reopened.

## KDE menu shortcuts

All 14 existing Autodesk desktop entries now call `run-kde-app.sh`, which uses
the configured research runtime. Wine's generated entries previously selected
the original prefix and the system Wine binary. Originals are backed up under
`logs/kde-shortcut-backup/`; `lib/install-kde-shortcuts.py` reapplies the changes.
KDE's service cache was refreshed and all entries passed desktop-file validation.
Inventor successfully launched through KDE's `kioclient exec` on its actual
desktop entry (`logs/research-20260922-035756/`).

Companion tools use their Windows shortcuts in the same configured prefix;
their individual functionality has not all been tested. The offline container
does not support Autodesk Access online/update functions. One standalone app or
appearance preview may run at a time; conflicting launches show a notification.
Desktop-launcher logs are saved under `logs/kde/`.

## Mouse zoom preference

Enabled Inventor's `DisplayOptions.ReverseZoomDirection` on 2026-09-22,
changing it from false to true and reading it back successfully. This reverses
Inventor zoom to match the requested wheel-down zoom-in / wheel-up zoom-out
behavior without changing KDE settings. Evidence:
`logs/dx12-integration/reverse-zoom.log`. Physical wheel behavior still needs
user confirmation. Autodesk documents this under Application Options > Display >
3D Navigation > Zoom behavior > Reverse direction:
https://www.autodesk.com/support/technical/article/caas/sfdcarticles/sfdcarticles/Reversing-the-Mouse-scroll-in-Inventor.html

## GPU ray tracing status

GPU ray tracing is disabled in the normal working configuration. The experimental
`--debug-raytracing` launcher option installs the scoped Hydra WGL pixel-format
workaround from `compat/inventor-raytracing.c`, using a helper loader. This gets
past eligibility but, by itself, still encounters a translated-shader defect
that freezes rendering and turns the viewport white on resize. An isolated
diagnostic run with corrected captured RayGenShader overrides successfully
rendered the gear on the GPU, survived resizing, and toggled ray tracing off/on.
Those overrides are sample-specific and are not enabled by this launcher option.
Keep GPU ray tracing disabled for normal use until the translator is corrected
generally. See `ISSUES.md` and `DX12-RESEARCH.md` for evidence.

The experimental hook's former two-minute wait for Hydra has been removed,
and the loader now verifies injection instead of silently reporting success.
Delayed-start rendering was tested with a matching diagnostic shader override.
Repeated maximize/restore subsequently exposed a separate stale X11 shape bug,
including in raster mode. The first-flush shape patch now passes nine resize
cycles across raster, GPU rendering, and KDE window-state changes. The contained
launcher loads the verified driver through `result-winex11`; see
`DX12-RESEARCH.md` for rebuild and rollback instructions. GPU shader limitations
and the cosmetic white status-bar strip remain separate unresolved issues.

Normal launch still uses DX12 and now reports the real NVIDIA adapter identity
via `DXVK_CONFIG='dxgi.hideNvidiaGpu = False'`. Recovery session
`logs/research-20260922-055422` reopened the saved recovery part and visibly
restored the gear viewport; diagnostics still report `VirtualDeviceDx12`.

## Local-only configuration

`confidential/` is ignored by Git. If `confidential/lib/research-session.sh` exists,
the contained launcher uses that local session script. Otherwise it uses the
versioned general-purpose session; licensing must already be configured.
Archives, local configuration, and private investigation notes are not published.
