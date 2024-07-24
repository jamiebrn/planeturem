#include "GUI/BuildGUI.hpp"

int BuildGUI::selectedItem = 0;

void BuildGUI::changeSelectedObject(int change)
{
    selectedItem = std::max(selectedItem + change, 0);
}

ObjectType BuildGUI::getSelectedObject()
{
    std::map<ObjectType, std::map<ItemType, int>>::const_iterator iter = BuildRecipes.begin();
    std::advance(iter, selectedItem);
    return iter->first;
}

void BuildGUI::draw(sf::RenderWindow& window)
{
    // Draw background
    sf::RectangleShape background({800, 100});

    background.setOrigin({400, 50});
    background.setPosition({1280 / 2, 650});
    background.setFillColor({40, 40, 40, 130});

    window.draw(background);

    // Draw recipes
    int recipeIndex = 0;
    sf::Vector2f recipeBoxPosition(1280 / 2 - 390, 650 - 40);

    for (auto& recipePair : BuildRecipes)
    {
        // If selected, draw white box to show selected
        if (selectedItem == recipeIndex)
        {
            // Draw selection background box
            sf::RectangleShape selectionBox({90, 90});
            selectionBox.setPosition(recipeBoxPosition - sf::Vector2f(5, 5));
            selectionBox.setFillColor({220, 220, 220, 140});

            window.draw(selectionBox);
        }

        // Draw background box
        sf::RectangleShape recipeBox({80, 80});
        recipeBox.setPosition(recipeBoxPosition);
        recipeBox.setFillColor({40, 40, 40, 140});

        window.draw(recipeBox);

        // Draw recipe object
        ObjectType type = recipePair.first;

        std::unique_ptr<BuildableObject> recipeObject(static_cast<BuildableObject*>(createObjectFromType(type, recipeBoxPosition + sf::Vector2f(40, 40)).release()));

        recipeObject->drawGUI(window, 0, 255);

        // Draw recipe data if object selected to build
        if (selectedItem == recipeIndex)
        {
            sf::RectangleShape recipeItemsBox({400, 68});

            recipeItemsBox.setOrigin({200, 34});
            recipeItemsBox.setPosition({1280 / 2, 556});
            recipeItemsBox.setFillColor({40, 40, 40, 130});

            window.draw(recipeItemsBox);

            // Draw recipe data
            sf::Vector2f recipeItemPos({1280 / 2 - 190, 556 - 24});

            for (auto& recipeItemPair : recipePair.second)
            {
                // Draw item
                ItemType type = recipeItemPair.first;
                TextureManager::drawSubTexture(window, {TextureType::Items, recipeItemPos + sf::Vector2f(24, 24), 0, 3, {0.5, 0.5}}, ItemDataMap.at(type).textureRect);

                // Draw amount
                TextDraw::drawText(window, {
                    std::to_string(recipeItemPair.second), recipeItemPos + sf::Vector2f(16, 16) * 3.0f, {255, 255, 255}, 24, {0, 0, 0}, 0, true, true
                    });
                
                recipeItemPos.x += 68;
            }
        }

        recipeIndex++;
        recipeBoxPosition.x += 100;
    }

}