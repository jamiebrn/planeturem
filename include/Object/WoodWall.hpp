#pragma once

#include <SFML/Graphics.hpp>

#include "WorldObject.hpp"

class WoodWall : public WorldObject
{
public:
    WoodWall(sf::Vector2f position) : WorldObject(position) {}
    void update(float dt) override;
    void draw(sf::RenderWindow& window, float dt, float alpha) override;

    void interact() override;

    inline bool isAlive() override {return true;}

};