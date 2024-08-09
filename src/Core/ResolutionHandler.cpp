#include "Core/ResolutionHandler.hpp"

sf::Vector2u ResolutionHandler::currentResolution = {0, 0};
int ResolutionHandler::scale = 3;

void ResolutionHandler::setResolution(sf::Vector2u resolution)
{
    currentResolution = resolution;
}

const sf::Vector2u& ResolutionHandler::getResolution()
{
    return currentResolution;
}

void ResolutionHandler::changeScale(int amount)
{
    scale = std::min(std::max(scale + amount, 1), 5);
}

int ResolutionHandler::getResolutionIntegerScale()
{
    float resolutionXScale = currentResolution.x / 1920.0f;
    float resolutionYScale = currentResolution.y / 1080.0f;
    return std::max(static_cast<int>(resolutionXScale), static_cast<int>(resolutionYScale));
}