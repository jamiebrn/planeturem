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