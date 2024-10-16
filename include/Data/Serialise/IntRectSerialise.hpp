#pragma once

#include <Core/json.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace sf
{

inline void from_json(const nlohmann::json& json, sf::IntRect& rect)
{
    rect.left = json[0];
    rect.top = json[1];
    rect.width = json[2];
    rect.height = json[3];
}

};