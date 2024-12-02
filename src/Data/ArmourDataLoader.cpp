#include "Data/ArmourDataLoader.hpp"

std::vector<ArmourData> ArmourDataLoader::loaded_armourData;
std::unordered_map<std::string, ToolType> ArmourDataLoader::armourNameToTypeMap;

bool ArmourDataLoader::loadData(std::string armourDataPath)
{
    std::ifstream file(armourDataPath);
    nlohmann::json data = nlohmann::json::parse(file);

    // Load tool data
    for (nlohmann::json::iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        ArmourData armourData;
        auto jsonArmourData = iter.value();

        armourData.name = jsonArmourData.at("name");

        armourData.armourWearType = getArmourWearTypeFromStr(jsonArmourData.at("type"));

        armourData.itemTexture = jsonArmourData.at("texture");

        sf::Vector2f wearTextureSize(16, 16);
        if (jsonArmourData.contains("wear-texture-size"))
        {
            wearTextureSize = jsonArmourData.at("wear-texture-size");
        }

        auto wearTextures = jsonArmourData.at("wear-textures");
        for (nlohmann::json::iterator texturePositionIter = wearTextures.begin(); texturePositionIter != wearTextures.end(); ++texturePositionIter)
        {
            sf::IntRect textureRect;
            textureRect.left = texturePositionIter.value()[0];
            textureRect.top = texturePositionIter.value()[1];
            textureRect.width = wearTextureSize.x;
            textureRect.height = wearTextureSize.y;

            armourData.wearTextures.push_back(textureRect);
        }

        if (jsonArmourData.contains("wear-texture-offset"))
        {
            armourData.wearTextureOffset = jsonArmourData.at("wear-texture-offset");
        }

        float sellValue = 0;
        if (jsonArmourData.contains("sell-value")) sellValue = jsonArmourData.at("sell-value");

        armourData.defence = jsonArmourData.at("defence");

        armourNameToTypeMap[armourData.name] = loaded_armourData.size();

        // Create item representing armour
        ItemDataLoader::createItemFromArmour(loaded_armourData.size(), armourData, sellValue);

        loaded_armourData.push_back(armourData);
    }

    return true;
}

const ArmourData& ArmourDataLoader::getArmourData(ArmourType armourType)
{
    return loaded_armourData[armourType];
}

ToolType ArmourDataLoader::getArmourTypeFromName(const std::string& armourName)
{
    return armourNameToTypeMap[armourName];
}

ArmourWearType ArmourDataLoader::getArmourWearTypeFromStr(const std::string& armourWearStr)
{
    static const std::unordered_map<std::string, ArmourWearType> armourWearTypeStrMap = {
        {"Head", ArmourWearType::Head},
        {"Chest", ArmourWearType::Chest},
        {"Feet", ArmourWearType::Feet}
    };

    if (armourWearTypeStrMap.contains(armourWearStr))
    {
        return armourWearTypeStrMap.at(armourWearStr);
    }

    // Default case - armour wear type string not found
    return ArmourWearType::Head;
}