# Aggregate bitcast lowering prototype

Status: **prototype tested in an isolated Inventor runtime; normal runtime unchanged**.
The patch contains no shader hashes or Autodesk shader code. It targets
[dxil-spirv](https://github.com/HansKristian-Work/dxil-spirv) commit
`c5e5522a121c5908a1c4fd472d68fa70c8fde9f4`.

## Failure and change

A pointer cast can cause a load/store to reinterpret a flattened floating-point
vector as a structure containing an array of row vectors. The converter remembers
the underlying allocation type but then emits a plain SPIR-V `OpBitcast` between
the loaded value and the aggregate. SPIR-V does not permit aggregate operands in
that instruction. The same issue can arise in direct value-cast lowering.

[`aggregate-bitcast.patch`](aggregate-bitcast.patch) recursively extracts scalar
lanes and reconstructs the destination's structure. Equal-width scalar lanes of
different numeric types use scalar bitcasts. Different lane counts/widths and
unsupported types fail explicitly. Pointer-cast handling remains unchanged.

An initial experiment converted all long vectors to arrays, which broke existing
long-vector arithmetic. That change was removed: this upstream revision enables `SPV_EXT_long_vector` for them. The final upstream patch changes only aggregate
cast emission in `opcodes/opcodes_llvm_builtins.cpp`.

## Reproduce without Inventor

Requirements: CMake, a C++ compiler, Python 3, DXC's development headers and
`libdxcompiler`, and dxil-spirv's pinned submodules. From an upstream checkout at
the commit above:

```sh
git submodule update --init --recursive
cmake -S . -B build-baseline -DCMAKE_BUILD_TYPE=Release
cmake --build build-baseline --target dxil-spirv -j4
# Use an absolute path to this repository's patch:
git apply /path/to/inventor-wine/research/translator/aggregate-bitcast.patch
cmake -S . -B build-patched -DCMAKE_BUILD_TYPE=Release
cmake --build build-patched --target dxil-spirv -j4
```

Build our small DXC assembler from the inventor-wine repository root. Set
`DXC_INCLUDE_DIR` to the SDK include directory containing `dxc/dxcapi.h`, and
`DXC_LIB_DIR` to the directory containing `libdxcompiler.so`:

```sh
mkdir -p fixtures/translator
c++ -std=c++14 -O2 -I"$DXC_INCLUDE_DIR" \
  research/translator/assemble-dxil.cpp \
  -L"$DXC_LIB_DIR" -Wl,-rpath,"$DXC_LIB_DIR" -ldxcompiler \
  -o fixtures/translator/assemble-dxil
python3 research/translator/check.py \
  --assembler fixtures/translator/assemble-dxil \
  --baseline /path/to/dxil-spirv/build-baseline/dxil-spirv \
  --patched /path/to/dxil-spirv/build-patched/dxil-spirv
```

If testing saved executables rather than separate build directories, use the
harness's `--baseline-library-dir` and `--patched-library-dir` options to ensure
each loads the intended `libdxil-spirv-c-shared.so.2`.

The assembler uses DXC's public `IDxcAssembler` interface to turn the self-authored
LLVM module into a DXIL container. The harness generates 2x3, 3x3, 3x4, and 4x4
variants. All four fail baseline SPIR-V validation on `OpBitcast` and pass with
the patch. An eight-to-twelve-lane reinterpretation is explicitly rejected.
The LLVM module uses uninitialized temporary storage: these are **validation
reproductions, not numerical-output tests or shaders intended for execution**.

## Recorded checks

Tested with DXC 1.10.2605.2 and the upstream commit above:

- Four synthetic matrix cases: baseline failure, patched success.
- Negative unequal-lane case: patched rejection with an explicit diagnostic.
- 82 upstream cases passed: `asm` (7), `long-vector` (14), `llvm-builtin` (45),
  and `alloca-opts` (16). Baseline also passed these groups.
- Two locally captured Aurora RayGen variants translated and passed the CLI's
  validation plus standalone `spirv-val --target-env vulkan1.3 --scalar-block-layout`.
  Captures and generated binaries remain outside Git.

Machine-readable synthetic and upstream-group results are in [`results/`](results/).
To repeat an upstream group, run in that checkout:

```sh
python3 test_shaders.py shaders --subfolder long-vector \
  --dxil-spirv ./build-patched/dxil-spirv --dxc /path/to/dxc
```

## What remains before deployment/upstream submission

Validate numerical behavior, including lane ordering, padding/layout assumptions,
and nested or mixed-type aggregates. Passing SPIR-V validation does not establish
semantic equivalence. The patch deliberately does not implement arbitrary
cross-width packing, all pointer reinterpretations, or all aggregate types.
The 4096-lane diagnostic limit also needs upstream review.

Integrate into a matching vkd3d-proton build and test Inventor with shader overrides
**disabled**, across materials, camera changes, resizing, and repeated renders.
The CLI uses its own descriptor mapping; its output is not a drop-in replacement
for an application pipeline. Keep the existing working runtime until that test
passes. No upstream issue/PR has been submitted by this investigation.

The older translator pinned by vkd3d-proton 3.0.1 does not emit that long-vector
extension. Runtime testing therefore also needs a separate representation fix
for wide matrix temporaries; the current-upstream validation result alone does
not establish that the older runtime can consume the output.

## Isolated runtime retry — 2026-09-22

Built vkd3d-proton v3.0.1 (`3b10bd7a7ec6a7347e616cf8bea59333afec2255`),
whose dxil-spirv submodule is `62dbb07f771534c8ce924479efdc6c8fa510361d`.
Apply `aggregate-bitcast.patch` in that submodule, then the separate
[`runtime-wide-temporaries.patch`](runtime-wide-temporaries.patch).
The latter globally represents vectors wider than four lanes as arrays. It is
an **experimental workaround for this workload**, not general wide-vector support:
wide-vector arithmetic still needs separate lowering. Do not apply it to current
upstream as a general fix; that approach failed the arithmetic regression tests.

The MinGW build initially failed because `-pthread` could not find `libpthread.a`.
Adding the Nix cross `windows.pthreads` library directory to both Meson
`c_link_args` and `cpp_link_args` resolved linking. The mcfgthread compiler also
needed its development include path and library directory. Both DLLs were built
with static runtime libraries; imported DLL inspection showed standard Windows
and DXGI dependencies, without an extra compiler-runtime DLL dependency.

Installed only in the isolated overlay, with shader overrides unset and
`VKD3D_SHADER_CACHE_PATH=0`. Maintenance7 and zero-initialized device memory
extensions remained disabled as in the diagnostic setup. Session
`research-20260922-110310` produced a visible GPU ray-traced gear, a fresh
RayGen shader, and 27 dumped SPIR-V modules that all passed Vulkan 1.3 validation
with scalar block layout. The loaded d3d12core file matched the candidate SHA256
`8288862f67c63c13dbf183209dd6996960c1bbce32a3d55a40a999ebdf3a74ec`.

Three restore/maximize cycles completed with ray tracing enabled: each ended at
2560x1368 with no X11 window/border shape, and compositor captures showed the
rendered viewport. A disable/re-enable render command also completed. The view
reported ray tracing enabled, progress 1, and paused after completion. GPU
utilization sampling occurred after completion and showed idle values; it does
not independently establish rendering activity. Fresh shader output, the GPU
preference/UI, and the rendered image are the evidence for this limited test.

Validation warnings 01742, 01878, and 08065 remain. This does not establish general
numerical correctness, material coverage, or long-running stability. The normal
runtime was not replaced. Raw captures and shader dumps remain ignored.

## Broader coverage — 2026-09-22 evening

Using disposable sample copies, the gear rendered with polished chrome, black
rubber, clear glass, and a wood texture. Six camera orientations completed with
GPU ray tracing enabled. NVIDIA samples during camera changes recorded up to
96% SM utilization. All 67 accumulated SPIR-V dumps passed validation.

The original tool-attached launcher exited 143 during the first assembly attempt.
No signal sender or shader crash backtrace was captured; do not label it fixed or
attribute it to the assembly. A retry launched through a systemd user service,
with signal tracing attached, rendered the bundled scissors assembly. It completed
18 camera changes and six restore/maximize pairs over roughly three minutes.
Final compositor output was intact, ray tracing remained enabled, and the app
answered status/document checks. RSS ranged from 2,713,972 to 2,734,868 KiB and
GPU memory from 1,041 to 1,075 MiB. These samples do not show sustained growth,
but this is not a long-duration leak test. Tracing also affects timing.

Warnings 01742, 01878, and 08065 persisted. See the
[warning investigation](../raytracing/validation-notes.md),
[coverage helper](../raytracing/coverage.md), and
[machine-readable results](results/runtime-coverage.json).
The isolated app was left open on the assembly and the tracer detached.
No normal-runtime deployment or public upstream submission was made.
