# Direct3D 12 over Vulkan investigation

2026-09-22: **Inventor now renders a real model using DirectX 12 hardware rendering through vkd3d-proton.** The independent device probe and subsequent Inventor integration both passed. Long-session stability and large assemblies have not been tested.

## Measured results

- Host: NVIDIA GeForce RTX 3080, proprietary driver 595.99.02. `vulkaninfo` reports Vulkan 1.4.329 for this GPU.
- Same custom Wine staging 11.8 and custom wineserver as Inventor, but a fresh prefix under `fixtures/dx12-research/prefix`; neither Inventor prefix was changed by this investigation.
- Native vkd3d-proton 3.0.1 `d3d12.dll` + `d3d12core.dll`, DXVK 3.1.1 `dxgi.dll`: D3D12 feature-level 12.0 device creation succeeded, direct command queue and fence creation succeeded, GPU fence signal completed within 10 seconds (wait=0, completed=1). Exit status 0.
- With the same probe using Wine's builtin d3d12/d3d12core/dxgi: adapter enumeration succeeded, but D3D12 feature-level 12.0 device creation returned `0x80070057` (E_INVALIDARG). Exit status 1. This identifies a useful difference but does not establish the exact builtin failure cause.
- vkd3d-proton logs report shader model 6.8 and DX Ultimate support. These are reported capabilities, not coverage tests.

## Artifacts and reproduction

All downloads and generated files are ignored local fixtures:

- `fixtures/dx12-research/run-probe.sh`: creates a separate bubblewrap namespace with fresh prefix, private home, offline networking, X11/GPU access, read-only `/nix/store`, `/etc`, `/sys`, and `/run/opengl-driver`.
- `fixtures/dx12-research/probe.c` and `probe.exe`: tiny Windows x64 program, dynamically loads graphics DLLs, enumerates adapter, creates device/queue/fence and waits for signal. It does not render or present a swapchain.
- `fixtures/dx12-research/probe-session.sh`: session, with native overrides for d3d12/d3d12core/dxgi. Run `fixtures/dx12-research/run-probe.sh` from the project to repeat.
- `fixtures/dx12-research/probe-vkd3d-proton.log`, `probe-wine-builtin.log`, `vulkan-summary.txt`: preserved test evidence.
- `fixtures/dx12-research/vkd3d-proton-3.0.1/x64/`: D3D12 DLLs.
- `fixtures/dx12-research/dxvk-3.1.1/x64/`: DXGI and D3D11 DLLs, plus other DXVK components.
- `fixtures/dx12-research/{d3d12.dll,d3d12core.dll,dxgi.dll,probe.exe}`: self-contained probe directory contents. Copy into a separate directory in a target prefix and use `WINEDLLOVERRIDES='d3d12,d3d12core,dxgi=n'` for the probe process.

The probe explicitly selects the NVIDIA Vulkan ICD at `/run/opengl-driver/share/vulkan/icd.d/nvidia_icd.json`, with DXVK/VKD3D device-name filters. The sandbox exposes `/sys` read-only, the NVIDIA and DRI character devices, and the OpenGL driver path. X11/GPU access is shared with the host; this is not a VM security boundary.

Downloads came directly from official GitHub release assets, and their SHA256 hashes match the digests returned by the GitHub release API:

| Artifact | SHA256 |
| --- | --- |
| vkd3d-proton-3.0.1.tar.zst | `3cf2315522af5e43605ef6d3c41dad91387040bf97199934f3f7ab76caaa2f0c` |
| dxvk-3.1.1.tar.gz | `40565b4a724aadc4433fa4e010b4b23916d9b1f1baeee64e17186db94f54e608` |

The GitHub API digest verifies download integrity against that service; it is not an independently authenticated publisher signature.

## Inventor integration and limits

The initial integration used DirectX 11 through DXVK. An earlier DX12 setter
failed with `0x80020009`. Retrying with all documents closed succeeded:
`HardwareOptions.GraphicsDriverType=61449` (`kDirectX12GraphicsDriver`). After
restart, the setting persisted and diagnostics with a model open reported
`AIRViz Device Manager / VirtualDeviceDx12`, Hardware Renderer, and
`UseSoftwareGraphics=0`. The persisted Hardware Driver Type is now `0x208`.

Native vkd3d-proton 3.0.1 d3d12/d3d12core and DXVK 3.1.1 dxgi/d3d11 are beside
research Inventor.exe with application-specific overrides. The process maps
confirm the native D3D12 DLLs loaded. The original prefix is separate.

