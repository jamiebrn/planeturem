#pragma once

#include <memory>
#include <cstdint>
#include <string>

#include <extlib/cereal/archives/binary.hpp>

#include <Core/json.hpp>

struct ChunkPosition
{
    int16_t x, y;
    ChunkPosition() : x(0), y(0) {}
    ChunkPosition(int _x, int _y) : x(_x), y(_y) {}
    inline bool operator==(const ChunkPosition& other) const
    {
        return (x == other.x && y == other.y);
    }
    inline bool operator!=(const ChunkPosition& other) const
    {
        return (x != other.x || y != other.y);
    }
    inline bool operator<(const ChunkPosition& other) const
    {
        if (y != other.y)
        {
            return y < other.y;
        }
        return x < other.x;
    }

    static constexpr int xPrime = 2;
    static constexpr int yPrime = 10007;

    inline int hash()
    {
        return (x * xPrime) ^ (y * yPrime);
    }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(x, y);
    }

    inline std::string toString()
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

template<>
struct std::hash<ChunkPosition>
{
    std::size_t operator()(const ChunkPosition& chunk) const
    {
        return std::hash<int>()(chunk.x * ChunkPosition::xPrime) ^ std::hash<int>()(chunk.y * ChunkPosition::yPrime);
    }
};

inline void from_json(const nlohmann::json& json, ChunkPosition& chunkPosition)
{
    chunkPosition.x = json[0];
    chunkPosition.y = json[1];
}

inline void to_json(nlohmann::json& json, const ChunkPosition& chunkPosition)
{
    json[0] = chunkPosition.x;
    json[1] = chunkPosition.y;
}