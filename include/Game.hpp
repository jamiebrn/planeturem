#pragma once

#include <SFML/Graphics.hpp>
#include <steam_api.h>

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
#include "Core/Sounds.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/Tween.hpp"

#include "World/ChunkManager.hpp"

#include "Player/Player.hpp"
#include "Player/Cursor.hpp"
#include "Player/FurnaceJob.hpp"

#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"
#include "Data/FurnaceRecipeLoader.hpp"
#include "Data/EntityDataLoader.hpp"
#include "Data/ToolDataLoader.hpp"

#include "Types/GameState.hpp"
#include "Types/WorldMenuState.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/BuildGUI.hpp"
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

    // Maps furnace IDs to current jobs
    std::unordered_map<uint64_t, FurnaceJob> furnaceJobs;

    GameState gameState;

    WorldMenuState worldMenuState;
    uint64_t interactedObjectID;
    sf::Vector2f interactedObjectPos;

    Tween<float> floatTween;
};