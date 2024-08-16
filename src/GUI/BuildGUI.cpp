#include "GUI/BuildGUI.hpp"

int BuildGUI::selectedItem = 0;
int BuildGUI::selectedCategory = 0;

void BuildGUI::changeSelectedObject(int change)
{
    std::string categoryString = BuildRecipeLoader::getCategoryString(selectedCategory);

    selectedItem = std::min(std::max(selectedItem + change, 0), (int)BuildRecipeLoader::getBuildRecipeDataCategory(categoryString).size() - 1);
}

void BuildGUI::changeSelectedCategory(int change)
{
    int categoryCount = static_cast<int>(BuildRecipeLoader::getBuildRecipeData().size());
    selectedCategory = ((selectedCategory + change) % categoryCount + categoryCount) % categoryCount;
    selectedItem = 0;
}

ObjectType BuildGUI::getSelectedObject()
{
    return BuildRecipeLoader::getRecipeProductObject(selectedItem, selectedCategory);
}

void BuildGUI::draw(sf::RenderWindow& window)
{
    // Get resolution  
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw background
    sf::RectangleShape background({800, 140});

    background.setOrigin(background.getSize() / 2.0f);
    background.setPosition({resolution.x / 2.0f, resolution.y - 90.0f});
    background.setFillColor({40, 40, 40, 130});

    window.draw(background);

    // Draw recipes
    static const sf::Vector2f recipeBoxSize(80, 80);

    int xGap = 20;
    float xOffset = -selectedItem * (recipeBoxSize.x + xGap);
    sf::Vector2f recipeBoxPosition(resolution.x / 2 + xOffset - recipeBoxSize.x - xGap, resolution.y - 70.0f);

    int selectionBoxExtraSize = 10;

    int maxRecipesEachSide = 3;

    int recipeIndex = -1;

    std::string categoryString = BuildRecipeLoader::getCategoryString(selectedCategory);

    bool drawScrollLeftPrompt = false;
    bool drawScrollRightPrompt = false;

    for (auto& recipePair : BuildRecipeLoader::getBuildRecipeDataCategory(categoryString))
    {
        recipeIndex++;
        recipeBoxPosition.x += recipeBoxSize.x + xGap;

        if (recipeIndex < selectedItem - maxRecipesEachSide)
        {
            drawScrollLeftPrompt = true;
            continue;
        }
        
        if (recipeIndex > selectedItem + maxRecipesEachSide)
        {
            drawScrollRightPrompt = true;
            continue;
        }

        // If selected, draw white box to show selected
        if (selectedItem == recipeIndex)
        {
            // Draw selection background box
            sf::RectangleShape selectionBox(sf::Vector2f(recipeBoxSize.x + selectionBoxExtraSize, recipeBoxSize.y + selectionBoxExtraSize));
            selectionBox.setPosition(recipeBoxPosition);
            selectionBox.setOrigin(selectionBox.getSize() / 2.0f);
            selectionBox.setFillColor({220, 220, 220, 140});

            window.draw(selectionBox);
        }

        // Draw background box
        sf::RectangleShape recipeBox(recipeBoxSize);
        recipeBox.setPosition(recipeBoxPosition);
        recipeBox.setOrigin(recipeBoxSize / 2.0f);
        recipeBox.setFillColor({40, 40, 40, 140});

        window.draw(recipeBox);

        // Draw recipe object
        // ObjectType type = recipePair.first;

        // BuildableObject recipeObject(static_cast<BuildableObject*>(createObjectFromType(type, recipeBoxPosition + sf::Vector2f(40, 40)).release()));
        BuildableObject recipeObject(recipeBoxPosition, recipePair.first);

        recipeObject.drawGUI(window, 0, {255, 255, 255, 255});

        // Draw recipe data if object selected to build
        if (selectedItem == recipeIndex)
        {
            sf::RectangleShape recipeItemsBox({400, 68});

            recipeItemsBox.setOrigin(recipeItemsBox.getSize() / 2.0f);
            recipeItemsBox.setPosition({resolution.x / 2.0f, resolution.y - 200.0f});
            recipeItemsBox.setFillColor({40, 40, 40, 130});

            window.draw(recipeItemsBox);

            // Draw recipe data
            sf::Vector2f recipeItemPos({resolution.x / 2 - 190.0f, resolution.y - 224.0f});

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

            // Draw name of object
            const ObjectData& objectData = ObjectDataLoader::getObjectData(recipePair.first);

            TextDraw::drawText(window, {objectData.name, sf::Vector2f(resolution.x / 2.0f, resolution.y - 130.0f), {255, 255, 255}, 24, {0, 0, 0}, 0, true, true});
        }
    }

    TextDraw::drawText(window, {categoryString, sf::Vector2f(resolution.x / 2.0f, resolution.y - 150.0f), {255, 255, 255}, 32, {0, 0, 0}, 0, true, true});

    drawUIKeyboardButton(window, "Q", background.getPosition() - background.getSize() / 2.0f, 3, false);
    drawUIKeyboardButton(window, "E", background.getPosition() + sf::Vector2f(background.getSize().x, -background.getSize().y) / 2.0f - sf::Vector2f(16, 0) * 3.0f, 3, false);

    if (drawScrollLeftPrompt)
        drawUIMouseScrollWheelDirection(window, 1, background.getPosition() + sf::Vector2f(-background.getSize().x, 0) / 2.0f, 3, false);
    
    if (drawScrollRightPrompt)
        drawUIMouseScrollWheelDirection(window, -1, background.getPosition() + sf::Vector2f(background.getSize().x, 0) / 2.0f - sf::Vector2f(16, 0) * 3.0f, 3, false);
}