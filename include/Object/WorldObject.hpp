#pragma once

#include <SFML/Graphics.hpp>
#include "TextureManager.hpp"
#include "Shaders.hpp"
#include "Inventory.hpp"
#include "TextureType.hpp"
#include "Camera.hpp"

class WorldObject
{
public:
    WorldObject(sf::Vector2f position) : position(position) {}
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    virtual void interact() = 0;

    virtual bool isAlive() = 0;

    inline sf::Vector2f getPosition() {return position;}

protected:
    sf::Vector2f position;

};