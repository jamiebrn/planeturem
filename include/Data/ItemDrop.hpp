#pragma once

#include "Data/typedefs.hpp"

struct ItemDrop
{
    ItemType item;
    unsigned int minAmount;
    unsigned int maxAmount;
    float chance;
};