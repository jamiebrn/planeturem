## Networking/Multiplayer
Multiplayer in Planeturem was never an intended feature - the game was written with a single player architecture in mind from day 1.

Regardless, I thought it would be an interesting challenge and improve the game considerably, assuming it was well implemented.

I decided to start working on this in February 2025. After many architectural changes to allow for multiple player simulation/management and actually implementing the networking, I had full multiplayer completed in June 2025 (4 months of development).

NOTE: To explain the whole multiplayer architecture would be very complex and probably akin to simply writing the code out again with explanations. This document is designed to give an overview of the foundations on which the multiplayer is built.

## Technical Overview
Planeturem's multiplayer architecture is based around a peer-to-peer client-host model, with one player being the host. The world is simulated on the host system and is synced on client systems. Each client sends data about their player and any actions they take, e.g. moving items into chests etc. The host redistributes each client's position to every other client.

The game was going to be listed on Steam, so I wanted Steam integration, e.g. invites etc. For this reason I chose to use the [SteamNetworkingMessages](https://partner.steamgames.com/doc/api/ISteamnetworkingMessages) API from the Steamworks SDK. This allows the sending of arbitrary data to other Steam users using their SteamID. It is built on top of UDP and allows reliable and unreliable transmission of data across numbered channels.

Now that I had a method of sending data, I needed a standard serialisation format. I was already using [cereal](https://uscilab.github.io/cereal/) for save game binary serialisation, so would be convenient to also use here.

I did not implement this until a few weeks into development, but I needed compression of data to save bandwidth. For this, I decided to use [lzav](https://github.com/avaneev/lzav) due to its straightfoward API and promising benchmarks.

## Packet Implementation
I first created a generic `Packet` struct that would facilitate all data transfer. This would contain the payload `data` and type `PacketType`, as well as compression information (added later, but will list here for simplicity).

The structure looks something like this:
```cpp

struct Packet
{
    PacketType type;
    std::vector<char> data;
    bool compressed = false;
    uint32_t uncompressedSize = 0;

    std::vector<char> serialise() const
    {
        // ... copy PacketType, compression information, and data payload into contiguous memory for transfer

        return serialisedData;
    }

    bool deserialise(const char* serialisedData, size_t serialisedDataSize)
    {
        // .. copy PacketType, compression information, and data payload from passed in data into Packet memory
        // .. decompress if required

        return success;
    }

    EResult sendToUser(const SteamNetworkingIdentity &identityRemote, int nSendFlags, int nRemoteChannel) const
    {
        // Serialise into contiguous memory
        std::vector<char> serialised = serialise();

        // Use SteamNetworkingMessages API to send bytes
        return SteamNetworkingMessages()->SendMessageToUser(identityRemote, serialised.data(), serialised.size(), nSendFlags, nRemoteChannel);
    }

    void set(const IPacketData& packetData, bool applyCompression)
    {
        // .. copy IPacketData's PacketType as this Packet's PacketType
        // .. serialise IPacketData's data using cereal, and copy into Packet data payload

        // .. apply compression to data if required
    }
```

You may have noticed the `IPacketData` type - I created this next. This is an abstract class which acts as an interface into derived PacketData types, which each specify different types of packets, e.g. `PacketDataCharacterInfo` or `PacketDataChunkDatas`.

The `IPacketData` struct is defined as follows:
```cpp
struct IPacketData
{
    virtual std::vector<char> serialise() const = 0;
    virtual void deserialise(const std::vector<char>& data) = 0;

    virtual PacketType getType() const = 0;
};
```
Each `IPacketData` derived type can then implement the `serialize` cereal function (not `serialise`), or `save` and `load` if different instructions are required for each process.

For ease of reading I will refer to cereal's `serialize` function as `cereal_serialize`.

Back to `IPacketData` - the `serialise` and `deserialise` functions are defined in the same way for each `IPacketData` derived type:
```cpp
std::vector<char> serialise() const
{
    std::stringstream stream;
    {
        cereal::BinaryOutputArchive archive(stream);
        archive(*this);
    }
    std::string streamStr = stream.str();
    return std::vector<char>(streamStr.begin(), streamStr.end());
}

void deserialise(const std::vector<char>& data)
{
    std::stringstream stream(std::string(data.begin(), data.end()));
    cereal::BinaryInputArchive archive(stream);
    archive(*this);
}
```
Which comes with the question - why are these virtual abstract functions in `IPacketData`, and not simply defined there?

This is due to how cereal handles serialisation of types - templated functions are used for serialisation, meaning when `archive(*this)` is called in `IPacketData`, we are essentially calling `IPacketData::cereal_serialize` as there is no virtual dispatch at compile time. This means each `IPacketData` derived struct must contain its own `serialise` and `deserialise` implementation.

To simplify this I just wrote a macro called `PACKET_SERIALISATION()` which could be written in every derived type, which would define the functions for that type. With `IPacketData` having virtual `serialise` and `deserialise` functions, the correct function corresponding to that type will be dispatched to at runtime, ultimately calling the respective type's `cereal_serialize` function.

### Sending packets
Now that we have an extensible packet system, we need to be able to send packets to other computers. This is straightforward as we simply need to construct a `SteamNetworkingIdentity` using a valid SteamID and pass it into the packet `sendToUser(...)` function.

But how is this integrated with Steam matchmaking using invites etc? This is done through Steam's callback system, which allows easily binding functions to internal Steam events. For example:
```cpp
STEAM_CALLBACK(NetworkHandler, callbackLobbyUpdated, LobbyChatUpdate_t);
```
This will bind Steam's `LobbyChatUpdate_t` event to the `callbackLobbyUpdated(...)` function in the `NetworkHandler` class, which will be triggered whenever a player joins or leaves the Steam lobby.

This can then be used to register/store any new player's SteamIDs, and then begin sending them game data using the packet `sendToUser(...`) function.

Here is an example of how a packet containing `PacketDataJoinQuery` may be sent to a user with the "clientID" `SteamNetworkingIdentity`:
```cpp
PacketDataJoinQuery packetData;
// If client has not joined before, require them to enter a username
packetData.requiresNameInput = isNewUser(clientID);
// Send hash of game data (assets etc) to ensure data integrity
packetData.gameDataHash = getGameDataHash();

Packet packet;
// Pass "PacketDataJoinQuery" into packet set function, which will be serialised through "IPacketData" interface
packet.set(packetData);

// Send packet reliably on channel 0
packet.sendToUser(clientID, k_nSteamNetworkingSend_Reliable, 0);
```

### Receiving packets
The `sendToUser(...)` function serialises the packet and sends the bytes over the Steam network. In order to receive the correct message, we must receive these bytes and deserialise them.

In order to receive these bytes, the `SteamNetworkingMessages()->ReceiveMessageOnChannel(...)` function. This can be run every frame until there are no more messages to be received.

These bytes can then be deserialised into a `Packet` by using the `deserialise(const char* data, int size)` function. This can then be deserialised into the correct `IPacketData` derived struct depending on the packet's `PacketType` data.

The process looks something like this:
```cpp
void receiveMessages()
{
    static constexpr int MAX_MESSAGES = 10;

    // Allocate memory for potential messages
    SteamNetworkingMessage_t* messages[MAX_MESSAGES];

    // Continue to receive messages until there are none remain
    while (true)
    {
        int messageCount = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, MAX_MESSAGES);

        // No messages received - finish receiving
        if (messageCount <= 0)
            break;

        // Iterate over received messages and deserialise
        for (int i = 0; i < messageCount; i++)
        {
            // Deserialise bytes into packet
            Packet packet;
            packet.deserialise((char*)messages[i]->GetData(), messages[i]->GetSize());

            // Deserialise into correct IPacketData type depending on PacketType
            // This section is split into multiple functions in the actual code, but is simplified here
            switch (packet.type)
            {
                case PacketType::PacketDataJoinQuery:
                    // Deserialise into correct packet type
                    PacketDataJoinQuery packetData;
                    packetData.deserialise(packet.data);

                    // ... do stuff with data

                    break;

                // ... handle other packet types
            }

            // Free message memory
            messages[i]->Release();
        }
    }
}
```
We can now successfully transfer any `IPacketData` derived type over the network to other Steam users. This allows us to send any data defined inside of these structs, which allows us to implement the entirety of multiplayer essentially (with a lot of work in data coordination).

#### PacketType enum and PacketData structs
You may have noticed that every `IPacketData` type requires a corresponding `PacketType` in order to deserialise a received packet into the correct type. While this feels slightly messy and duplicative, a method of storing data type is required when sending arbitrary bytes over the network. Unfortunately C++ does not have type reflection, so this needs to be done manually.
