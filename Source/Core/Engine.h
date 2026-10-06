#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace tf {
constexpr int laneCount = 9;
constexpr int styleCount = 16;
enum Lane { Ostinato, Pulse, Motif, Percussion, Chords, Bass, Transition, Atmosphere, Design };
enum Mode { Minor, Major, Dorian, Phrygian, HarmonicMinor };
inline constexpr std::array<int, laneCount> channels{1,2,3,10,4,5,6,7,8};
inline constexpr std::array<const char*, laneCount> laneNames{"Ostinato", "Pulse", "Motif", "Percussion", "Chords", "Bass", "Transitions", "Atmosphere", "Sound design"};
inline constexpr std::array<const char*, styleCount> styles{"Hybrid action", "Epic orchestral", "Neoclassical", "Dark thriller", "Horror", "Emotional", "Sci-fi", "Trailer pop", "Swagger / hip-hop", "Industrial hybrid", "Dark cover framework", "Fantasy adventure", "Comedy / heist", "Emotional documentary", "Trailer rock", "Minimal crime"};
inline constexpr std::array<const char*, 6> soundNames{"Braam", "Impact", "Riser", "Downer", "Whoosh", "Signature"};
struct Settings {
  int style=0, key=2, mode=Minor, numerator=4, denominator=4;
  double bpm=120, density=.6, complexity=.5, variation=.35, humanize=.12;
  uint32_t seed=42;
  std::array<int,4> bars{8,16,16,8};
  std::array<bool,laneCount> enabled{true,true,true,true,true,true,true,true,true};
  bool breaks=true, button=true, hostSync=true, fullArrangement=true;
  int selectedAct=1, selectedLane=0, patternBars=4, editEvery=4;
  double atmosphere=.35, motion=.3, darkness=.6, soundIntensity=.7, soundLength=4, climax=.8;
  int soundType=0;
  int harmony=0, groove=0;
  double swing=0, gate=.65;
  bool smartLayers=true, finalLift=true, expression=false;
  std::string brief="Dark hybrid action, D minor, 120 BPM. Sparse opening, rising tension, massive climax and a short button ending.";
  std::string mood="Dark / urgent", instruments="Low strings, brass, synth pulse, cinematic drums";
  double beatsPerBar() const { return numerator * 4.0 / denominator; }
};
struct Note { double beat=0, length=1; int pitch=60, velocity=90, channel=1, lane=0; };
struct Marker { double beat; std::string text; };
struct Control { double beat; int channel, number, value, lane; };
struct Sequence { std::vector<Control> controls; std::vector<Note> notes; std::vector<Marker> markers; double beats=16, bpm=120; int numerator=4, denominator=4; };
struct Event { double beat; uint8_t status, data1, data2; };
struct BriefResult { Settings settings; std::string report; };
Settings sanitise(Settings);
Settings preset(int);
BriefResult parseBrief(const std::string&, const Settings&);
Sequence generate(const Settings&);
Sequence soundGesture(const Settings&);
std::vector<Event> events(const Sequence&, int lane=-1);
std::vector<uint8_t> midiFile(const Sequence&, int lane=-1);
std::string nextStep(const Settings&, const Sequence&);
}
