#pragma once


#include <SFML/Graphics.hpp>
#include "utility.hpp"

struct Manifold
{
    bool collide = false;

    float depth;
    sf::Vector2f normal;
};

Manifold CircleVsRect(const sf::Vector2f& circleCenter, float radius, const sf::FloatRect& rect);

Manifold CircleVsCircle(const sf::Vector2f& c1, float r1, const sf::Vector2f& c2, float r2);
