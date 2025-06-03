#pragma once

#include <Core/json.hpp>

#include <Graphics/Color.hpp>

#include <extlib/cereal/archives/binary.hpp>

namespace pl
{

inline void from_json(const nlohmann::json& json, pl::Color& colour)
{
    colour.r = json[0];
    colour.g = json[1];
    colour.b = json[2];
}

inline void to_json(nlohmann::json& json, const pl::Color& colour)
{
    json[0] = colour.r;
    json[1] = colour.g;
    json[2] = colour.b;
}

}

namespace cereal
{

template <class Archive>
void serialize(Archive& ar, pl::Color& colour)
{
    uint8_t r = colour.r;
    uint8_t g = colour.g;
    uint8_t b = colour.b;
    ar(r, g, b);
}

}
