#pragma once

#include <SFML/Graphics.hpp>

#include "WorldObject.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

class BuildableObject : public WorldObject
{
public:
    BuildableObject(sf::Vector2f position, unsigned int objectType);

    inline void update(float dt) override;

    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;
    void drawGUI(sf::RenderWindow& window, float dt, const sf::Color& color);

    void interact() override;

    inline unsigned int getObjectType() {return objectType;}

    inline bool isAlive() override {return health > 0;}

private:
    unsigned int objectType;
    int health;
    float flash_amount;

};