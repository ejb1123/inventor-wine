# Development handoff

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

On NixOS:

```sh
nix-build custom-wine.nix --no-out-link
```

Or enter the project environment (which selects the custom Wine):

```sh
nix-shell
wine --version
```

The first full WoW64 build is large and can take tens of minutes. Subsequent
builds are cached by Nix.

## Validation still required

Before treating the patch as production-ready:

1. Add an upstream-style Wine regression test for `RegLoadAppKeyW` and/or
   `NtLoadKeyEx(REG_APP_HIVE)` using a small native `regf` fixture.
2. Test loading an Autodesk `Registry.dat`, opening
   `REGISTRY\\MACHINE\\Software` relative to the returned handle, reading a
   known value, and closing the handle.
3. Verify same-file handle reuse and automatic unload-on-last-close behavior.
   The current implementation uses generated hidden keys but does not yet
   implement complete native app-hive lifetime semantics.
4. Test `REG_PROCESS_APPKEY` isolation and invalid parameter/access cases.
5. Re-run the Inventor installer and capture the next compatibility failure,
   if any.

## Installer notes

The Autodesk signing certificate encountered during this investigation had
expired on 2026-08-14. Wine did not decode the Authenticode timestamp used by
the installer, so `run-setup.sh` starts the wineserver under `libfaketime` with
the default date `2026-07-01`. Starting only the installer under fake time is
not sufficient; an already-running wineserver retains real time.

The previous installation attempt reached roughly 3%, installed several
shared components, then failed in ADIX registry loading and rolled back. The
Wine prefix and Autodesk logs should be preserved when continuing diagnosis.

## Repository hygiene

Do not commit Autodesk installers, extracted payloads, Wine prefixes, logs, or
registry fixtures. They may be large, licensed, machine-specific, or contain
user data. `.gitignore` excludes the local working source tree, fixtures, logs,
and Nix result symlink.
