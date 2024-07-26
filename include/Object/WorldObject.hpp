#pragma once

#include <SFML/Graphics.hpp>

class WorldObject
{
public:
    WorldObject(sf::Vector2f position) : position(position) {}
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window, float dt, const sf::Color& color) = 0;

    virtual void interact() = 0;

    virtual bool isAlive() = 0;

    inline sf::Vector2f getPosition() {return position;}
    inline void setPosition(sf::Vector2f pos) {position = pos;}

    inline int getDrawLayer() {return drawLayer;}

protected:
    sf::Vector2f position;
    int drawLayer = 0;

};