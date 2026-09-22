# Development handoff

See [ISSUES.md](ISSUES.md) for the current error backlog, confirmed cancellation
chain, and preserved evidence from the 2026-09-21 installation attempt.

## Goal

Run Autodesk Inventor 2027 under Wine on NixOS/Wayland, with fixes implemented
in Wine rather than installer-specific registry-import workarounds.

## Confirmed root cause

The Autodesk ADIX installer calls `RegLoadAppKeyW` to open the native binary
`Registry.dat` embedded in each `.adix` archive. Wine 11.8 implements that API
as a stub which returns `0xdeadbeef`. The next `RegOpenKeyExW` therefore fails
with `ERROR_INVALID_HANDLE` (`0x80070006`). Autodesk reports the failure from
`MsixCoreLib::WriteAdIXRegistry::WriteRegistries` and rolls the install back.

Wine's lower-level `NtLoadKeyEx` also ignored `REG_APP_HIVE`, did not return a
root handle, and its server loader only parsed Wine's text registry format.

## Patch contents

`patches/0001-implement-RegLoadAppKey-and-binary-hives.patch` currently:

- Implements `RegLoadAppKeyW` through `NtLoadKeyEx`.
- Passes app-hive flags, requested access, and a returned key handle through
  Wine's client/server protocol.
- Adds a bounds-checked parser for native Windows `regf` hive files.
- Loads the hive below a generated `\\Registry\\A\\WineAppHive_*` key and
  returns its root handle.
- Preserves Wine's existing text-registry loader for non-`regf` input.

The parser supports the key-list forms seen in Autodesk hives (`lf`, `lh`,
`li`, and `ri`), compressed and UTF-16 names, inline value data, and ordinary
value-data cells.

## Build

On NixOS (pinned and recommended):

```sh
nix build
nix develop
```

The legacy channel-based equivalents are:

```sh
nix-build custom-wine.nix
nix-shell
wine --version
```

The first full WoW64 build is large and can take tens of minutes. Subsequent
builds are cached by Nix.

## Validation status

The first custom Wine package built successfully. A focused test loaded the
real `Registry.dat` from Inventor 2027's `InvCore.adix`, opened
`REGISTRY\\MACHINE\\Software\\Autodesk\\Inventor\\RegistryVersion31.0\\RemoveFiles`
relative to the returned app-hive handle, and read its default `REG_DWORD`
value (`1`). All calls succeeded and the test exited zero. The test source is
in `tests/regloadappkey.c`; Autodesk's hive fixture is intentionally excluded.

## Validation still required

Before treating the patch as production-ready:

1. Add an upstream-style Wine regression test for `RegLoadAppKeyW` and/or
   `NtLoadKeyEx(REG_APP_HIVE)` using a small native `regf` fixture.
2. Verify same-file handle reuse and automatic unload-on-last-close behavior.
   The current implementation uses generated hidden keys but does not yet
   implement complete native app-hive lifetime semantics.
3. Test `REG_PROCESS_APPKEY` isolation and invalid parameter/access cases.
4. Re-run the Inventor installer and capture the next compatibility failure,
   if any.

## Installer notes

Inventor Core's signing certificate expires on 2026-08-14, but the material
libraries use an older certificate expiring on 2025-09-26. The setup launcher
now uses `2025-09-01`, inside both certificates' validity periods. Autodesk's
own MSIX reader validates all three material packages at that date; July 2026
reproduces `0x8BAD0042` for Material Library 5. Signature validation remains
enabled and the host clock is unchanged.

The installer and its server both run under `libfaketime`. A companion server
from `custom-wineserver.nix` adds `WINE_DISABLE_NTSYNC=1` to opt out of kernel
waits for setup. A 200 ms absolute wait otherwise expires immediately against
the host's real clock. The fallback waited approximately 199 ms in the isolated
probe. Only the server is rebuilt, using the same source/protocol patches and
runtime data as the existing Wine package. Both Nix shells set `WINESERVER`;
`env.sh` also recognizes a local `result-server` build.

The previous installation attempt reached roughly 3%, installed several
shared components, then failed in ADIX registry loading and rolled back. The
Wine prefix and Autodesk logs should be preserved when continuing diagnosis.

## Resume on current machine (2026-09-21)

The base media extracted successfully. The bootstrap's "Run installer" action
failed with `c0000135` because it did not find the bundled wxWidgets DLLs.
`run-setup.sh` now discovers the extracted folder rather than assuming the
previous machine's ` (1)` suffix and derives `WINEPATH` from that folder.

Initial setup launches then reported `nodrv_CreateWindow` and an explorer
startup failure with Mesa EGL errors on this NVIDIA host. Unsetting
`WAYLAND_DISPLAY` did not resolve this. Explicitly disabling `winewayland.drv`
did: run `20260921-140034-setup` reached the ODIS UI, whose log reported
`mainWindow loaded` and UI initialization. The launcher now applies that
override and preserves monotonic timers under libfaketime. The user confirmed
the installer window is visible and installation is running. `Install.log`
records the start of Inventor Core 2027 installation; completion remains pending.

Per-package logs from that attempt identify an earlier failure than Anark:
Material Library 5 and both material image libraries return `0x8bad0042`
(`CertNotTrusted`, installer code 4005) in `PopulatePackageInfo`. ODIS requests
cancellation after the first material-library failure. Anark's later code 4000
is cancellation, not an independent root cause; its package log explicitly
says the installation was cancelled. Core also records a cancellation request
while in `PopulatePackageInfo`, so active processes/CPU are not evidence of
successful installation progress. Shared Components additionally returned
code 18 / exit status 2; its underlying cause is not yet established. The
July 2026 fake date has not solved trust validation for these material packages.

## Repository hygiene

Do not commit Autodesk installers, extracted payloads, Wine prefixes, logs, or
registry fixtures. They may be large, licensed, machine-specific, or contain
user data. `.gitignore` excludes the local working source tree, fixtures, logs,
and Nix result symlink.

## 2026-09-21 completed installation and first launch

`20260921-161005-setup` completed successfully, including Electrical Catalog
Browser. The decisive performance fix was native MSXML6: Core package reading
fell from minutes to 609 ms in the full installer (382 ms in the isolated probe).
`prepare-compat.sh` now installs both the archive bridge and the verified
Microsoft XML runtime. `prepare-materials.sh` completes a separate local bundle
of the original material packages at September 2025 before the full setup at
July 2026. This commits the package-file records that cancellation had lost.

`./run-inventor.sh` starts Inventor at the real date and records logs. First
launch reached the rendered Autodesk license/sign-in screen, with Licensing
Service RUNNING. The window is left open for the user to authenticate. No model
has been opened; graphics and post-authentication operation remain unverified.
See ISSUES.md for screenshots, backups, fixes, and residual startup messages.
