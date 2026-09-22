# Inventor on Wine: research and reproductions

This directory preserves our authored investigation tools so others can inspect,
repeat, and extend the work. The supported compatibility changes live in
[`../patches/`](../patches/) and [`../compat/`](../compat/). These research helpers
include incomplete and unsuccessful experiments; publishing a probe is not a
claim that it fixes the application.

## Findings worth building on

| Area | Finding and current limit | Starting point |
| --- | --- | --- |
| X11 maximize/restore | A replacement surface can retain its predecessor's X shape. First-flush synchronization fixed the reproduced clipping; nine isolated cycles plus a normal-launch check passed. | [Patch](../patches/0005-sync-x11-shape-on-first-flush.patch), [instrumentation](raytracing/trace-x11-shape.patch), [resize test](raytracing/check-resize-cycles.sh) |
| DX12/DXR | Native translation libraries exposed DXR 1.1 on the tested RTX 3080. Device capability queries are not proof of correct ray dispatch. | [Capabilities](raytracing/caps.c), [memory sharing](raytracing/sharing.c), [device/queue/fence](dx12-research/probe.c) |
| RayGen translation | Captured translation contained an illegal 12-component SPIR-V vector and aggregate bitcasts. Two exact shader variants rendered after experimental repairs; no general translator fix yet. | [Synthetic type reproducer](spirv/), [historical repair script](raytracing/fix-raygen-prototype.py), [findings](../DX12-RESEARCH.md) |
| Lazy initialization | The WGL hook stopped waiting before Hydra loaded when the user stayed on Home. A process-lifetime wait and verified injection passed delayed-open testing. | [Hook](../compat/inventor-raytracing.c), [loader](../compat/inventor-raytracing-loader.c) |
| External GPU memory | Disabling zero initialization removed one initial-layout validation error. Import warnings remain ambiguous because numeric descriptors are reused. | [Observational Vulkan layer](raytracing/memory-layer/), [issue record](../ISSUES.md) |
| Embedded Home browser | A scoped browser compatibility version and disabled browser GPU path restored Home on the tested setup. | [Browser findings](../HOME-BROWSER-RESEARCH.md), [CDP helpers](home-debug/) |
| Installer/services | MSIX verification, clock-sensitive waits, extraction, and service-session reporting were isolated with focused probes. | [Test guide](../tests/README.md), [development notes](../DEVELOPMENT.md) |

The recorded environment was Wine-staging 11.8, Inventor 2027, NixOS/KDE
Wayland with Xwayland, an NVIDIA RTX 3080, DXVK 3.1.1, and vkd3d-proton 3.0.1.
Behavior on other versions and drivers needs independent testing.

## Small standalone reproduction

Install SPIRV-Tools, then run from the repository root:

```sh
bash research/spirv/check.sh
```

This assembles an intentionally invalid vector type and verifies that validation
rejects it, then validates a 12-element array representation. Both modules are
synthetic. This isolates the type restriction; it does **not** reproduce Aurora's
full DXIL translation, aggregate-bitcast lowering, or rendered output. The
historical repair script requires your own matching disassembly and asserts
specific identifiers. It must not be applied blindly to other shaders.

## Compile the small graphics probes

With an x86_64 Windows MinGW compiler available:

```sh
mkdir -p fixtures/published-research-check
for probe in caps sharing; do
  x86_64-w64-mingw32-gcc -O2 -nostdlib -Wl,--entry,mainCRTStartup \
    "research/raytracing/$probe.c" \
    -o "fixtures/published-research-check/$probe.exe" -luser32 -lkernel32
done
x86_64-w64-mingw32-gcc -O2 -nostdlib -Wl,--entry,mainCRTStartup \
  research/raytracing/gl-probe.c \
  -o fixtures/published-research-check/gl-probe.exe \
  -lopengl32 -lgdi32 -luser32 -lkernel32
```

These commands were compile-checked when publishing the sources. Run the
executables in a disposable Wine prefix with your own graphics libraries. DX12
probes dynamically load `d3d12.dll` and `dxgi.dll`; record which versions actually
load. DLLs, SDK headers, Autodesk installation media, and sample models are not
bundled. The `dx12-research/` launcher is a historical NixOS/NVIDIA-specific
example, not a portable setup script.

## Historical experiments and dependencies

- `dx12-integration/` and `research-runtime/` use Inventor's COM automation API.
  Some create/save models, change preferences, or close documents/application.
  Inspect the selected helper and use disposable documents.
- `raytracing/` includes capability probes, rendering/eligibility experiments,
  exception instrumentation, and resize diagnostics. Injection experiments and
  hard-coded module offsets are specific to the tested application build.
- `home-debug/` includes CDP and native-window experiments. Some manipulate
  windows; the WebView2 probe needs separately obtained SDK headers.
- `dialog-style/` preserves appearance experiments; `compat/dialog-style.c`
  is the current versioned compatibility implementation.
- `cert-investigation/no-ntsync.c` is the earlier diagnostic interception
  experiment. The companion server patch supersedes it for the supported path.

Many historical scripts refer to ignored `fixtures/` paths, a particular host
PID, or Nix store paths. To create missing source links at their old locations:

```sh
python3 research/prepare-workspace.py
```

This never replaces existing files and does not download dependencies, build
executables, create licensing configuration, or start Inventor. Adapt the host
paths and compile the required helpers before running a historical session.
The Vulkan memory layer is diagnostic, retains bounded dispatch tables, and is
not intended as a production layer. Its manifest expects a locally built
`libmemorytrace.so` beside it.

[`source-manifest.json`](source-manifest.json) records the original local path
and SHA-256 of each preserved source. Raw desktop screenshots, full logs, Wine
prefixes, shader captures, third-party source checkouts, and confidential material
remain untracked. The committed [issue record](../ISSUES.md) preserves outcomes,
failed experiments, limitations, and local evidence filenames.

## Extending the work

Useful next steps are a general matrix/aggregate lowering fix in the shader
translator, an application-independent X11 shape regression test, and external
memory tracing that distinguishes actual payload identity from descriptor
numbers. Keep reduced reproductions independent of Autodesk binaries where
possible. Report exact versions, expected versus observed behavior, and whether
an experiment merely suppresses a diagnostic or demonstrably repairs rendering.
