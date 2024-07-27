#include "Core/ResolutionHandler.hpp"

sf::Vector2u ResolutionHandler::currentResolution = {0, 0};

void ResolutionHandler::setResolution(sf::Vector2u resolution)
{
    currentResolution = resolution;
}

const sf::Vector2u& ResolutionHandler::getResolution()
{
    return currentResolution;
}