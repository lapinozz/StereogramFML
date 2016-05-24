#include "collision.hpp"

Manifold CircleVsRect(const sf::Vector2f& circleCenter, float radius, const sf::FloatRect& rect)
{
    Manifold manifold;

    sf::Vector2f halfSize = sf::Vector2f(rect.width, rect.height)*0.5f;
    sf::Vector2f rectCenter = sf::Vector2f(rect.left, rect.top) + halfSize;

    auto difference = circleCenter - rectCenter;
    sf::Vector2f closest = rectCenter + clamp(difference, -halfSize, halfSize);

    difference = circleCenter - closest;

    manifold.depth = radius - length(difference);
    manifold.collide = manifold.depth > 0;

    if(!manifold.collide)
        return manifold;

    manifold.normal = normalize(difference);

    return manifold;
}

Manifold CircleVsCircle(const sf::Vector2f& c1, float r1, const sf::Vector2f& c2, float r2)
{
    auto diff = c2 - c1;
    auto dist = length(diff);

    Manifold manifold;

    manifold.depth = -(dist - (r1 + r2));

    if(manifold.depth > 0)
    {
        manifold.collide = true;
        manifold.normal = -normalize(diff);
    }

    return manifold;
}
