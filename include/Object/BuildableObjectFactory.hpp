#pragma once

#include "Object/WorldObject.hpp"
#include "Object/BuildableObject.hpp"
#include "Object/ChestObject.hpp"
#include "Object/RocketObject.hpp"

class BuildableObjectFactory
{
    BuildableObjectFactory() = delete;
public:
    static std::unique_ptr<BuildableObject> create(sf::Vector2f position, ObjectType objectType);
};