#pragma once

#include <Core/json.hpp>

#include <Graphics/Color.hpp>

#include <extlib/cereal/archives/binary.hpp>

namespace pl
{

inline void from_json(const nlohmann::json& json, pl::Color& color)
{
    color.r = json[0];
    color.g = json[1];
    color.b = json[2];
}

inline void to_json(nlohmann::json& json, const pl::Color& color)
{
    json[0] = color.r;
    json[1] = color.g;
    json[2] = color.b;
}

}

namespace cereal
{

template <class Archive>
void save(Archive& ar, const pl::Color& color)
{
    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;
    ar(r, g, b);
}

template <class Archive>
void load(Archive& ar, pl::Color& color)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    ar(r, g, b);

    color.r = r;
    color.g = g;
    color.b = b;
}

}
