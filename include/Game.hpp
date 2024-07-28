#pragma once

#include <SFML/Graphics.hpp>
#include <World/FastNoiseLite.h>
#include <Core/json.hpp>
#include <math.h>
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
#include "World/ChunkManager.hpp"
#include "Player/Player.hpp"
#include "Player/Cursor.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"

#include "GUI/InventoryGUI.hpp"
#include "GUI/BuildGUI.hpp"

class Game
{
public:
    Game();

    bool initialise();

    void run();

private:
    sf::RenderWindow window;
    sf::View view;

    FastNoiseLite noise;

    Player player;
    ChunkManager chunkManager;

    bool inventoryOpen;
    bool buildMenuOpen;
};