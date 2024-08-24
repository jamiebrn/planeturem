#pragma once

#include <SFML/Graphics.hpp>

class AnimatedTexture
{
public:
    AnimatedTexture() = default;
    AnimatedTexture(int frameCount, int frameWidth, int frameHeight, int xStart, int y, float maxFrameTick);

    void create(int frameCount, int frameWidth, int frameHeight, int xStart, int y, float maxFrameTick);
    
    void update(float dt);

    sf::IntRect getTextureRect();

    inline void setFrame(int frame) {this->frame = frame;}
    inline int getFrame() {return frame;}

private:
    int xStart;
    int y;

    int frameCount;
    int frameWidth;
    int frameHeight;
    int frame;

    float maxFrameTick;
    float frameTick;

};

// Use to save memory when texture rects, frame count, and max frame tick are stored elsewhere
// Used when animations are used across many objects
class AnimatedTextureMinimal
{
public:
    AnimatedTextureMinimal() = default;

    void update(float dt, int frameCount, float maxFrameTick);

    inline void setFrame(int frame) {this->frame = frame;}
    inline int getFrame() {return frame;}

private:
    int frame = 0;

    float frameTick = 0;
};