#include "Object/ObjectBuilder.hpp"

std::unique_ptr<WorldObject> createObjectFromType(ObjectType type, sf::Vector2f position)
{
    switch (type)
    {
    
    case ObjectType::WoodWall:
        return std::move(std::make_unique<WoodWall>(position));
    case ObjectType::WoodFloor:
        return std::move(std::make_unique<WoodFloor>(position));

    }

    return nullptr;
}