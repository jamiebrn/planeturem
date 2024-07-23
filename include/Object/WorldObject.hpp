#pragma once

#include <SFML/Graphics.hpp>
#include "TextureManager.hpp"
#include "TextureType.hpp"
#include "Camera.hpp"

class WorldObject
{
public:
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

};