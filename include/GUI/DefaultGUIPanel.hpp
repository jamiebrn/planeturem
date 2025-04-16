#pragma once

#include <limits>

#include <SDL_events.h>

#include <Graphics/VertexArray.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/InputManager.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/Base/GUIInputState.hpp"

class DefaultGUIPanel
{
public:
    DefaultGUIPanel() = default;

    void handleEvent(const SDL_Event& event);

    void resetHoverRect();

    const GUIContext& getGUIContext();
    
protected:
    void drawPanel(pl::RenderTarget& window);

    void updateAndDrawSelectionHoverRect(pl::RenderTarget& window, float dt);
    void setSelectedElement(ElementID selected);

    int getScaledPanelPaddingX();

protected:
    static const int panelPaddingX = 250;
    static const int panelWidth = 500;

    static const ButtonStyle buttonStyle;

    GUIContext guiContext;

    ElementID selectedElementId = std::numeric_limits<uint64_t>::max();
    pl::Rect<float> selectionHoverRect;
    
    bool deferHoverRectReset = false;
    bool deferForceElementActivation = false;
};