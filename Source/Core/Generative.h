#pragma once
#include "Engine.h"
#include "Profiles.h"
namespace tf {
struct MusicalIdea {
 std::array<int,8> melody{};
 std::array<int,4> roots{};
 std::array<int,4> ostinato{};
 std::array<char,32> rhythm{},kick{};
 int motifCount=3;
};
MusicalIdea designIdea(const Settings&,const Profile&);
}
