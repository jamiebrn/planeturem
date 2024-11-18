#include "GUI/TravelSelectGUI.hpp"

GUIContext TravelSelectGUI::guiContext;
std::vector<PlanetType> TravelSelectGUI::availablePlanetDestinations;
std::vector<RoomType> TravelSelectGUI::availableRoomDestinations;
sf::FloatRect TravelSelectGUI::selectionHoverRect;
sf::FloatRect TravelSelectGUI::selectionHoverRectDestination;

void TravelSelectGUI::processEventGUI(const sf::Event& event)
{
    guiContext.processEvent(event);
}

void TravelSelectGUI::setAvailableDestinations(const std::vector<PlanetType>& availablePlanetDestinations, const std::vector<RoomType>& availableRoomDestinations)
{
    TravelSelectGUI::availablePlanetDestinations = availablePlanetDestinations;
    TravelSelectGUI::availableRoomDestinations = availableRoomDestinations;

    guiContext.resetActiveElement();
    resetHoverRect();
}

void TravelSelectGUI::updateSelectionHoverRect(sf::IntRect destinationRect)
{
    selectionHoverRectDestination = static_cast<sf::FloatRect>(destinationRect);

    // If hover rect is 0, 0, 0, 0 (i.e. null), do not lerp, immediately set to destination
    if (selectionHoverRect == sf::FloatRect(0, 0, 0, 0))
    {
        selectionHoverRect = selectionHoverRectDestination;
    }
}

void TravelSelectGUI::resetHoverRect()
{
    selectionHoverRectDestination = sf::FloatRect(0, 0, 0, 0);
    selectionHoverRect = selectionHoverRectDestination;
}

bool TravelSelectGUI::createAndDraw(sf::RenderWindow& window, float dt, PlanetType& selectedPlanetType, RoomType& selectedRoomType)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    const int panelPaddingX = 250 * resolution.x / 1920.0f;
    const int panelWidth = 500;

    // Draw panel
    sf::RectangleShape panelRect(sf::Vector2f(panelWidth * intScale, resolution.y));
    panelRect.setPosition(sf::Vector2f(panelPaddingX * intScale, 0));
    panelRect.setFillColor(sf::Color(30, 30, 30, 180));

    window.draw(panelRect);

    const ButtonStyle buttonStyle = {
        .colour = sf::Color(0, 0, 0, 0),
        .hoveredColour = sf::Color(0, 0, 0, 0),
        .clickedColour = sf::Color(0, 0, 0, 0),
        .textColour = sf::Color(200, 200, 200),
        .hoveredTextColour = sf::Color(50, 50, 50),
        .clickedTextColour = sf::Color(255, 255, 255)
    };

    int yPos = 100;

    TextDrawData titleTextDrawData;
    titleTextDrawData.position = sf::Vector2f(panelPaddingX + panelWidth / 2, yPos) * intScale;
    titleTextDrawData.size = 36;
    titleTextDrawData.colour = sf::Color(255, 255, 255);
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

            if (guiContext.createButton(panelPaddingX * intScale, yPos * intScale, panelWidth * intScale, 75 * intScale, planetData.displayName, buttonStyle).isClicked())
            {
                if (!clicked)
                {
                    selectedPlanetType = planetDestination;
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

            if (guiContext.createButton(panelPaddingX * intScale, yPos * intScale, panelWidth * intScale, 75 * intScale, roomData.displayName, buttonStyle).isClicked())
            {
                if (!clicked)
                {
                    selectedRoomType = roomDestination;
                }
                clicked = true;
            }

            yPos += 125 * intScale;
        }
    }

    if (const GUIElement* hoveredElement = guiContext.getHoveredElement();
        hoveredElement != nullptr)
    {
        updateSelectionHoverRect(hoveredElement->getBoundingBox());
    }

    CollisionRect panelCollisionRect(panelPaddingX * intScale, 0, panelWidth * intScale, resolution.y);

    if (!panelCollisionRect.isPointInRect(guiContext.getInputState().mouseX, guiContext.getInputState().mouseY))
    {
        resetHoverRect();
    }

    selectionHoverRect.left = Helper::lerp(selectionHoverRect.left, selectionHoverRectDestination.left, 15.0f * dt);
    selectionHoverRect.top = Helper::lerp(selectionHoverRect.top, selectionHoverRectDestination.top, 15.0f * dt);
    selectionHoverRect.width = Helper::lerp(selectionHoverRect.width, selectionHoverRectDestination.width, 15.0f * dt);
    selectionHoverRect.height = Helper::lerp(selectionHoverRect.height, selectionHoverRectDestination.height, 15.0f * dt);

    // Draw
    sf::RectangleShape selectionRect;
    selectionRect.setPosition(sf::Vector2f(selectionHoverRect.left, selectionHoverRect.top));
    selectionRect.setSize(sf::Vector2f(selectionHoverRect.width, selectionHoverRect.height));
    selectionRect.setFillColor(sf::Color(200, 200, 200, 150));

    window.draw(selectionRect);

    guiContext.draw(window);

    guiContext.endGUI();

    return clicked;
}

// void TravelSelectGUI::drawGUI(sf::RenderTarget& window)
// {
//     guiContext.draw(window);
//     guiContext.endGUI();
// }