#pragma once

#include <SFML/Graphics.hpp>

#include "GameConstants.hpp"

class ResolutionHandler
{
    ResolutionHandler() = delete;

public:
    static void setResolution(sf::Vector2u resolution);
    static const sf::Vector2u& getResolution();

    static void changeScale(int amount);
    static void changeZoom(int amount);

    static inline int getScale() {return (scale + currentZoom) * getResolutionIntegerScale();}

    static inline float getTileSize() {return getScale() * TILE_SIZE_PIXELS_UNSCALED;}

    // Get scale integer scale based on 1080p (used for UI etc)
    static int getResolutionIntegerScale();

private:
    static sf::Vector2u currentResolution;

    static int scale;

    static int currentZoom;
    static int destinationZoom;
};