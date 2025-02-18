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
    std::string data;

    inline std::vector<char> serialise()
    {
        size_t size = sizeof(PacketType) + data.size() + 1;
        std::vector<char> serialisedData(size);

        memcpy(serialisedData.data(), &type, sizeof(PacketType));

        size_t dataSize = data.size();
        // memcpy(serialisedData.data() + sizeof(PacketType), &dataSize, sizeof(size_t));
        memcpy(serialisedData.data() + sizeof(PacketType), data.c_str(), data.size() + 1);

        return serialisedData;
    }

    inline void deserialise(const char* serialisedData, size_t serialisedDataSize)
    {
        memcpy(&type, serialisedData, sizeof(PacketType));

        size_t dataSize = serialisedDataSize - sizeof(PacketType);
        char* str = new char[dataSize + 1];
        memcpy(str, serialisedData + sizeof(PacketType), dataSize + 1);
        data = str;
        delete[] str;
    }

    inline EResult sendToUser(const SteamNetworkingIdentity &identityRemote, int nSendFlags, int nRemoteChannel)
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