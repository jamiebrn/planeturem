#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Object/BuildableObject.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/BuildRecipeLoader.hpp"

#include "GUI/GUIDraw.hpp"

class BuildGUI
{
    BuildGUI() = delete;

public:
    static void changeSelectedObject(int change);
    static void changeSelectedCategory(int change);

    static ObjectType getSelectedObject();

    static void draw(sf::RenderWindow& window);

private:
    static int selectedItem;
    static int selectedCategory;

};