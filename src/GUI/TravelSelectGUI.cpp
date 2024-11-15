#include "GUI/TravelSelectGUI.hpp"

GUIContext TravelSelectGUI::guiContext;
std::vector<PlanetType> TravelSelectGUI::availablePlanetDestinations;
std::vector<RoomType> TravelSelectGUI::availableRoomDestinations;

void TravelSelectGUI::processEventGUI(const sf::Event& event)
{
    guiContext.processEvent(event);
}

void TravelSelectGUI::setAvailableDestinations(const std::vector<PlanetType>& availablePlanetDestinations, const std::vector<RoomType>& availableRoomDestinations)
{
    TravelSelectGUI::availablePlanetDestinations = availablePlanetDestinations;
    TravelSelectGUI::availableRoomDestinations = availableRoomDestinations;
}

bool TravelSelectGUI::createGUI(sf::RenderWindow& window, PlanetType& selectedPlanetType, RoomType& selectedRoomType)
{
    bool clicked = false;

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    int yPos = window.getSize().y / 2.0f - 300 * intScale;

    for (RoomType roomDestination : availableRoomDestinations)
    {
        const RoomData& roomData = StructureDataLoader::getRoomData(roomDestination);

        if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, yPos, 200 * intScale, 75 * intScale, roomData.name).isClicked())
        {
            clicked = true;
            selectedRoomType = roomDestination;
        }

        yPos += 125 * intScale;
    }

    yPos += 200 * intScale;

    for (PlanetType planetDestination : availablePlanetDestinations)
    {
        const PlanetGenData& planetData = PlanetGenDataLoader::getPlanetGenData(planetDestination);

        if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, yPos, 200 * intScale, 75 * intScale, planetData.name).isClicked())
        {
            clicked = true;
            selectedPlanetType = planetDestination;
        }

        yPos += 125 * intScale;
    }

    return clicked;
}

void TravelSelectGUI::drawGUI(sf::RenderTarget& window)
{
    guiContext.draw(window);
    guiContext.endGUI();
}