#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
research_tmp=$(mktemp -d)
trap 'rm -rf -- "$research_tmp"' EXIT
spirv-as --target-env vulkan1.3 vec12-invalid.spvasm -o "$research_tmp/invalid.spv"
if spirv-val --target-env vulkan1.3 "$research_tmp/invalid.spv" > "$research_tmp/invalid.log" 2>&1; then
  echo 'Unexpectedly accepted a 12-component vector' >&2
  exit 1
fi
cat "$research_tmp/invalid.log"
grep -Eq 'Illegal number of components|Invalid number of components' "$research_tmp/invalid.log"
spirv-as --target-env vulkan1.3 array12-valid.spvasm -o "$research_tmp/valid.spv"
spirv-val --target-env vulkan1.3 "$research_tmp/valid.spv"
echo 'Expected vector rejection and array validation both passed.'
