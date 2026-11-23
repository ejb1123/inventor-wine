# Möbius ribbon test model

`mobius.step` is a finite-thickness ribbon with one half-turn: centerline radius
60 mm, width 24 mm, and thickness 3 mm. `mobius-preview.svg` shows the exported
geometry. The zero-thickness mathematical Möbius surface is represented here by
a solid ribbon suitable for CAD and rendering tests.

Regenerate using Python with CadQuery 2.8.0 (tested with OCP 7.9.3.1.1):

```sh
python -m pip install cadquery==2.8.0
python research/models/mobius.py
```

The generator disables automatic loft wire matching to preserve the half-turn,
sews the endpoint edges, repairs solid orientation, and checks the result before
export. It then reads the STEP back and checks validity and solid count.
The tested output has one solid, four faces, and volume 27143.361 mm³.
These are Open CASCADE checks; they do not establish Inventor compatibility.

## Inventor/Wine status — 2026-09-24

**Not yet successfully imported or saved as an Inventor `.ipt`.**

In the isolated Inventor 2027/Wine 11.8 session, opening this STEP produced:

> Data Exchange Import: problems encountered while executing this command.

A separate 20 × 20 × 20 mm cube exported with CadQuery produced the same dialog.
This rules out the Möbius geometry being necessary to reproduce the failure;
the translator failure's cause is not yet established.

A separate native sketch/loft attempt stopped before creating geometry:
`PartDocument.ComponentDefinition` (DISPID 83886338) returned a `VT_I4` value
instead of an automation object. `DocumentType` returned 12290 as expected.
Querying the PartDocument interface and invoking from an injected worker thread
did not resolve it. Whether this is related to the translator failure is unknown.
The unfinished automation experiments remain in ignored fixtures rather than
being advertised as working modeling tools.

Local evidence is retained in ignored `logs/mobius-import.log`,
`logs/mobius-value.log`, `logs/mobius-control.log`, and
`logs/mobius-control.png`. Next investigate the cube import failure and the
unexpected automation return separately, then verify the Möbius model visually
and as one solid inside Inventor before saving an `.ipt` or testing ray tracing.
