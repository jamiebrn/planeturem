#pragma once

#include <Core/json.hpp>
// #include <SFML/System/Vector2.hpp>

#include <Vector.hpp>

namespace pl
{

inline void from_json(const nlohmann::json& json, pl::Vector2f& vector)
{
    vector.x = json[0];
    vector.y = json[1];
}

inline void to_json(nlohmann::json& json, const pl::Vector2f& vector)
{
    json[0] = vector.x;
    json[1] = vector.y;
}

inline void from_json(const nlohmann::json& json, pl::Vector2<int>& vector)
{
    vector.x = json[0];
    vector.y = json[1];
}

};