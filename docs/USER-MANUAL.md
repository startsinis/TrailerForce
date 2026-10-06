# Trailer Force by Vinci Sounds — User Manual
Version 0.2.0

## Install and open

Close your DAW, run the installer, then reopen the DAW and rescan plug-ins if needed.

- **Mac:** the universal PKG installs VST3 to `/Library/Audio/Plug-Ins/VST3` and both Audio Units to `/Library/Audio/Plug-Ins/Components`. It includes Intel and Apple Silicon architectures.
- **Windows:** the x64 installer installs to `C:\Program Files\Common Files\VST3\Trailer Force.vst3`.
- These are development installers without publisher trust certificates. Mac plug-in bundles have ad-hoc signatures; the package is not notarized. Only approve software you obtained from this repository. If a Mac blocks the installer, review it in System Settings > Privacy & Security. Do not disable system-wide security.

### Logic Pro: play your own sample instruments

1. Create a software instrument track and load your strings, brass, sampler or synth.
2. In that track's **MIDI FX** slot, select **Vinci Sounds > Trailer Force MIDI FX**.
3. Generate a pattern. Enable **DAW play**, start the Logic transport and listen through your instrument. Incoming MIDI is passed through.
4. Select a lane and enable **Solo selected lane** to send only that generated lane. Otherwise all enabled lanes reach the instrument on separate channels.
5. Drag MIDI from the plug-in into Logic's arrangement, or use Save MIDI and import the file. Use individual lanes for instruments that do not split channels.
6. After placing exported regions, disable **DAW play** to avoid doubling generated notes with the regions.

The regular **Trailer Force AU instrument** provides an internal sketch synthesizer and MIDI export. Use **Trailer Force MIDI FX** when you need live MIDI into another instrument on the same Logic track.

### VST3 in other DAWs

Load Trailer Force as an instrument. Its internal monitor makes the MIDI audible without another synth. To drive external sounds, route its MIDI output to a second instrument track if your DAW supports plug-in MIDI output. Exact routing depends on the host. Otherwise drag/save MIDI and put the resulting regions on your instrument tracks. Windows has VST3 only; AU is a macOS format.

## First cue in five minutes

1. Type: `Dark hybrid action in D minor, 120 BPM, 4/4. Act one: 4 bars, act two: 8 bars, act three: 8 bars, act four: 4 bars. Strings, brass and metal. Massive climax and a button ending.`
2. Click **Interpret Brief + Generate**. Read the interpretation below it.
3. In MIDI Designer, choose key/mode, density and complexity. Edits automatically regenerate after a brief pause; **Generate MIDI** applies immediately.
4. Click **Audition** while the DAW is stopped, or enable **DAW play** and start your DAW.
5. Choose **Motif** in the lane selector and Solo it. Change Variation or click **New Variation** until you have a useful hook.
6. Choose each lane and drag **Drag MIDI to DAW** onto its destination track. Dragging All Lanes exports a multitrack MIDI file; some hosts import it as multiple tracks, while others need File > Import.
7. Disable DAW play after importing. Replace the sketch sounds with your instruments.

## Creative Brief

The parser is local and rule based, with no online inference or API charges. It recognises:

- Styles: hybrid/action, epic/orchestral, neoclassical, thriller, horror, emotional, sci-fi, trailer pop, swagger/hip-hop, industrial, dark cover, fantasy/adventure, comedy/heist, documentary, trailer rock and minimal/true crime.
- Notes A–G with `#` or `b`, followed by minor, major, Dorian, Phrygian or harmonic minor; examples `F# harmonic minor`, `Bb minor`.
- A number followed by `BPM`, a time signature with denominator 4 or 8, and `act 2: 8 bars` or `act two: 8 bars`.
- Sparse, dense, massive, dark, hopeful, aggressive, no breaks, no button, button.
- Common instrument words as orchestration notes.

Style recognition applies that style's basic defaults; explicit key/tempo/meter override them. Unknown wording retains existing values. Merely typing a brief does not interpret it: click Interpret. The parser cannot reliably interpret arbitrary prose, picture timing, audio references, negative instrument requests or complex narrative directions. Instrument names do not load external plug-ins.

## MIDI Designer

| Control | Result |
|---|---|
| Key / mode | Changes pitched notes; percussion and sound trigger mapping stay fixed |
| BPM | Internal audition tempo and exported MIDI tempo |
| Sync to host BPM | Follows host BPM and quarter-note position during DAW playback |
| Density | Probability of rhythmic note generation, scaled by act energy |
| Complexity | Subdivision, motif count, fills and transition activity |
| Variation | Motif indexing and pitch changes within the same seed |
| Humanize | Bounded timing and velocity offsets; breaks stay clear |
| New Variation | Increments the deterministic seed and regenerates |
| Pattern bars / energy | Length and act character when Full Arrangement is off |
| Full Arrangement | Generates all four acts |
| Lane checkboxes | Include or exclude lanes from generation and exports |

