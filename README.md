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

See [OBSERVABILITY.md](OBSERVABILITY.md) for per-run investigation records,
focused Wine tracing, diagnostic snapshots, operator notes, targeted `strace`,
and sanitized support bundles.

Experimental, isolated Wine environment for Autodesk Inventor Professional
2027. The source installers remain in the ignored `installers/` directory and are never
modified.

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
./run-setup.sh
```

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

### Current CER service timeout workaround

The custom Wine patch gets the installer past Autodesk's ADIX binary-registry
hive failure. The next known failure is Autodesk CER Service: Wine defaults to
only 10 seconds for a Windows service to connect to the service manager. Set a
120-second timeout in the prefix before retrying setup:

```bash
nix --extra-experimental-features 'nix-command flakes' develop --command \
  wine reg add 'HKLM\System\CurrentControlSet\Control' \
  /v ServicesPipeTimeout /t REG_SZ /d 120000 /f
nix --extra-experimental-features 'nix-command flakes' develop --command \
  wineserver -k
./run-setup.sh
```

`ServicesPipeTimeout` is measured in milliseconds. Restarting `wineserver` is
required because Wine's service manager reads this setting when it starts. This
is presently a diagnostic workaround, not yet a confirmed fix for CER.

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
