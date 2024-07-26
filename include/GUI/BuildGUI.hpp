#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "Core/TextDraw.hpp"
// #include "Object/ObjectBuilder.hpp"
// #include "Data/BuildRecipes.hpp"
#include "Object/BuildableObject.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"

class BuildGUI
{
    BuildGUI() = delete;

public:
    static void changeSelectedObject(int change);

    static unsigned int getSelectedObject();

    static void draw(sf::RenderWindow& window);

private:
    static int selectedItem;

};