#include "directionresolver.h"

void DirectionResolver::upActive(bool b)
{
    up = b;
}

void DirectionResolver::downActive(bool b)
{
    down = b;
}

void DirectionResolver::leftActive(bool b)
{
    left = b;
}

void DirectionResolver::rightActive(bool b)
{
    right = b;
}

void DirectionResolver::upMaybeActive(bool b)
{
    up |= b;
}

void DirectionResolver::downMaybeActive(bool b)
{
    down |= b;
}

void DirectionResolver::leftMaybeActive(bool b)
{
    left |= b;
}

void DirectionResolver::rightMaybeActive(bool b)
{
    right |= b;
}

void DirectionResolver::upMaybeInactive(bool b)
{
    up &= !b;
}

void DirectionResolver::downMaybeInactive(bool b)
{
    down &= !b;
}

void DirectionResolver::leftMaybeInactive(bool b)
{
    left &= !b;
}

void DirectionResolver::rightMaybeInactive(bool b)
{
    right &= !b;
}

sf::Vector2f DirectionResolver::getDirection()
{
    return {-left + right, -up + down};
}
