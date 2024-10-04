#pragma once

#include <memory>
#include <cstdint>

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
};

template<>
struct std::hash<ChunkPosition>
{
    std::size_t operator()(const ChunkPosition& chunk) const
    {
        return std::hash<int>()(chunk.x * ChunkPosition::xPrime) ^ std::hash<int>()(chunk.y * ChunkPosition::yPrime);
    }
};