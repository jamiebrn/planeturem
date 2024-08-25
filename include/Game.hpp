#pragma once

#include <SFML/Graphics.hpp>
#include <steam_api.h>

#include <World/FastNoise.h>
#include <Core/json.hpp>
#include <cmath>
#include <array>
#include <unordered_map>
#include <iostream>
#include <string>
#include <memory>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/Sounds.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/Tween.hpp"

#include "World/ChunkManager.hpp"

#include "Player/Player.hpp"
#include "Player/Cursor.hpp"
#include "Player/FurnaceJob.hpp"

#include "Data/ItemData.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ItemData.hpp"
#include "Data/RecipeData.hpp"
#include "Data/EntityData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/RecipeDataLoader.hpp"
#include "Data/EntityDataLoader.hpp"
#include "Data/ToolDataLoader.hpp"

#include "Types/GameState.hpp"
#include "Types/WorldMenuState.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/FurnaceGUI.hpp"

class Game
{
public:
    Game();

    bool initialise();

    void run();

private:
    void runMenu(float dt);
    void runInShip(float dt);
    void runOnPlanet(float dt);

    void attemptUseTool();
    void attemptObjectInteract();
    void attemptBuildObject();
    void attemptPlaceLand();

    void drawGhostPlaceTileAtCursor();

    void generateWaterNoiseTexture();

    void handleZoom(int zoomChange);

    void handleEventsWindow(sf::Event& event);

    void toggleFullScreen();
    void handleWindowResize(sf::Vector2u newSize);

private:
    sf::RenderWindow window;
    sf::View view;
    sf::Image icon;
    bool fullScreen = true;

    bool steamInitialised;

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

    GameState gameState;

    WorldMenuState worldMenuState;

    std::unordered_map<std::string, int> nearbyCraftingStationLevels;

    // Not used yet
    uint64_t interactedObjectID;
    sf::Vector2f interactedObjectPos;

    Tween<float> floatTween;
};