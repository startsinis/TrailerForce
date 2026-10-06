#include "Generative.h"
#include <algorithm>
#include <random>
namespace tf {
static uint64_t mix(uint64_t x) {
 x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
 x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);
}
Settings newVariation(Settings s) {
 s=sanitise(s);++s.generation;
 for(int i=0;i<3;++i)if(!s.ideaLocks[size_t(i)] && (s.randomScope==0 || s.randomScope==i+1))
   s.ideaSeeds[size_t(i)]=uint32_t(mix(s.generation^(uint64_t(s.seed)<<32)^uint64_t(i+1)));
 if(!s.procedural)++s.seed;
 return s;
}
MusicalIdea designIdea(const Settings& s,const Profile& p) {
 MusicalIdea d;d.roots=p.roots;d.ostinato={0,4,2,4};
 for(int i=0;i<8;++i)d.melody[size_t(i)]=p.motif[size_t(i%4)];
 for(int i=0;i<32;++i){d.rhythm[size_t(i)]=p.rhythm[i%16];d.kick[size_t(i)]=p.kick[i%16];}
 if(!s.procedural)return d;
 std::mt19937 melody(s.ideaSeeds[0]),harmony(s.ideaSeeds[1]),rhythm(s.ideaSeeds[2]);
 auto chance=[](std::mt19937& g){return double(g())/4294967296.;};
 // Small connected intervals create an identifiable call, then a related answer.
 const int intervals[]{-2,-1,-1,1,1,2,3};int degree=0;
 for(int i=1;i<8;++i){
   if(i==4)degree=d.melody[0];
   degree=std::clamp(degree+intervals[melody()%7],-2,7);
   if(chance(melody)<s.exploration)d.melody[size_t(i)]=degree;
 }
 d.melody[7]=(melody()%2)?0:2; // answer lands on tonic or third
 d.motifCount=2+int(melody()%3);
 for(int i=1;i<4;++i)if(chance(melody)<s.exploration)d.ostinato[size_t(i)]=int(melody()%3)*2;
 // Keep tonic at the opening and favour functional/cinematic destinations.
 const int destinations[]{0,2,3,4,5,5,6};
 for(int i=1;i<4;++i)if(chance(harmony)<s.exploration)d.roots[size_t(i)]=destinations[harmony()%7];
 for(int i=1;i<32;++i){
   if(chance(rhythm)<s.exploration){
     const double weight=i%4==0?.85:i%2==0?.65:.25+.25*s.complexity;
     d.rhythm[size_t(i)]=chance(rhythm)<weight?(i%4==0?'X':'x'):'.';
   }
   if(i%16!=0 && chance(rhythm)<s.exploration*.6)
     d.kick[size_t(i)]=chance(rhythm)<(i%4==0?.55:.12*s.complexity)?'X':'.';
 }
 d.rhythm[0]='X';d.kick[0]=d.kick[16]='X';
 return d;
}
}
