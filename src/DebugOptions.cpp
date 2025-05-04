#include "DebugOptions.hpp"

bool DebugOptions::debugOptionsMenuOpen = false;

std::map<int, bool> DebugOptions::tileMapsVisible;

bool DebugOptions::drawCollisionRects = false;
bool DebugOptions::drawChunkBoundaries = false;
bool DebugOptions::drawEntityChunkParents = false;
bool DebugOptions::godMode = false;
float DebugOptions::godSpeedMultiplier = 1.0f;
bool DebugOptions::limitlessZoom = false;
bool DebugOptions::crazyAttack = false;

float DebugOptions::lightPropMult = 0.93f;