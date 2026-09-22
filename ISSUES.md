# Inventor Wine investigation backlog

Last reviewed: 2026-09-21, 16:18 local. **Installation completed successfully**
in `20260921-161005-setup`. Inventor launches at the real date and displays its
Autodesk license/sign-in screen (`20260921-161627-inventor`). Licensing Service
is RUNNING with zero exit codes. User authentication/license selection is the
next step; opening or editing a model is not yet verified.

Confirmed workarounds: native registry hives, fake-clock server waits, service
session reporting, the Electrical Catalog tar bridge, separate material-library
preparation, and native MSXML6. Screenshots: `logs/inventor-install-complete.png`
and `logs/inventor-first-launch.png`. Start the application with
`./run-inventor.sh`. The window is left open for the user.

The original baseline below describes run `20260921-140034-setup`; subsequent
dated investigations and updated evidence follow it.
Environment: patched Wine-staging 11.8, Inventor 2027 English, NixOS/NVIDIA,
setup using X11/XWayland and a process-local date of 2026-07-01.

In that original attempt installation did not succeed; it cancelled and rolled back.
Core eventually completed package inspection after approximately 20 minutes,
then honored cancellation. The launcher's status file remained `running` after
the interrupted session, although installer processes had exited. Times below
come from Autodesk logs and use the fake July date.

## Original issue baseline (see later resolutions)

| ID | Component | Evidence | Status / next investigation |
| --- | --- | --- | --- |
| INV-001 | Material Library 5, Base Resolution Image Library 5, Low Resolution Image Library 5 | `PopulatePackageInfo` / `CreatePackageReader` returns `-1951596478` = `0x8BAD0042` (`CertNotTrusted`), installer code 4005. | Mitigated and verified with Autodesk's own reader: all three packages pass full validation at 2025-09-01. July 2026 reproduces the failure. Full installation retry remains pending. See certificate investigation below. |
| INV-002 | Autodesk Shared Components | Delegated `Installer.exe` invocation fails at 12:01:34 with installer code 18 and child exit status 2. | Underlying cause unknown. Correlate delegated installer logs and its extracted manifest; determine whether this is independent or a consequence of cancellation. |
| INV-003 | Inventor Core 2027 | Enters `PopulatePackageInfo` at 12:01:40; receives cancellation at 12:02:58; finishes inspection at 12:21:26, then cancels and rolls back. | Slow cancellation confirmed; not a permanent deadlock. Package-read performance remains worth investigating if it recurs. |
| INV-004 | Autodesk CER service | Earlier-machine handoff reports a service startup timeout; the 120-second `ServicesPipeTimeout` workaround was not confirmed. | Historical issue, not established as the cause of this attempt. Recheck when installation reaches this stage. |
| INV-005 | Fake-clock kernel waits / desktop startup | A 200 ms absolute NT wait returns immediately with ntsync; relative waits behave normally. | Patched companion server supports `WINE_DISABLE_NTSYNC=1`; setup selects it. Isolated regression probe passes with a 199 ms absolute wait and exit code 0. |
| INV-006 | Launcher never finalizes its logs | The `faketime` CLI waits for the persistent Wine-server descendant even after setup exits. | Launcher now applies libfaketime directly, using one clock offset for server and clients and finite server persistence (`-p3`). A normal Windows exit-code probe returns 7 as expected; the wait probe returns 0. Existing installer processes are allowed to finish. |

Microsoft's [MSIX Core error reference](https://learn.microsoft.com/en-us/windows/msix/msix-core/msixcore-troubleshoot)
identifies `0x8BAD0042` as `CertNotTrusted`. It does not establish whether this
particular failure is an expired certificate, a missing trust anchor, or a Wine
implementation problem. Do not treat noisy Wine `fixme` messages alone as root
causes.

## Consequential failure

Inventor Anark 2027's code **4000 is cancellation**. Its package log records a
cancellation request at 12:02:58, finishes reading package information at
12:05:26, and explicitly says `Installation of AdIX package is cancelled.`
ODIS had already requested overall cancellation at 12:01:25 following INV-001.
There is no confirmed independent Anark defect in this attempt.

## Startup issues addressed

- Bootstrap's "Run installer" action failed with `c0000135`: bundled wxWidgets
  DLLs were not found. `run-setup.sh` supplies their ODIS directories in
  `WINEPATH` and discovers the local extraction directory instead of using the
  previous machine's ` (1)` suffix. Use that launcher for retries.
- Setup reported `nodrv_CreateWindow` and explorer startup failure with Mesa EGL
  errors. Unsetting `WAYLAND_DISPLAY` did not resolve it; disabling
  `winewayland.drv` allowed an initial successful launch. Later cold starts
  still failed until the fake-clock synchronization issue (INV-005) was
  addressed. The launcher retains X11/XWayland and the clock-compatible server;
  do not attribute every desktop failure solely to the graphics driver. The
  user confirmed the first visible UI and started the original installation.
- The launcher preserves monotonic timers under libfaketime. This alone did
  not resolve the window failure; do not credit it as the confirmed graphics fix.

## Local evidence

Snapshot directory (ignored by Git):
`logs/runs/20260921-140034-setup/error-snapshot-20260921/`.
See its `snapshot.txt` for capture time and scope and `SHA256SUMS` for hashes.
This is a live-run snapshot, not the final result of installation.

