#pragma once

#include <string>

#define RELEASE_BUILD 0

static const std::string GAME_TITLE = "Planeturem";
static const std::string GAME_VERSION = "Demo";

static const std::string SOCIAL_YOUTUBE_URL = "https://www.youtube.com/@jamiebrn25";
static const std::string SOCIAL_DISCORD_URL = "https://discord.gg/JWYcTG2QME";
static const std::string SOCIAL_REDDIT_URL = "https://www.reddit.com/r/planeturem";

static constexpr int STEAM_APP_ID = 3323260;

static constexpr float TILE_SIZE_PIXELS_UNSCALED = 16;
static constexpr float CHUNK_TILE_SIZE = 8;
static constexpr int CHUNK_VIEW_LOAD_BORDER = 1;
static constexpr int TILE_LIGHTING_RESOLUTION = 3;
static constexpr float LIGHTING_TICK_TIME = 0.1f;

static const std::string CLOCK_CRAFTING_STATION = "CLOCK_TIME";