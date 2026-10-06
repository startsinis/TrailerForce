#include "Sound.h"
#include <algorithm>
#include <cmath>
namespace tf {
constexpr double pi=3.14159265358979323846;
void Sound::reset() { for(auto& v:voices) v={}; }
void Sound::message(uint8_t status,uint8_t pitch,uint8_t velocity) {
 int ch=(status&15)+1,type=status&240;
 if(type==0xb0 && (pitch==120 || pitch==123)) { for(auto& v:voices) if(v.channel==ch) v.active=false; return; }
 if(type==0x80 || (type==0x90 && !velocity)) { for(auto& v:voices) if(v.channel==ch && v.pitch==pitch) v.release=true; return; }
 if(type!=0x90) return;
 Voice* slot=nullptr;
 for(auto& v:voices) if(!v.active) {slot=&v;break;}
 if(!slot) slot=&*std::max_element(voices.begin(),voices.end(),[](auto& a,auto& b){return a.age<b.age;});
 *slot={true,false,ch,pitch,0,0,0,velocity/127.,0};
}
std::array<float,2> Sound::sample() {
 double left=0,right=0;
 for(auto& v:voices) if(v.active) {
  double freq=440.*std::pow(2.,(v.pitch-69)/12.); double attack=v.channel==7?.45:.008;
  if(v.release) v.level*=std::exp(-1./(sampleRate*(v.channel==7?.45:.08)));
  else v.level=std::min(1.,v.level+1./(sampleRate*attack));
  random^=random<<13;random^=random>>17;random^=random<<5;
  double noise=double(random)/2147483648.-1.; v.noise+=.08*(noise-v.noise);
  double wave=0;
  if(v.channel==10) {
   freq=v.pitch==36?48.+100*std::exp(-v.age*35):v.pitch==38?150:5000;
   wave=(v.pitch==36?std::sin(v.phase):noise)*std::exp(-v.age*(v.pitch==42?45:12));
   if(v.age>.8) v.active=false;
  } else if(v.channel==8) {
   int type=std::clamp((v.pitch-24)/12,0,5); double t=std::clamp(v.age/std::max(.1,gestureSeconds),0.,1.);
   switch(type) {
    case 0: freq=38.+25*std::exp(-v.age*4); wave=std::tanh(3*(std::sin(v.phase)+.35*std::sin(v.phase*1.006)))*std::exp(-v.age*.65); break;
    case 1: freq=35.+140*std::exp(-v.age*22); wave=(std::sin(v.phase)+noise*.65*std::exp(-v.age*14))*std::exp(-v.age*3); break;
    case 2: freq=80.*std::pow(28.,t); wave=(std::sin(v.phase)*.35+v.noise*2)*std::pow(t,.8); break;
    case 3: freq=45.+1600*std::pow(1-t,3); wave=(std::sin(v.phase)*.6+v.noise)*std::pow(1-t,.5); break;
    case 4: freq=200; wave=v.noise*4*std::pow(std::sin(pi*t),2); break;
    default: freq=52.+40*std::sin(v.age*3); wave=std::tanh(2*std::sin(v.phase+3*std::sin(v.phase*1.414)))*(.5+.5*std::sin(v.age*(7+20*motion)))*std::exp(-v.age*.4);
   }
   if(v.age>gestureSeconds+.5) v.active=false;
  } else if(v.channel==7) {
   wave=(std::sin(v.phase)+.3*std::sin(v.phase*1.003)+.18*(1-darkness)*std::sin(v.phase*3))*(.75+.25*std::sin(v.age*(.3+motion*6)));
  } else {
   wave=std::sin(v.phase)+.18*std::sin(v.phase*2)+.08*(1-darkness)*std::sin(v.phase*4);
   wave*=v.channel==4?.65:std::exp(-v.age*1.2);
  }
  v.phase=std::fmod(v.phase+2*pi*freq/sampleRate,2*pi); v.age+=1./sampleRate;
  double value=wave*v.level*v.velocity*.08;
  double pan=(v.channel==5 || v.channel==8 || v.channel==10)?0.:((v.pitch%7)-3)*.13;
  left+=value*(1-pan); right+=value*(1+pan);
  if(v.release && v.level<.0001) v.active=false;
 }
 return {float(std::tanh(left*.8)),float(std::tanh(right*.8))};
}
std::vector<uint8_t> renderWave(const Sequence& q,const Settings& s,int lane,double rate) {
 Sound synth; synth.prepare(rate); synth.configure(s.darkness,s.motion,s.soundLength*60./q.bpm);
 auto ev=events(q,lane); size_t cursor=0;
 auto frames=uint32_t(std::ceil((q.beats*60./q.bpm+1.5)*rate));
 std::vector<uint8_t> out; out.reserve(44+frames*4);
 auto text=[&](const char* t){for(int n=0;n<4;++n)out.push_back(uint8_t(t[n]));};
 auto le=[&](uint32_t v,int n){for(int i=0;i<n;++i)out.push_back(uint8_t(v>>(i*8)));};
 text("RIFF");le(36+frames*4,4);text("WAVE");text("fmt ");le(16,4);le(1,2);le(2,2);le(uint32_t(rate),4);le(uint32_t(rate)*4,4);le(4,2);le(16,2);text("data");le(frames*4,4);
 for(uint32_t i=0;i<frames;++i) {
  double beat=i/rate*q.bpm/60.;
  while(cursor<ev.size() && ev[cursor].beat<=beat) {auto& e=ev[cursor++];synth.message(e.status,e.data1,e.data2);}
  auto f=synth.sample(); for(float v:f) le(uint16_t(int16_t(std::clamp(v,-1.f,1.f)*32767)),2);
 }
 return out;
}
}
