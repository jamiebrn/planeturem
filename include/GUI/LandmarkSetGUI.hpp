#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <string>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextDraw.hpp"
#include "Core/Shaders.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

#include "Object/ObjectReference.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

struct LandmarkSetGUIEvent
{
    bool modified = false;
    bool closed = false;
};

class LandmarkSetGUI : public DefaultGUIPanel
{
public:
    LandmarkSetGUI() = default;

    void initialise(ObjectReference landmarkObject, sf::Color colourA, sf::Color colourB);

    LandmarkSetGUIEvent createAndDraw(sf::RenderWindow& window, float dt);

    sf::Color getColourA() const;
    sf::Color getColourB() const;
    const ObjectReference& getLandmarkObjectReference() const;

private:
    float aColour[3];
    float bColour[3];

    ObjectReference landmarkSettingObjectReference;

    int colourPage;

};