#pragma once

#include <string>

#define RELEASE_BUILD 1

static const std::string GAME_TITLE = "Planeturem";
static const std::string GAME_VERSION = "release-v1.0.3";

static const std::string SOCIAL_YOUTUBE_URL = "https://www.youtube.com/@jamiebrn25";
static const std::string SOCIAL_DISCORD_URL = "https://discord.gg/JWYcTG2QME";
static const std::string SOCIAL_REDDIT_URL = "https://www.reddit.com/r/planeturem";

static constexpr int STEAM_APP_ID = 3323260;

static constexpr float TILE_SIZE_PIXELS_UNSCALED = 16;
static constexpr float CHUNK_TILE_SIZE = 8;
static constexpr int CHUNK_MAP_TILE_SIZE = 4;
static constexpr int CHUNK_VIEW_LOAD_BORDER = 1;
static constexpr int TILE_LIGHTING_RESOLUTION = 2;
static constexpr float LIGHTING_TICK_TIME = 0.1f;

static constexpr float SERVER_UPDATE_TICK = 1 / 45.0f;

static constexpr float PI = 3.14159265358979f;

static const std::string CLOCK_CRAFTING_STATION = "CLOCK_TIME";

#define BIT_MASK(bitLevel) (0xFFFFFFFFFFFFFFFF >> (64 - bitLevel))