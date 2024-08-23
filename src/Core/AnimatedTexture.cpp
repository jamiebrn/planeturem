#include "Core/AnimatedTexture.hpp"

AnimatedTexture::AnimatedTexture(int frameCount, int frameWidth, int frameHeight, int xStart, int y, float maxFrameTick)
{
    create(frameCount, frameWidth, frameHeight, xStart, y, maxFrameTick);
}

void AnimatedTexture::create(int frameCount, int frameWidth, int frameHeight, int xStart, int y, float maxFrameTick)
{
    this->frameCount = frameCount;
    this->frameWidth = frameWidth;
    this->frameHeight = frameHeight;
    this->xStart = xStart;
    this->y = y;
    this->maxFrameTick = maxFrameTick;

    frame = 0;
    frameTick = 0;
}

void AnimatedTexture::update(float dt)
{
    frameTick += dt;
    if (frameTick >= maxFrameTick)
    {
        frameTick = 0;
        frame++;

        if (frame >= frameCount)
        {
            frame = 0;
        }
    }
}

sf::IntRect AnimatedTexture::getTextureRect()
{
    sf::IntRect textureRect;
    textureRect.left = xStart + frame * frameWidth;
    textureRect.top = y;
    textureRect.width = frameWidth;
    textureRect.height = frameHeight;

    return textureRect;
}

void AnimatedTextureMinimal::update(float dt, int frameCount, float maxFrameTick)
{
    frameTick += dt;
    if (frameTick >= maxFrameTick)
    {
        frameTick = 0;
        frame++;

        if (frame >= frameCount)
        {
            frame = 0;
        }
    }
}