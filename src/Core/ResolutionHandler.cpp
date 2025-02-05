#include "Core/ResolutionHandler.hpp"

sf::Vector2u ResolutionHandler::currentResolution = {0, 0};
int ResolutionHandler::scale = 3;
int ResolutionHandler::currentZoom = 0;

void ResolutionHandler::setResolution(sf::Vector2u resolution)
{
    currentResolution = resolution;
}

const sf::Vector2u& ResolutionHandler::getResolution()
{
    return currentResolution;
}

// void ResolutionHandler::changeScale(int amount)
// {
//     scale = std::min(std::max(scale + amount, 1), 5);
// }

void ResolutionHandler::changeZoom(int amount)
{
    int lowLimit = 0;
    int highLimit = 5;
    #if (!RELEASE_BUILD)
    if (DebugOptions::limitlessZoom)
    {
        lowLimit = -2;
        highLimit = 100000;
    }
    #endif

    currentZoom = std::min(std::max(currentZoom + amount, lowLimit), highLimit);
}

void ResolutionHandler::overrideZoom(int zoom)
{
    currentZoom = zoom;
}

int ResolutionHandler::getResolutionIntegerScale()
{
    float resolutionXScale = currentResolution.x / 1920.0f;
    float resolutionYScale = currentResolution.y / 1080.0f;
    return std::max(std::min(static_cast<int>(resolutionXScale), static_cast<int>(resolutionYScale)), 1);
}