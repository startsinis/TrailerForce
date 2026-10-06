#include "Engine.h"
#include "Profiles.h"
#include "Sound.h"
#include "Playback.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <limits>
#include <set>
void require(bool ok,const char* msg) {if(!ok)throw std::runtime_error(msg);}
int main() { try {
 auto s=tf::preset(0); auto b=tf::parseBrief("Horror in F# harmonic minor 97 BPM, 7/8, act one: 3 bars, act 2: 6 bars. piano metal. no breaks",s);
 require(b.settings.style==4 && b.settings.key==6 && b.settings.mode==tf::HarmonicMinor,"brief key/style");
 require(b.settings.bpm==97 && b.settings.numerator==7 && b.settings.denominator==8,"brief tempo/meter");
 require(b.settings.bars[0]==3 && b.settings.bars[1]==6 && !b.settings.breaks,"brief acts");
 require(tf::parseBrief("Bb minor",s).settings.key==10,"flat key");
 auto q=tf::generate(s); auto q2=tf::generate(s);
 require(!q.notes.empty() && q.notes.size()==q2.notes.size(),"determinism size");
 for(size_t i=0;i<q.notes.size();++i) require(q.notes[i].beat==q2.notes[i].beat && q.notes[i].pitch==q2.notes[i].pitch,"determinism values");
 for(int style=0;style<tf::styleCount;++style) for(int mode=0;mode<5;++mode) for(int meter:{3,4,7}) {
  auto p=tf::preset(style);p.mode=mode;p.numerator=meter;p.denominator=meter==7?8:4;
  auto seq=tf::generate(p); std::array<int,9> counts{};
  for(auto& n:seq.notes) {require(n.beat>=0 && n.length>0 && n.beat+n.length<=seq.beats+1e-8,"note bounds");require(n.pitch>=0 && n.pitch<=127 && n.velocity>0 && n.velocity<=127,"midi bounds");counts[n.lane]++;}
  for(int c:counts)require(c>0,"every lane has usable notes");
  auto ev=tf::events(seq); for(size_t i=1;i<ev.size();++i)require(ev[i].beat>=ev[i-1].beat,"event ordering");
 }
 s.humanize=0; auto clean=tf::generate(s);
 for(auto& marker:clean.markers) if(marker.text=="BREAK") for(auto& n:clean.notes) require(!(n.beat<marker.beat+1 && n.beat+n.length>marker.beat+.001),"break is silent");
 s.enabled[tf::Motif]=false;for(auto& n:tf::generate(s).notes)require(n.lane!=tf::Motif,"mute");
 s.fullArrangement=false;s.patternBars=3;require(tf::generate(s).beats==12,"pattern length");
 auto data=tf::midiFile(q);require(data.size()>100 && data[0]=='M' && data[9]==1 && data[11]==10,"SMF type1 tracks");
 auto single=tf::midiFile(q,tf::Bass);require(single[11]==2,"single lane export");
 s.bpm=std::numeric_limits<double>::quiet_NaN(); s.bars[0]=-5;require(tf::sanitise(s).bpm==120 && tf::sanitise(s).bars[0]==1,"bad state sanitised");
 for(int type=0;type<6;++type) {s=tf::preset(0);s.soundType=type;auto w=tf::renderWave(tf::soundGesture(s),s,tf::Design,8000);require(w.size()>44 && w[0]=='R',"WAV output");bool nonzero=false;for(size_t i=44;i<w.size();++i)nonzero|=w[i]!=0;require(nonzero,"sound is audible");}
 // Test block scheduling at exact loop boundaries for common buffer sizes.
 tf::Sequence loop;loop.beats=4;loop.notes={{0,4,60,100,1,0}};auto loopEvents=tf::events(loop);
 for(int block:{32,64,512,2048}) {
   int on=0,off=0;bool boundaryOff=false;
   // 0.5 beat/block ensures exact loop boundaries, independent of buffer size.
   for(int k=0;k<17;++k)tf::schedule(loopEvents.data(),loopEvents.size(),4,k*.5,.5/block,block,-1,[&](const tf::Event& e,int offset){
    require(offset>=0 && offset<block,"sample offset bounds");
    if((e.status&240)==0x90)++on;else {++off;if(k==8 && offset==0)boundaryOff=true;}
   });
   require(on==3 && off==3 && boundaryOff,"loop-boundary note-off");
 }
 int isolated=0;tf::schedule(loopEvents.data(),loopEvents.size(),4,0,.01,100,tf::Bass,[&](const tf::Event&,int){++isolated;});require(isolated==0,"solo scheduler filter");
 // Humanization may not leak into edit breaks.
 s=tf::preset(0);s.humanize=1;auto human=tf::generate(s);
 for(auto& marker:human.markers)if(marker.text=="BREAK")for(auto& n:human.notes)require(!(n.beat<marker.beat+1 && n.beat+n.length>marker.beat+1.e-8),"humanized break");
 // Genre controls must change the exported music, not just display text.
 auto hip=tf::parseBrief("Swagger hip-hop in C minor, half-time, swing and a second climax",tf::Settings{}).settings;
 require(hip.style==8 && hip.groove==2 && hip.swing>0 && hip.finalLift,"new genre brief");
 require(tf::parseBrief("true crime with a quiet outro",tf::Settings{}).settings.style==15,"crime brief");
 auto staged=tf::preset(0);staged.humanize=0;auto arranged=tf::generate(staged);
 for(auto& n:arranged.notes)if(n.beat<staged.bars[0]*staged.beatsPerBar())require(n.lane!=tf::Bass && n.lane!=tf::Percussion,"intro leaves rhythm space");
 staged.expression=true;auto express=tf::generate(staged);require(!express.controls.empty(),"expression curves exist");
 for(auto& c:express.controls)require(c.value>=0 && c.value<=127 && c.beat>=0 && c.beat<express.beats,"controller bounds");
 require(tf::events(express,tf::Chords).size()>tf::events(arranged,tf::Chords).size(),"CC exported with lane");
 staged.fullArrangement=false;staged.selectedAct=2;staged.button=false;staged.breaks=false;
 auto straight=tf::generate(staged);staged.swing=.3;auto swung=tf::generate(staged);bool timingChanged=false;
 for(size_t i=0;i<std::min(straight.notes.size(),swung.notes.size());++i)timingChanged|=straight.notes[i].beat!=swung.notes[i].beat;
 require(timingChanged,"swing changes note timing");
 staged.swing=0;staged.harmony=2;auto reharmonised=tf::generate(staged);bool chordChanged=false;
 auto original=tf::events(straight,tf::Chords),changed=tf::events(reharmonised,tf::Chords);
 for(size_t i=0;i<std::min(original.size(),changed.size());++i)chordChanged|=original[i].data1!=changed[i].data1;
 require(chordChanged,"harmony changes pitched chords");
 // Same-pitch notes cannot overlap after maximum humanization.
 for(auto& n:human.notes)for(auto& other:human.notes)if(&n!=&other && n.lane==other.lane && n.pitch==other.pitch && n.beat<other.beat)require(n.beat+n.length<=other.beat+1.e-8,"same pitch overlap");
 // Randomization must change actual composition with humanization disabled.
 auto gen=tf::preset(0);gen.humanize=0;gen.exploration=1;gen.fullArrangement=false;gen.selectedAct=2;gen.patternBars=8;gen.button=false;
 std::set<std::vector<uint8_t>> melodies,grooves,harmonies;
 for(int i=0;i<64;++i){auto result=tf::generate(gen);melodies.insert(tf::midiFile(result,tf::Motif));grooves.insert(tf::midiFile(result,tf::Percussion));harmonies.insert(tf::midiFile(result,tf::Chords));gen=tf::newVariation(gen);}
 require(melodies.size()>40 && grooves.size()>40 && harmonies.size()>20,"procedural musical diversity");
 gen.humanize=1;gen.ideaLocks={true,true,true};auto before=tf::midiFile(tf::generate(gen));auto locked=tf::newVariation(gen);
 require(before==tf::midiFile(tf::generate(locked)),"all locks preserve exact MIDI");
 gen.ideaLocks={false,false,false};gen.randomScope=3;auto rhythmOnly=tf::newVariation(gen);
 require(tf::midiFile(tf::generate(gen),tf::Motif)==tf::midiFile(tf::generate(rhythmOnly),tf::Motif),"rhythm scope preserves motif including performance");
 require(tf::midiFile(tf::generate(gen),tf::Chords)==tf::midiFile(tf::generate(rhythmOnly),tf::Chords),"rhythm scope preserves chords");
 require(gen.key==rhythmOnly.key && gen.bpm==rhythmOnly.bpm && gen.bars==rhythmOnly.bars,"randomization preserves brief constraints");
 require(tf::midiFile(tf::generate(rhythmOnly))==tf::midiFile(tf::generate(tf::newVariation(gen))),"variation recall deterministic");
 auto ccFile=tf::midiFile(express);if(auto f=std::ofstream("test-expression.mid",std::ios::binary))f.write(reinterpret_cast<const char*>(ccFile.data()),static_cast<std::streamsize>(ccFile.size()));
 if(auto f=std::ofstream("test-arrangement.mid",std::ios::binary))f.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size()));
 std::cout<<"PASS: brief parsing, 240 style/mode/meter combinations, determinism, bounds, breaks, mutes, MIDI and six audio gestures\n";
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;} }
