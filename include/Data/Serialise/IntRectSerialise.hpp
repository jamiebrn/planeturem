#pragma once

#include <Core/json.hpp>

#include <Rect.hpp>

#include "Core/CollisionRect.hpp"

namespace pl
{

inline void from_json(const nlohmann::json& json, pl::Rect<int>& rect)
{
    rect.x = json[0];
    rect.y = json[1];
    rect.width = json[2];
    rect.height = json[3];
}

template <class Archive>
void serialize(Archive& ar, pl::Rect<uint16_t>& rect)
{
    ar(rect.x, rect.y, rect.width, rect.height);
}

};

inline void from_json(const nlohmann::json& json, CollisionRect& rect)
{
    rect.x = json[0];
    rect.y = json[1];
    rect.width = json[2];
    rect.height = json[3];
}