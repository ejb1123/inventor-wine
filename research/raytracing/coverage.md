# Disposable-document ray-tracing coverage

`coverage.c` drives the running Inventor instance through COM. It never saves a
model. It changes cameras and appearances only when the active document's path
starts with `C:\ResearchUI\coverage\`; put **disposable copies** there, including
all referenced assembly parts. This path check is an accidental-use guard, not a
security boundary. Do not put valuable documents in that directory.

Build:

```sh
x86_64-w64-mingw32-gcc -O2 -nostdlib -Wl,--entry,mainCRTStartup \
  research/raytracing/coverage.c -o coverage.exe \
  -lole32 -loleaut32 -luuid -luser32 -lshell32 -lkernel32
```

Run inside the same Wine prefix/container as Inventor:

```text
coverage.exe open C:\ResearchUI\coverage\bevel_gear-1.ipt
coverage.exe assets
coverage.exe camera 10759
coverage.exe appearance 1 110
coverage.exe observe
```

`assets` lists library and appearance indices. The indices are installation
specific: use the list from the running application, not the example above.
`appearance` sets the document's ActiveAppearance directly to the selected
library asset. An earlier CopyTo experiment failed with DISP_E_EXCEPTION; direct
assignment succeeded. `observe` samples progress and pause state for 60 seconds.
The helper does not enable ray tracing: use Inventor's UI or `render.c` (which
**toggles** ray tracing, so check its current state before using it).

Camera values and methods follow Autodesk's
[orientation enumeration](https://help.autodesk.com/cloudhelp/2025/ENU/Inventor-API/files/ViewOrientationTypeEnum.htm)
and [ApplyWithoutTransition](https://help.autodesk.com/cloudhelp/2025/ENU/Inventor-API/files/Camera_ApplyWithoutTransition.htm).
Appearance assignment uses
[PartDocument.ActiveAppearance](https://help.autodesk.com/cloudhelp/2025/ENU/Inventor-API/files/PartDocument_ActiveAppearance.htm).

Check actual compositor screenshots, process responsiveness, shader validation,
and GPU activity alongside COM status. Progress=1 alone is not proof of a good
render. Keep raw screenshots, proprietary assets/shaders, and machine logs out of
Git. A public result summary belongs in the translator research results.

`coverage-soak.sh HOST_PID 18 OUTPUT_DIRECTORY` repeats four isometric camera
orientations and restores/maximizes every third iteration. Set `WINE_BINARY` to
the absolute Wine executable path available inside the target namespace. It
requires the coverage and layout helpers under `C:\ResearchUI`, rootless user
namespace entry through `sudo -n nsenter`, and an already enabled ray-tracing view.
Use an isolated instance: this visibly drives the desktop. Take compositor
screenshots separately; the script records commands and memory samples only.