No external instrument receives articulation keyswitches or proprietary expression maps. Assign articulations in your DAW. Regenerating or changing solo sends all-notes-off on all channels to avoid hanging notes; this also releases incoming live notes.

### MIDI lane/channel map

| Lane | MIDI channel | Suggested destination |
|---|---:|---|
| Ostinato | 1 | Short strings / plucked synth |
| Pulse | 2 | Pulsing synth / muted strings |
| Motif | 3 | Piano / brass / lead |
| Chords | 4 | Long strings / pad / choir |
| Bass | 5 | Low strings / synth bass |
| Transitions | 6 | Tuned percussion / rising synth |
| Atmosphere | 7 | Pad / drone / texture sampler |
| Sound design | 8 | Trigger-mapped effects sampler |
| Percussion | 10 | GM kick 36, snare 38 and hi-hat 42; remap to your library |

The internal sounds are a sketch monitor, not realistic orchestral samples. Monitor and gain affect internal audio only. The AU MIDI FX emits no audio.

## Structure

Set Act I–IV to 1–32 bars each. Act I introduces the hook, Act II raises energy, Act III reaches peak density/velocity, and Act IV can deliver a second climax or resolve, using the Production tab. Edit Every places MIDI markers at regular bar boundaries. Breaks reserve the final quarter-note beat (or a shorter fraction in short bars) before non-final edit points. All generated lanes are shortened to respect the break. External instrument releases or reverb can still ring.

Transitions occupy the final bar of each act up to the break. With Button Ending enabled, the final bar begins with a short chord/bass/percussion/impact hit and leaves the remainder open. Disable it for continuing patterns.

Exported files contain track names, tempo, time signature and marker meta-events. A DAW may choose not to show/import those metadata. Host sync never writes tempo or time signature back into the DAW. Match the DAW meter to the generated meter yourself.

## Atmosphere Designer

Atmosphere controls drone velocity; Motion adds upper-note pulses at higher values; Darkness changes open voicing and the internal synth's harmonic brightness. Select the Atmosphere lane and export it to your preferred pad or texture instrument. The internal atmosphere oscillator swells with slow amplitude motion.

## Sound Design Director

Six procedural sounds are available:

| Sound | MIDI note number | Character |
|---|---:|---|
| Braam | 24 | Low detuned saturated oscillator |
| Impact | 36 | Falling sine sub and noise transient |
| Riser | 48 | Rising oscillator and noise envelope |
| Downer | 60 | Descending oscillator with falling envelope |
| Whoosh | 72 | Filtered noise with a swell |
| Signature | 84 | Metallic frequency modulation and rhythmic amplitude |

Set Intensity and Length (quarter-note beats). **Generate Isolated Gesture** places one trigger in the preview and starts audition. **Export Gesture WAV** renders the chosen sound to a 16-bit, 44.1 kHz stereo WAV using the internal synth, independent of the DAW monitor volume. Generation is local. A full arrangement uses the selected gesture at act starts and risers at transitions. The final button uses an impact.

Pitch mapping is by MIDI note number because DAWs label octaves differently. For external instruments, assign your own samples to these notes. MIDI alone does not carry the generated waveform. Clicking Generate MIDI restores the arrangement. The isolated gesture is temporary and is not recalled after reopening the project.

## Climax and next steps

Build Climax Layers creates a focused Act III pattern with all lanes active and increased density/complexity. Pattern Bars controls its length. Re-enable Full Arrangement to return to the complete cue. The What Next tab provides guidance based on density, instrumentation and note count; it does not listen to or analyse the DAW mix.

## Playback, saving and recall

- **Audition:** loops from the start at the configured BPM when the host is stopped. Host transport takes priority while playing.
- **DAW play:** enables generation during host playback. With sync enabled, the arrangement repeats against the DAW's absolute quarter-note timeline, anchored at beat zero. Seeking/looping flushes notes; sustained notes begin again at their next note-on rather than being chased from a seek into the middle.
- **Stop / Panic:** stops audition, disables DAW play and releases notes. Re-enable DAW play when ready.
- **Solo selected lane:** affects live playback only, not which lanes are saved in All Lanes exports. The export dropdown selects the exported lane.
- DAW project state saves the brief, generation settings, seed and lane enables. Monitor, level and DAW-play enable are host-automatable. Other controls are editor settings, not automation parameters.
- Temporary MIDI files are kept in the OS temporary folder so the DAW can finish importing. Save important exported files to your project folder.

## Troubleshooting

**Silent?** Generate MIDI, enable DAW play or Audition, and check lane enables/solo. The regular instrument requires Sketch Sound on for internal audio. The MIDI FX needs a downstream instrument. An effect-trigger lane may be silent with a pitched instrument whose range excludes the trigger note.

