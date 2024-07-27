#pragma once

#include <SFML/Graphics.hpp>

class ResolutionHandler
{
    ResolutionHandler() = delete;

public:
    static void setResolution(sf::Vector2u resolution);
    static const sf::Vector2u& getResolution();

private:
    static sf::Vector2u currentResolution;
};