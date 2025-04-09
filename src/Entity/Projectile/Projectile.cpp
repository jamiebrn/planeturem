#include "Entity/Projectile/Projectile.hpp"

Projectile::Projectile(pl::Vector2f position, float angle, ProjectileType type, float damageMult, float shootPower)
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

void Projectile::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera)
{
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(projectileType);

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::Tools);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = camera.worldToScreenTransform(position);
    drawData.rotation = angle;
    drawData.textureRect = projectileData.textureRect;

    float scale = ResolutionHandler::getScale();
    drawData.scale = pl::Vector2f(scale, scale);
    drawData.centerRatio = projectileData.origin;

    spriteBatch.draw(window, drawData);
}

int Projectile::getDamage() const
{
    return damage;
}

void Projectile::onCollision()
{
    alive = false;
}

pl::Vector2f Projectile::getPosition() const
{
    return position;
}

void Projectile::handleWorldWrap(pl::Vector2f positionDelta)
{
    position += positionDelta;
}

bool Projectile::isAlive()
{
    return alive;
}

CollisionCircle Projectile::getCollisionCircle() const
{
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(projectileType);

    pl::Vector2f collisionPos = position;
    collisionPos += projectileData.collisionOffset.rotate(angle / 180.0f * M_PI);

    return CollisionCircle(collisionPos.x, collisionPos.y, projectileData.collisionRadius);
}