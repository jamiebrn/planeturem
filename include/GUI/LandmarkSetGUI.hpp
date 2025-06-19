#pragma once

#include <vector>
#include <array>
#include <string>

#include <Graphics/VertexArray.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

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

    void initialise(ObjectReference landmarkObject, pl::Color colorA, pl::Color colorB);

    LandmarkSetGUIEvent createAndDraw(pl::RenderTarget& window, float dt);

    pl::Color getColorA() const;
    pl::Color getColorB() const;
    const ObjectReference& getLandmarkObjectReference() const;

private:
    pl::Color aColor;
    float aColorValueHSV = 100.0f;
    pl::Color bColor;
    float bColorValueHSV = 100.0f;

    ObjectReference landmarkSettingObjectReference;

};