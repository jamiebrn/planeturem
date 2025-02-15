#include "World/WeatherSystem.hpp"

WeatherParticle::WeatherParticle(sf::Vector2f position, const WeatherTypeData& weatherTypeData)
    : WorldObject(position)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();
    float scale = ResolutionHandler::getScale();

    animatedTexture = weatherTypeData.particleAnimatedTexture;

    float fallSpeed = Helper::randFloat(weatherTypeData.fallSpeedMin, weatherTypeData.fallSpeedMax);
    velocity = Helper::rotateVector(sf::Vector2f(1, 0), weatherTypeData.fallAngle / 180.0f * M_PI) * fallSpeed;

    fallTime = 0.0f;
    targetFallTime = Helper::randFloat(0.2f, resolution.y / velocity.y / scale * 1.2f);
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

WeatherSystem::WeatherSystem(float gameTime, int seed)
{
    currentWeatherType = WeatherType::None;
    
    redLightBias = 1.0f;
    greenLightBias = 1.0f;
    blueLightBias = 1.0f;
    
    particleSpawnTimer = 0.0f;

    weatherNoise.SetSeed(seed);
    weatherNoise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
    
    updateCurrentWeather(gameTime, true);
}

void WeatherSystem::update(float dt, float gameTime, const Camera& camera, ChunkManager& chunkManager)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();
    float scale = ResolutionHandler::getScale();
    
    const WeatherTypeData& destinationWeatherTypeData = weatherTypeDatas.at(destinationWeatherType);

    particleSpawnTimer += dt * Helper::randFloat(1.0f, 2.0f);

    float particleSpawnRateMult = resolution.x / 1920.0f;
    
    while (particleSpawnTimer * particleSpawnRateMult >= PARTICLE_SPAWN_RATE)
    {
        particleSpawnTimer -= PARTICLE_SPAWN_RATE;

        sf::Vector2f position;
        position.x = Helper::randInt(-camera.getDrawOffset().x - resolution.x / 2, -camera.getDrawOffset().x + resolution.x / 3 + resolution.x / 2);
        position.y = -camera.getDrawOffset().y;

        // Spawn some particles at bottom of screen to keep particle density if player moves downwards
        if (Helper::randFloat(0.0f, 1.0f) <= 0.3f)
        {
            position.y += resolution.y / scale + 32;
        }
        
        // Make either current weather or destination weather particle more likely depending on transition progress
        if (Helper::randFloat(0.0f, 1.0f) >= getDestinationTransitionProgress())
        {
            // Spawn current weather particle
            if (currentWeatherType != WeatherType::None)
            {
                weatherParticles.push_back(WeatherParticle(position, weatherTypeDatas.at(currentWeatherType)));
            }
        }
        else
        {
            // Spawn destination weather particle
            if (destinationWeatherType != WeatherType::None)
            {
                weatherParticles.push_back(WeatherParticle(position, destinationWeatherTypeData));
            }
        }
    }
    
    redLightBias = Helper::step(redLightBias, destinationWeatherTypeData.redLightBias, LIGHT_BIAS_TRANSITION_SPEED * dt);
    greenLightBias = Helper::step(greenLightBias, destinationWeatherTypeData.greenLightBias, LIGHT_BIAS_TRANSITION_SPEED * dt);
    blueLightBias = Helper::step(blueLightBias, destinationWeatherTypeData.blueLightBias, LIGHT_BIAS_TRANSITION_SPEED * dt);

    updateCurrentWeather(gameTime, false);

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

void WeatherSystem::updateCurrentWeather(float gameTime, bool initialise)
{
    float noiseValue = sampleWeatherFunction(gameTime);
    
    WeatherType newWeatherType;
    if (noiseValue >= 0)
    {
        newWeatherType = WeatherType::None;
    }
    else
    {
        newWeatherType = WeatherType::Rain;
    }

    if (initialise)
    {
        currentWeatherType = newWeatherType;
        destinationWeatherType = newWeatherType;

        const WeatherTypeData& currentWeatherTypeData = weatherTypeDatas.at(currentWeatherType);
        redLightBias = currentWeatherTypeData.redLightBias;
        greenLightBias = currentWeatherTypeData.greenLightBias;
        blueLightBias = currentWeatherTypeData.blueLightBias;
    }

    if (newWeatherType != currentWeatherType)
    {
        destinationWeatherType = newWeatherType;
    }

    static constexpr float TRANSITION_PROGRESS_COMPLETE_THRESHOLD = 0.95f;

    if (getDestinationTransitionProgress() >= TRANSITION_PROGRESS_COMPLETE_THRESHOLD)
    {
        currentWeatherType = destinationWeatherType;
            
        const WeatherTypeData& destinationWeatherTypeData = weatherTypeDatas.at(destinationWeatherType);

        redLightBias = destinationWeatherTypeData.redLightBias;
        greenLightBias = destinationWeatherTypeData.greenLightBias;
        blueLightBias = destinationWeatherTypeData.blueLightBias;
    }
}

float WeatherSystem::getDestinationTransitionProgress() const
{
    const WeatherTypeData& weatherTypeData = weatherTypeDatas.at(currentWeatherType);
    const WeatherTypeData& destinationWeatherTypeData = weatherTypeDatas.at(destinationWeatherType);

    return (1.0f - (redLightBias - destinationWeatherTypeData.redLightBias) / (weatherTypeData.redLightBias - destinationWeatherTypeData.redLightBias));
}

void WeatherSystem::presimulateWeather(float gameTime, const Camera& camera, ChunkManager& chunkManager)
{
    if (currentWeatherType == WeatherType::None)
    {
        return;
    }

    for (int i = 0; i < WeatherSystem::PRESIMULATION_TICKS; i++)
    {
        update(1.0f / 30.0f, gameTime, camera, chunkManager);
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

float WeatherSystem::getRedLightBias() const
{
    return redLightBias;
}

float WeatherSystem::getGreenLightBias() const
{
    return greenLightBias;
}

float WeatherSystem::getBlueLightBias() const
{
    return blueLightBias;
}

float WeatherSystem::sampleWeatherFunction(float gameTime) const
{
    return weatherNoise.GetNoise(gameTime, 0);
}