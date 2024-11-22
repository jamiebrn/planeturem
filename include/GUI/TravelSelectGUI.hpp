#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextDraw.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

class TravelSelectGUI : public DefaultGUIPanel
{
public:
    TravelSelectGUI() = default;

    void setAvailableDestinations(const std::vector<PlanetType>& availablePlanetDestinations, const std::vector<RoomType>& availableRoomDestinations);

    bool createAndDraw(sf::RenderWindow& window, float dt, PlanetType& selectedPlanetType, RoomType& selectedRoomType);

private:
    std::vector<PlanetType> availablePlanetDestinations;
    std::vector<RoomType> availableRoomDestinations;

};