// #pragma once

// #include <iostream>
// #include "WorldObject.hpp"
// #include "Types/ObjectType.hpp"

// class Tree : public WorldObject
// {
// public:
//     Tree(sf::Vector2f position) : WorldObject(position) {}

//     inline void update(float dt) override;
//     void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;

//     void interact() override;
    
//     inline bool isAlive() override {return health > 0;}

//     inline ObjectType getObjectType() override {return ObjectType::Tree;}

// private:
//     float flash_amount = 0.0f;
//     int health = 4;

// };