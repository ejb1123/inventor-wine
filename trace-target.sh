#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
source ./env.sh
source ./lib/observability.sh

if (($# == 0)); then
  printf 'Usage: %s WINDOWS_PROGRAM [ARG ...]\n' "$0" >&2
  exit 2
fi

export WINEDEBUG="${WINEDEBUG:-+timestamp,+pid,+tid,+service,+process,+loaddll,+seh,+registry,err+all,warn+all}"
obs_init trace-target "$0" "$@"
trap obs_exit_trap EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

strace -ff -ttT -s 256 -yy -o "$OBS_RUN_DIR/strace" \
  wine "$@" 2>&1 | tee "$OBS_RUN_DIR/wine.log"
