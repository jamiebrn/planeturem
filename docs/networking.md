## Networking/Multiplayer
Multiplayer in Planeturem was never an intended feature - the game was written with a single player architecture in mind from day 1.

Regardless, I thought it would be an interesting challenge and improve the game considerably, assuming it was well implemented.

I decided to start working on this in February 2025. After many architectural changes to allow for multiple player simulation/management and actually implementing the networking, I had full multiplayer completed in June 2025 (4 months of development).

## Technical Overview
The game was going to be listed on Steam, so I wanted Steam integration, e.g. invites etc. For this reason I chose to use the [SteamNetworkingMessages](https://partner.steamgames.com/doc/api/ISteamnetworkingMessages) API from the Steamworks SDK. This allows the sending of arbitrary data to other Steam users using their SteamID.

Now that I had a method of sending data, I needed a standard way to serialise the data. I was already using [cereal](https://uscilab.github.io/cereal/) for save game binary serialisation, so would be convenient to also use here.

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

You may have noticed the `IPacketData` type - I created this next. This is an abstract class which acts as an interface into derived PacketData types, which each specify different types of packets, e.g. `PacketDataCharacterInfo` or `PacketDataChunks`.

The `IPacketData` struct is defined as follows:
```cpp
struct IPacketData
{
    virtual std::vector<char> serialise() const = 0;
    virtual void deserialise(const std::vector<char>& data) = 0;

    virtual PacketType getType() const = 0;
};
```
Nothing interesting, just an interface for the `Packet` struct to access. Each `IPacketData` derived type can then implement the `serialize` cereal function (not `serialise`), or `save` and `load` if different instructions are required for each process.

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

This is due to how cereal handles serialisation of types - templated functions are used for serialisation, meaning when `archive(*this)` is called in `IPacketData`, we are essentially calling `IPacketData::cereal_serialize` as there is no virtual dispatch at compile time.

This wil not serialise the correct type, despite the actual underlying type of a pointer to `IPacketData`. As we need to call `cereal_serialize` for the correct type (which is determined at compile time), we must put the `serialise` and `deserialise` functions in every `IPacketData` derived struct.

To simplify this I just wrote a macro called `PACKET_SERIALISATION()` which could be written in every derived type, which would define the functions for that type. With `IPacketData` having virtual `serialise` and `deserialise` functions, the correct function corresponding to that type will be dispatched to at runtime, ultimately calling the respective type's `cereal_serialize` function.
