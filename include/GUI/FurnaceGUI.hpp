#pragma once

#include <SFML/Graphics.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextDraw.hpp"

class FurnaceGUI
{
    FurnaceGUI() = delete;

public:
    static void draw(sf::RenderTarget& window);

};