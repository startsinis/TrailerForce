#include "Engine.h"
#include "Profiles.h"
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
  s.style=std::clamp(s.style,0,styleCount-1); s.key=std::clamp(s.key,0,11); s.mode=std::clamp(s.mode,0,4);
  s.bpm=std::clamp(finite(s.bpm,120),30.,240.); s.numerator=std::clamp(s.numerator,1,12);
  if(s.denominator!=4 && s.denominator!=8) s.denominator=4;
  for(auto& b:s.bars) b=std::clamp(b,1,32);
  for(auto p:{&s.density,&s.complexity,&s.variation,&s.humanize,&s.atmosphere,&s.motion,&s.darkness,&s.soundIntensity,&s.climax}) *p=std::clamp(finite(*p,.5),0.,1.);
  s.harmony=std::clamp(s.harmony,0,5);s.groove=std::clamp(s.groove,0,3);
  s.swing=std::clamp(finite(s.swing,0),0.,.45);s.gate=std::clamp(finite(s.gate,.65),.2,1.2);
  s.soundLength=std::clamp(finite(s.soundLength,4),.25,16.);
  s.soundType=std::clamp(s.soundType,0,5); s.patternBars=std::clamp(s.patternBars,1,16);
  s.selectedAct=std::clamp(s.selectedAct,0,3); s.selectedLane=std::clamp(s.selectedLane,0,8); s.editEvery=std::clamp(s.editEvery,1,16);
  s.brief=s.brief.substr(0,16000); s.mood=s.mood.substr(0,256); s.instruments=s.instruments.substr(0,1024);
  return s;
}
Settings preset(int id) {
  Settings s; s.style=std::clamp(id,0,styleCount-1);const auto& p=profile(s.style);
  s.bpm=p.bpm;s.mode=p.mode;s.density=p.density;s.complexity=p.complexity;s.instruments=p.palette;s.mood=p.mood;
  s.darkness=(s.style==4 || s.style==9 || s.style==15)?.85:(s.mode==Major?.25:.6);
  s.soundIntensity=(s.style==5 || s.style==12 || s.style==13)?.3:.7;
  s.finalLift=s.style!=5 && s.style!=12 && s.style!=13 && s.style!=15;
  s.swing=(s.style==8 || s.style==12)?.16:0.;
  s.soundType=(s.style==6 || s.style==9)?5:0;
  return s;
}
BriefResult parseBrief(const std::string& text, const Settings& previous) {
  auto s=previous; auto t=lower(text); std::smatch m; std::vector<std::string> found;
  int style=-1;
  for(auto pair:std::array<std::pair<const char*,int>,12>{{{"hybrid",0},{"action",0},{"epic",1},{"orchestral",1},{"neoclassical",2},{"neo-classical",2},{"thriller",3},{"horror",4},{"emotional",5},{"sci-fi",6},{"science fiction",6},{"pop",7}}})
    if(t.find(pair.first)!=std::string::npos) style=pair.second;
  if(t.find("hybrid")!=std::string::npos) style=0;
  for(auto pair:std::array<std::pair<const char*,int>,12>{{{"swagger",8},{"hip-hop",8},{"hip hop",8},{"industrial",9},{"dark cover",10},{"trailer cover",10},{"fantasy",11},{"adventure",11},{"comedy",12},{"heist",12},{"documentary",13},{"trailer rock",14}}})if(t.find(pair.first)!=std::string::npos)style=pair.second;
  if(t.find("minimal crime")!=std::string::npos || t.find("true crime")!=std::string::npos)style=15;
  if(style>=0) { auto p=preset(style); s.style=p.style; s.bpm=p.bpm; s.mode=p.mode; s.mood=p.mood; s.instruments=p.instruments; s.density=p.density; s.darkness=p.darkness; s.complexity=p.complexity;s.swing=p.swing;s.finalLift=p.finalLift;s.soundIntensity=p.soundIntensity; found.push_back("Style: "+std::string(styles[style])); }
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
  if(t.find("half time")!=std::string::npos || t.find("half-time")!=std::string::npos)s.groove=2;
  if(t.find("triplet")!=std::string::npos)s.groove=3;
  if(t.find("straight")!=std::string::npos)s.groove=1;
  if(t.find("swing")!=std::string::npos)s.swing=.2;
  if(t.find("second climax")!=std::string::npos || t.find("final lift")!=std::string::npos)s.finalLift=true;
  if(t.find("quiet outro")!=std::string::npos)s.finalLift=false;
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
  const auto s=sanitise(input); const auto& pr=profile(s.style);
  Sequence q; q.bpm=s.bpm; q.numerator=s.numerator; q.denominator=s.denominator;
  const double bpb=s.beatsPerBar();
  int totalBars=0; for(int b:s.bars)totalBars+=b;
  if(!s.fullArrangement)totalBars=s.patternBars;
  q.beats=totalBars*bpb;
  std::mt19937 rng(s.seed);auto random=[&](){return double(rng())/double(std::mt19937::max());};
  // Rhythm choices repeat as two-bar cells. Humanisation never changes the cell.
  auto cellChance=[&](int step,int bar,int salt){uint32_t x=s.seed+uint32_t(step*137+(bar%2)*977+salt*3701);x^=x>>16;x*=0x7feb352d;x^=x>>15;return (x%10000)/10000.;};
  auto pitch=[&](int degree,int octave){int oct=degree/7,d=degree%7;if(d<0){d+=7;--oct;}return std::clamp(12*octave+s.key+scales[s.mode][d]+12*oct,0,127);};
  auto add=[&](int lane,double beat,double length,int note,int velocity,bool exact=false){
    if(!s.enabled[size_t(lane)])return;
    beat=std::clamp(beat+(exact?0.:(random()-.5)*.06*s.humanize),0.,q.beats-.01);
    length=std::clamp(length,.01,q.beats-beat);
    q.notes.push_back({beat,length,std::clamp(note,0,127),std::clamp(velocity+int((random()-.5)*14*s.humanize),1,127),channels[size_t(lane)],lane});
  };
  auto roots=pr.roots;
  const std::array<std::array<int,4>,5> harmonies{{{{0,5,2,6}},{{0,4,5,3}},{{0,3,5,4}},{{0,0,5,0}},{{0,6,5,6}}}};
  if(s.harmony>0)roots=harmonies[size_t(s.harmony-1)];
  std::array<int,3> previous{pitch(0,4),pitch(2,4),pitch(4,4)};
  auto voicing=[&](int root){
    std::array<int,3> best=previous;int score=100000;
    for(int inv=0;inv<3;++inv)for(int shift=-1;shift<=1;++shift){
      std::array<int,3> candidate{pitch(root,4),pitch(root+2,4),pitch(root+4,4)};
      for(int i=0;i<inv;++i)candidate[size_t(i)]+=12;
      std::sort(candidate.begin(),candidate.end());for(auto& n:candidate)n+=shift*12;
      if(candidate[0]<48 || candidate[2]>84)continue;
      int movement=0;for(int i=0;i<3;++i)movement+=std::abs(candidate[size_t(i)]-previous[size_t(i)]);
      if(movement<score){score=movement;best=candidate;}
    }
    previous=best;return best;
  };
  const bool restrained=s.style==4 || s.style==5 || s.style==10 || s.style==13 || s.style==15;
  const bool half=s.groove==2 || (s.groove==0 && pr.halfTime);
  const double grid=s.groove==3?1./3.:.25;
  int startBar=0;
  for(int section=0;section<(s.fullArrangement?4:1);++section){
    const int act=s.fullArrangement?section:s.selectedAct;
    const int nBars=s.fullArrangement?s.bars[size_t(section)]:s.patternBars;
    const double start=startBar*bpb;
    const char* role=act==0?"SETUP":act==1?"BUILD":act==2?"CLIMAX I":s.finalLift?"CLIMAX II":"RESOLUTION";
    q.markers.push_back({start,"ACT "+std::to_string(act+1)+" / "+role});
    for(int bar=0;bar<nBars;++bar){
      const double base=start+bar*bpb, progress=double(bar)/std::max(1,nBars-1);
      double energy=act==0?.28+.12*progress:act==1?.45+.22*progress:act==2?.7+.2*s.climax:(s.finalLift?.8+.2*s.climax:.42);
      double density=std::clamp(s.density*(.65+energy),.08,1.);
      const bool last=bar==nBars-1, final=s.fullArrangement?(section==3 && last):last;
      const bool edit=(bar+1)%s.editEvery==0 || last;
      const double cut=(s.breaks && edit && !final)?base+bpb-std::min(1.,bpb*.25):base+bpb;
      if(edit)q.markers.push_back({base+bpb,"EDIT / BAR "+std::to_string(startBar+bar+2)});
      if(cut<base+bpb)q.markers.push_back({cut,"BREAK"});
      const int root=roots[size_t((bar/pr.harmonyBars)%4)];
      auto clip=[&](double at,double len){return std::max(.005,std::min(len,cut-at-.005));};
      auto active=[&](int lane){
        if(!s.smartLayers)return true;
        if(act==0){
          if(lane==Percussion || lane==Bass || lane==Transition)return false;
          if(lane==Ostinato)return !restrained && bar>=nBars/2;
          if(lane==Pulse)return s.style==3 || s.style==6 || s.style==15;
        }
        if(act==1 && lane==Percussion)return bar>=nBars/4;
        return true;
      };
      if(final && s.button){
        q.markers.push_back({base,"BUTTON ENDING"});
        for(int note:voicing(0))add(Chords,base,std::min(.65,bpb*.45),note,105,true);
        add(Bass,base,std::min(.65,bpb*.45),pitch(0,2),112,true);
        add(Percussion,base,.15,36,restrained?80:118,true);
        add(Design,base,std::min(s.soundLength,bpb*.7),36,int(55+55*s.soundIntensity),true);continue;
      }
      const int steps=std::max(1,int(std::ceil(bpb/grid-1.e-8)));
      for(int step=0;step<steps;++step){
        const int mask=(s.groove==3?int(step*4./3.):step)%16;
        double at=base+step*grid+((step%2 && s.groove!=3)?s.swing*grid:0.);
        if(at>=cut-.01)continue;
        char rhythm=pr.rhythm[mask];
        bool hit=rhythm=='X' || (rhythm=='x' && cellChance(step,bar,1)<density);
        if(hit && active(Ostinato)){
          const int cell[]{0,4,2,4};
          int degree=(s.style==8 || s.style==9 || s.style==14)?root:root+cell[(step/2)%4];
          if(s.style==2 || s.style==11)degree=root+cell[step%4];
          if(bar%4==3 && s.complexity>.6 && cellChance(step,bar,7)<s.variation*.25)degree+=2;
          add(Ostinato,at,clip(at,grid*s.gate*(s.style==2?1.25:1.)),pitch(degree,s.style==2?5:4),int(43+energy*42)+(rhythm=='X'?12:0));
        }
        if(active(Pulse) && step%(half?4:2)==0 && (rhythm!='.' || step==0))add(Pulse,at,clip(at,grid*s.gate),pitch(0,3),int(42+energy*38));
        if(active(Percussion)){
          bool kick=pr.kick[mask]=='X' || (pr.kick[mask]=='x' && energy>.6);
          bool snare=s.groove==1?(mask==4 || mask==12):half?mask==8:pr.snare[mask]=='X';
          if(kick)add(Percussion,at,clip(at,.12),36,int(58+energy*45));
          if(snare)add(Percussion,at,clip(at,.1),38,int(54+energy*43));
          if(!restrained && step%2==0 && act>0 && cellChance(step,bar,2)<density)add(Percussion,at,clip(at,.06),42,int(35+energy*25)+(step%4==0?10:0));
        }
        if(active(Bass) && (pr.kick[mask]=='X' || step==0)){
          double next=bpb;for(int j=step+1;j<steps;++j){int m=(s.groove==3?int(j*4./3.):j)%16;if(pr.kick[m]=='X'){next=j*grid;break;}}
          add(Bass,at,clip(at,std::max(.05,next-(at-base)-.06)),pitch(root,2),int(60+energy*35));
        }
      }
      // A persistent two-bar hook: second phrase answers it; only phrase ends vary.
      if(active(Motif) && (act>0 || bar%2==0)){
        int count=act==0?2:bar%2==0?3:2;
        for(int n=0;n<count;++n){
          double at=base+n*bpb/(count+1);if(at>=cut-.01)continue;
          int degree=pr.motif[size_t((n+(bar%2)*2)%4)];
          if(bar%4==3 && n==count-1 && s.variation>.5)degree=2;
          int octave=act==3 && s.finalLift && !restrained?6:5;
          add(Motif,at,clip(at,bpb/(count+1)*.7),std::min(88,pitch(degree,octave)),int(57+energy*35)+(n==0?8:0));
        }
      }
      if(active(Chords))for(int note:voicing(root))add(Chords,base,clip(base,bpb*.92),note,int(40+energy*38));
      if(active(Atmosphere) && bar%2==0){
        add(Atmosphere,base,clip(base,bpb),pitch(0,3),int(22+55*s.atmosphere));
        if(s.darkness<.5)add(Atmosphere,base,clip(base,bpb),pitch(4,4),int(20+40*s.atmosphere));
        if(s.motion>.55)for(int n=1;n<4;++n){double at=base+n*bpb/4;if(at<cut-.01)add(Atmosphere,at,clip(at,bpb/6),pitch(n%2?4:0,4),int(25+40*s.atmosphere));}
      }
      if(last && active(Transition)){
        q.markers.push_back({base,"TRANSITION / FILL"});
        int count=4+int(s.complexity*8);double span=cut-base;
        for(int n=0;n<count;++n){double at=base+span*(1.-std::pow(1.-double(n)/count,.7));add(Transition,at,clip(at,.12),pitch(n/3,4),55+n*4,true);}
        add(Design,base,std::min(s.soundLength,span),48,int(48+62*s.soundIntensity),true);
      }else if(bar==0)add(Design,base,std::min(s.soundLength,bpb*.85),24+s.soundType*12,int(38+energy*60*s.soundIntensity),true);
      if(s.expression)for(int lane:{Chords,Atmosphere})if(s.enabled[size_t(lane)])for(int n=0;n<4;++n){
        double at=base+n*bpb/4;if(at>=cut)continue;
        q.controls.push_back({at,channels[size_t(lane)],1,int(35+energy*65+5*n),lane});
        q.controls.push_back({at,channels[size_t(lane)],11,int(72+energy*24+2*n),lane});
      }
    }
    startBar+=nBars;
  }
  for(auto& marker:q.markers)if(marker.text=="BREAK"){
    double end=marker.beat+std::min(1.,bpb*.25);
    for(auto& n:q.notes){if(n.beat>=marker.beat && n.beat<end)n.length=0;else if(n.beat<marker.beat && n.beat+n.length>marker.beat)n.length=marker.beat-n.beat;}
  }
  q.notes.erase(std::remove_if(q.notes.begin(),q.notes.end(),[](auto& n){return n.length<.005;}),q.notes.end());
  // Prevent overlaps on the same lane/pitch after jitter (important for short articulations).
  std::stable_sort(q.notes.begin(),q.notes.end(),[](auto& a,auto& b){return a.beat<b.beat;});
  std::array<std::array<int,128>,laneCount> lastNote;for(auto& l:lastNote)l.fill(-1);
  for(size_t i=0;i<q.notes.size();++i){auto& n=q.notes[i];int& last=lastNote[size_t(n.lane)][size_t(n.pitch)];if(last>=0){auto& prev=q.notes[size_t(last)];if(prev.beat+prev.length>=n.beat)prev.length=std::max(0.,n.beat-prev.beat-.002);}last=int(i);}
  q.notes.erase(std::remove_if(q.notes.begin(),q.notes.end(),[](auto& n){return n.length<.005;}),q.notes.end());
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
 for(auto& c:s.controls)if(lane<0 || c.lane==lane)out.push_back({c.beat,uint8_t(0xb0+c.channel-1),uint8_t(c.number),uint8_t(std::clamp(c.value,0,127))});
 std::stable_sort(out.begin(),out.end(),[](auto& a,auto& b){return a.beat==b.beat?((a.status&0xf0)==0x90?2:((a.status&0xf0)==0xb0?1:0))<((b.status&0xf0)==0x90?2:((b.status&0xf0)==0xb0?1:0)):a.beat<b.beat;}); return out;
}
std::string nextStep(const Settings& s,const Sequence& q) {
 std::ostringstream o;
 o<<"WHAT DO I DO NEXT?\n\n";
 o<<(s.smartLayers?"1. Audition Setup, then Build. Check that each new lane has a distinct job.":"1. Try Stage Layer Entrances to create more space in the opening.");
 o<<"\n\n2. Solo the motif. Keep its two-bar identity; use register, accents and phrase endings to develop it.\n\n";
 o<<(s.density>.75?"3. Density is high. Mute Pulse briefly and check whether the ostinato reads more clearly.":"3. Match short articulations to the Gate control. Check the kick and bass attacks together.");
 o<<"\n\n4. Listen across the final lift and button. Leave the edit clean and control sample-library release tails.\n\n5. Export lanes to your instrument tracks. Audition at low volume, compare balances, and check the brief's stem and ending requirements.\n\n";
 o<<q.notes.size()<<" editable notes. Suggested palette: "<<s.instruments<<".\n\n"<<profile(s.style).production;
 return o.str();
}
}
