# Investigation records and diagnostics

Every `wineboot.sh`, `run-installer.sh`, and `run-setup.sh` execution creates a
self-contained record below `logs/runs/`. The `logs/latest` symlink points to
the newest run. These records are deliberately ignored by Git.

Each record contains:

- `manifest.env`: operation, command, Git revision and dirty state, Wine build,
  Nix system, prefix, selected environment values, and installer hashes.
- `status.json` and `events.jsonl`: machine-readable state and event history.
- `wine.log`: complete console output from the launched Wine process.
- `progress.tsv`, `samples/`, and `latest-installer-progress.txt`: periodic
  resource, process, log-activity, and Autodesk progress samples.
- `prefix-before/`, `prefix-after/`, and `prefix-file-changes.diff`: bounded
  filesystem, Autodesk registry, installed-package, and service inventories.
- `autodesk-logs/`: Autodesk text logs changed during this attempt.
- `failures.txt`: a concise cross-log extraction of likely failure lines.
- `notes.tsv`: timestamped observations recorded by the operator.

## Trace levels

The default `normal` mode records Wine errors. Select a focused service trace
for the CER investigation:

```bash
INVENTOR_TRACE_MODE=service ./run-setup.sh
```

Available values are `normal`, `service`, `full`, and `custom`. `full` is very
large. With `custom`, set `WINEDEBUG` yourself:

```bash
INVENTOR_TRACE_MODE=custom \
WINEDEBUG='+timestamp,+pid,+tid,+service,+msi,+registry,err+all' \
./run-setup.sh
```

The sample interval defaults to 15 seconds and can be changed with
`OBS_SAMPLE_SECONDS`.

Inspect the latest run at any time without disturbing it:

```bash
./status.sh
```

Pass a run directory to inspect an older recorded attempt.

## Record observations

While or after an attempt, attach a durable note to the latest run:

```bash
./record-note.sh 'CER dialog displayed: service failed to start (1053)'
```

Running `./record-note.sh` without arguments reads one line from standard
input. Notes are stored in `notes.tsv` and included in support bundles.

## Machine diagnostic record

```bash
./diagnose.sh
```

This creates a normal run record plus CPU, memory, GPU, Vulkan, process, flake,
Wine store-path, prefix, registry, and service diagnostics.

## Trace one failing Windows program

Use this only after isolating a specific executable. It records focused Wine
channels and per-process Linux system calls:

```bash
./trace-target.sh 'C:\Program Files\Autodesk\Autodesk CER\service\cer_service.exe'
```

Raw `strace` output can include file contents and is intentionally omitted from
the shareable support bundle.

## Create a sanitized support bundle

Bundle the latest run:

```bash
./support-bundle.sh
```

Or select a run:

```bash
./support-bundle.sh logs/runs/20260921-120000-setup
```

The archive is written to `logs/`. The bundler excludes installers, prefixes,
binaries, registry hives, dumps, raw `strace`, and large files, and redacts
common credentials and home-directory names. Automated redaction is not a
guarantee: inspect every archive before sharing it publicly.
