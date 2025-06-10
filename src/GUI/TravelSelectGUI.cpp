#include "GUI/TravelSelectGUI.hpp"

void TravelSelectGUI::setAvailableDestinations(const std::vector<PlanetType>& availablePlanetDestinations, const std::vector<RoomType>& availableRoomDestinations)
{
    TravelSelectGUI::availablePlanetDestinations = availablePlanetDestinations;
    TravelSelectGUI::availableRoomDestinations = availableRoomDestinations;

    guiContext.resetActiveElement();
    resetHoverRect();
}

bool TravelSelectGUI::createAndDraw(pl::RenderTarget& window, float dt, LocationState& selectedLocationState)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    drawPanel(window);

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    int yPos = 100;

    pl::TextDrawData titleTextDrawData;
    titleTextDrawData.position = pl::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, yPos * intScale);
    titleTextDrawData.size = 36 * intScale;
    titleTextDrawData.color = pl::Color(255, 255, 255);
    titleTextDrawData.centeredX = true;
    titleTextDrawData.centeredY = true;

    bool clicked = false;

    if (availablePlanetDestinations.size() > 0)
    {
        titleTextDrawData.text = "Planets";
        TextDraw::drawText(window, titleTextDrawData);

        yPos += 50;

        for (PlanetType planetDestination : availablePlanetDestinations)
        {
            const PlanetGenData& planetData = PlanetGenDataLoader::getPlanetGenData(planetDestination);

            if (guiContext.createButton(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale, 24 * intScale, planetData.displayName, buttonStyle).isClicked())
            {
                if (!clicked)
                {
                    selectedLocationState.setPlanetType(planetDestination);
                }
                clicked = true;
            }

            yPos += 100 * intScale;
        }
        
        yPos += 150;
    }
    
    if (availableRoomDestinations.size() > 0)
    {
        titleTextDrawData.position.y = yPos * intScale;
        titleTextDrawData.text = "Other";
        TextDraw::drawText(window, titleTextDrawData);

        yPos += 50;

        for (RoomType roomDestination : availableRoomDestinations)
        {
            const RoomData& roomData = StructureDataLoader::getRoomData(roomDestination);

            if (guiContext.createButton(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale, 24 * intScale, roomData.displayName, buttonStyle).isClicked())
            {
                if (!clicked)
                {
                    selectedLocationState.setRoomDestType(roomDestination);
                }
                clicked = true;
            }

            yPos += 125 * intScale;
        }
    }
    
    updateAndDrawSelectionHoverRect(window, dt);

    guiContext.draw(window);

    guiContext.endGUI();

    return clicked;
}