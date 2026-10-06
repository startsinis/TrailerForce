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
// Reference metadata is taken from the user's saved reference report, not a transcription.
Settings applyReference(Settings s,int id) {
 id=std::clamp(id,0,16);s.referenceDirection=id;if(id==0)return s;
 const int stylesByRef[]{9,9,0,0,6,1,6,9,1,9,0,14,0,4,9,6};
 const int tempos[]{100,130,145,152,150,75,130,130,150,100,125,131,150,140,150,114};
 const int keys[]{7,4,4,4,2,4,2,0,2,7,4,2,2,5,0,4};
 auto base=preset(stylesByRef[id-1]);s.style=base.style;s.instruments=base.instruments;s.mood=base.mood;
 s.bpm=tempos[id-1];s.key=keys[id-1];s.mode=Minor;s.density=base.density;s.complexity=base.complexity;
 s.procedural=true;s.extendedIdeas=true;s.smartLayers=true;s.finalLift=true;s.harmony=0;s.groove=0;s.swing=base.swing;
 s.breaks=true;s.editEvery=(id==2 || id==10 || id==12)?2:4;
 s.soundType=(id==5 || id==8)?0:(id==14?5:1);s.darkness=.8;
 return sanitise(s);
}
static MusicalIdea designExtended(const Settings& s,const Profile& p) {
 MusicalIdea d;d.phraseBars=8;d.rootCount=8;
 const bool sparse=s.style==3 || s.style==4 || s.style==5 || s.style==10 || s.style==13 || s.style==15;
 const bool agile=s.style==2 || s.style==11;
 const bool riff=s.style==8 || s.style==9 || s.style==14;
 std::mt19937 m(s.ideaSeeds[0]),h(s.ideaSeeds[1]),r(s.ideaSeeds[2]);
 auto unit=[](std::mt19937& g){return double(g())/4294967296.;};
 const double freedom=s.exploration*(1.-.65*s.styleFidelity);
 d.motifCount=sparse?2:2+int(m()%3);
 // Each bar develops the initial hook: sequence, answer and resolution, with quantized spaces.
 for(int b=0;b<8;++b)for(int n=0;n<4;++n){
   int i=b*4+n,degree=p.motif[size_t(n)];
   if(b>0 && unit(m)<s.development)degree+=int(m()%3)-1;
   if(unit(m)<freedom)degree+=int(m()%5)-2;
   d.melody[size_t(i)]=std::clamp(degree,-2,sparse?4:7);
   d.onset[size_t(i)]=(n+((n>0 && unit(m)<s.development)? .25*int(m()%3):0.))/4.;
   d.duration[size_t(i)]=(sparse?.7:.4)+unit(m)*.35;
 }
 d.melody[31]=0;
 for(int i=0;i<4;++i)d.ostinato[size_t(i)]=riff?(i%2?0:4):int(m()%3)*2;
 d.ostinato[0]=0;
 for(int i=0;i<8;++i){
   int root=p.roots[size_t(i%4)];
   if(i>0 && unit(h)<freedom){
     if(sparse)root=(h()%3==0)?p.roots[size_t(h()%4)]:0;
     else root=p.roots[size_t(h()%4)];
   }
   d.roots[size_t(i)]=root;
 }
 d.roots[0]=0;
 for(int b=0;b<8;++b)for(int n=0;n<16;++n){
   int i=b*16+n;char c=p.rhythm[n],k=p.kick[n];
   // Protect strong genre anchors; phrase-end fills only occupy weak subdivisions.
   if(c!='X' && unit(r)<freedom){double rate=sparse?.14:agile?.8:riff?.38:.5;c=unit(r)<rate?'x':'.';}
   if(b%4==3 && n>=12 && c!='X' && unit(r)<s.development*(sparse?.15:.65))c='x';
   if(k!='X' && unit(r)<freedom*.5)k=unit(r)<(sparse?.08:.22)?'X':'.';
   d.rhythm[size_t(i)]=c;d.kick[size_t(i)]=k;
 }
 return d;
}
MusicalIdea designIdea(const Settings& s,const Profile& p) {
 MusicalIdea d;std::copy(p.roots.begin(),p.roots.end(),d.roots.begin());d.ostinato={0,4,2,4};
 for(int i=0;i<8;++i)d.melody[size_t(i)]=p.motif[size_t(i%4)];
 for(int i=0;i<32;++i){d.rhythm[size_t(i)]=p.rhythm[i%16];d.kick[size_t(i)]=p.kick[i%16];}
 if(!s.procedural)return d;
 if(s.extendedIdeas)return designExtended(s,p);
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
