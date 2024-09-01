#include "Core/AnimatedTexture.hpp"

AnimatedTexture::AnimatedTexture(int frameCount, int frameWidth, int frameHeight, int xStart, int y, float maxFrameTick, bool looping)
{
    create(frameCount, frameWidth, frameHeight, xStart, y, maxFrameTick);
}

void AnimatedTexture::create(int frameCount, int frameWidth, int frameHeight, int xStart, int y, float maxFrameTick, bool looping)
{
    this->frameCount = frameCount;
    this->frameWidth = frameWidth;
    this->frameHeight = frameHeight;
    this->xStart = xStart;
    this->y = y;
    this->maxFrameTick = maxFrameTick;

    frame = 0;
    frameTick = 0;

    this->looping = looping;
}

void AnimatedTexture::update(float dt, int direction)
{
    frameTick += dt;
    if (frameTick >= maxFrameTick)
    {
        frameTick = 0;
        frame += direction;

        if (direction > 0)
        {
            if (frame >= frameCount)
            {
                if (looping) frame = 0;
                else frame = frameCount - 1;
            }
        }
        else
        {
            if (frame < 0)
            {
                if (looping) frame = frameCount - 1;
                else frame = 0;
            }
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

void AnimatedTextureMinimal::update(float dt, int direction, int frameCount, float maxFrameTick, bool looping)
{
    frameTick += dt;
    if (frameTick >= maxFrameTick)
    {
        frameTick = 0;
        frame += direction;

        if (frame >= frameCount)
        {
            if (looping) frame = 0;
            else frame = frameCount - 1;
        }
        else if (frame < 0)
        {
            if (looping) frame = frameCount - 1;
            else frame = 0;
        }
    }
}