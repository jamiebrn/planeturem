#include "GUI/HitMarkers.hpp"

void HitMarkers::addHitMarker(sf::Vector2f position, int damageAmount, bool critical)
{
    HitMarker hitMarker;
    hitMarker.position = position;
    hitMarker.damageAmount = damageAmount;
    hitMarker.critical = critical;

    hitMarkers.push_back(hitMarker);
}

void HitMarkers::update(float dt)
{
    for (auto iter = hitMarkers.begin(); iter != hitMarkers.end();)
    {
        if (iter->isAlive())
        {
            iter->update(dt);
            iter++;
        }
        else
        {
            iter = hitMarkers.erase(iter);
        }
    }
}

void HitMarkers::draw(sf::RenderTarget& window)
{
    for (const HitMarker& hitMarker : hitMarkers)
    {
        hitMarker.draw(window);
    }
}

void HitMarkers::handleWorldWrap(sf::Vector2f positionDelta)
{
    for (HitMarker& hitMarker : hitMarkers)
    {
        hitMarker.position += positionDelta;
    }
}

void HitMarkers::HitMarker::update(float dt)
{
    lifetime += dt;
}

bool HitMarkers::HitMarker::isAlive() const
{
    return (lifetime < HIT_MARKER_LIFETIME);
}

void HitMarkers::HitMarker::draw(sf::RenderTarget& window) const
{
    TextDrawData drawData;
    drawData.text = std::to_string(damageAmount);

    float alpha = 1.0f - (lifetime / HIT_MARKER_LIFETIME);
    drawData.colour = sf::Color(247, 150, 23, 255 * alpha);
    drawData.position = Camera::worldToScreenTransform(position - sf::Vector2f(0.0f, LIFT_PER_SECOND * lifetime));
    drawData.centeredX = true;
    drawData.centeredY = true;
    drawData.size = UNSCALED_FONT_SIZE * ResolutionHandler::getScale();

    TextDraw::drawText(window, drawData);
}