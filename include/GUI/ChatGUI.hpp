#pragma once

#include <cmath>
#include <vector>
#include <string>

#include <SDL2/SDL_events.h>

#include <Graphics/VertexArray.hpp>
#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"

class ChatGUI
{
public:
    void initialise();

    void setShowing(bool enabled);
    void activate();

    bool isActive();

    void handleEvent(const SDL_Event& event);

    void draw(pl::RenderTarget& window);

private:
    void attemptSendMessage();

    std::vector<std::string> chatLog;

    std::string messageBuffer;

    bool showing = false;
    bool active = false;

};