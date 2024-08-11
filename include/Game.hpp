#pragma once

#include <SFML/Graphics.hpp>
#include <World/FastNoise.h>
#include <Core/json.hpp>
#include <cmath>
#include <array>
#include <map>
#include <iostream>
#include <string>
#include <memory>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/Tween.hpp"
#include "World/ChunkManager.hpp"
#include "Player/Player.hpp"
#include "Player/Cursor.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"
#include "Data/EntityDataLoader.hpp"
#include "Data/ToolDataLoader.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/BuildGUI.hpp"

class Game
{
public:
    Game();

    bool initialise();

    void run();

private:
    void toggleFullScreen();
    void handleWindowResize(sf::Vector2u newSize);

    void generateWaterNoiseTexture();

    void handleZoom(int zoomChange);

    void handleEvents();
    void attemptUseTool();
    void attemptBuildObject();

private:
    sf::RenderWindow window;
    sf::View view;
    sf::Image icon;
    bool fullScreen = true;

    sf::Clock clock;
    float gameTime;

    int worldSize;

    float dayNightToggleTimer;
    float worldDarkness;
    bool isDay;

    FastNoise noise;

    Player player;
    ChunkManager chunkManager;

    std::array<sf::Texture, 2> waterNoiseTextures;

    bool inventoryOpen;
    bool buildMenuOpen;

    Tween<float> floatTween;
};