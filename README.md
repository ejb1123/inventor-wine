# Inventor 2027 on Wine

This repository builds a custom Wine-staging with native Windows binary-hive
loading and a functional `RegLoadAppKeyW`. Autodesk's ADIX installer requires
that API; upstream Wine 11.8 currently returns a fake handle from a stub.

The Wine changes live in
`patches/0001-implement-RegLoadAppKey-and-binary-hives.patch`, while `custom-wine.nix`
applies them reproducibly to the Nixpkgs Wine source. Run commands through
`nix-shell` (the supplied launcher scripts do this automatically).

`custom-wineserver.nix` also builds a matching server with the setup-only
kernel-synchronization opt-out in patch 0002. See [ISSUES.md](ISSUES.md) for
compatibility findings and [tests/README.md](tests/README.md) for focused validation.

See [DEVELOPMENT.md](DEVELOPMENT.md) for the confirmed failure signature,
implementation details, current limitations, validation checklist, and the
information needed to continue this work on another machine.

See [OBSERVABILITY.md](OBSERVABILITY.md) for per-run investigation records,
focused Wine tracing, diagnostic snapshots, operator notes, targeted `strace`,
and sanitized support bundles.

Experimental, isolated Wine environment for Autodesk Inventor Professional
2027. The source installers remain in the ignored `installers/` directory and
are never modified.

## Build and resume on a NixOS machine

The Autodesk installers are not stored in Git. Create `installers/` in the
cloned repository and copy these three files into it before starting (the names
must match):

```text
Inventor_Professional_2027_English_Win_64bit_db_001_002.exe
Inventor_Professional_2027_English_Win_64bit_db_002_002.7z
Inventor_2027.1_Update.exe
```

Clone and build the patched Wine package:

```bash
git clone https://github.com/ejb1123/inventor-wine.git
cd inventor-wine
mkdir -p installers
# Copy the three Autodesk files into ./installers before continuing.
nix --extra-experimental-features 'nix-command flakes' build -L .#wine
nix --extra-experimental-features 'nix-command flakes' build -L .#wineserver -o result-server
./result/bin/wine --version
```

The flake pins Nixpkgs, so another x86_64 NixOS machine builds the same Wine
source and applies the repository patch automatically. The first build can take
a while; later rebuilds reuse the Nix store cache.

Create the isolated Wine prefix and extract the installation media:

```bash
./wineboot.sh
./run-installer.sh
```

When the Autodesk bootstrap has finished extracting its files, close it if it
does not launch `Setup.exe` itself. Then run the extracted setup directly:

```bash
./prepare-compat.sh
./prepare-materials.sh
./run-setup.sh
```

The setup launcher finds the extracted Inventor folder automatically and adds
its bundled ODIS DLL directories to Wine's search path. If the bootstrap's
"Run installer" button closes without showing setup, use this launcher. It
requires exactly one matching extracted setup folder and reports any ambiguity.
Setup uses X11/XWayland as a desktop workaround on the current NVIDIA host.
Its certificate-date workaround preserves real
monotonic timers and defaults to `2026-07-01`, within the shared validity period
of the Inventor Core and RSA/REX signing certificates. The development
shell selects a companion Wine server built from the same patched source and
protocol. During setup, `WINE_DISABLE_NTSYNC=1` selects server-managed waits:
kernel absolute deadlines otherwise use the host's real date and expire early.
Normal Wine use keeps kernel synchronization enabled by default.

The project also enables `WINE_SERVICE_SESSION_ZERO=1` on the companion server.
This narrowly reports session zero for `services.exe`, allowing Go-based
Autodesk services to detect their service context. It does not implement full
Windows session isolation or change process credentials. CER and Licensing
reach RUNNING in isolated tests with their original executables.
`prepare-compat.sh` supplies the missing `tar.exe` operation used by the
Electrical Catalog installer, forwarding its extraction to Nix's `bsdtar`.
It supports that specific command form, not the complete Windows tar CLI.
It also runs `prepare-msxml.sh`, which downloads Microsoft's XML 6 runtime,
verifies the checksum from the Winetricks recipe, and preserves the previous
DLLs. Setup selects this runtime because Wine's built-in XML reader takes
minutes to read large ADIX packages; the native runtime read Inventor Core
in 382 ms with full validation in the isolated comparison.

There is **no single certificate-valid date for every bundled package**:
materials expire September 26, 2025, while RSA/REX start September 29.
`prepare-materials.sh` installs the three original material packages at a
September 2025 date using Autodesk's installer and full signature validation.
A separate local bundle owns these shared packages so ODIS commits their file
records and retains them. A canceled full setup does not reliably commit those
records. The main setup then uses July 2026. This sequence completed the full
Inventor installation on this machine, including Electrical Catalog Browser.
Application operation is a separate validation step. See [ISSUES.md](ISSUES.md)
for current outcomes.

After setup completes, close it and stop its dedicated Wine server to leave
the simulated installer date before launching the application:

```bash
source ./env.sh
"$WINESERVER" -k
"$WINESERVER" -w
./run-inventor.sh
```

Subsequent launches use `./run-inventor.sh` directly. It starts the companion
server and records application logs at the real date. Do not stop the server
while an installer or Inventor session is still doing work.

Each launcher records the complete attempt below `logs/runs/`; `logs/latest`
always points to the newest record. For the current CER service investigation,
use focused tracing:

