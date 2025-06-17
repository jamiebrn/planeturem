#pragma once

#include <vector>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/DrawData.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Data/Serialise/IntRectSerialise.hpp"

#include "Network/CompactFloat.hpp"

#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"

#include "Object/WorldObject.hpp"

class Game;

struct ParticleStyle
{
    std::vector<pl::Rect<uint16_t>> textureRects;
    float timePerFrame;
    float alpha = 1.0f;

    template <class Archive>
    void save(Archive& ar) const
    {
        CompactFloat<uint8_t> alphaCompact(alpha, 2);
        CompactFloat<uint8_t> timePerFrameCompact(timePerFrame, 2);
        ar(textureRects, timePerFrameCompact, alphaCompact);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        CompactFloat<uint8_t> alphaCompact;
        CompactFloat<uint8_t> timePerFrameCompact(timePerFrame, 2);
        ar(textureRects, timePerFrameCompact, alphaCompact);
        alpha = alphaCompact.getValue(2);
        timePerFrame = timePerFrameCompact.getValue(2);
    }
};

class Particle : public WorldObject
{
public:
    Particle();
    Particle(pl::Vector2f position, pl::Vector2f velocity, pl::Vector2f acceleration, int drawLayer, const ParticleStyle& style);

    void update(float dt);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
        const pl::Color& color) const override;
    
    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize) const;

    bool isAlive();

    template <class Archive>
    void save(Archive& ar) const
    {
        uint64_t positionVelocityCompact = position.x * 10;
        positionVelocityCompact = (positionVelocityCompact << 19) | (static_cast<int>(position.y * 10) & BIT_MASK(19));

        pl::Vector2f direction = velocity.normalise();

        CompactFloat<uint8_t> directionXCompact(direction.x, 2);
        CompactFloat<uint8_t> directionYCompact(direction.y, 2);

        uint8_t speed = velocity.getLength();

        positionVelocityCompact = (positionVelocityCompact << 8) | (directionXCompact.getCompactValue());
        positionVelocityCompact = (positionVelocityCompact << 8) | (directionYCompact.getCompactValue());
        positionVelocityCompact = (positionVelocityCompact << 8) | (speed);

        pl::Vector2f accelerationDir = acceleration.normalise();

        CompactFloat<uint8_t> accelerationDirXCompact(accelerationDir.x, 2);
        CompactFloat<uint8_t> accelerationDirYCompact(accelerationDir.y, 2);

        uint8_t accelerationMagnitude = acceleration.getLength();

        int8_t drawLayerCompact = drawLayer;

        ar(positionVelocityCompact, accelerationDirXCompact, accelerationDirYCompact, accelerationMagnitude, drawLayerCompact, particleStyle);
    }
    
    template <class Archive>
    void load(Archive& ar)
    {
        uint64_t positionVelocityCompact;
        CompactFloat<uint8_t> accelerationDirXCompact;
        CompactFloat<uint8_t> accelerationDirYCompact;
        
        uint8_t accelerationMagnitude;    
        int8_t drawLayerCompact;

        ar(positionVelocityCompact, accelerationDirXCompact, accelerationDirYCompact, accelerationMagnitude, drawLayerCompact, particleStyle);

        uint8_t speed = positionVelocityCompact & BIT_MASK(8);

        CompactFloat<uint8_t> directionXCompact;
        CompactFloat<uint8_t> directionYCompact;

        directionYCompact.setCompactValue((positionVelocityCompact >> 8) & BIT_MASK(8));
        directionXCompact.setCompactValue((positionVelocityCompact >> 16) & BIT_MASK(8));

        velocity.x = directionXCompact.getValue(2) * speed;
        velocity.y = directionXCompact.getValue(2) * speed;

        acceleration.x = accelerationDirXCompact.getValue(2) * accelerationMagnitude;
        acceleration.y = accelerationDirYCompact.getValue(2) * accelerationMagnitude;

        drawLayer = drawLayerCompact;
    }

private:
    pl::Vector2f velocity;
    pl::Vector2f acceleration;

    // std::vector<pl::Rect<uint16_t>> textureRects;
    // float alpha;
    int currentFrame;
    float frameTimer;
    // float timePerFrame;
    ParticleStyle particleStyle;
};

class ParticleSystem
{
public:
    ParticleSystem() = default;

    // Pass in game ptr for automatic networking as host
    void addParticle(const Particle& particle, const LocationState& locationState, Game* game);

    void update(float dt);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize) const;

    // void handleWorldWrap(pl::Vector2f positionDelta);

    void clear();

    std::vector<WorldObject*> getParticleWorldObjects();

private:
    std::vector<Particle> particles;
};