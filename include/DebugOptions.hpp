#pragma once

#include <map>

namespace DebugOptions
{

extern bool debugOptionsMenuOpen;

extern std::map<int, bool> tileMapsVisible;

extern bool drawCollisionRects;
extern bool drawChunkBoundaries;
extern bool drawEntityChunkParents;

}