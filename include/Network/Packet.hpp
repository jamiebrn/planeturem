#pragma once

#include <extlib/steam/steam_api.h>
#include <extlib/lzav.h>
#include <cstring>
#include <string>
#include <vector>
#include <cstdio>

#include "Network/PacketType.hpp"
#include "Network/IPacketData.hpp"

struct Packet
{
    PacketType type;
    std::vector<char> data;
    bool compressed = false;
    uint32_t uncompressedSize = 0;

    inline std::vector<char> serialise() const
    {
        int size = sizeof(type) + sizeof(compressed) + sizeof(uncompressedSize) + data.size();
        std::vector<char> serialisedData(size);

        memcpy(serialisedData.data(), &type, sizeof(type));
        memcpy(serialisedData.data() + sizeof(type), &compressed, sizeof(compressed));
        memcpy(serialisedData.data() + sizeof(type) + sizeof(compressed), &uncompressedSize, sizeof(uncompressedSize));
        memcpy(serialisedData.data() + sizeof(type) + sizeof(compressed) + sizeof(uncompressedSize), data.data(), data.size());

        return serialisedData;
    }

    inline bool deserialise(const char* serialisedData, size_t serialisedDataSize)
    {
        memcpy(&type, serialisedData, sizeof(type));
        memcpy(&compressed, serialisedData + sizeof(type), sizeof(compressed));
        memcpy(&uncompressedSize, serialisedData + sizeof(type) + sizeof(compressed), sizeof(uncompressedSize));

        int dataSize = serialisedDataSize - sizeof(type) - sizeof(compressed) - sizeof(uncompressedSize);
        data.resize(dataSize);

        memcpy(data.data(), serialisedData + sizeof(type) + sizeof(compressed) + sizeof(uncompressedSize), dataSize);

        if (compressed)
        {
            // Decompress
            std::vector<char> uncompressedData(uncompressedSize);
            int l = lzav_decompress(data.data(), uncompressedData.data(), data.size(), uncompressedSize);

            if (l < 0)
            {
                // Failed
                return false;
            }

            data = uncompressedData;
        }

        return true;
    }

    inline EResult sendToUser(const SteamNetworkingIdentity &identityRemote, int nSendFlags, int nRemoteChannel) const
    {
        std::vector<char> serialised = serialise();
        return SteamNetworkingMessages()->SendMessageToUser(identityRemote, serialised.data(), serialised.size(), nSendFlags, nRemoteChannel);
    }

    inline void set(const IPacketData& packetData, bool applyCompression = false)
    {
        type = packetData.getType();
        data = packetData.serialise();

        if (applyCompression)
        {
            compressed = true;
            uncompressedSize = data.size();

            int bufferSize = lzav_compress_bound(data.size());

            std::vector<char> compressedData;
            compressedData.resize(bufferSize);

            int compressedSize = lzav_compress_default(data.data(), compressedData.data(), data.size(), bufferSize);

            if (compressedSize < uncompressedSize)
            {
                compressedData.resize(compressedSize);
                data = compressedData;
            }
            else
            {
                compressed = false;
            }
        }
    }

    inline int getSize()
    {
        return (sizeof(type) + sizeof(compressed) + sizeof(uncompressedSize) + data.size());
    }

    inline int getUncompressedSize()
    {
        if (!compressed)
        {
            return getSize();
        }
        return (sizeof(type) + sizeof(compressed) + sizeof(uncompressedSize) + uncompressedSize);
    }

    inline float getCompressionRatio()
    {
        if (!compressed)
        {
            return 1.0f;
        }
        return static_cast<float>(getUncompressedSize()) / static_cast<float>(getSize());
    }

    inline std::string getSizeStr()
    {
        return ("(size: " + std::to_string(getSize()) + " bytes, uncompressed: " + std::to_string(getUncompressedSize()) +
            " bytes, ratio: " + std::to_string(getCompressionRatio()) + ")");
    }
};