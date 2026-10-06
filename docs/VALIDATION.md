# Validation and release checklist

## Automated

- Portable parser checks: sharp/flat key, mode, BPM, meter, numeric and written act lengths.
- 240 style/mode/meter combinations with nonempty lanes, note bounds and valid MIDI values.
- Deterministic seeded composition, ordered events, note clipping through breaks, lane exclusion, pattern length, invalid-value sanitisation.
- MIDI type-1 header/track counts and individual-lane export; independent Python SMF decoding checks all track chunks, end positions, note-on/off pairs and markers; six non-silent procedural WAV gestures.
- Block scheduling regression checks at 32/64/512/2048 sample buffers, exact loop-end note-offs, solo filtering and humanized break boundaries.
- Native compilation in GitHub Actions for Windows VST3 and universal macOS VST3/AU/AU MIDI FX.
- auval for AU instrument and AU MIDI FX; universal-slice verification and ad-hoc signature checks.

## Manual DAW acceptance — required before production release

These checks cannot be inferred from a successful compiler or auval run. Record host/version/OS and result for each test.

- [ ] Logic Intel: insert MIDI FX before sampler, play all nine channels and solo lanes.
- [ ] Logic Apple Silicon native: scan both AUs, confirm MIDI FX registration.
- [ ] Windows VST3 host: instrument playback, live MIDI output routing where supported.
- [ ] Mac VST3 host: scan and route/import MIDI.
- [ ] Drag individual and multitrack MIDI, verify exact lengths and note-offs against exported files.
- [ ] Save and reopen project: identical seed, settings, brief and MIDI.
- [ ] Transport stop, start, loop, seek backwards, seek mid-note and tempo changes: no hanging notes.
- [ ] Buffer sizes 32, 64, 512 and 2048; 44.1, 48 and 96 kHz.
- [ ] Longest arrangement, all lanes, maximum density and humanize: responsiveness and dropout test.
- [ ] Multiple instances and open/close editor during playback.
- [ ] Verify audible gaps and button with actual sample-library release/reverb settings.
- [ ] Export all six WAV gestures and audition in a DAW.
- [ ] Install, upgrade and uninstall without disturbing other plug-ins or project files.
- [ ] Publisher-sign/notarize final deliverables and test on a clean machine.

0.2 regression coverage additionally checks staged entrances, new brief genres, same-pitch overlap removal, harmony and swing changes, controller bounds and independently decoded expression MIDI.