```bash
INVENTOR_TRACE_MODE=service ./run-setup.sh
```

All scripts derive paths from `$HOME` and the cloned repository. The complete
`installers/` directory is ignored by Git, so the large copyrighted Autodesk
files cannot be committed accidentally. To use another installer directory,
set it for each command, for example:

```bash
INVENTOR_MEDIA_DIR=/path/to/installers ./run-installer.sh
```

The dedicated prefix is stored at
`~/.local/share/wineprefixes/inventor-2027`; installation logs produced by the
launchers are stored under `./logs`.

## Observe and preserve investigations

`wineboot.sh`, `run-installer.sh`, and `run-setup.sh` automatically create a
durable run directory such as `logs/runs/20260921-120000-setup`. The
`logs/latest` symlink always selects the newest run. Each record includes:

- The exact command, Git revision, dirty state, Wine build, Nix platform,
  prefix, trace mode, fake date, and installer hashes.
- Complete Wine output and Autodesk logs changed during the attempt.
- Process, CPU, memory, disk, changed-file, and installer-progress samples.
- Before-and-after Autodesk registry, package, service, and prefix inventories.
- Machine-readable status and events, operator notes, prefix differences, and
  an automatically extracted failure summary.

Inspect a running or completed attempt without disturbing it:

```bash
./status.sh
```

Attach an observation that should survive beyond the current terminal or chat:

```bash
./record-note.sh 'CER service dialog appeared with error 1053'
```

Capture a standalone system, GPU, Vulkan, Wine, prefix, registry, and service
diagnostic record:

```bash
./diagnose.sh
```

Trace modes can be selected for any launcher. `service` is recommended for the
current CER investigation; `full` can produce extremely large logs:

```bash
INVENTOR_TRACE_MODE=normal ./run-setup.sh
INVENTOR_TRACE_MODE=service ./run-setup.sh
INVENTOR_TRACE_MODE=full ./run-setup.sh
```

After isolating one failing Windows executable, capture its Wine activity and
Linux system calls directly:

```bash
./trace-target.sh 'C:\Program Files\Autodesk\Autodesk CER\service\cer_service.exe'
```

Create a sanitized archive of the latest attempt for later investigation or
transfer to another machine:

```bash
./support-bundle.sh
```

The bundle excludes installers, prefixes, executable files, registry hives,
crash dumps, raw `strace`, and oversized files. It also redacts common secrets
and home-directory names. Review the resulting archive manually before sharing
it publicly. See [OBSERVABILITY.md](OBSERVABILITY.md) for complete details and
custom tracing examples.

### CER and Licensing service startup

Increasing `ServicesPipeTimeout` was an earlier diagnostic suggestion, not a
confirmed fix. The current tests identify Go service-context detection as the
blocker: services start in interactive mode because Wine reports session one
for their `services.exe` parent. Use the companion server and the project's
`WINE_SERVICE_SESSION_ZERO=1` setting described above. Both Autodesk services
reach RUNNING in isolated tests without increasing the default timeout.

## Baseline

- Wine staging 11.8 from the Nixpkgs revision pinned by `flake.lock`
- Dedicated 64-bit prefix at
  `/home/ej/.local/share/wineprefixes/inventor-2027`
- Base installer SHA-256:
  `d00f49b42dafc5fb1847e64e36b2b4f88a9acf6c23439164cf42d33f531740ca`
- Payload SHA-256:
  `87a0196bd51eeb046695f7d2eee477db162378c5d03060403709f53d05db33fd`
- 2027.1 update SHA-256:
  `dcad0867fee39434a62f845adb7ddd2376204774ccd39ebd1afa9a6cef2597a6`

Initialize the prefix with `./wineboot.sh`, then start the first controlled
installer run with `./run-installer.sh`. Logs are written under `logs/`.

Do not add account passwords, license keys, browser cookies, or authentication
tokens to this directory or its logs.

## First bootstrap result

The Create Installer self-extractor and its Qt 6 `db-bootstrap.exe` both start
under Wine 11.8. The bootstrap locates the adjacent second archive correctly
and begins extracting it. The initial diagnostic run was stopped during
extraction because broad module/SEH tracing was excessively noisy; subsequent
runs use warning/error/fixme logging only.

The host currently has about 70 GiB free after creating the prefix. Before a
full installation and Wine debug builds, increase available space to at least
120 GiB, preferably 150 GiB.

The extracted `Setup.exe` uses private Win32 assemblies under
`ODIS/odis.bs.win` and `ODIS/odis.bs.wx`. Wine 11.8 does not resolve those
assemblies through the application's `probing privatePath="ODIS"` setting, so
`run-setup.sh` adds both directories to `WINEPATH`. This leaves the signed
Autodesk binaries unchanged.
# Local research configuration

The separate offline research installation, verified model open/save test, and
graphics compatibility settings are documented in [RESEARCH-SETUP.md](RESEARCH-SETUP.md).
Use `./run-research-contained.sh` for that copy. The original launcher remains
`./run-inventor.sh`. DirectX-to-Vulkan findings are in [DX12-RESEARCH.md](DX12-RESEARCH.md).

## Published research

The [research guide](research/README.md) indexes the authored graphics probes,
WebView diagnostics, resize experiments, and a synthetic SPIR-V validation
reproducer. It distinguishes verified fixes from sample-specific workarounds
and unresolved findings. Sources are versioned under `research/`; generated
artifacts and confidential local material remain ignored.
