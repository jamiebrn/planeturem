// #pragma once

// #include <SFML/Graphics.hpp>

// #include "WorldObject.hpp"
// #include "BuildableObject.hpp"
// #include "Types/ObjectType.hpp"

// class WoodFloor : public BuildableObject
// {
// public:
//     WoodFloor(sf::Vector2f position) : BuildableObject(position) {drawLayer = 1;}
//     void update(float dt) override;
//     void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;
//     void drawGUI(sf::RenderWindow& window, float dt, const sf::Color& color) override;

//     inline ObjectType getObjectType() override {return ObjectType::WoodFloor;}

//     void interact() override;

//     inline bool isAlive() override {return true;}

// };