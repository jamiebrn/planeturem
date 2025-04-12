#pragma once

#include <string>

#define RELEASE_BUILD 0

static const std::string GAME_TITLE = "Planeturem";
static const std::string GAME_VERSION = "alpha-v1.5";

static constexpr float TILE_SIZE_PIXELS_UNSCALED = 16;
static constexpr float CHUNK_TILE_SIZE = 8;
static constexpr int CHUNK_VIEW_LOAD_BORDER = 1;
static constexpr int TILE_LIGHTING_RESOLUTION = 3;
static constexpr int LIGHTING_TICK = 10;

static const std::string CLOCK_CRAFTING_STATION = "CLOCK_TIME";