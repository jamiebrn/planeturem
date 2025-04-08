#pragma once

// #include <SFML/Graphics.hpp>

#include "Vector.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class ResolutionHandler
{
    ResolutionHandler() = delete;

public:
    static void setResolution(pl::Vector2<uint32_t> resolution);
    static const pl::Vector2<uint32_t>& getResolution();

    // static void changeScale(int amount);
    static void changeZoom(int amount);

    static void overrideZoom(int zoom);

    static inline int getScale() {return (scale + currentZoom) * getResolutionIntegerScale();}

    static inline float getTileSize() {return getScale() * TILE_SIZE_PIXELS_UNSCALED;}

    // Get scale integer scale based on 1080p (used for UI etc)
    static int getResolutionIntegerScale();

private:
    static pl::Vector2<uint32_t> currentResolution;

    static int scale;

    static int currentZoom;
};