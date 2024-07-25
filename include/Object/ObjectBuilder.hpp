#pragma once

#include <memory>
#include "Object/WorldObject.hpp"
#include "Types/ObjectType.hpp"

#include "Object/WoodWall.hpp"
#include "Object/WoodFloor.hpp"

std::unique_ptr<WorldObject> createObjectFromType(ObjectType type, sf::Vector2f position);