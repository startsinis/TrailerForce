#pragma once
#include "Engine.h"
#include "Profiles.h"
namespace tf {
struct MusicalIdea {
 std::array<int,32> melody{};
 std::array<int,8> roots{};
 std::array<int,4> ostinato{};
 std::array<char,128> rhythm{},kick{};
 int motifCount=3,phraseBars=2,rootCount=4;
 std::array<double,32> onset{},duration{};
};
MusicalIdea designIdea(const Settings&,const Profile&);
}
