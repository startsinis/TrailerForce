# Trailer Force by Vinci Sounds

Native JUCE trailer-composition plug-in. **Not a web app.** Generates playable, editable MIDI and synthesised sound-design gestures entirely offline. No account, API key or recurring service required.

## Formats

| Platform | Format | Purpose |
|---|---|---|
| macOS Intel + Apple Silicon | VST3 | MIDI generator with internal sketch synthesizer |
| macOS Intel + Apple Silicon | AU instrument | Sketch playback and MIDI file export |
| macOS Intel + Apple Silicon | AU MIDI FX | Insert before a software instrument in Logic |
| Windows x64 | VST3 | MIDI generator with internal sketch synthesizer |

## Included

- Large creative brief editor; deterministic keyword/rule parser for style, mood, BPM, key/mode, meter, act lengths and instrumentation notes.
- Four-act arrangements with transitions, edit points, silent MIDI breaks and button endings.
- Nine MIDI lanes: ostinato, pulse, motif, percussion, chords, bass, transitions, atmosphere and sound design.
- Key, five modes, tempo/host sync, density, complexity, variation, seeded regeneration and humanization.
- Type-1 MIDI export with separate named tracks, tempo, time signature and arrangement markers; individual lane export and native external file drag.
- Atmosphere drone/motion generator; six synthesised braam/impact/riser/downer/whoosh/signature gestures with WAV export.
- Climax builder, sixteen production profiles, contextual next-step guidance, native piano-roll overview, transport cursor, lane solo and panic.
- DAW state recall; automatable sketch monitor, monitor gain and DAW-play enable parameters.
- Universal Mac PKG and Windows Inno Setup EXE build workflows.

## Download / install

Open [GitHub Actions](https://github.com/startsinis/TrailerForce/actions), choose a **successful** build, and download the installer artifact for your OS. Successful main and tagged `v*` builds publish prerelease installers on [Releases](https://github.com/startsinis/TrailerForce/releases).

Installers are development builds: macOS bundles are ad-hoc signed, but the PKG is not Developer ID signed/notarized; Windows is not Authenticode signed. Platform trust prompts may appear. No claim of public-release certification is made.

Read [User Manual](docs/USER-MANUAL.md), [Build Guide](docs/BUILD.md), [Architecture](docs/ARCHITECTURE.md), and [Validation Checklist](docs/VALIDATION.md).

## Build

JUCE **8.0.6**, CMake >= 3.22, C++17. JUCE is fetched automatically, or supply `-DJUCE_SOURCE_DIR=/path/to/JUCE`.

```sh
# Engine tests without JUCE, SDKs or a GUI
cmake -S . -B build-core -DTF_CORE_ONLY=ON
cmake --build build-core
ctest --test-dir build-core --output-on-failure

# macOS universal
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13
cmake --build build --config Release --parallel 3
bash packaging/macos.sh build dist 0.2.0
```

This is a functional first release, not a pretrained AI assistant or a bundled orchestral sample library. Instrument names in a brief are orchestration notes, not automatic discovery/loading of commercial instruments. Host MIDI-output routing and drag behavior differ; MIDI file export is the portable workflow.

## Licensing

Project source copyright © 2026 Vinci Sounds. All rights reserved; no open-source license is granted for this project source. Third-party JUCE code remains under its own license. Before distributing a commercial build, the distributor must ensure their use complies with JUCE's applicable licensing terms; this repository does not grant a JUCE commercial license.

## 0.2 production revision

Sixteen distinct profiles now drive rhythmic cells, kick/bass placement, harmonic rhythm and recurring motifs. Chords use nearby inversions; short-note overlaps are removed. A new Production tab adds harmonic palettes, groove, swing, articulation gate, staged entrances, a second-climax option and optional CC1/CC11 curves for Chords/Atmosphere. New default arrangements use 8/16/16/8 bars.

[Production research and listening references](docs/PRODUCTION-RESEARCH.md) explains the first-party sources, original design choices and a practical DAW finishing pass. Existing projects regenerate under the revised engine; export important old MIDI before replacing 0.1.0 if exact reproduction matters.
