# Trailer Force 0.2 — production research and design notes
Research checked 6 October 2026.

The revision uses published composer guidance and first-party music catalogs to inform **original generation rules**. It does not transcribe reference tracks, reproduce copyrighted hooks, identify a trailer's exact harmony/tempo, or analyse downloaded trailer audio. The BPM values, rhythms, voicings and production recipes below are starting points designed for this plug-in, not measurements of the linked references.

## Research translated into working features

| Evidence / reference | Design decision | What changes in the MIDI |
|---|---|---|
| [Evenant: 10 Essential Trailer Music Tips](https://www.evenant.com/articles/10-essential-trailer-music-tips) discusses edit-friendly sections, multiple climax stages and focused arrangements | Optional second climax and staged layer entrances | Intro excludes bass/drums; build introduces percussion later; final lift raises motif register in energetic styles |
| [Evenant: Hits, Ostinatos and Rhythms](https://www.evenant.com/articles/making-your-music-more-cinematic-hits-ostinatos-and-rhythms) discusses the relationship between repeating motion and a foreground melody | Recurring rhythmic cells and a persistent hook | Two-bar rhythm decisions repeat; motif changes are concentrated at phrase endings |
| [Universal Production Music: Neoclassical Trailer](https://www.universalproductionmusic.com/discover/albums/41480/neoclassical-trailer) documents the neoclassical trailer category | Dedicated neoclassical profile | Faster arpeggio cell, shorter harmonic cycle, piano-oriented register |
| [West One / Fired Earth: Champions](https://search.westonemusic.com/album/fem-053/champions) describes swaggering hip-hop with trailer backends | Swagger profile with a half-time pocket | Syncopated kick/bass placements, sparse stabs and a half-time snare |
| [Audiomachine: Here and Now](https://audiomachine.com/music/latest/here-and-now/) describes an organic, human approach with strings and synths | Emotional documentary profile | Sparse motion and a restrained final section by default |
| [Native Instruments: Jacob Yoffee interview](https://blog.native-instruments.com/jacob-yoffee-on-scoring-movie-trailers/) describes customising and combining acoustic and electronic sources | Production recipes for hybrid palettes | Separate rhythm, lead, sustain and effect lanes remain independently exportable |

The specific musical algorithms are our implementation choices. These sources do not endorse Trailer Force or establish a single mandatory trailer formula.

## Sixteen production starting points

| Profile | Default BPM | Rhythmic / harmonic identity | Production task |
|---|---:|---|---|
| Hybrid action | 120 | Accented short-note cell; minor 1–6–3–7 | Separate low ostinato, lead and impact sub |
| Epic orchestral | 100 | Broad theme; major 1–5–6–4 | Stage brass/choir layers and control deep voicings |
| Neoclassical | 112 | Flowing arpeggios; chord change each bar | Preserve piano detail and match string articulation |
| Dark thriller | 108 | Pedal harmony; half-time punctuation | Establish one signature ticking/metallic identity |
| Horror | 78 | Phrygian colour; widely spaced attacks | Contrast exposed texture, silence and shocks |
| Emotional | 76 | Broad piano hook; restrained rhythm | Shape long-note expression without flattening the opening |
| Sci-fi | 126 | Repeated asymmetric accent cell | Contrast stable sub against animated upper timbre |
| Trailer pop | 100 | Recognisable hook and backbeat | Build around the hook rather than adding competing hooks |
| Swagger / hip-hop | 92 | Half-time snare, syncopation and light swing | Lock bass to the kick; keep room between stabs |
| Industrial hybrid | 132 | Repeated low riff and mechanical accents | Keep a clear attack through distortion |
| Dark cover framework | 72 | Sparse original placeholder hook | Rebuild around a separately licensed song when appropriate |
| Fantasy adventure | 132 | Agile upper motion and bright arrivals | Let motion answer the broader theme |
| Comedy / heist | 112 | Short call-and-response cells with swing | Keep pauses available for picture and dialogue |
| Emotional documentary | 88 | Organic repeated pulse; soft final section | Use small voicing/register changes before adding drums |
| Trailer rock | 118 | Riff, kick/bass lock and backbeat | Treat the rhythm section as a band |
| Minimal crime | 96 | Pedal-based restrained motion | Leave room for factual dialogue and avoid an oversized ending |

Scale degrees describe roots within the selected mode; they do not prescribe major triads. The generator constructs diatonic triads and chooses nearby inversions. Genre defaults can be overridden with five explicit harmonic palettes, straight/half-time/triplet grooves, swing and gate controls.

## Listening references and exercises

These are verified public links for the user to inspect. No timestamped musical analysis is claimed.

- [Dune: Part Two — Official Trailer 2](https://www.youtube.com/watch?v=_YUzQa_1RCE): a reference viewing exercise for how large-scale sci-fi marketing handles dialogue, texture, scale and arrivals. Mark those functions while listening; create your own palette.
- [Mission: Impossible — The Final Reckoning, Paramount official page](https://www.paramountpictures.com/movies/mission-impossible-the-final-reckoning): map the trailer's pace and edit points, then consider how an original recurring rhythm could support that editorial shape.
- [Audiomachine trailer placements](https://audiomachine.com/trailers/): compare several campaigns and identify changes in musical function. This page also documents custom trailerizations and varied repertoire.
- [ALIBI: Royal Games](https://alibimusic.com/track/adventure-neoclassical-royal-games-full): a catalog reference for adventure/neoclassical instrumentation and descriptive tags.
- [Megatrax trailers](https://megatrax.com/music/trailers): browse the range of hybrid, orchestral and swagger-oriented trailer descriptions.
- [Spitfire / Marco Beltrami on A Quiet Place Part II](https://composer.spitfireaudio.com/en/articles/marco-beltrami-used-shepard-tones-to-create-tension-on-a-quiet-place-part-ii): a film-score tension reference; it is not evidence for a specific trailer cue or a claim that this plug-in implements Shepard tones.

## A practical production pass

1. **Hook:** solo the motif. Can you remember its shape after two bars? Revise the cell before adding more layers.
2. **Rhythm:** audition kick, bass and ostinato together. Decide which owns the low attack. Check articulation length against the tempo.
3. **Hierarchy:** separate foreground hook, rhythmic support and sustained background by register, level and note length.
4. **Development:** compare Setup, Build, Climax I and Climax II/Resolution. Each entrance should have a purpose.
5. **Dynamics:** enable CC1/CC11 only for compatible sustaining patches. These controllers affect Chords and Atmosphere, not every instrument. The sketch synth ignores them.
6. **Edits:** hear the break with real sample-library release tails and reverb enabled. MIDI silence alone cannot mute those tails.
7. **Delivery:** follow the actual client's requirements for duration, stems, alternate endings and loudness. This plug-in does not enforce a universal delivery standard or mastering target.

The built-in synth remains a sketch monitor. Realistic instruments, articulation selection, mix decisions and listening review are still part of production; these changes improve the underlying musical sketch rather than guarantee a finished commercial master.

## Brand X Music reference, October 2026

Official source: https://brandxmusic.bandcamp.com/album/chronos. Catalog identifies an orchestral motion-picture advertising library with hybrid, epic and trailer tags. Research was metadata-only; no audio was auditioned or transcribed. The new call/answer grammar, functional root choices and constrained rhythm mutation are original engineering choices, not measured attributes of this album.
