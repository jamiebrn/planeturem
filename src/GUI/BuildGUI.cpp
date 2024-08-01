#include "GUI/BuildGUI.hpp"

int BuildGUI::selectedItem = 0;

void BuildGUI::changeSelectedObject(int change)
{
    selectedItem = std::min(std::max(selectedItem + change, 0), (int)BuildRecipeLoader::getBuildRecipeData().size() - 1);
}

unsigned int BuildGUI::getSelectedObject()
{
    std::map<unsigned int, BuildRecipe>::const_iterator iter = BuildRecipeLoader::getBuildRecipeData().begin();
    std::advance(iter, selectedItem);
    return iter->first;
}

void BuildGUI::draw(sf::RenderWindow& window)
{
    // Get resolution  
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();

    // Draw background
    sf::RectangleShape background({800, 100});

    background.setOrigin({400, 50});
    background.setPosition({resolution.x / 2.0f, resolution.y - 70.0f});
    background.setFillColor({40, 40, 40, 130});

    window.draw(background);

    // Draw recipes
    int recipeIndex = 0;
    sf::Vector2f recipeBoxPosition(resolution.x / 2 - 390, resolution.y - 110.0f);

    for (auto& recipePair : BuildRecipeLoader::getBuildRecipeData())
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
        // ObjectType type = recipePair.first;

        // BuildableObject recipeObject(static_cast<BuildableObject*>(createObjectFromType(type, recipeBoxPosition + sf::Vector2f(40, 40)).release()));
        BuildableObject recipeObject(recipeBoxPosition + sf::Vector2f(40, 40), recipePair.first);

        recipeObject.drawGUI(window, 0, {255, 255, 255, 255});

        // Draw recipe data if object selected to build
        if (selectedItem == recipeIndex)
        {
            sf::RectangleShape recipeItemsBox({400, 68});

            recipeItemsBox.setOrigin({200, 34});
            recipeItemsBox.setPosition({resolution.x / 2.0f, resolution.y - 164.0f});
            recipeItemsBox.setFillColor({40, 40, 40, 130});

            window.draw(recipeItemsBox);

            // Draw recipe data
            sf::Vector2f recipeItemPos({resolution.x / 2 - 190.0f, resolution.y - 188.0f});

            for (auto& recipeItemPair : recipePair.second.itemRequirements)
            {
                // Draw item
                unsigned int type = recipeItemPair.first;
                TextureManager::drawSubTexture(window, {
                    TextureType::Items, recipeItemPos + sf::Vector2f(24, 24), 0, {3, 3}, {0.5, 0.5}
                    }, ItemDataLoader::getItemData(type).textureRect);

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