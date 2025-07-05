#pragma once

#include <fstream>
#include <string>
#include <unordered_map>

#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/json.hpp"
#include "Data/Serialise/Vector2Serialise.hpp"
#include "Data/Serialise/IntRectSerialise.hpp"
#include "Data/typedefs.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ItemDataLoader.hpp"

class ArmourDataLoader
{
    ArmourDataLoader() = delete;

public:
    static bool loadData(std::string armourDataPath);

    static const ArmourData& getArmourData(ArmourType armourType);

    static ArmourType getArmourTypeFromName(const std::string& armourName);
    
    static inline const std::string& getDataHash() {return dataHash;}

private:
    static ArmourWearType getArmourWearTypeFromStr(const std::string& armourWearStr);

private:
    static std::vector<ArmourData> loaded_armourData;

    static std::unordered_map<std::string, ArmourType> armourNameToTypeMap;

    static std::string dataHash;

};