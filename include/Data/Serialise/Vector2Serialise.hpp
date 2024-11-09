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

inline void to_json(nlohmann::json& json, const sf::Vector2f& vector)
{
    json[0] = vector.x;
    json[1] = vector.y;
}

inline void from_json(const nlohmann::json& json, sf::Vector2i& vector)
{
    vector.x = json[0];
    vector.y = json[1];
}

};