#pragma once

#include <Core/json.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf
{

inline void from_json(const nlohmann::json& json, sf::Vector2f& vector)
{
    vector.x = json[0];
    vector.y = json[1];
}

};