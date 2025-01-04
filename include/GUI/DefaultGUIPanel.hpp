#pragma once

#include <SFML/Graphics.hpp>
#include <limits>

#include "Core/ResolutionHandler.hpp"
#include "Core/InputManager.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/Base/GUIInputState.hpp"

class DefaultGUIPanel
{
public:
    DefaultGUIPanel() = default;

    void handleEvent(sf::Event& event);

    void resetHoverRect();
    
protected:
    void drawPanel(sf::RenderTarget& window);

    void updateAndDrawSelectionHoverRect(sf::RenderTarget& window, float dt);
    void setSelectedElement(ElementID selected);

    int getScaledPanelPaddingX();

protected:
    static const int panelPaddingX = 250;
    static const int panelWidth = 500;

    static const ButtonStyle buttonStyle;

    GUIContext guiContext;

    ElementID selectedElementId = std::numeric_limits<uint64_t>::max();
    sf::FloatRect selectionHoverRect;
    // sf::FloatRect selectionHoverRectDestination;

    bool deferHoverRectReset = false;
    bool deferForceElementActivation = false;
};