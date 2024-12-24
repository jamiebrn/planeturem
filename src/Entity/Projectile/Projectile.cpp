#include "Entity/Projectile/Projectile.hpp"

Projectile::Projectile(sf::Vector2f position, float angle, ProjectileType type, float damageMult, float shootPower)
{
    this->position = position;

    this->angle = angle;

    projectileType = type;
    
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(type);
    
    speed = projectileData.speed * shootPower;

    // Randomise damage
    int damageBaseValue = Helper::randInt(projectileData.damageLow, projectileData.damageHigh);
    this->damage = std::round(damageBaseValue * damageMult);

    // Calculate velocity
    float angleRadians = M_PI * angle / 180.0f;
    velocity.x = std::cos(angleRadians) * speed;
    velocity.y = std::sin(angleRadians) * speed;

    alive = true;
    timeAlive = 0.0f;
}

void Projectile::update(float dt)
{
    position += velocity * dt;

    timeAlive += dt;

    // TODO: Change later
    if (timeAlive > 3.0f)
    {
        alive = false;
    }
}

void Projectile::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera)
{
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(projectileType);

    TextureDrawData drawData;
    drawData.type = TextureType::Tools;
    drawData.position = camera.worldToScreenTransform(position);
    drawData.rotation = angle;

    float scale = ResolutionHandler::getScale();
    drawData.scale = sf::Vector2f(scale, scale);
    drawData.centerRatio = projectileData.origin;

    // TextureManager::drawSubTexture(window, drawData, projectileData.textureRect);
    spriteBatch.draw(window, drawData, projectileData.textureRect);
}

int Projectile::getDamage() const
{
    return damage;
}

void Projectile::onCollision()
{
    alive = false;
}

sf::Vector2f Projectile::getPosition() const
{
    return position;
}

void Projectile::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;
}

bool Projectile::isAlive()
{
    return alive;
}

const CollisionCircle Projectile::getCollisionCircle() const
{
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(projectileType);

    sf::Vector2f collisionPos = position;
    collisionPos += Helper::rotateVector(projectileData.collisionOffset, angle / 180.0f * M_PI);

    return CollisionCircle(collisionPos.x, collisionPos.y, projectileData.collisionRadius);
}