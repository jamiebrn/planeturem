#pragma once

#include <string>
#include <vector>
#include <sstream>

#include <extlib/cereal/archives/binary.hpp>

#include "Network/PacketType.hpp"

#define PACKET_SERIALISATION() \
    std::vector<char> serialise() const override\
    {\
        std::stringstream stream;\
        {\
            cereal::BinaryOutputArchive archive(stream);\
            archive(*this);\
        }\
        std::string streamStr = stream.str();\
        return std::vector<char>(streamStr.begin(), streamStr.end());\
    }\
    void deserialise(const std::vector<char>& data) override\
    {\
        std::stringstream stream(std::string(data.begin(), data.end()));\
        cereal::BinaryInputArchive archive(stream);\
        archive(*this);\
    }\

struct IPacketData
{
    virtual std::vector<char> serialise() const = 0;
    virtual void deserialise(const std::vector<char>& data) = 0;

    // template <typename T>
    // std::vector<char> serialiseData(const T& data) const
    // {
    //     std::stringstream stream;
    //     {
    //         cereal::BinaryOutputArchive archive(stream);
    //         archive(data);
    //     }

    //     std::string streamStr = stream.str();
    //     return std::vector<char>(streamStr.begin(), streamStr.end());
    // }

    // template <typename T>
    // T deserialiseData(const std::vector<char>& data)
    // {
    //     std::stringstream stream(std::string(data.begin(), data.end()));
    //     cereal::BinaryInputArchive archive(stream);
    //     T t;
    //     archive(t);
    //     return t;
    // }

    virtual PacketType getType() const = 0;
};