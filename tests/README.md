# Focused diagnostic probes

These probes require the locally built Wine and, for MSIX, the user's Autodesk
installation media. No Autodesk binaries or certificate fixtures belong in Git.

Inside `nix develop`, build the probes with:

```sh
winegcc -m64 -Wall -Wextra -Werror -o tests/msix-reader.exe tests/msix-reader.c -lole32
winegcc -m64 -Wall -Wextra -Werror -o tests/wait-timeout.exe tests/wait-timeout.c
```

`msix-reader.exe` takes two Windows paths: Autodesk's extracted `msix.dll` and
an `.adix` package. It opens the package read-only through that DLL, requests
**full validation**, prints the Windows UTC time and HRESULT, and exits nonzero
on failure. It does not install, extract, import certificates, or bypass
signature validation. Pass Windows paths rather than Unix paths.

The probe uses the documented factory interface and export signatures from
[Microsoft's MSIX header](https://github.com/microsoft/msix-packaging/blob/master/src/inc/public/AppxPackaging.hpp).
The DLL is proprietary and its ABI could change; missing exports are a probe
failure, not evidence of a broken package.

`wait-timeout.exe` waits on an unsignaled event twice: once with a relative
200 ms timeout, once with an absolute deadline 200 ms in the future. It checks
both the timeout status and elapsed performance-counter time, exiting zero only
when both waits last between 100 ms and 5 seconds. Run it in a **separate test
prefix** when comparing server modes; changing `WINE_DISABLE_NTSYNC` requires
stopping and restarting that prefix's server.

For a fake-clock test, set `FAKETIME_DONT_FAKE_MONOTONIC=1`, compute a signed
seconds offset from the real clock to September 1, 2025, and apply
`LD_PRELOAD=$INVENTOR_FAKETIME_LIBRARY` plus `FAKETIME=<offset>` to **both** the
server and probe. Start `$WINESERVER -p3` before the probe. The Nix shell sets
both paths. Prefer applying the library directly: the `faketime` command-line
wrapper waits for descendants and can obscure the probe's exit status if the
server is persistent. See `run-setup.sh` for the clock environment setup.

The generated Winegcc programs can be invoked using their executable launcher
or `wine tests/wait-timeout.exe.so`. The same C sources can also be compiled as
native Windows PE executables with MinGW; that form was used for the final
isolated wait comparison.

## Verified on 2026-09-21

| Probe | Expected / observed |
| --- | --- |
| Material Library 5, July 2026 | `0x8BAD0042` (`CertNotTrusted`) |
| Material Library 5, September 2025 | `S_OK`, full validation |
| Base and Low Resolution Image Libraries, September 2025 | `S_OK`, full validation |
| Kernel synchronization enabled, fake clock | Relative wait ~200 ms, absolute wait ~0 ms, exit 1 |
| Patched server with `WINE_DISABLE_NTSYNC=1`, fake clock | Relative wait ~200 ms, absolute wait ~199 ms, exit 0 |

These are focused compatibility checks, not proof that Inventor installation
or the application itself works. See [ISSUES.md](../ISSUES.md) for remaining
installation issues and the local evidence paths.

## Service detection and archive extraction

`go-service/` uses the same `golang.org/x/sys/windows/svc` version as the
Autodesk Licensing Service. Build inside that directory with
`GOOS=windows GOARCH=amd64 go build -o probe.exe .`. Register the executable
as the demand-start `InventorProbe` service in an **isolated test prefix**.
It records service detection and its parent's session in `C:\service-probe.txt`
and deliberately enters the dispatcher even if detection is false. Stop the
service after observing RUNNING. Start the test server with `-p60` so it does
not shut down between separate `sc` commands.

The baseline reports `IsWindowsService=false`, parent session 1. The companion
server with `WINE_SERVICE_SESSION_ZERO=1` reports true, parent session 0.
The original CER and Licensing services both reached RUNNING with that setting
(`logs/autodesk-services-patched.txt`). This is a process-reporting workaround;
token session IDs and Wine's actual session isolation are unchanged.

The Electrical Catalog MSI was tested unmodified in the isolated prefix with
`ADSK_ODIS_SETUP=1`, `INSTALLDIR=C:\ElectricalProbe`, and
`CONTENTPATH=C:\ElectricalProbe`. Without `tar.exe`, its archive custom action
fails with return 1359 / MSI 1603. With the packaged Winelib archive bridge,
the MSI returns 0 and its 20 extracted files compare equal to independent
7-Zip extraction. A shell launcher named `tar.exe` can yield a misleading
successful MSI with no payload: install the ELF from `.#compat-tools` directly.

`logs/repair-tools-final-validation.txt` verifies the final packaged tools at
the July 2026 clock: both 200 ms waits pass, service detection succeeds,
archive contents match, and a missing archive produces a nonzero exit code.

## Large-package XML comparison

Using the same Core ADIX and full-validation probe at July 2026, the built-in
XML runtime remained busy after three minutes. With the Microsoft MSXML6
runtime installed by `prepare-msxml.sh` and `WINEDLLOVERRIDES='msxml6=n,b'`,
`CreatePackageReader` returned S_OK in 382 ms. The comparison used separate
prefixes and did not change package signatures or validation options. Evidence:
`logs/core-isolated-validation.log`, `logs/core-isolated-reader-backtrace.txt`,
and `logs/core-native-xml-validation.log`.