Opened the bundled bevel gear, changed its camera, saved a new copy as
`WineResearchDX12Gear-20260922-0514.ipt`, and reopened that copy successfully.
Evidence: `logs/dx12-integration/diagnostics-final.log`, `validate.log`,
`gear-first.png`, and `gear-reopened.png`. Session: `research-20260922-051055`.
This verifies a small model workflow, not large assemblies, performance,
ray tracing, or long-session stability. To return to DX11, select driver 61448
with documents closed and restart Inventor.

Home rendering was fixed separately with browser-only Windows 8 compatibility
and disabled browser GPU acceleration; see `HOME-BROWSER-RESEARCH.md`.
The white bottom strip is the native `msctls_statusbar32` control, not a failed
DX12 viewport (`logs/dx12-integration/windows-statusbar.log`).

## Primary references

[vkd3d-proton documentation](https://github.com/HansKristian-Work/vkd3d-proton/blob/master/README.md) describes the D3D12-over-Vulkan DLL interface, Vulkan 1.3 requirement, NVIDIA 535 minimum, and its use of DXVK's DXGI implementation. Our installed driver is newer than that minimum, and the local device test passes.

[vkd3d-proton releases](https://github.com/HansKristian-Work/vkd3d-proton/releases/tag/v3.0.1) and [DXVK releases](https://github.com/doitsujin/dxvk/releases/tag/v3.1.1) provide the tested binaries. [DXVK documentation](https://github.com/doitsujin/dxvk/blob/master/README.md) covers its Direct3D 8–11 Vulkan translation and DLL installation.

## GPU ray tracing follow-up

DXR 1.1 capability queries pass using these DLLs. The bundled Aurora renderer
can initialize in a standalone probe. Inventor's Hydra eligibility routine also
requires OpenGL 4.5 and fails its window pixel-format selection under Wine.
An experimental scoped RGB24 fallback passes that check. The subsequent freeze
was traced to invalid translated SPIR-V in Aurora's RayGenShader: a 12-component
vector and aggregate bitcasts. A diagnostic correction of two captured shader
variants passes spirv-val. With a matching override, an isolated Inventor run
rendered the gear on the GPU, survived resizing, returned to raster rendering,
and completed a second GPU render (`research-20260922-063207`).

This is **a successful sample test, not a general fix**: generated shader hashes
can change with material compilation, and the translator still needs a general
correction. Validation exposed additional Wine/device-query and resource issues
recorded in `ISSUES.md`. Shader overrides and validation are confined to
`fixtures/raytracing/run-isolated.sh`; `--debug-raytracing` alone only enables
the WGL eligibility workaround. Normal launches retain DX12 raster rendering
with GPU ray tracing disabled. Screenshots and logs are in `logs/raytracing/`.

Follow-up on 2026-09-22: the experimental hook now waits for Hydra's lazy load
without its former two-minute deadline. The loader checks that the DLL actually
loaded and reports failures. A model opened after 2:22 on Home rendered on the
GPU with the matching shader override. Verified binaries are installed in the
research prefix; originals are backed up under
`fixtures/raytracing/startup-backup-20260922`.

Repeated maximize/restore testing subsequently reproduced clipping, including
in a fresh raster-only session. The prior successful resize test does not mean
the layout issue is fixed. X11 shape extents remain at the restored size even
when Wine requests a correctly sized new surface. An extra surface-size check
did not fix it and is excluded from the normal build.

The separate `run-memory-test.sh` diagnostic also disables
`VK_EXT_zero_initialize_device_memory`: external-image initial-layout error
01443 disappears while the sample still renders. Memory import errors 01742
and 01878 remain under investigation because export descriptors are reused;
the observed numeric descriptor alone cannot identify the underlying allocation.
These diagnostic settings are not applied to the normal launcher.

The later first-flush shape fix (`0005-sync-x11-shape-on-first-flush.patch`)
passed nine repeated restore/maximize cycles in session
`research-20260922-075242`: three raster, three GPU-ray-tracing, and three KDE
window-state cycles. Compositor captures showed full viewport rendering; the
obsolete X11 shape was cleared. This fixes the reproduced clipping case, not
the outstanding shader translation or Vulkan validation issues.

`custom-winex11.nix` builds only the patched driver against the same Wine
derivation. The contained/menu launcher uses it when `result-winex11` exists,
and records its resolved path in each session's `x11-driver.txt`. Build it with
`nix build .#winex11 --out-link result-winex11` once the new Nix/patch files are
included in the flake's Git source. Removing that symlink restores the original
driver without altering the prefix. The installed driver was tested through
a private bind mount; the immutable Wine store path itself was not modified.