- `Install.log`: overall failure order, package error codes, cancellation.
- `DDA.log`: UI/backend progress and cancellation handling.
- `MaterialLibrary5.log_2025_5.0.0.4_install.log`,
  `BaseImageLibrary5.log_2025_5.0.0.4_install.log`,
  `LowImageLibrary5.log_2025_5.0.0.4_install.log`: trust errors.
- `InvAnark_2027_31.0.19200.0_install.log`: explicit cancellation.
- `InvCore_2027_31.0.19200.0_install.log`: unfinished package and cancellation.
- `wine.log`, `status.json`, `manifest.env`, `progress.tsv`: run context.

Keep raw logs local: they may contain machine paths and session identifiers.
The summary above can be retained in Git; installers and raw evidence remain
excluded. The original registry-loading patch still needs the validation listed
in [DEVELOPMENT.md](DEVELOPMENT.md#validation-still-required).

## Certificate and cancellation investigation, 2026-09-21

Extracted `AppxSignature.p7x` from the unmodified packages and inspected their
embedded certificates with OpenSSL:

| Packages | Autodesk signing certificate validity (UTC) |
| --- | --- |
| Material Library 5 and both image libraries | 2024-09-27 through 2025-09-26 |
| Inventor Core 2027 | 2025-08-15 through 2026-08-14 |

The shared validity interval is 2025-08-15 through 2025-09-26. OpenSSL chain
verification succeeds for both the material-library and Core leaf certificates
at 2025-09-01; material-library verification at July 2026 fails as expired.
This identifies an expiry problem with the previous fake date. The read-only
`tests/msix-reader.c` probe calls Autodesk's bundled `msix.dll` with validation
option 0 (full validation). Material Library 5 returned `0x8BAD0042` at July
2026 and `S_OK` at September 2025; both image libraries also returned `S_OK`
in September. No certificate import, signature bypass, or package modification
was used. `run-setup.sh` now defaults to `2025-09-01 12:00:00`. A full installation
still needs validation. Certificate fixtures remain ignored under
`fixtures/cert-investigation/`.

INV-003 is now **slow cancellation**, not a confirmed permanent deadlock.
Core's `PopulatePackageInfo` completed at fake 12:21:26, after starting at
12:01:40. It then reported cancellation and rolled back. Final logs are in
`logs/runs/20260921-140034-setup/post-cancellation/`. Debugger captures and a
four-second synchronization trace are alongside them in the run directory.
Linux ptrace restrictions required elevated debugger access; captures detached
normally. The captures do not establish the cause of the slow package read.

Additional local evidence:

- `logs/msix-material-july.log`: full validation fails, `0x8BAD0042`.
- `logs/msix-{material,base,low}-september.log`: full validation succeeds for all
  three material packages (about 37 s, 5 s, and 9 s respectively).
- `logs/wait-final-0.log`: relative wait 200 ms, absolute wait 0 ms, exit 1.
- `logs/wait-final-1.log`: relative wait 200 ms, absolute wait 199 ms, exit 0,
  using the final patched server package and an isolated test prefix.
- `logs/build-wineserver.log`: successful server-only Nix build. Its runtime
  data is linked from the original Wine build; source/protocol patches match.

The temporary `fixtures/cert-investigation/no-ntsync.so` library was only an
experiment used to establish the workaround before rebuilding Wine's server.
Normal launchers use the patched server and do not load that diagnostic library.

Run `20260921-145438-setup` uses the final launcher configuration and opened the
Autodesk Inventor installer window again. The user started installation, and
package activity is confirmed. Its `validation/` directory contains checksummed copies of
the certificate, wait, and process-exit probe results. The full installation
result and Shared Components outcome remain pending.

### Active retry, September 21 at approximately 15:07 local time

The retry remains in progress; this is not a final installation result.
Base and Low Resolution Image Library 5 completed their installation operations.
New failures are preserved in
`logs/runs/20260921-145438-setup/progress-snapshot-1507/`, including a copy of
ODIS logs, CER's MSI log, Wine output, capture time, and SHA256 checksums.

- **INV-007: RSA Engine and REX Inventor** fail full signature validation at
  the September 2025 clock: `PopulatePackageInfo`, `0x8BAD0042`, installer 4005.
  The material-library date fix does not establish compatibility for every
  package. Inspect these packages' certificate validity before choosing another
  date or retry strategy.
- **INV-004 reproduced:** CER returns 1627, child exit 2. Wine records service
  startup error 1053, then `StartServices` and `InstallFinalize` returning 1627.
  The cause of the service startup failure remains unresolved.
- **INV-008: Inventor Electrical Catalog Browser CA 2027** returns installer
  1603, child exit 2. Wine records an Electrical Library `AceUn...` custom
  action failure (thread return 1359; MSI action 1603). Root cause unresolved.

The running installer was left undisturbed. Core and overall installation
success remain unverified.

### Further investigation while the user is away

ODIS requested cancellation at fake `2025-09-01 12:14:42`, while its window
still displayed Installing, 3%. Core remains in package inspection and has
received cancellation. This is no longer a healthy installation in progress.
All three material libraries reported successful installation before that.

Certificate audit (`logs/adix-certificate-audit.txt`) covers all 17 bundled
ADIX packages. RSA Engine, REX Inventor, and REX Framework leaf certificates
start on **2025-09-29**, after the material certificates expire on
**2025-09-26**. No single simulated date can satisfy all leaf lifetimes.
If material installations survive cancellation, a repair at July 1, 2026
can cover the remaining certificate intervals; this strategy is not yet tested.
Wine's `SoftpubLoadMessage` logs `unimplemented for 3` (`WTD_CHOICE_BLOB`).
Microsoft's MSIX [signature validator](https://github.com/microsoft/msix-packaging/blob/master/src/msix/PAL/Signature/Win32/SignatureValidator.cpp)
uses that path as its timestamp-aware fallback. This identifies a Wine API
gap; it is not justification to bypass validation.

**INV-009: Autodesk Licensing Service** fails with installer code 69009748,
helper code 108, and Windows error 1053. Its own service log shows its HTTP
server starting before Wine's service-manager timeout. CER likewise starts an
HTTP server before its timeout. Their Go service-detection code is a suspect:
an isolated Go `x/sys/windows/svc` v0.37.0 test launched by Wine's service
manager reports `IsWindowsService=false`, despite successfully running its
service dispatcher when instructed explicitly. Go's implementation checks
that its parent is `services.exe` in session zero. A scoped server-session
experiment is being tested outside the installation prefix.

### Verified compatibility fixes, approximately 15:28 local time

- **INV-008:** The Electrical Catalog custom action invokes Windows `tar -xf
  <zip> -C <directory> --keep-newer-files`; Wine had no `tar.exe`. The archive
  passes 7-Zip integrity checking. `compat/tar-bridge.c` translates paths and
  waits for native `bsdtar`. It must be installed as the Winelib ELF itself,
  not the generated shell launcher. With that correction, the original MSI
  returns 0 and all 20 payload files match independent extraction byte-for-byte.
  Evidence: `logs/electrical-tar-elf-msi.log`,
  `logs/electrical-payload-elf-diff.txt` (empty = equal). Earlier bridge tests
  returned 0 without copying files and do **not** establish success.
- **INV-004 / INV-009:** The scoped opt-in server patch reports session zero
  for `services.exe` in process queries. The Go probe changes from false to
  true; both original Autodesk services reach RUNNING with zero error codes.
  Evidence: `logs/autodesk-services-patched.txt` and corresponding service
  trace. This is a reporting workaround, not complete Windows session
  isolation. An earlier experiment moving the actual service session broke
  Wine's startup rendezvous and was discarded. No service executables or
  license checks were modified.
- **INV-007:** RSA Engine, REX Inventor, and REX Framework each pass full
  MSIX reader validation at July 1, 2026. Evidence:
  `logs/msix-{RSAEngine,REXInventor,REXFramework}-july2026.log`.

Full installation remains pending the repair pass. The existing install is
still unwinding cancellation; do not infer completion from an unchanged UI.

At 15:32 local time the canceled September run was stopped using its dedicated
prefix's Wine server. Core still had not left `PopulatePackageInfo`; a sampled
busy thread was in Wine's `heap_allocate_block`, so package-reader/heap
performance warrants separate investigation. The ODIS database was backed up
through SQLite's backup API before stopping. Launcher exit 0 records process
termination only and does not mean installation succeeded.
Repair run `20260921-153228-setup` starts with the July 2026 clock, the rebuilt
service-compatible server, and the verified archive bridge.

At 15:56 the July repair was stopped after it had already requested cancellation.
CER and Licensing both completed successfully with the service-reporting fix.
The three material libraries failed certificate validation again: their files
existed, but the canceled earlier run had not committed Package.db file records.
Registry and SQLite backups plus ODIS logs are in the repair run's
`before-material-preparation/` directory (captured after stopping).

A separate local material-preparation bundle was validated in an isolated
prefix using the original Autodesk installer and original signed packages.
It owns the three shared packages, preventing ODIS from removing them as
unreferenced. Installation commits 1029 LowImageLibrary5, 2569 MaterialLibrary5,
and 1064 BaseImageLibrary5 file records. A second run at July 2026 reports the
bundle installed and completes without certificate errors. Production preparation
is now running. Evidence: `logs/material-bundle-console3.log`,
`logs/material-preparation-july-detection.log`.

Production material preparation completed successfully at approximately 15:59.
All three expected PackageFile counts match the isolated test. Both ODIS databases
were backed up to `logs/material-production-success/`. Persistent Autodesk
background processes kept the console pipe open after bundle completion; the
prefix was stopped to transition to the July setup. The completed bundle and
committed records, not the launcher's process lifetime, establish success.

## INV-010 — Wine MSXML package-reader performance

The July run `20260921-160026-setup` correctly recognized the prepared material
libraries and completed RealDWG Shared, but Core remained in PopulatePackageInfo.
A standalone full-validation Core reader was still busy after over 3 minutes;
a sampled stack was in Wine MSXML/oleaut32. The same package, reader, clock,
and custom server with Microsoft's native MSXML6 completed successfully in
382 ms (`logs/core-native-xml-validation.log`). No signature validation was
disabled. This comparison establishes a practical workaround, not the precise
algorithmic cause in Wine's XML implementation.

`prepare-msxml.sh` installs the Microsoft redistributable used by the pinned
Winetricks recipe, verifies its SHA-256, and backs up previous DLLs. It is now
called by `prepare-compat.sh`; setup and material launchers select `msxml6=n,b`.
The main run was stopped to apply the change, with ODIS logs and SQLite backups
under `logs/runs/20260921-160026-setup/before-native-xml/`. Full installation with
native XML is now being tested. The baseline isolated reader was stopped too.

In production run `20260921-161005-setup`, Core PopulatePackageInfo completed
in 609 ms and proceeded to extraction of 3991 payload files. Anark and RealDWG
Shared have completed successfully. This confirms the XML workaround inside
the actual installer; final bundle completion and application launch remain
pending.

At 16:15 local time, Autodesk setup displayed **Inventor Professional 2027 —
Install complete**. Electrical Catalog Browser also completed, including its
previously failing custom action. ODIS emitted POST_INSTALL_COMPLETE with
restartRequired=false. Screenshot: `logs/inventor-install-complete.png`.
Databases, registry, and ODIS logs: `logs/inventor-install-complete/`.
This establishes installation success; application operation is being tested
separately at the real host date. DWG TrueView was not selected.

## Application startup — authentication pending

At 16:17, `run-inventor.sh` launched Inventor with the real date and the custom
server. The application reached its rendered "Let's Get Started" screen,
offering Autodesk ID, serial number, or network licensing. No credentials were
entered and no license mechanism was bypassed. The screen is left open for the
user. `AdskLicensingService` reports RUNNING and both exit codes zero
(`logs/inventor-launch-licensing-service.txt`).

Startup logged missing WinRT authentication/diagnostics activation factories,
a UIAutomation COM class error, and a Direct3D pixel-format fallback. These are
preserved in `logs/inventor-launch-errors.txt`; they did not prevent the initial
license screen, but sign-in and graphics/model operation remain untested.
Do not mark them resolved or assume they cause a failure without a reproducer.
The desktop-input daemon started for this investigation has been stopped.

## 2026-09-22 — overlapping startup and sign-in windows

The two visible windows are Inventor's startup splash (`inventor.exe`) and
its separate licensing dialog (`adsklicensingagent.exe`), not two Inventor
instances. The sign-in dialog requests no Motif decorations. Moving it through
X11 succeeds; requesting a normal title bar did not produce frame extents, and
Wine restores its decoration hints. No restart or authentication was performed.
KWin supportInformation confirms `commandAll1: MouseUnrestrictedMove` and
`keyCmdAllModKey: 16777250` (Qt Key_Meta): hold Meta/Windows and left-drag anywhere
inside the dialog. Temporary diagnostic KWin scripts were unloaded.

## 2026-09-22 — contained research graphics

The original installation remains separate from the research clone.
See `RESEARCH-SETUP.md` for launcher,
data locations, containment limits, and rollback context.

- Wine Mono crashed loading Shared Views and Interactive Tutorial .NET 10
  assemblies. Disabled only their startup loading in the research clone and
  backed up their manifests under `logs/research-setup/`.
- Built-in graphics produced white surfaces, and initial part creation displayed
  a DirectX 12 unavailable dialog. Inventor was switched to DX11 through its API.
- DXVK 3.1.1 native DXGI/D3D11 beside research Inventor.exe fixed the CAD viewport.
  Created/saved/reopened an empty part; opened and saved a copy of the bundled
  bevel gear and visually confirmed geometry, axes, cube and feature tree.
  Evidence: `logs/research-20260922-033243/gear.log` and `gear.png`.
- Independent vkd3d-proton 3.0.1 probe passes DX12 device/queue/fence on RTX3080;
  builtin Wine fails the same device creation. The first Inventor DX12 setter
  returned `0x80020009`; a later retry with documents closed succeeded. After
  restart, diagnostics confirm DX12 hardware rendering. Rendered the bevel gear,
  changed camera, saved a new copy and reopened it. See `DX12-RESEARCH.md`.
- Home/WebView2 rendering was subsequently fixed; see `HOME-BROWSER-RESEARCH.md`.
- Remaining: full add-in compatibility and long-run
  stability. Two earlier exits lacked a conclusive crash cause; no OOM evidence.
  Experimental `cube.exe` helper crashed during COM handling; Inventor survived.
  It is not used by the launcher.

Logs are preserved per run. The launcher now prevents overlapping future research
sessions using flock and snapshots runtime/server logs at normal session cleanup.

### KDE menu launch failure, September 22, 04:08–04:19

- The menu still invoked stock Wine after the desktop files were edited. Its
  AdskLicensingService failed to start; journal evidence is retained in
  `logs/kde-stale-menu-failure.log`.
- The shell and Plasma used different KSycoca caches (locale and XDG data paths).
  Shortcut installation now refreshes the cache using Plasma's environment and
  the unwrapped Nix kbuildsycoca6 binary.
- After refreshing, the actual menu reached our launcher but exited 69 because
  KDE's PATH lacked bwrap. Evidence: `logs/research-20260922-041738/console.log`.
  The launcher now uses `fixtures/desktop-tools/bwrap`, with a PATH fallback and
  explicit missing-dependency error. The local symlink points to bubblewrap 0.12.0.
- Retested by opening KDE's menu, searching Inventor Professional and pressing
  Enter. Run `logs/research-20260922-041841` checked out the Inventor feature at
  04:18:52; main window captured in `logs/kde-menu-success.png`.

### Blank Home panel: fixed with browser-only Windows 8 compatibility

The local Home page rendered internally but failed to display under Wine's
Windows 10 compatibility path. A minimal native WebView2 host reproduced it.
Setting only `msedgewebview2.exe` to `win8`, with `--disable-gpu`, restored the
standalone test and Inventor Home. Normal launch `20260922-045821` displays the
recent-file list and Open/New controls. The launcher persists the override;
Inventor itself retains Windows 10. Experimental probes were removed on restart.
Evidence and reproduction: `HOME-BROWSER-RESEARCH.md`.

### Native status bar theme mismatch (2026-09-22)

The white strip at the bottom is Inventor's visible `msctls_statusbar32`
status bar, showing "For Help, press F1". Native control enumeration confirms
its rectangle at the bottom of the window. Its light background clashes with
Inventor's dark UI; recorded as a cosmetic Wine/theme issue, separate from the
working DX12 viewport. Evidence: `logs/dx12-integration/windows-statusbar.log`.

### GPU ray tracing eligibility rejected (2026-09-22)

User reports Hardware options says GPU does not support ray tracing. A separate
capability probe in the running research namespace, using copies of the same
native d3d12/d3d12core/dxgi DLLs, successfully queries D3D12_OPTIONS5 and returns
RaytracingTier=11 (DXR 1.1). This tests advertised support, not ray dispatch or
Inventor's ray renderer. Evidence: `logs/raytracing/caps.log`.

DXVK defaults to hiding NVIDIA as AMD: adapter 1002:73df while retaining RTX3080
name. Probe-only `DXVK_CONFIG='dxgi.hideNvidiaGpu = False'` reports the real
10de:2216 adapter and still exposes DXR1.1 (`caps-nvidia.log`). Inventor diagnostics
also show unknown driver and 0GB graphics memory. These are detection leads;
none has yet been proven to cause the disabled ray-tracing option. No persistent
configuration change or restart was made during these probes. The current DX12
modeling session remains running. Next investigate Inventor/Aurora eligibility
checks and test accurate adapter reporting on a controlled subsequent launch.

DXVK documents this default in its release configuration:
https://github.com/doitsujin/dxvk/blob/v3.1.1/dxvk.conf

### Ray-tracing eligibility workaround and rendering failure (2026-09-22)

Correct NVIDIA reporting (`dxgi.hideNvidiaGpu=False`) alone did not fix the
eligibility check. Disassembly and isolated execution of the installed
agp_hydra_bridge.dll's Aurora eligibility routine identified a failed OpenGL
window pixel-format request: 32 color bits plus 8 alpha bits produces zero
matching window formats on this Wine/Xwayland/NVIDIA configuration. A narrowly
scoped retry with 24 color bits and no window alpha returns pixel format 86,
OpenGL 4.6, and a successful DXR eligibility result. No hardware capability
return value was forced. Logs: `logs/raytracing/eligibility-trace2.log` and
`eligibility-rgb24.log`.

Experimental source: `compat/inventor-raytracing.c` hooks only the Hydra module's
WGL function lookup and retries the specific failed format request. Its loader
is enabled ONLY with `./run-research-contained.sh --debug-raytracing`; normal KDE
launches do not inject it. The helper binaries are in C:\ResearchUI. This is an
experimental workaround, not a completed ray-tracing fix. It does not change
render-target alpha formats or the Autodesk binaries on disk.

In session `research-20260922-055009`, the hook installed, its fallback matched,
Inventor accepted EnableViewportGPURayTracing=True, and Aurora/hdAurora/NRI
loaded. GPU ray tracing then left the viewport frozen; resizing made it white.
Turning off view ray tracing did not restore the viewport, requiring a restart.
An e06d7363 exception was logged at initialization; its source is not established.
A later exception probe recorded identity-plugin exceptions, which do not
establish the renderer failure cause. Do not claim successful GPU ray rendering.
Subsequent isolated runs (`research-20260922-060548` and
`research-20260922-061216`) captured the first Aurora rendering exception:
`HRESULT of 0x80004005` (E_FAIL), at `aurora.dll+0xdc142`, reached from
`aurora.dll+0x9d187`. Read-only disassembly follows the latter call through
thunk `0x4115` to `0xda4e0`; its failing HRESULT branch follows the device
vtable call at `0xdb40d`, slot `0x1f0` (`ID3D12Device5::CreateStateObject`).
This localizes the failure to creating the ray-tracing shader pipeline.
The subsequent PTRenderer.cpp:239 message about an operation taking too long
does not itself establish a GPU timeout. The exact pipeline rejection reason
still needs tracing. Basic shared D3D12 texture creation, handle creation, and
handle reopening passed a separate probe (`logs/raytracing/sharing.log`), which
does not establish that the full renderer interoperability path works.

These reproductions use a private writable overlay of the research prefix;
the user's normal modeling instance is left running. No GDB attachment was
used to obtain the initial failure. Exception records are in the overlay's
`drive_c/ResearchLicense/ray-exceptions.log`; the failing function disassembly
is `fixtures/raytracing/aurora-failing-function.asm`.

The later `research-20260922-061634` attempt did not establish another pipeline
result: `open-dx12-gear.exe` faulted at +0x10d1 after ActiveView returned success
with no object. This is a diagnostic helper bug, not the Aurora exception.
The helper now checks null dispatch pointers and returned VARIANT types; its
deliberate null-input test exits with code 2 and a diagnostic instead of raising
a page fault (`logs/raytracing/null-helper-test.log`). The failed isolated
session was stopped; normal Inventor was left running. Exception evidence was
copied to `logs/raytracing/isolated-ray-exceptions-20260922.log`.

Further tracing in `research-20260922-061935` unwound the exception context
to the Aurora frame and resolved the actual failing function pointer to
`d3d12core.dll+0x4d80` (vkd3d-proton CreateStateObject). Enabling warning-level
API/shader logs exposed `hresult_from_vk_result: Unhandled VkResult -3`, i.e.
VK_ERROR_INITIALIZATION_FAILED, immediately before the Aurora exception.
There is also a conflicting-local-root-signatures warning. The warning's
causal role is unproven. This is a Vulkan pipeline/setup failure, not evidence
of a timeout or absent hardware DXR support. Vulkan validation is the next test.

Validation itself exposed a separate startup defect in this Wine configuration:
`research-20260922-062345`/`062508` assert in winevulkan's
vkGetPhysicalDeviceProperties2 wrapper. With the Vulkan layer logging directly
to stderr, it reports VUID-VkPhysicalDeviceLayeredApiVulkanPropertiesKHR-pNext-10011
on the maintenance7 layered-driver-property query. This occurs before rendering
and must not be confused with the existing GPU-ray failure. Subsequent diagnostic
tests disable VK_KHR_maintenance7 only in the isolated instance to bypass that
query; the normal launcher is unchanged.

**Confirmed shader translation defect (06:28):** validation in
`research-20260922-062614` reached the GPU renderer and rejected its RayGenShader
module with VUID-VkShaderModuleCreateInfo-pCode-08737:
`Illegal number of components (12) for TypeVector`,
`%v12float = OpTypeVector %float 12`. Standalone spirv-val reproduces the failure
on the captured SPIR-V (`logs/raytracing/raygen-spirv-validation.log`). The
translator also emitted bitcasts between that invalid vector and a struct
containing float4[3]. This is concrete evidence of invalid translated shader
code, beyond the generic Vulkan initialization error.

The diagnostic script `fixtures/raytracing/fix-raygen-prototype.py` replaces
this vector with a 12-float array and the two aggregate bitcasts with explicit
scalar extraction/construction, preserving order. The resulting captured
shader passes spirv-val for Vulkan 1.3 with scalar-block-layout. This is not a
general translator fix. Aurora generated at least two different DXIL library
hashes (`4d8bd6a92842dfd5`, `3d54ffe8ce2a831e`) across restarts; shader overrides
must match the actual hash and must not be substituted across different shader
contents. The first in-app override attempt did not match and therefore did
not establish successful rendering. All overrides remain isolated test data.

**Successful limited GPU rendering test (06:33):** in
`research-20260922-063207`, the log explicitly confirms loading the corrected
`4d8bd6a92842dfd5.lib.RayGenShader.spv`. The gear rendered with the viewport's
GPU ray-tracing indicator visible and API progress reaching 1. Resizing the
test window to 1500x950 triggered a fresh render (progress 0.146...) instead
of a white viewport. Disabling ray tracing restored raster rendering, and
enabling it again completed another GPU render. Evidence:
`logs/raytracing/override2-render.png`, `override2-resized.png`,
`override2-disabled.png`, `override2-repeat.png`, and matching `*-view.log` files.
The status helper exits with a COM exception if it asks IsRayTracingPaused after
RayTracing=False; that unsupported query is not an Inventor crash.

This proves the captured shader correction gets this sample past the original
rendering failure. It does not prove arbitrary materials/models work. The next
implementation task is a general dxil-spirv correction for large flattened
matrix vectors and aggregate bitcasts, rather than shipping a fixed shader
override list. Validation also reports descriptor-buffer binding and shared
image initial-layout issues; these did not prevent this sample from rendering
and remain separately tracked, not assumed harmless in every case.

The successful isolated test instance was left open for inspection (host PID
4130806; normal modeling instance remains PID 4041040). Only its launcher enables
Vulkan validation, skips maintenance7, and loads the shader overrides. Normal
launches still default to GPU ray tracing disabled. Test helper opening now
explicitly activates the document, because a loaded document can coexist with
Home as the active tab and ActiveView=NULL.

### GPU completion evidence and remaining uncertainty

The user reported that completion felt too quick and rendering still seemed
buggy. A fresh read (`logs/raytracing/progress-verification-live.log`) returned
RayTracing=True, IsRayTracingPaused=True, RayTracingProgress=1. These are
application-reported states, not independent proof of correct pixels or sample
counts. The panel showed a full bar with Continue available.

Resuming through IsRayTracingPaused=False while polling for about six seconds
produced real progress transitions: approximately 0.11 initially, fluctuations
down to 0.07, then 0.25, 0.41, 0.63, 0.91 and 1 with the paused flag set.
The progress reached 1 at 5814 ms (`logs/raytracing/continue-observe.log`).
Concurrent NVIDIA per-process monitoring recorded substantial GPU activity for
Inventor PID 4130806 (`logs/raytracing/continue-gpu-pmon.log`). This supports
active GPU work and advancement to the reported target. It does not prove
correct rendering. The early nonmonotonic progress could reflect view/scene
changes or unintended resets; user interaction was not excluded during capture.

Validation errors still include shared-memory allocation-size mismatches
(e.g. imported allocation 7,274,496 bytes versus exported 3,637,248 bytes) and
dedicated-image mismatches, in addition to initial-layout errors. See
`logs/research-20260922-063207/inventor.log`, around lines 2692 and 3864.
Do not describe this configuration as fully reliable or validated merely
because its progress bar fills. The original normal process PID 4041040 was
no longer running at this later check; the isolated test instance remained.

CPU ray tracing did render the gear in the preceding session (`render.png`).
Regular DX12 modeling remains the intended default. The test GPU preference was
turned off and both diagnostic DLLs are unloaded by the recovery restart.
Recovery files: `research-home/Documents/RayTest-Recovery-20260922-0555-1.ipt`
and the earlier `RayTest-Recovery-20260922-0547-2.idw`. The 05:30 session exit during
recovery was explicitly confirmed by the user as manually closing Inventor.

### KDE maximize layout reproduction (2026-09-22, memory-test session)

User reported broken layout when maximizing with ray tracing enabled. In session
`logs/research-20260922-064548`, requesting KDE/X11 maximization for window
`0x3000005` reproduced large black areas at right/bottom and clipped ribbon.
X11 reported 2560x1368. Evidence: `logs/raytracing/maximized-layout-latest.png`
and `memory1-maximized.png`. Restoring also left duplicated/clipped ribbon pixels
(`memory1-restored.png`). This is a confirmed visible layout failure, but its
cause is not established. This run had no Aurora module loaded despite earlier
API ray-tracing enablement, so it does not validate GPU-ray-tracing resize or
prove a shared-memory fault caused the layout failure.

Window geometry enumeration was saved in `layout-windows-restored.log`.
A diagnostic WM_SIZE/redraw helper was built, but the Inventor process exited
before it could run; no redraw workaround is validated. Do not call this fixed.

### Follow-up: maximize capture reliability and experimental loader (07:00–07:16)

Direct X11 `import -window` captures can omit GPU content: one capture showed a
black viewport while the subsequent KDE/Spectacle desktop capture showed the
rendered gear. Use compositor captures for final visual validation. Native
`WM_SYSCOMMAND/SC_MAXIMIZE` produced a full layout in
`logs/raytracing/layout3-desktop.png`; the user also confirmed maximize works
now. This does not establish a permanent KDE resize fix. Earlier xdotool calls
with repeated `--add` options did not reliably request both maximize axes, so
those tests cannot alone establish a full-maximize failure.

The test launcher in the 06:57 session exited with status 143 (SIGTERM), without
a corresponding application exception in its last log lines. User explicitly
said they did not close/kill it. Sender/cause remains unknown; do not label it a
confirmed Inventor crash or user closure. Later intentional closes via the
native system command exited with status 0.

The experimental hook worker had a finite 12000 x 10 ms wait for the lazily
loaded Hydra module and then silently gave up. The DLL loader also returned
success without checking whether remote LoadLibrary actually loaded its DLL.
Source changes remove the hook's two-minute deadline and add loader result and
module-presence checks, failure exit codes, handle/memory cleanup, and hook
startup/install diagnostics. A late diagnostic injection successfully installed
the previously absent hook in session `research-20260922-070848`.

Loader failure-path test: intentionally missing DLL returned exit status 1 and
logged a zero LoadLibrary result plus missing-module diagnosis
(`logs/raytracing/loader-missing-dll-test.log`). Startup verification passed in
`research-20260922-071444/raytracing-loader.log`. Delayed model-open / rendering
verification is still in progress; no general GPU-render reliability claim.

Delayed-open verification succeeded: process 129581 was left on Home for 2:22
before opening the model (`delayed-hook-open-timing.log`). The updated hook then
installed and logged RGB24 fallbacks. Aurora loaded; the captured RayGen shader
override was used, and GPU activity reached 76% during the maximize test
(`delayed-hook-max-gpu.log`). This confirms the finite hook wait was a real
failure mode. Rendering still uses the limited shader override experiment.

Maximize clipping was independently verified with the compositor and XShape:
window `0x3000005` had geometry 2560x1368 but bounding shape 2006x1174, the old
window size. Win32 GetWindowRgn returned no custom region; its client size and
child panels had expanded. Clearing the X11 bounding shape exposed black space,
so stale backing-surface content is involved too. See `gpu-cycle-max-desktop.png`,
`gpu-window-region.log`, `gpu-shape-reset-desktop.png`. Testing per-application
`ClientSideGraphics=N` next; not yet a validated fix.

`ClientSideGraphics=N` was rejected: it removed the stale bounding shape but
left black ribbon/panel areas (`server-gdi-max-desktop.png`). The per-app setting
was removed. A targeted X11 driver build is being tested via a private bind
mount, leaving the installed Wine runtime untouched. The candidate guards
surface reuse with a rectangle comparison; the win32u caller already performs
related checks, so effectiveness must be measured rather than assumed.

### Repeated resize and external-memory follow-up (07:30–07:40)

The extra X11 surface rectangle check failed all three restore/maximize cycles:
outer geometry 2560x1368, bounding shape 2006x1396. See
`logs/raytracing/patched-x11-cycle-{1,2,3}-shape.log` and
`patched-x11-cycles-desktop.png`. The candidate patch is explicitly marked
experimental/rejected and is not part of the normal Wine build. Clipping also
persisted after disabling ray tracing (`resize-raster-shape.log`); this does not
prove a fresh raster-only session has the same failure.

Installed the verified delayed-hook worker and checked loader in the base
research prefix after the test overlay closed. Previous binaries are backed up
in `fixtures/raytracing/startup-backup-20260922`. Only the experimental ray-tracing
launch path uses these helpers; normal GPU-ray-tracing defaults are unchanged.

Session `research-20260922-073804` used the original X11 driver, an observational
Vulkan memory layer, and disabled VK_EXT_zero_initialize_device_memory alongside
maintenance7. Actual RayGen override use and compositor-rendered gear were
verified (`memory-trace-desktop.png`). External images now use initialLayout=0;
01443 was absent. 01742, 01878 and descriptor-buffer 08065 remained.

Memory tracing showed two exports (RGBA16F/format97 at 22,937,600 bytes and
R32F/format100 at 11,468,800 bytes) reusing numeric fd509. A subsequent RGBA16F
import triggered validation against the most recent R32F export for fd509.
This raises a descriptor-reuse tracking hypothesis; it is not proof of an
incorrect actual allocation. A follow-up layer records fstat identities before
Vulkan consumes imported descriptors. Do not claim the 2x mismatch is the
rendering/resize root cause without confirming underlying handle identity.

Session `research-20260922-074019` confirmed that fstat is insufficient to identify
these NVIDIA opaque FDs: both different exports and all imports have dev7,
inode818, size0. The memory-warning hypothesis remains unresolved. Three
restore/maximize cycles still clipped with zero initialization disabled;
minimize/maximize also failed to recover it.

Session `research-20260922-074331` reproduced the same clipping in a fresh
raster-only session, without injecting the GPU-ray-tracing hook or enabling
ray tracing (`surface-raster-status.log` reports RayTracing=0). Wine's bitblt
trace requests 2048x1408 on restore and 2560x1408 on maximize for main HWND
0x500dc, yet the X11 bounding shape stays 2006x1396. This rules out simply
requesting the old surface size. Resetting XShape plus WM_SIZE/redraw exposes
black viewport space (`shape-redraw-desktop.png`), not a complete recovery.
A diagnostic-only X11 driver adds surface/shape flush logs to locate the stale
shape application; it changes no painting behavior.

The instrumented driver in `research-20260922-074852` narrowed the shape failure:
restore flushed a shaped 2048x1408 surface, maximize flushed a shaped temporary
2688x1408 surface, then created the final 2560x1408 surface without a subsequent
shape update. The X window retained the previous 2006x1396 shape. Candidate
`0005-sync-x11-shape-on-first-flush.patch` adds a per-surface first-flush flag so
an unshaped replacement surface explicitly clears its predecessor's X shape.
This differs from the rejected rectangle-reuse candidate; validation is pending.

### Verified first-flush shape fix (07:52)

Patch 0005 passed nine restore/maximize cycles in session
`research-20260922-075242`: three raster, three with GPU ray tracing enabled,
and three using separate KDE/EWMH maximize-axis changes. All maximized windows
were 2560x1368 with no stale X11 bounding shape. Compositor captures show full
viewport/ribbon painting, including the previously black right-hand area.
Evidence: `shape-fix-raster-{1,2,3}-*`, `shape-fix-gpu-{1,2,3}-*`,
`shape-fix-kde-{1,2,3}-shape.log`, `shape-fix-kde-desktop.png`.
Matching RayGen override was used, and the GPU ray-tracing panel/rendered gear
were visible. This validates the reproduced resize fix, not arbitrary shaders.

The normal contained/KDE launcher now binds the verified `result-winex11` driver
when present and logs the resolved store path. Driver built at
`/nix/store/4x3l0c0j867bdal4hx7199b8gjbqvlw4-winex11-inventor-shape-reset-11.8`.
The rejected 0004 size-comparison experiment is not included. Shell syntax and
Git whitespace checks passed. Documents were checked SAVED before each close.
White status-bar strip, general RayGen lowering, descriptor-buffer validation,
opaque-handle import warnings and unexplained prior SIGTERM remain unresolved.

Fresh normal-launch verification succeeded in `research-20260922-075532`:
`x11-driver.txt` names the fixed driver; the sample opens, restores/maximizes,
and fills the viewport with no stale shape (`normal-fixed-desktop.png`,
`normal-fixed-shape.log`). Inventor was left running in normal raster mode.

Additional unresolved log finding: Wine Mono emits “Native Crash Reporting” /
“Got a UNKNOWN while executing native code”, followed by unregistered COM class
`{cfe9f29b-e1b6-4240-aed7-360846769314}`. This also occurs in earlier diagnostic
sessions, so it is not newly introduced by the X11 patch. Inventor continues
running/rendering afterward. The affected component and functional impact are
not yet identified; do not describe this as a full application crash or fixed.