**Doubled notes?** Turn off DAW play after importing regions. Do not play the exported MIDI and generator simultaneously unless intentional.

**Drag rejected?** Use Save MIDI and your host's MIDI import command. Some hosts do not support external file dragging from all plug-in windows.

**Wrong instruments?** The brief produces instrumentation notes; you must load and route your sample libraries. Percussion maps may need translation.

**Tempo mismatch?** Sync follows host tempo only during host playback with valid transport position. Exported files use the configured BPM. Set both explicitly when preparing files.

**Plug-in missing?** Check that you installed the right format, rescan, and check the DAW's plug-in manager. Logic's MIDI FX version appears in MIDI FX slots, not audio effect slots.

## Uninstall

Windows: Settings > Apps > Trailer Force by Vinci Sounds > Uninstall. Mac: run `/Library/Application Support/Vinci Sounds/Trailer Force/Uninstall-TrailerForce.command`, confirm and enter your administrator password. Both remove only installed application files; exports and DAW projects are preserved.

## Production tab — new in 0.2

- **Harmonic palette:** genre defaults or five root-degree progressions. Chord inversions minimise voice movement in a middle register; bass remains a separate root-based line.
- **Groove:** genre pocket, straight backbeat, half-time backbeat or triplet subdivision.
- **Swing:** delays alternating short-grid steps; disabled by the triplet grid.
- **Short-note gate:** changes ostinato/pulse note lengths for articulation matching.
- **Stage layer entrances:** leaves bass/drums out of the opening, introduces motion later and delays percussion during the build. Lane checkboxes still provide overall mutes.
- **Act IV: second climax:** raises final-section energy and lifts the motif register in energetic styles. Turn off for a quieter resolution.
- **CC1 + CC11:** sends/exports quarter-bar controller points for Chords and Atmosphere only. Enable only if your target instruments use modulation for dynamics and expression for level. Other lanes use note velocity. Controllers may remain latched in external instruments; override/reset them in your DAW as needed. The internal sketch sound does not render those CC curves.
- **Open Genre Reference:** opens the selected profile's public research/catalog link in your browser when clicked. Generation itself remains offline. The text panel contains a genre-specific production plan and can be scrolled.

Profiles now use persistent two-bar rhythm/hook cells, phrase-end variation, nearby chord inversions and kick-aligned bass. Default new cues are 8/16/16/8 bars (96 seconds at 120 BPM in 4/4). This is a starting length, not a required trailer duration.

Brief parsing also recognises half-time, triplet, straight, swing, second climax, final lift and quiet outro. Dark Cover Framework creates an original placeholder; it neither imports nor recreates a commercial song.

Existing projects retain their stored settings, with defaults for new controls, but regenerate using the revised engine. Export old MIDI before upgrading if exact old notes are needed. The AU/VST3 identifiers and installer locations remain the same.

Read [Production research and listening references](PRODUCTION-RESEARCH.md) for sources and practical finishing exercises.

## Procedural composition (0.3)

The Generative tab adds composition beyond the sixteen starting profiles. Enable Procedural composition, select a genre, then press NEW VARIATION. The generator designs an eight-note call/answer vocabulary, a four-chord journey, ostinato intervals and a two-bar rhythm cell. Notes remain in the selected mode, kick and bass share attacks, chord inversions minimise movement, and arrangement breaks remain clean.

Exploration controls how often the generator departs from the genre's vocabulary. It is not an audio similarity score. Melody length and answer resolution can vary even at zero exploration. All unlocked ideas, Melody + ostinato, Harmony, and Rhythm + bass attacks are separate randomization scopes. Locks prevent randomizing their design seeds. Changing key, genre, density, exploration or arrangement still changes the result; a harmony change can transpose bass and ostinato despite a melodic-design lock. Explicit harmony palettes in Production override generated chord roots.

Key, BPM, meter, act lengths, edit points and lane enables are never randomized. A/B: PREVIOUS VARIATION swaps the last two variation settings while the editor remains open; this comparison is not saved. The active design seeds, 64-bit generation counter, scope and locks save with the DAW session. Existing version 1/2 sessions load with procedural composition disabled to retain their old music. Enable it to adopt the new generator.

There is a large deterministic combination space, not a promise of literally infinite distinct or commercially finished compositions. Variations may repeat. Export the usable MIDI to your own instruments and develop the best hook. Sound-design synthesis and atmosphere controls remain separate from these three randomization domains.

The catalog button opens [Brand X Music — Chronos](https://brandxmusic.bandcamp.com/album/chronos). Its official catalog identifies orchestral advertising music and hybrid/epic trailer genres. This release used catalog metadata, not listening analysis or transcription; it contains no Brand X recordings, copied melodies, trained imitation model, or affiliation.
