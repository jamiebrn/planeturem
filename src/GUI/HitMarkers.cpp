#include "GUI/HitMarkers.hpp"

void HitMarkers::addHitMarker(pl::Vector2f position, int damageAmount, pl::Color colour)
{
    HitMarker hitMarker;
    hitMarker.position = position;
    hitMarker.damageAmount = damageAmount;
    hitMarker.colour = colour;

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

void HitMarkers::draw(pl::RenderTarget& window, const Camera& camera)
{
    for (const HitMarker& hitMarker : hitMarkers)
    {
        hitMarker.draw(window, camera);
    }
}

void HitMarkers::handleWorldWrap(pl::Vector2f positionDelta)
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

void HitMarkers::HitMarker::draw(pl::RenderTarget& window, const Camera& camera) const
{
    pl::TextDrawData drawData;
    drawData.text = std::to_string(damageAmount);

    float alpha = 1.0f - (lifetime / HIT_MARKER_LIFETIME);
    drawData.color = colour;
    drawData.color.a = 255 * alpha;
    drawData.position = camera.worldToScreenTransform(position - pl::Vector2f(0.0f, LIFT_PER_SECOND * lifetime));
    drawData.centeredX = true;
    drawData.centeredY = true;
    drawData.size = UNSCALED_FONT_SIZE * ResolutionHandler::getScale();

    TextDraw::drawText(window, drawData);
}