#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

class WorldObject
{
public:
    WorldObject(sf::Vector2f position) : position(position) {}

    // General world object functionality

    inline sf::Vector2f getPosition() {return position;}
    inline void setPosition(sf::Vector2f pos) {position = pos;}

    inline int getDrawLayer() {return drawLayer;}

    // Overriden by inherited classes (specific)
    virtual void draw(sf::RenderTarget& window, float dt, const sf::Color& color) = 0;

protected:
    sf::Vector2f position;
    int drawLayer = 0;

};