#include "World/WeatherSystem.hpp"

WeatherParticle::WeatherParticle(sf::Vector2f position, const WeatherTypeData& weatherTypeData)
    : WorldObject(position)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();

    animatedTexture = weatherTypeData.particleAnimatedTexture;

    float fallSpeed = Helper::randFloat(weatherTypeData.fallSpeedMin, weatherTypeData.fallSpeedMax);
    velocity = Helper::rotateVector(sf::Vector2f(1, 0), weatherTypeData.fallAngle / 180.0f * M_PI) * fallSpeed;

    fallTime = 0.0f;
    targetFallTime = Helper::randFloat(0.7f, resolution.y / velocity.y * 1.2f);
}

bool WeatherParticle::update(float dt, const Camera& camera, ChunkManager& chunkManager)
{
    fallTime += dt;
    if (fallTime < targetFallTime)
    {
        position += velocity * dt;
    }
    else
    {
        animatedTexture.update(dt);
        if (animatedTexture.isFinished())
        {
            return false;
        }
    }

    return true;
}

sf::Vector2f WeatherParticle::getPosition() const
{
    return (position + (targetFallTime - fallTime) * velocity);
}

void WeatherParticle::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;
}

void WeatherParticle::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    float scale = ResolutionHandler::getScale();

    TextureDrawData drawData;
    drawData.type = TextureType::Objects;
    drawData.position = camera.worldToScreenTransform(position);
    drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);
    drawData.scale = sf::Vector2f(scale, scale);

    spriteBatch.draw(window, drawData, animatedTexture.getTextureRect());
}

const std::unordered_map<WeatherType, WeatherTypeData> WeatherSystem::weatherTypeDatas = {
    {WeatherType::None, {{}, 1.0f, 1.0f, 1.0f}},
    {WeatherType::Rain, {{4, 16, 16, 0, 272, 0.05f, false}, 0.6f, 0.75f, 0.85f, 110.0f, 130.0f, 175.0f}}
};

WeatherSystem::WeatherSystem()
{
    currentWeatherType = WeatherType::None;
    particleSpawnTimer = 0.0f;
}

void WeatherSystem::update(float dt, const Camera& camera, ChunkManager& chunkManager)
{
    if (currentWeatherType != WeatherType::None)
    {
        sf::Vector2u resolution = ResolutionHandler::getResolution();

        static const float PARTICLE_SPAWN_PADDING = 0;

        particleSpawnTimer += dt * Helper::randFloat(1.0f, 2.0f);

        if (particleSpawnTimer >= PARTICLE_SPAWN_RATE)
        {
            particleSpawnTimer = 0.0;
            sf::Vector2f position = camera.screenToWorldTransform(sf::Vector2f(Helper::randInt(0, resolution.x), PARTICLE_SPAWN_PADDING));
            weatherParticles.push_back(WeatherParticle(position, weatherTypeDatas.at(currentWeatherType)));
        }
    }

    for (auto iter = weatherParticles.begin(); iter != weatherParticles.end();)
    {
        if (!iter->update(dt, camera, chunkManager))
        {
            iter = weatherParticles.erase(iter);
            continue;
        }
        iter++;
    }
}

void WeatherSystem::handleWorldWrap(sf::Vector2f positionDelta)
{
    for (WeatherParticle& weatherParticle : weatherParticles)
    {
        weatherParticle.handleWorldWrap(positionDelta);
    }
}

std::vector<WorldObject*> WeatherSystem::getWeatherParticles()
{
    std::vector<WorldObject*> particles;
    for (WeatherParticle& weatherParticle : weatherParticles)
    {
        particles.push_back(&weatherParticle);
    }
    return particles;
}

const WeatherTypeData& WeatherSystem::getWeatherTypeData() const
{
    return weatherTypeDatas.at(currentWeatherType);
}