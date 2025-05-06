#include "Entity/Projectile/Projectile.hpp"
#include "World/ChunkManager.hpp"

Projectile::Projectile(pl::Vector2f position, float angle, ProjectileType type, float damageMult, float shootPower, HitLayer hitLayer)
{
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(type);
    
    int speed = projectileData.speed * shootPower;
    
    // Calculate velocity
    float angleRadians = M_PI * angle / 180.0f;
    velocity.x = std::cos(angleRadians) * speed;
    velocity.y = std::sin(angleRadians) * speed;
    
    initialise(position, velocity, type, damageMult, hitLayer);
}

Projectile::Projectile(pl::Vector2f position, pl::Vector2f velocity, ProjectileType type, float damageMult, HitLayer hitLayer)
{
    initialise(position, velocity, type, damageMult, hitLayer);
}

void Projectile::initialise(pl::Vector2f position, pl::Vector2f velocity, ProjectileType type, float damageMult, HitLayer hitLayer)
{   
    this->position = position;
    this->velocity = velocity;
    
    projectileType = type;
    
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(type);
    
    // Randomise damage
    int damageBaseValue = Helper::randInt(projectileData.damageLow, projectileData.damageHigh);
    this->damage = std::round(damageBaseValue * damageMult);

    this->hitLayer = hitLayer;

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

void Projectile::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const ChunkManager& chunkManager, pl::Vector2f playerPos, const Camera& camera)
{
    const ProjectileData& projectileData = ToolDataLoader::getProjectileData(projectileType);

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::Tools);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = camera.worldToScreenTransform(chunkManager.translatePositionAroundWorld(position, playerPos));
    drawData.rotation = std::atan2(velocity.y, velocity.x) / M_PI * 180.0f;
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

pl::Vector2f Projectile::getVelocity() const
{
    return velocity;
}

ProjectileType Projectile::getType() const
{
    return projectileType;
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
    collisionPos += projectileData.collisionOffset.rotate(std::atan2(velocity.y, velocity.x));

    return CollisionCircle(collisionPos.x, collisionPos.y, projectileData.collisionRadius);
}

HitLayer Projectile::getHitLayer() const
{
    return hitLayer;
}