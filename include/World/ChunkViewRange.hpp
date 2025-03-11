#pragma once

#include <algorithm>
#include <unordered_set>
#include <vector>
#include <optional>

#include <extlib/cereal/archives/binary.hpp>

#include "Core/Helper.hpp"
#include "World/ChunkPosition.hpp"

struct ChunkViewRange
{
    class iterator
    {
    public:
        inline iterator(const ChunkViewRange& chunkViewRange, int index) : chunkViewRange(chunkViewRange), index(index)
        {
            maxSize = chunkViewRange.getSize();
        }
        inline iterator& operator++(int)
        {
            index = std::min(index + 1, maxSize - 1);
            return *this;
        }
        inline iterator& operator--(int)
        {
            index = std::max(index - 1, 0);
            return *this;
        }
        inline bool operator!=(const iterator& other)
        {
            return (index != other.index);
        }
        inline ChunkPosition get(std::optional<int> worldSize)
        {
            ChunkPosition chunkPos;
            int width = chunkViewRange.bottomRight.x - chunkViewRange.topLeft.x + 1;
            chunkPos.x = chunkViewRange.topLeft.x + index % width;
            chunkPos.y = chunkViewRange.topLeft.y + std::floor(index / width);

            if (worldSize.has_value())
            {
                chunkPos.x = Helper::wrap(chunkPos.x, worldSize.value());                
                chunkPos.y = Helper::wrap(chunkPos.y, worldSize.value());                
            }

            return chunkPos;
        }
        inline ChunkPosition getUnwrapped()
        {
            return get(std::nullopt);
        }

    private:
        int index = 0;
        int maxSize = 0;
        const ChunkViewRange& chunkViewRange;

    };

    ChunkPosition topLeft;
    ChunkPosition bottomRight;

    inline int getSize() const
    {
        return (bottomRight.x - topLeft.x + 1) * (bottomRight.y - topLeft.y + 1);
    }

    inline iterator begin() const
    {
        return iterator(*this, 0);
    }

    inline iterator end() const
    {
        return iterator(*this, getSize() - 1);
    }

    inline bool isChunkInRange(ChunkPosition chunkPos, int worldSize) const
    {
        int normalizedChunkX = Helper::wrap(chunkPos.x, worldSize);
        int normalizedChunkY = Helper::wrap(chunkPos.y, worldSize);

        int normalizedViewLeft = Helper::wrap(topLeft.x, worldSize);
        int normalizedViewRight = Helper::wrap(bottomRight.x, worldSize);
        int normalizedViewTop = Helper::wrap(topLeft.y, worldSize);
        int normalizedViewBottom = Helper::wrap(bottomRight.y, worldSize);

        // Use normalized chunk coordinates to determine whether the chunk is visible
        bool chunkInRangeX = (normalizedChunkX >= normalizedViewLeft && normalizedChunkX <= normalizedViewRight) ||
                        (normalizedViewLeft > normalizedViewRight && 
                        (normalizedChunkX >= normalizedViewLeft || normalizedChunkX <= normalizedViewRight));

        bool chunkInRangeY = (normalizedChunkY >= normalizedViewTop && normalizedChunkY <= normalizedViewBottom) ||
                        (normalizedViewTop > normalizedViewBottom && 
                        (normalizedChunkY >= normalizedViewTop || normalizedChunkY <= normalizedViewBottom));
        
        return (chunkInRangeX && chunkInRangeY);
    }

    inline std::unordered_set<ChunkPosition> getChunkSet(int worldSize) const
    {
        std::unordered_set<ChunkPosition> chunkSet;
        for (auto iter = begin(); iter != end(); iter++)
        {
            chunkSet.insert(iter.get(worldSize));
        }
        return chunkSet;
    }

    static inline std::unordered_set<ChunkPosition> getCombinedChunkSet(const std::vector<ChunkViewRange>& viewRanges, int worldSize)
    {
        std::unordered_set<ChunkPosition> combinedChunkSet;
        for (auto iter = viewRanges.begin(); iter != viewRanges.end(); iter++)
        {
            std::unordered_set<ChunkPosition> chunkSet = iter->getChunkSet(worldSize);
            combinedChunkSet.insert(chunkSet.begin(), chunkSet.end());
        }
        return combinedChunkSet;
    }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(topLeft, bottomRight);
    }
};