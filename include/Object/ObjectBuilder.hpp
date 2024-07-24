#pragma once

#include <memory>
#include "WorldObject.hpp"
#include "ObjectType.hpp"

#include "Object/WoodWall.hpp"

std::unique_ptr<WorldObject> createObjectFromType(ObjectType type, sf::Vector2f position);