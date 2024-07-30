#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Object/WorldObject.hpp"
#include "Object/ObjectReference.hpp"
#include "Player/Inventory.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"

class BuildableObject : public WorldObject
{
public:
    BuildableObject(sf::Vector2f position, unsigned int objectType);

    void update(float dt);

    void draw(sf::RenderWindow& window, float dt, const sf::Color& color) override;
    void drawGUI(sf::RenderWindow& window, float dt, const sf::Color& color);

    void interact() override;

    void setWorldPosition(sf::Vector2f position);

    inline unsigned int getObjectType() const {return objectType;}

    inline bool isAlive() override {return health > 0;}

    // When used as a reference to another object
    BuildableObject(ObjectReference _objectReference);

    inline bool isObjectReference() const {return objectReference.has_value();}

    inline const std::optional<ObjectReference>& getObjectReference() const {return objectReference;}

private:
    unsigned int objectType = 0;
    int health = 0;
    float flash_amount;

    // If reference to a buildable object
    std::optional<ObjectReference> objectReference = std::nullopt;

};