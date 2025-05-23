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

    Packet() = default;
    inline Packet(const IPacketData& packetData, bool applyCompression = true)
    {
        set(packetData, applyCompression);
    }

    inline std::vector<char> serialise() const
    {
        int size = sizeof(type) + sizeof(compressed) + data.size();
        if (compressed)
        {
            size += sizeof(uncompressedSize);
        }
        std::vector<char> serialisedData(size);

        char* bufferPtr = serialisedData.data();

        memcpy(bufferPtr, &type, sizeof(type));
        bufferPtr += sizeof(type);

        memcpy(bufferPtr, &compressed, sizeof(compressed));
        bufferPtr += sizeof(compressed);

        if (compressed)
        {
            memcpy(bufferPtr, &uncompressedSize, sizeof(uncompressedSize));
            bufferPtr += sizeof(uncompressedSize);
        }

        memcpy(bufferPtr, data.data(), data.size());

        return serialisedData;
    }

    inline bool deserialise(const char* serialisedData, size_t serialisedDataSize)
    {
        const char* bufferPtr = serialisedData;
        
        memcpy(&type, bufferPtr, sizeof(type));
        bufferPtr += sizeof(type);

        memcpy(&compressed, bufferPtr, sizeof(compressed));
        bufferPtr += sizeof(compressed);

        if (compressed)
        {
            memcpy(&uncompressedSize, bufferPtr, sizeof(uncompressedSize));
            bufferPtr += sizeof(uncompressedSize);
        }

        int dataSize = serialisedDataSize - (bufferPtr - serialisedData);
        data.resize(dataSize);

        memcpy(data.data(), bufferPtr, dataSize);

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

    inline void set(const IPacketData& packetData, bool applyCompression = true)
    {
        type = packetData.getType();
        data = packetData.serialise();

        if (applyCompression)
        {
            uncompressedSize = data.size();
            
            int bufferSize = lzav_compress_bound(data.size());
            
            std::vector<char> compressedData;
            compressedData.resize(bufferSize);
            
            int compressedSize = lzav_compress_default(data.data(), compressedData.data(), data.size(), bufferSize);

            // Only apply compression if compressed size is smaller, including size of integer storing size of uncompressed data
            // Ensures data does not expand after applying compression (e.g. data is compressed by 2 bytes but requires extra 4 bytes
            // in packet for original size, causing a 2 byte expansion :( )
            if ((compressedSize + sizeof(uncompressedSize)) < uncompressedSize)
            {
                compressedData.resize(compressedSize);
                data = compressedData;
                compressed = true;
            }
        }
    }

    inline int getSize() const
    {
        return (sizeof(type) + sizeof(compressed) + sizeof(uncompressedSize) + data.size());
    }

    inline int getUncompressedSize() const
    {
        if (!compressed)
        {
            return getSize();
        }
        return (sizeof(type) + sizeof(compressed) + sizeof(uncompressedSize) + uncompressedSize);
    }

    inline float getCompressionRatio() const
    {
        if (!compressed)
        {
            return 1.0f;
        }
        return static_cast<float>(getUncompressedSize()) / static_cast<float>(getSize());
    }

    inline std::string getSizeStr() const
    {
        return ("(size: " + std::to_string(getSize()) + " bytes, uncompressed: " + std::to_string(getUncompressedSize()) +
            " bytes, ratio: " + std::to_string(getCompressionRatio()) + ")");
    }
};