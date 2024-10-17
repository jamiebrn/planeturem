#pragma once

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ArmourDataLoader.hpp"

#include "Player/InventoryData.hpp"

namespace PlayerStats
{

int calculateDefence(InventoryData& armourInventory);

}