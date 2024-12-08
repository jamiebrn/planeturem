#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <string>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextDraw.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

#include "Object/ObjectReference.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

struct LandmarkSetGUIEvent
{
    bool selected = false;
    sf::Color colourA, colourB;
    ObjectReference landmarkObjectReference;
};

class LandmarkSetGUI : public DefaultGUIPanel
{
public:
    LandmarkSetGUI() = default;

    void initialise(ObjectReference landmarkObject);

    LandmarkSetGUIEvent createAndDraw(sf::RenderWindow& window, float dt);

private:
    float aColour[3];
    float bColour[3];

    ObjectReference landmarkSettingObjectReference;

};