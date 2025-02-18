#pragma once

#include <extlib/steam/steam_api.h>
#include <cstring>
#include <string>
#include <vector>

enum class PacketType
{
    JoinQuery,
    JoinReply
};

struct Packet
{
    PacketType type;
    std::string data;

    inline std::vector<char> serialise()
    {
        size_t size = sizeof(PacketType) + data.size();
        std::vector<char> serialisedData(size);

        memcpy(serialisedData.data(), &type, sizeof(PacketType));

        size_t dataSize = data.size();
        // memcpy(serialisedData.data() + sizeof(PacketType), &dataSize, sizeof(size_t));
        memcpy(serialisedData.data() + sizeof(PacketType), data.data(), data.size());

        return serialisedData;
    }

    inline void deserialise(const char* serialisedData, size_t serialisedDataSize)
    {
        memcpy(&type, serialisedData, sizeof(PacketType));

        size_t dataSize = serialisedDataSize - sizeof(PacketType);
        data.reserve(dataSize);
        memcpy(data.data(), serialisedData + sizeof(PacketType), dataSize);
    }

    inline EResult sendToUser(const SteamNetworkingIdentity &identityRemote, int nSendFlags, int nRemoteChannel)
    {
        std::vector<char> serialised = serialise();
        return SteamNetworkingMessages()->SendMessageToUser(identityRemote, serialised.data(), serialised.size(), nSendFlags, nRemoteChannel);
    }
};