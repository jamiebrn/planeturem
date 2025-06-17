#pragma once

#include <cstdint>
#include <cmath>

#include <extlib/cereal/archives/binary.hpp>

template <class T>
struct CompactFloat
{
    T compactValue;

    CompactFloat() = default;
    CompactFloat(float value, uint8_t precision)
    {
        compactValue = value * pow(10, precision);
    }

    float getValue(uint8_t precision) const
    {
        return static_cast<float>(compactValue) / static_cast<float>(pow(10, precision));
    }

    T getCompactValue() const
    {
        return compactValue;
    }

    void setCompactValue(T value)
    {
        compactValue = value;
    }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(compactValue);
    }
};