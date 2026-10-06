# Architecture

- `Source/Core/Engine.*`: independent C++17 musical model, presets, brief parsing, deterministic MIDI composition, markers and advice. No JUCE dependency.
- `Source/Core/Profiles.*`: sixteen musical profiles, pattern masks, harmonic rhythm, motif cells and research-linked production notes.
- `Source/Core/MidiFile.cpp`: Standard MIDI File type 1 encoder, 960 PPQ, conductor track, tempo/meter/markers, note-on/off ordering and per-lane export.
- `Source/Core/Sound.*`: bounded 96-voice procedural sketch synthesizer; six effect voices and offline WAV rendering.
- `Source/Plugin/Processor.*`: JUCE format adapter, host transport scheduler, MIDI passthrough, instrument audio, state serialization and parameter automation.
- `Source/Plugin/Editor.*`: native JUCE UI, eight workflow tabs, overview, native file drag/export and safe asynchronous file dialogs.
- `packaging`: macOS universal PKG and Windows x64 Inno Setup scripts.
- `tests`: portable engine invariants and export/audio checks.

## Threading

Generation runs outside the audio callback. A bounded event snapshot is published under a critical section. The audio callback uses only a try-lock; if the editor is publishing it continues with its previous snapshot. The audio-side snapshot never resizes. All expensive parsing, composition and file I/O stay outside the callback. Snapshot copying is bounded but still costs time on the callback; this is a first-release tradeoff. JUCE's host-provided MIDI buffer may allocate when growing. This is not a claim of a formally allocation-free real-time pipeline.

The synth voice pool is fixed. Host transport position determines sample-offset scheduling; a binary search skips events before the block. State changes, seeks, stops and solo changes flush notes. Incoming MIDI passes through. The generated sequence loops at its end, aligned to absolute host PPQ zero when synced; internal audition has its own beat clock. Notes are not chased when seeking mid-sustain.

## Extension points

Add a style through `preset`, add recognisers to `parseBrief`, add generators to `generate`, or replace the internal sketch synth without changing MIDI export. External articulation maps, MIDI learn, per-act lane overrides, swing grids, custom chord progressions, user preset files, arbitrary user-entered chord progressions, audio analysis and cloud models are not implemented in 0.2.0. Do not expose these as active product features until real implementations exist.

## Formats

Two JUCE targets avoid incompatible AU roles. `TrailerForce` is an instrument with MIDI input/output and an audio output bus (VST3 plus AU on macOS). `TrailerForceMidi` is a true `IS_MIDI_EFFECT` AU, built with no audio buses and a separate plug-in code. JUCE maps MIDI effects to Apple's `aumi` type; the regular synth uses `aumu`.

Reference: https://github.com/juce-framework/JUCE/blob/8.0.6/docs/CMake%20API.md and https://developer.apple.com/documentation/audiotoolbox/kaudiounittype_midiprocessor

## Procedural design

Generative.cpp separates melodic, harmonic and rhythmic design RNG streams. Settings store three seeds, design locks, scope, exploration and a 64-bit variation counter. newVariation hashes the counter and master seed into only the unlocked domain seeds. Engine uses independent per-lane performance RNGs in procedural mode so unrelated lane density cannot perturb a locked motif. Legacy mode retains the original sequential performance RNG. New state schema version 3 defaults old sessions to legacy mode.

## Engine schema 4

extendedIdeas selects the eight-bar grammar independently from the previous procedural switch. Old states default this flag off. Design arrays hold 32 melodic positions, eight roots and 128 rhythm/kick steps. Each domain retains its independent seed. All randomization stays on the message thread; audio scheduling is unchanged. Reference directions change starting constraints but never select recorded MIDI patterns.
