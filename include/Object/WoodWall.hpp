#pragma once

#include <SFML/Graphics.hpp>

#include "WorldObject.hpp"
#include "BuildableObject.hpp"

class WoodWall : public BuildableObject
{
public:
    WoodWall(sf::Vector2f position) : BuildableObject(position) {}
    void update(float dt) override;
    void draw(sf::RenderWindow& window, float dt, int alpha) override;
    void drawGUI(sf::RenderWindow& window, float dt, int alpha) override;

    void interact() override;

    inline bool isAlive() override {return true;}

};