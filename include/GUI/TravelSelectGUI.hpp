#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Core/ResolutionHandler.hpp"

#include "Base/GUIContext.hpp"

#include "Data/typedefs.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

class TravelSelectGUI
{
    TravelSelectGUI() = delete;

public:
    static void processEventGUI(const sf::Event& event);

    static void setAvailableDestinations(const std::vector<PlanetType>& availablePlanetDestinations, const std::vector<RoomType>& availableRoomDestinations);

    static bool createGUI(sf::RenderWindow& window, PlanetType& selectedPlanetType, RoomType& selectedRoomType);

    static void drawGUI(sf::RenderTarget& window);

private:
    static void updateSelectionHoverRect(sf::IntRect destinationRect);
    static void resetHoverRect();

private:
    static GUIContext guiContext;

    static std::vector<PlanetType> availablePlanetDestinations;
    static std::vector<RoomType> availableRoomDestinations;

    static sf::FloatRect selectionHoverRect;
    static sf::FloatRect selectionHoverRectDestination;

};