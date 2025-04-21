#pragma once

#include "GameConstants.hpp"

#if (!RELEASE_BUILD)

#include <map>

namespace DebugOptions
{

extern bool debugOptionsMenuOpen;

extern std::map<int, bool> tileMapsVisible;

extern bool drawCollisionRects;
extern bool drawChunkBoundaries;
extern bool drawEntityChunkParents;
extern bool godMode;
extern float godSpeedMultiplier;
extern bool limitlessZoom;
extern bool crazyAttack;

}

#endif