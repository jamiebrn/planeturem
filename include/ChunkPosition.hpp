#pragma once

struct ChunkPosition
{
    int x, y;
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