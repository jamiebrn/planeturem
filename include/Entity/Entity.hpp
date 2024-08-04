#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Object/WorldObject.hpp"

#include "Data/EntityData.hpp"
#include "Data/EntityDataLoader.hpp"

class Entity : public WorldObject
{
public:
    Entity(sf::Vector2f position, unsigned int entityType);

    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;

private:
    unsigned int entityType;
    int health;

};