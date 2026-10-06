#pragma once
#include "Engine.h"
#include <algorithm>
#include <cmath>
namespace tf {
// Events at the loop end belong to the previous cycle and must precede the
// next cycle's note-ons, including when a block begins exactly on the boundary.
template<class Emit>
void schedule(const Event* data,size_t count,double length,double start,double beatsPerSample,int samples,int lane,Emit emit) {
 if(!data || length<=0 || beatsPerSample<=0 || samples<=0 || !std::isfinite(start) || std::abs(start)>1.e12)return;
 double end=start+samples*beatsPerSample;
 auto first=static_cast<int64_t>(std::floor(start/length))-1;
 auto last=static_cast<int64_t>(std::floor(end/length));
 for(auto cycle=first;cycle<=last && cycle<=first+3;++cycle) {
  double origin=double(cycle)*length;
  auto begin=std::lower_bound(data,data+count,start-origin,[](const Event& e,double beat){return e.beat<beat;});
  for(auto it=begin;it!=data+count;++it) {
   double at=it->beat+origin;if(at>=end)break;if(at<start)continue;
   if(lane>=0 && lane<laneCount && (it->status&15)+1!=channels[size_t(lane)])continue;
   int offset=std::clamp(int(std::floor((at-start)/beatsPerSample+1.e-7)),0,samples-1);
   emit(*it,offset);
  }
 }
}
}
