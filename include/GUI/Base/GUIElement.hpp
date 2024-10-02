#pragma once

#include <SFML/Graphics.hpp>

class GUIElement
{
public:
    GUIElement() = default;
    
    virtual void draw(sf::RenderTarget& window) = 0;

};