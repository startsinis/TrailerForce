#pragma once
#include "Engine.h"
namespace tf {
struct Profile {
 double bpm; int mode; double density, complexity;
 std::array<int,4> roots, motif;
 const char* rhythm; const char* kick; const char* snare;
 int harmonyBars; bool halfTime;
 const char* palette; const char* mood; const char* production; const char* source;
};
const Profile& profile(int style);
std::string productionNotes(const Settings&);
}
