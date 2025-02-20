#pragma once

#include <extlib/steam/steam_api.h>
#include <cstring>
#include <string>
#include <vector>

#include "Network/PacketType.hpp"
#include "Network/IPacketData.hpp"

struct Packet
{
    PacketType type;
    std::vector<char> data;

    inline std::vector<char> serialise() const
    {
        size_t size = sizeof(PacketType) + data.size() + 1;
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
        data.resize(dataSize);
        memcpy(data.data(), serialisedData + sizeof(PacketType), dataSize);
    }

    inline EResult sendToUser(const SteamNetworkingIdentity &identityRemote, int nSendFlags, int nRemoteChannel) const
    {
        std::vector<char> serialised = serialise();
        return SteamNetworkingMessages()->SendMessageToUser(identityRemote, serialised.data(), serialised.size(), nSendFlags, nRemoteChannel);
    }

    inline void set(const IPacketData& packetData)
    {
        type = packetData.getType();
        data = packetData.serialise();
    }
};