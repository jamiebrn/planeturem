#include "IO/CompressedData.hpp"
#include "IO/Log.hpp"

CompressedData::CompressedData(const std::vector<char> uncompressedData)
{
    uncompressedSize = uncompressedData.size();

    int bufferSize = lzav_compress_bound(uncompressedData.size());

    data.resize(bufferSize);

    int compressedSize = lzav_compress_default(uncompressedData.data(), data.data(), uncompressedData.size(), bufferSize);

    data.resize(compressedSize);
}

std::vector<char> CompressedData::decompress()
{
    std::vector<char> uncompressedData(uncompressedSize);
    int l = lzav_decompress(data.data(), uncompressedData.data(), data.size(), uncompressedSize);

    if (l < 0)
    {
        Log::push("ERROR: Data decompression failed\n");
    }

    return uncompressedData;
}