# Inventor 2027 on Wine

This repository builds a custom Wine-staging with native Windows binary-hive
loading and a functional `RegLoadAppKeyW`. Autodesk's ADIX installer requires
that API; upstream Wine 11.8 currently returns a fake handle from a stub.

The Wine changes live in
`patches/0001-implement-RegLoadAppKey-and-binary-hives.patch`, while `custom-wine.nix`
applies them reproducibly to the Nixpkgs Wine source. Run commands through
`nix-shell` (the supplied launcher scripts do this automatically).

See [DEVELOPMENT.md](DEVELOPMENT.md) for the confirmed failure signature,
implementation details, current limitations, validation checklist, and the
information needed to continue this work on another machine.

Experimental, isolated Wine environment for Autodesk Inventor Professional
2027. The source installers remain in `/home/ej/Downloads` and are never
modified.

## Baseline

- Wine staging 11.8 from the current NixOS channel
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
