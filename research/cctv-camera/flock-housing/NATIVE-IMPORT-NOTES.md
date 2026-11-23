# Native Inventor conversion — 2026-09-24

## Released result

26 native Inventor 2027 IPT documents are in outputs/inventor/parts.
All were saved/reopened, then exported by Inventor's STEP translator and independently
checked with CadQuery/OCC. Every part contains one valid solid, with matching
source bounds (within 0.01 mm) and volume (relative tolerance 1e-5).
Largest measured relative volume difference: 2.584e-8.
See outputs/native-parts-validation.json and validate-native.py.

These are imported solid parts. They support downstream Inventor modeling, but
do not reconstruct sketches, dimensions or an editable original feature history.
Their coordinates retain the source assembly positions. STL files remain the
print-oriented exports; reference parts are not intended to be printed.

## Import defect and workaround

The original STEP imports of 23_rail_clamp_-220_180 and
24_cross_rail_REFERENCE displayed “Data Exchange Import … Could not Insert Object.”
Inventor still allowed saving and reopening those documents. Exporting them back
to STEP revealed zero solids. Save/reopen alone is therefore an inadequate test.

Rebuilt equivalent geometry exported as AP203 without surface pcurves imported
successfully and passed the independent roundtrip checks. Those repaired IPTs
replace the empty documents in the released parts folder. Repair STEP sources
and the alternative-generator script are under import-repair/. The experiment
changed geometry construction and STEP encoding together; it does not isolate
which change avoided the defect.

## Assembly status: unresolved, excluded from release

The first camera IAM had 13 of 15 expected solids; the full site IAM had 25 of 26.
Referenced files resolved, but some were empty. Camera was missing upper body
and splice; site was missing upper body. Normal and locally centered hierarchical
AP203 retries also displayed import warnings. During the last retries the Inventor
process disappeared and automation calls failed with 0x800706be. The process exit
cause has not been established; overlapping outstanding import calls are a
confounding factor. Future retries must run one at a time after all dialogs close.

Incomplete IAMs are retained only in ignored fixtures/flock-incomplete-native
and the research Wine prefix, not in the deliverables. The original neutral STEP
assemblies remain available, but this Wine configuration has not imported them
reliably. No native IAM success is claimed.

Local evidence: logs/flock-native-repair203.log, logs/flock-roundtrip203.log,
logs/flock-assembly203.log, logs/flock-assemblylocal.log.
The native-*.c files are experimental probes with fixed research paths, not a
general unattended converter. native-convert.c's final message refers only to
API save/reopen success; always run geometry validation independently.
