#include "GUI/TravelSelectGUI.hpp"

GUIContext TravelSelectGUI::guiContext;

void TravelSelectGUI::processEventGUI(const sf::Event& event)
{
    guiContext.processEvent(event);
}

bool TravelSelectGUI::createGUI(sf::RenderWindow& window, const std::vector<PlanetType>& availableDestinations, PlanetType& selectedPlanetType)
{
    bool clicked = false;

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    int yPos = window.getSize().y / 2.0f - 200 * intScale;

    for (PlanetType destination : availableDestinations)
    {
        const PlanetGenData& planetData = PlanetGenDataLoader::getPlanetGenData(destination);

        if (guiContext.createButton(window.getSize().x / 2.0f - 100 * intScale, yPos, 200 * intScale, 75 * intScale, planetData.name))
        {
            clicked = true;
            selectedPlanetType = destination;
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