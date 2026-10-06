#include "Engine.h"
#include <algorithm>
#include <cmath>
namespace tf {
namespace {
void be(std::vector<uint8_t>& b,uint32_t v,int n) { for(int i=n-1;i>=0;--i) b.push_back(uint8_t(v>>(8*i))); }
void variable(std::vector<uint8_t>& b,uint32_t v) { uint8_t a[5]; int n=0; a[n++]=uint8_t(v&127); while((v>>=7)!=0) a[n++]=uint8_t((v&127)|128); while(n) b.push_back(a[--n]); }
void tag(std::vector<uint8_t>& b,const char* t) { for(int i=0;i<4;++i) b.push_back(uint8_t(t[i])); }
struct ME { uint32_t tick; int order; std::vector<uint8_t> bytes; };
std::vector<uint8_t> meta(uint8_t type,const std::string& str) { std::vector<uint8_t> v{255,type}; variable(v,uint32_t(str.size())); v.insert(v.end(),str.begin(),str.end()); return v; }
void track(std::vector<uint8_t>& out,std::vector<ME> ev,uint32_t end) {
 std::stable_sort(ev.begin(),ev.end(),[](auto& a,auto& b){return a.tick==b.tick?a.order<b.order:a.tick<b.tick;});
 std::vector<uint8_t> data; uint32_t time=0;
 for(auto& e:ev) { variable(data,e.tick-time); data.insert(data.end(),e.bytes.begin(),e.bytes.end()); time=e.tick; }
 variable(data,std::max(time,end)-time); data.insert(data.end(),{255,47,0});
 tag(out,"MTrk"); be(out,uint32_t(data.size()),4); out.insert(out.end(),data.begin(),data.end());
}
}
std::vector<uint8_t> midiFile(const Sequence& s,int lane) {
 constexpr double ppq=960.; auto tick=[](double beat){return uint32_t(std::max(0.,std::round(beat*ppq)));};
 std::vector<uint8_t> out; tag(out,"MThd"); be(out,6,4); be(out,1,2); be(out,lane<0?laneCount+1:2,2); be(out,960,2);
 uint32_t tempo=uint32_t(60000000./s.bpm);
 std::vector<ME> conductor{{0,0,meta(3,"Trailer Force / Vinci Sounds")},{0,1,{255,81,3,uint8_t(tempo>>16),uint8_t(tempo>>8),uint8_t(tempo)}},{0,2,{255,88,4,uint8_t(s.numerator),uint8_t(s.denominator==8?3:2),24,8}}};
 for(auto& m:s.markers) conductor.push_back({tick(m.beat),3,meta(6,m.text)});
 track(out,conductor,tick(s.beats));
 for(int l=0;l<laneCount;++l) if(lane<0 || lane==l) {
  std::vector<ME> ev{{0,0,meta(3,laneNames[l])}};
  for(auto& e:events(s,l)) ev.push_back({tick(e.beat),(e.status&0xf0)==0x80?1:2,{e.status,e.data1,e.data2}});
  track(out,ev,tick(s.beats));
 }
 return out;
}
}
