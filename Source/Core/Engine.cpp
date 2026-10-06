#include "Engine.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <regex>
#include <sstream>
#include <cctype>
namespace tf {
static std::string lower(std::string s) { for(auto& c:s) c=static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return s; }
static double finite(double x,double d) { return std::isfinite(x)?x:d; }
Settings sanitise(Settings s) {
  s.style=std::clamp(s.style,0,7); s.key=std::clamp(s.key,0,11); s.mode=std::clamp(s.mode,0,4);
  s.bpm=std::clamp(finite(s.bpm,120),30.,240.); s.numerator=std::clamp(s.numerator,1,12);
  if(s.denominator!=4 && s.denominator!=8) s.denominator=4;
  for(auto& b:s.bars) b=std::clamp(b,1,32);
  for(auto p:{&s.density,&s.complexity,&s.variation,&s.humanize,&s.atmosphere,&s.motion,&s.darkness,&s.soundIntensity,&s.climax}) *p=std::clamp(finite(*p,.5),0.,1.);
  s.soundLength=std::clamp(finite(s.soundLength,4),.25,16.);
  s.soundType=std::clamp(s.soundType,0,5); s.patternBars=std::clamp(s.patternBars,1,16);
  s.selectedAct=std::clamp(s.selectedAct,0,3); s.selectedLane=std::clamp(s.selectedLane,0,8); s.editEvery=std::clamp(s.editEvery,1,16);
  s.brief=s.brief.substr(0,16000); s.mood=s.mood.substr(0,256); s.instruments=s.instruments.substr(0,1024);
  return s;
}
Settings preset(int id) {
  Settings s; s.style=std::clamp(id,0,7);
  const double tempos[]{120,100,108,110,80,76,126,96}; s.bpm=tempos[s.style];
  const char* instr[]{"Low strings, brass, synth pulse, cinematic drums","Strings, horns, choir, timpani, cymbals","Piano, chamber strings, ticking percussion","Muted strings, sub bass, ticking pulse, metal hits","Clusters, bowed metal, drones, sparse impacts","Piano, solo cello, warm pads, soft percussion","Modular pulse, granular textures, distorted bass, metal drums","Piano hook, synth bass, drums, strings"};
  const char* moods[]{"Dark / urgent","Heroic / expansive","Driving / elegant","Tense / restrained","Unsettling / dread","Intimate / hopeful","Futuristic / relentless","Anthemic / bold"};
  s.instruments=instr[s.style]; s.mood=moods[s.style];
  if(id==1) { s.mode=Major; s.density=.65; }
  if(id==2) { s.complexity=.75; s.soundIntensity=.35; }
  if(id==3) { s.density=.4; s.darkness=.8; }
  if(id==4) { s.mode=Phrygian; s.density=.25; s.darkness=.95; s.soundType=5; }
  if(id==5) { s.mode=Major; s.density=.35; s.soundIntensity=.25; s.darkness=.2; }
  if(id==6) { s.complexity=.8; s.motion=.8; s.soundType=5; }
  if(id==7) { s.mode=Minor; s.density=.7; s.humanize=.2; }
  return s;
}
BriefResult parseBrief(const std::string& text, const Settings& previous) {
  auto s=previous; auto t=lower(text); std::smatch m; std::vector<std::string> found;
  int style=-1;
  for(auto pair:std::array<std::pair<const char*,int>,12>{{{"hybrid",0},{"action",0},{"epic",1},{"orchestral",1},{"neoclassical",2},{"neo-classical",2},{"thriller",3},{"horror",4},{"emotional",5},{"sci-fi",6},{"science fiction",6},{"pop",7}}})
    if(t.find(pair.first)!=std::string::npos) style=pair.second;
  if(t.find("hybrid")!=std::string::npos) style=0;
  if(style>=0) { auto p=preset(style); s.style=p.style; s.bpm=p.bpm; s.mode=p.mode; s.mood=p.mood; s.instruments=p.instruments; s.density=p.density; s.darkness=p.darkness; found.push_back("Style: "+std::string(styles[style])); }
  if(std::regex_search(t,m,std::regex(R"((\d{2,3}(?:\.\d+)?)\s*bpm\b)"))) { s.bpm=std::stod(m[1]); found.push_back("Tempo: "+m[1].str()+" BPM"); }
  if(std::regex_search(t,m,std::regex(R"(\b([a-g])\s*([#b]?)\s*(harmonic minor|minor|major|dorian|phrygian)\b)"))) {
    const std::string notes="c d ef g a b"; auto pos=notes.find(m[1].str()); s.key=static_cast<int>(pos);
    if(m[2]=="#") ++s.key;
    if(m[2]=="b") --s.key;
    s.key=(s.key+12)%12;
    const std::string mode=m[3]; s.mode=mode=="major"?Major:mode=="dorian"?Dorian:mode=="phrygian"?Phrygian:mode=="harmonic minor"?HarmonicMinor:Minor;
    found.push_back("Key/mode: "+m[0].str());
  }
  if(std::regex_search(t,m,std::regex(R"(\b([1-9]|1[0-2])\s*/\s*(4|8)\b)"))) { s.numerator=std::stoi(m[1]); s.denominator=std::stoi(m[2]); found.push_back("Meter: "+m[0].str()); }
  const char* words[]{"one","two","three","four"};
  for(int i=0;i<4;++i) if(std::regex_search(t,m,std::regex("act\\s*(?:"+std::to_string(i+1)+"|"+words[i]+")\\s*[:=,-]?\\s*(\\d+)\\s*bars?"))) { s.bars[i]=std::stoi(m[1].str().substr(0,3)); found.push_back("Act "+std::to_string(i+1)+": "+std::to_string(s.bars[i])+" bars"); }
  if(t.find("sparse")!=std::string::npos) s.density=.3;
  if(t.find("dense")!=std::string::npos || t.find("massive")!=std::string::npos) s.climax=1.;
  if(t.find("dark")!=std::string::npos) { s.mood="Dark / tense"; s.darkness=.8; }
  if(t.find("hopeful")!=std::string::npos) { s.mood="Hopeful / uplifting"; s.darkness=.2; }
  if(t.find("aggressive")!=std::string::npos) { s.mood="Aggressive / urgent"; s.soundIntensity=.95; }
  std::string named;
  for(auto word:{"piano","strings","brass","choir","drums","cello","synth","guitar","flute","metal","bass"})
    if(t.find(word)!=std::string::npos) { if(!named.empty()) named+=", "; named+=word; }
  if(!named.empty()) s.instruments=named;
  if(t.find("no breaks")!=std::string::npos) s.breaks=false;
  if(t.find("no button")!=std::string::npos) s.button=false;
  else if(t.find("button")!=std::string::npos) s.button=true;
  s.brief=text; s=sanitise(s);
  std::ostringstream report;
  report<<"Offline brief interpretation\n";
  for(auto& f:found) report<<f<<"\n";
  report<<"Mood: "<<s.mood<<"\nInstrumentation: "<<s.instruments<<"\n";
  report<<"Recognises style, mood, key/mode, BPM, meter and 'act 2: 8 bars'. Unrecognised instructions keep existing settings. Instrument names are orchestration notes; assign your own sounds to the MIDI lanes.";
  return {s,report.str()};
}
static const int scales[5][7]={{0,2,3,5,7,8,10},{0,2,4,5,7,9,11},{0,2,3,5,7,9,10},{0,1,3,5,7,8,10},{0,2,3,5,7,8,11}};
Sequence generate(const Settings& input) {
  auto s=sanitise(input); Sequence q; q.bpm=s.bpm; q.numerator=s.numerator; q.denominator=s.denominator;
  std::mt19937 rng(s.seed); auto random=[&](){ return double(rng())/double(std::mt19937::max()); };
  const double bpb=s.beatsPerBar(); int totalBars=0; for(int b:s.bars) totalBars+=b;
  if(!s.fullArrangement) totalBars=s.patternBars;
  q.beats=totalBars*bpb;
  auto pitch=[&](int degree,int octave) { int oct=degree/7; int d=degree%7; if(d<0){d+=7;--oct;} return std::clamp(12*octave+s.key+scales[s.mode][d]+12*oct,0,127); };
  auto add=[&](int lane,double beat,double length,int note,int velocity,bool exact=false) {
    if(!s.enabled[lane]) return;
    double jitter=exact?0.:(random()-.5)*.08*s.humanize;
    beat=std::clamp(beat+jitter,0.,q.beats-.01);
    length=std::clamp(length,.01,q.beats-beat);
    velocity=std::clamp(velocity+int((random()-.5)*18*s.humanize),1,127);
    q.notes.push_back({beat,length,note,velocity,channels[lane],lane});
  };
  int startBar=0;
  const int roots[4]={0,5,2,6};
  for(int ai=0;ai<(s.fullArrangement?4:1);++ai) {
    int act=s.fullArrangement?ai:s.selectedAct, nBars=s.fullArrangement?s.bars[ai]:s.patternBars;
    double start=startBar*bpb;
    q.markers.push_back({start,"ACT "+std::to_string(act+1)+(act==0?" / SETUP":act==1?" / BUILD":act==2?" / CLIMAX":" / RESOLUTION")});
    double energy=act==0?.35:act==1?.65:act==2?(.8+.2*s.climax):.5;
    double density=std::clamp(s.density*(.55+energy),.05,1.);
    for(int bar=0;bar<nBars;++bar) {
      double base=start+bar*bpb;
      bool last=bar==nBars-1;
      bool edit=((bar+1)%s.editEvery==0)||last;
      bool final=s.fullArrangement?ai==3 && last:last;
      double breakStart=(s.breaks && edit && !final)?base+bpb-std::min(1.,bpb*.25):base+bpb;
      if(edit) q.markers.push_back({base+bpb,"EDIT / "+std::to_string(startBar+bar+2)});
      if(breakStart<base+bpb) q.markers.push_back({breakStart,"BREAK"});
      int root=roots[(bar/2+(s.style==5?1:0))%4];
      if(final && s.button) {
        q.markers.push_back({base,"BUTTON ENDING"});
        for(int d:{0,2,4}) add(Chords,base,.8,pitch(d,4),108,true);
        add(Bass,base,.8,pitch(0,2),115,true); add(Percussion,base,.25,36,120,true);
        add(Design,base,std::min(s.soundLength,bpb*.8),36,115,true); continue;
      }
      auto clip=[&](double b,double len){ return std::max(.01,std::min(len,breakStart-b-.005)); };
      const double step=s.complexity>.7?.25:s.style==2?.25:.5;
      for(double b=0.;b<bpb-.001;b+=step) {
        double at=base+b; if(at>=breakStart-.01) continue;
        int index=int(std::round(b/step));
        if(random()<density) {
          int degree=root+(index%((s.complexity>.5)?6:4)==0?0:std::array<int,4>{0,4,2,4}[index%4]);
          if(random()<s.variation*.3) degree+=2;
          add(Ostinato,at,clip(at,step*.7),pitch(degree,4+(s.style==2)),int(55+energy*38)+(index%4==0?12:0));
        }
        if(index%2==0 && random()<density+.15) add(Pulse,at,clip(at,step*.55),pitch(root,3),int(55+energy*35));
        if(index%int(std::max(1.,1./step))==0) {
          int beat=int(b); add(Percussion,at,clip(at,.12),beat%2==0?36:38,int(65+energy*45));
          if(beat%2==0) add(Bass,at,clip(at,1.5),pitch(root,2),int(65+energy*35));
        } else if(act>=1 && random()<s.complexity*density) add(Percussion,at,clip(at,.08),42,int(40+energy*35));
      }
      for(int d:{0,2,4}) add(Chords,base,clip(base,bpb*.92),pitch(root+d,4),int(48+energy*35));
      const int motif[]{0,2,4,2,6,4,2,0};
      for(int n=0;n<2+(s.complexity>.7);++n) {
        double at=base+n*bpb/3.; if(at>=breakStart-.01) continue;
        if(act>0 || bar%2==0) add(Motif,at,clip(at,bpb/4.),pitch(motif[(bar*2+n+int(s.variation*4))%8],5),int(65+energy*30));
      }
      if(bar%2==0) {
        double dur=std::min(bpb*1.9,breakStart-base);
        add(Atmosphere,base,dur,pitch(0,3),int(25+65*s.atmosphere));
        if(s.darkness<.5) add(Atmosphere,base,dur,pitch(4,4),int(20+45*s.atmosphere));
        if(s.motion>.55) for(int n=1;n<4;++n) add(Atmosphere,base+n*bpb/4,clip(base+n*bpb/4,bpb/5),pitch(n%2?4:0,4),int(30+40*s.atmosphere));
      }
      if(last) {
        double len=std::min(bpb,s.soundLength); double at=base;
        q.markers.push_back({at,"TRANSITION"});
        int count=4+int(s.complexity*12);
        for(int n=0;n<count;++n) {
          double t=at+n*(breakStart-at)/count;
          add(Transition,t,clip(t,.18),pitch(n/2,4),std::min(120,50+n*4),true);
        }
        add(Design,at,std::min(len,breakStart-at),48,int(50+65*s.soundIntensity),true);
      } else if(bar==0) add(Design,base,std::min(s.soundLength,bpb*.85),24+s.soundType*12,int(50+65*s.soundIntensity),true);
    }
    startBar+=nBars;
  }
  for(auto& marker:q.markers) if(marker.text=="BREAK") {
    const double end=marker.beat+std::min(1.,bpb*.25);
    for(auto& n:q.notes) {
      if(n.beat>=marker.beat && n.beat<end) n.length=0;
      else if(n.beat<marker.beat && n.beat+n.length>marker.beat) n.length=marker.beat-n.beat;
    }
  }
  q.notes.erase(std::remove_if(q.notes.begin(),q.notes.end(),[](auto& n){return n.length<.005;}),q.notes.end());
  std::stable_sort(q.notes.begin(),q.notes.end(),[](auto& a,auto& b){return a.beat<b.beat;});
  std::stable_sort(q.markers.begin(),q.markers.end(),[](auto& a,auto& b){return a.beat<b.beat;});
  return q;
}
Sequence soundGesture(const Settings& input) {
 auto s=sanitise(input); Sequence q; q.bpm=s.bpm; q.numerator=s.numerator; q.denominator=s.denominator;
 q.beats=std::max(s.beatsPerBar(),s.soundLength+1.);
 q.notes.push_back({0,s.soundLength,24+s.soundType*12,int(45+80*s.soundIntensity),channels[Design],Design});
 q.markers.push_back({0,soundNames[s.soundType]}); return q;
}
std::vector<Event> events(const Sequence& s,int lane) {
 std::vector<Event> out; out.reserve(s.notes.size()*2);
 for(auto& n:s.notes) if(lane<0 || n.lane==lane) { out.push_back({n.beat,uint8_t(0x90+n.channel-1),uint8_t(n.pitch),uint8_t(n.velocity)}); out.push_back({n.beat+n.length,uint8_t(0x80+n.channel-1),uint8_t(n.pitch),0}); }
 std::stable_sort(out.begin(),out.end(),[](auto& a,auto& b){return a.beat==b.beat?a.status<b.status:a.beat<b.beat;}); return out;
}
std::string nextStep(const Settings& s,const Sequence& q) {
 std::ostringstream o;
 o<<"WHAT DO I DO NEXT?\n\n1. Establish the hook: solo Motif. Keep the strongest two bars and repeat with one change.\n\n";
 o<<(s.density>.75?"2. Leave room for dialogue: reduce Density or mute Pulse in busy scenes.":"2. Build contrast: add percussion and octave strings when Act II begins.");
 o<<"\n\n3. Climax: use Act III energy to lift velocity and density; export its chord, bass and percussion lanes to separate instruments.\n\n4. Edit: listen across every BREAK and TRANSITION marker. Leave the break clear for picture or dialogue.\n\n5. Finish: audition the button ending. Shorten instrument releases if the final hit needs a clean cut.\n\n";
 o<<q.notes.size()<<" editable MIDI notes / "<<int(q.beats/s.beatsPerBar())<<" bars.\n\nSuggested palette: "<<s.instruments<<".\n\nUse the MIDI FX version before your instrument in Logic, or drag each lane to its own DAW track. The internal synth is a sketching monitor, not an orchestral library.";
 return o.str();
}
}
