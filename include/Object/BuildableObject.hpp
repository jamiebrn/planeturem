#pragma once

#include <SFML/Graphics.hpp>

#include "WorldObject.hpp"

class BuildableObject : public WorldObject
{
public:
    BuildableObject(sf::Vector2f position) : WorldObject(position) {}
    
    virtual void drawGUI(sf::RenderWindow& window, float dt, const sf::Color& color) = 0;
};