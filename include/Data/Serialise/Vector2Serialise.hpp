#pragma once

#include <Core/json.hpp>

#include <Vector.hpp>

namespace pl
{

template <class T>
inline void from_json(const nlohmann::json& json, pl::Vector2<T>& vector)
{
    vector.x = json[0];
    vector.y = json[1];
}

template <class T>
inline void to_json(nlohmann::json& json, const pl::Vector2<T>& vector)
{
    json[0] = vector.x;
    json[1] = vector.y;
}

};