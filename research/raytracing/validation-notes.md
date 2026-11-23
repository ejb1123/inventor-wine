# Remaining Vulkan validation findings

Observed with Vulkan Validation Layers 1.4.341.0 during both successful rendering
and the broader coverage run. These are unresolved findings, not confirmed
causes of the earlier viewport freezes.

## External memory: 01742 and 01878

The local trace repeatedly imports format 97 images at 22,937,600 bytes after
exporting format 100 images at 11,468,800 bytes. Sometimes the import uses the
same integer FD as the preceding export, and validation associates it with the
smaller exported image. Both image creation and memory allocation return success.
All inspected NVIDIA FD identities have the same device/inode and zero size,
so `fstat` does not establish that these are the same external-memory payload.

The matching upstream validation source looks up opaque-memory metadata by
integer FD. Export records/overwrites that entry; successful Vulkan import
removes it. Ordinary `close` calls are not observed. Consequently, reuse after
an operation outside the layer's visibility is a plausible explanation, not a
proven diagnosis. The next useful test must distinguish payload identity across
Vulkan/OpenGL interop and FD close/reuse, rather than suppress these warnings.
Sources: [allocation checks](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/vulkan-sdk-1.4.341.0/layers/core_checks/cc_device_memory.cpp),
[FD tracking](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/vulkan-sdk-1.4.341.0/layers/state_tracker/state_tracker.cpp),
[FD lookup](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/vulkan-sdk-1.4.341.0/layers/state_tracker/state_tracker.h).

## Descriptor offsets: 08065

Validation reports offset binding on command buffers with no recorded descriptor
buffer binding. It occurs during raster initialization too. In vkd3d-proton
v3.0.1, `d3d12_command_list_update_descriptor_heaps` calls the descriptor-buffer
update function before setting offsets, but the actual bind is conditional on
`heap_dirty`. Source inspection alone does not identify which path loses that
state, or establish whether this is a tracking problem. Record bind/reset/offset
calls per command buffer before changing that logic.

Khronos documents that
[descriptor buffers must be bound before their offsets](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/descriptor_buffer.adoc).
A successful render does not make a validation violation harmless. No descriptor
or allocation workaround was introduced on the basis of these warnings.

## FD lifecycle capture — evening follow-up

A bounded `strace` capture during a live restore operation now establishes actual
FD close/reuse, rather than just repeated integer values in Vulkan logs:

| Relative time (ms) | Observed operation |
| --- | --- |
| 0.000 | Export 23,265,280-byte format-97 image as FD 417 |
| 0.269 | `close(417)` succeeds |
| 2.800 | Export 11,632,640-byte format-100 image as FD 417 |
| 3.378 | `close(417)` succeeds |
| 10.045 | `recvmsg` receives FD 417 through `SCM_RIGHTS` |
| 10.177 | Begin importing FD 417 for a format-97 image |
| 10.610 | Import allocation succeeds with size 23,265,280 |

Raw evidence: ignored `logs/raytracing/fd-resize.trace`, main-thread timestamp
1790124397.986488 as the origin. Additional ordinary file opens/closes reuse 417
between the export and receive. The preceding camera-only capture also repeatedly
closes/reuses FD 52 during successful imports.

This **confirms FD-number reuse across Wine IPC**. Combined with the validation
layer's integer-keyed map, it strengthens the stale-entry explanation for
01742/01878. It does not independently prove the payload identity received from
the Wine server or rule out other external-memory errors. No allocation sizes
were changed and no warnings were suppressed. Both tracers were detached after
the bounded captures.

## Descriptor follow-up: independent reproduction

The read-only descriptor tracer recorded a two-buffer bind followed by three-set
offset updates in the live app. On affected command buffers, a later offset
update emits 08065 despite the earlier bind (session `research-20260922-204821`).
This rules out interpreting the warning literally as proof that the app never
called the buffer bind. The tracer does not yet log classic-set binding calls.

A new [standalone reproducer](descriptor-rebind-repro.md) isolates the validation
state behavior without Wine or Inventor. On validation 1.4.341.0:

| Sequence | Errors before classic bind | Total errors | 08065 |
| --- | ---: | ---: | ---: |
| Buffer bind → offsets → classic set bind → offsets | 0 | 1 | 1 |
| Omit classic set bind | 0 | 0 | 0 |
| Rebind buffer after classic set bind | 0 | 0 | 0 |

The source clears the buffer-address binding array on classic-set binding. The
specification instead describes invalidating the offsets/sets. This strongly
indicates a validation tracking defect for the reproduced sequence, rather than
a missing initial bind. It is not proof that all application warnings or crashes
are harmless. No graphics rebinding workaround or validation suppression was
installed. The temporary extended layer was atomically replaced with the original
memory tracer for subsequent launches; the current process retains its mapped
read-only tracer until exit.

## Termination follow-up

The earlier launch's exit 143 still has no captured sender. The kernel journal
query for 20:19–20:22 returned no entries, and the inspected user journal did not
identify a kill source. The systemd-managed session survived subsequent camera
and resize captures. Its documents were checked saved before an intentional
native close for the descriptor-instrumentation restart. Do not classify that
intentional close as another failure. Root cause of the earlier exit remains open.
