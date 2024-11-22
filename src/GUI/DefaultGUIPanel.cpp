#include "GUI/DefaultGUIPanel.hpp"

const ButtonStyle DefaultGUIPanel::buttonStyle = {
    .colour = sf::Color(0, 0, 0, 0),
    .hoveredColour = sf::Color(0, 0, 0, 0),
    .clickedColour = sf::Color(0, 0, 0, 0),
    .textColour = sf::Color(200, 200, 200),
    .hoveredTextColour = sf::Color(50, 50, 50),
    .clickedTextColour = sf::Color(255, 255, 255)
};

void DefaultGUIPanel::handleEvent(sf::Event& event)
{
    guiContext.processEvent(event);
}

void DefaultGUIPanel::drawPanel(sf::RenderTarget& window)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    sf::RectangleShape panelRect(sf::Vector2f(panelWidth * intScale, resolution.y));
    panelRect.setPosition(sf::Vector2f(panelPaddingX * intScale, 0));
    panelRect.setFillColor(sf::Color(30, 30, 30, 180));

    window.draw(panelRect);
}

int DefaultGUIPanel::getScaledPanelPaddingX()
{
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    return panelPaddingX * resolution.x / 1920.0f;
}

void DefaultGUIPanel::updateAndDrawSelectionHoverRect(sf::RenderTarget& window, float dt)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    if (const GUIElement* hoveredElement = guiContext.getHoveredElement();
        hoveredElement != nullptr)
    {
        updateSelectionHoverRectDestination(hoveredElement->getBoundingBox());
    }

    CollisionRect panelCollisionRect(scaledPanelPaddingX * intScale, 0, panelWidth * intScale, resolution.y);

    if (!panelCollisionRect.isPointInRect(guiContext.getInputState().mouseX, guiContext.getInputState().mouseY))
    {
        resetHoverRect();
    }

    // Update
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
}

void DefaultGUIPanel::updateSelectionHoverRectDestination(sf::IntRect destinationRect)
{
    selectionHoverRectDestination = static_cast<sf::FloatRect>(destinationRect);

    // If hover rect is 0, 0, 0, 0 (i.e. null), do not lerp, immediately set to destination
    if (selectionHoverRect == sf::FloatRect(0, 0, 0, 0))
    {
        selectionHoverRect = selectionHoverRectDestination;
    }
}

void DefaultGUIPanel::resetHoverRect()
{
    selectionHoverRectDestination = sf::FloatRect(0, 0, 0, 0);
    selectionHoverRect = selectionHoverRectDestination;
}