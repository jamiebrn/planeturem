#pragma once

struct ChunkPosition
{
    uint16_t x, y;
    ChunkPosition() : x(0), y(0) {}
    ChunkPosition(int _x, int _y) : x(_x), y(_y) {}
    bool operator==(const ChunkPosition& other) const
    {
        return (x == other.x && y == other.y);
    }
    bool operator<(const ChunkPosition& other) const
    {
        if (y != other.y)
        {
            return y < other.y;
        }
        return x < other.x;
    }
};

template<>
struct std::hash<ChunkPosition>
{
    std::size_t operator()(const ChunkPosition& chunk) const
    {
        return std::hash<int>()(chunk.x) ^ std::hash<int>()(chunk.y);
    }
};