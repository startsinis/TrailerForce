#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "Engine.h"
namespace tf {
// Allocation-free sketch synth. MIDI lanes remain usable with any external instrument.
class Sound {
public:
 void prepare(double rate) { sampleRate=rate; reset(); }
 void reset();
 void message(uint8_t status,uint8_t pitch,uint8_t velocity);
 void configure(double dark,double moving,double seconds) { darkness=dark; motion=moving; gestureSeconds=seconds; }
 std::array<float,2> sample();
private:
 struct Voice { bool active=false, release=false; int channel=0,pitch=0; double phase=0,age=0,level=0,velocity=0,noise=0; };
 std::array<Voice,96> voices{};
 double sampleRate=44100,darkness=.6,motion=.3,gestureSeconds=2;
 uint32_t random=0x12345678;
};
std::vector<uint8_t> renderWave(const Sequence&,const Settings&,int lane=-1,double rate=44100);
}
