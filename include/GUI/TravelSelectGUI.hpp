#pragma once

#include <vector>

#include <Graphics/VertexArray.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/TextDraw.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

#include "Player/LocationState.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

class TravelSelectGUI : public DefaultGUIPanel
{
public:
    TravelSelectGUI() = default;

    void setAvailableDestinations(const std::vector<PlanetType>& availablePlanetDestinations, const std::vector<RoomType>& availableRoomDestinations);

    bool createAndDraw(pl::RenderTarget& window, float dt, LocationState& selectedLocationState);

private:
    std::vector<PlanetType> availablePlanetDestinations;
    std::vector<RoomType> availableRoomDestinations;

};