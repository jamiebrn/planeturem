#include "DebugOptions.hpp"

bool DebugOptions::debugOptionsMenuOpen = false;

std::map<int, bool> DebugOptions::tileMapsVisible;

bool DebugOptions::drawCollisionRects = false;
bool DebugOptions::drawChunkBoundaries = false;
bool DebugOptions::drawEntityChunkParents = false;