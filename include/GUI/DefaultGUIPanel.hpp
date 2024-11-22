#pragma once

#include <SFML/Graphics.hpp>

#include "Core/ResolutionHandler.hpp"

#include "GUI/Base/GUIContext.hpp"

class DefaultGUIPanel
{
public:
    DefaultGUIPanel() = default;

    void handleEvent(sf::Event& event);

protected:
    void drawPanel(sf::RenderTarget& window);

    void updateAndDrawSelectionHoverRect(sf::RenderTarget& window, float dt);
    void updateSelectionHoverRectDestination(sf::IntRect destinationRect);
    void resetHoverRect();

    int getScaledPanelPaddingX();

protected:
    static const int panelPaddingX = 250;
    static const int panelWidth = 500;

    static const ButtonStyle buttonStyle;

    GUIContext guiContext;

    sf::FloatRect selectionHoverRect;
    sf::FloatRect selectionHoverRectDestination;
};