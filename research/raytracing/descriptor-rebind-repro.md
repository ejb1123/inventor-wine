# Descriptor-buffer validation reproducer

This standalone C program records Vulkan commands. It uses no Wine, Autodesk
assets, shaders, draw calls, queue submissions, or proprietary code. It needs
Vulkan 1.3, VK_EXT_descriptor_buffer, buffer device addresses, and Khronos
validation. Tested with the NVIDIA RTX 3080 and validation 1.4.341.0.

```sh
cc -O2 descriptor-rebind-repro.c -o descriptor-rebind-repro -lvulkan
./descriptor-rebind-repro               # reproduced: exit 1, exactly one 08065
./descriptor-rebind-repro --no-classic  # control: exit 0, no validation errors
./descriptor-rebind-repro --rebind      # control: exit 0, no validation errors
```

Provide Vulkan headers/library paths if not in the compiler's defaults, and set
`VK_LAYER_PATH` to the installed Khronos validation manifest directory if needed.
Setup failures exit 2. Successful execution exits 1 if validation reported errors,
otherwise 0. Inspect the printed counts; exit 1 alone is not a passing reproduction.
All cases reported `errors_before_classic=0` on the test machine.

The default sequence is:

1. Bind a descriptor buffer.
2. Set its descriptor-set offset.
3. Bind a classic descriptor set using a separate compatible classic layout.
4. Set the descriptor-buffer offset again without rebinding the buffer.

`--no-classic` omits step 3. `--rebind` adds a buffer bind before step 4.
The recorder does not submit work, so uninitialized descriptors are never consumed.

The validation source at `vulkan-sdk-1.4.341.0` clears
`descriptor_buffer.binding_info` in `PostCallRecordCmdBindDescriptorSets` and its
Sets2 counterpart. The specification for
[vkCmdBindDescriptorSets](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdBindDescriptorSets.html)
describes invalidation of descriptor-set/offset/embedded-sampler bindings, rather
than the buffer-address bindings established by `vkCmdBindDescriptorBuffersEXT`.
This is strong evidence for a validation tracking defect in this sequence. It
requires upstream review; no validation-layer patch has been deployed or submitted.

The read-only `descriptor-layer.c` extends the existing local memory tracer with
begin/bind/offset logging, capped at 2,000 combined command events. Device and
command-buffer dispatch keys resolve the next function through the actual device.
Build it as a native shared library with Vulkan headers, `-fPIC -shared -pthread
-Wl,-Bsymbolic`, and use the existing memory-layer manifest in an isolated test.
Do not use capped logs to claim absence of calls after the cap is reached.

The live Inventor capture also records a buffer bind earlier in the same command
buffer before the 08065 warning. The standalone reproduction establishes the
validation behavior; the live tracer does not yet record classic-set calls, so
that intervening call is not independently captured in Inventor itself.
