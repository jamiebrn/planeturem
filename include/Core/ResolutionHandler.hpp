#pragma once

#include <SFML/Graphics.hpp>

class ResolutionHandler
{
    ResolutionHandler() = delete;

public:
    static void setResolution(sf::Vector2u resolution);
    static const sf::Vector2u& getResolution();

    static void changeScale(int amount);

    static inline int getScale() {return scale;}

    static inline float getTileSize() {return scale * tileSize;}

private:
    static sf::Vector2u currentResolution;

    static int scale;
    static constexpr float tileSize = 16.0f;
};