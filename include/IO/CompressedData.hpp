#pragma once

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>

#include <extlib/lzav.h>

struct CompressedData
{
    CompressedData() = default;
    CompressedData(const std::vector<char> uncompressedData);
    std::vector<char> decompress();

    std::vector<char> data;
    uint32_t uncompressedSize;

    template <class Archive>
    void serialize(Archive& ar, const std::uint32_t version)
    {
        ar(data, uncompressedSize);
    }
};