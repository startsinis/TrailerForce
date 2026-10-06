#include "Engine.h"
#include "Sound.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <limits>
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
 for(int style=0;style<8;++style) for(int mode=0;mode<5;++mode) for(int meter:{3,4,7}) {
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
 if(auto f=std::ofstream("test-arrangement.mid",std::ios::binary))f.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size()));
 std::cout<<"PASS: brief parsing, 120 style/mode/meter combinations, determinism, bounds, breaks, mutes, MIDI and six audio gestures\n";
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;} }
