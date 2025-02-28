#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <SFML/Graphics.hpp>

#include "Core/SpriteBatch.hpp"
#include "Core/TextureManager.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Camera.hpp"
#include "Core/Helper.hpp"
#include "Core/CollisionCircle.hpp"

#include "Data/typedefs.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

class Projectile
{
public:
    Projectile() = default;
    // Angle in DEGREES
    Projectile(sf::Vector2f position, float angle, ProjectileType type, float damageMult, float shootPower);

    void update(float dt);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera);

    int getDamage() const;

    // Called on collision with entity
    void onCollision();

    sf::Vector2f getPosition() const;

    void handleWorldWrap(sf::Vector2f positionDelta);

    bool isAlive();

    CollisionCircle getCollisionCircle() const;

private:
    ProjectileType projectileType;
    int damage;

    float speed;

    sf::Vector2f position;
    sf::Vector2f velocity;
    float angle;

    bool alive;
    float timeAlive;

};