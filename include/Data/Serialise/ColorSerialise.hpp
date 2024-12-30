#pragma once

#include <Core/json.hpp>
#include <SFML/Graphics/Color.hpp>

#include <extlib/cereal/archives/binary.hpp>

namespace sf
{

inline void from_json(const nlohmann::json& json, sf::Color& colour)
{
    colour.r = json[0];
    colour.g = json[1];
    colour.b = json[2];
}

inline void to_json(nlohmann::json& json, const sf::Color& colour)
{
    json[0] = colour.r;
    json[1] = colour.g;
    json[2] = colour.b;
}

template <class Archive>
void serialize(Archive& ar, sf::Color colour)
{
    ar(colour.r, colour.g, colour.b);
}

};