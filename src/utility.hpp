#pragma once

#include <SFML/System.hpp>

#include "mappedvector.h"

template <typename Iterator, typename T, typename MemberType>
Iterator find_member_if_equal(const Iterator& begin, const Iterator& end, const MemberType T::*memberPtr, const MemberType& value)
{
    for(auto it = begin; it != end; it++)
    {
        if((*it).*memberPtr == value)
            return it;
    }

    return end;
};

template <typename T>
T clamp(const T value, const T min, const T max)
{
    return std::min(max, std::max(min, value));
}

template <typename T>
sf::Vector2<T> clamp(const sf::Vector2<T> value, const sf::Vector2<T> min, const sf::Vector2<T> max)
{
    return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y)};
}

template<class T> T toRad(T val)
{
    return val * M_PI / 180;
}

template<class T> T toDeg(T val)
{
    return val * 180 / M_PI;
}

template<class T> T length(sf::Vector2<T> vector)
{
		return sqrt(vector.x * vector.x + vector.y * vector.y);
}

template<class T> T normalize(T vector)
{
    float length = ::length(vector);
    if (length == 0)
        return sf::Vector2f(0, 0);

    return vector / length;
}

template<typename T> T perpandicular(T vector)
{
    return T(-vector.y, vector.x);
}

template<class T> T dot(sf::Vector2<T> vector1, sf::Vector2<T> vector2)
{
    return vector1.x*vector2.x + vector1.y*vector2.y;
}

template<class T> sf::Vector2<T> angleToVector(T val)
{
    return normalize(sf::Vector2<T>(sin(toRad(val)), -cos(toRad(val))));
}

template<class T> T vectorToAngle(sf::Vector2<T> val)
{
    return toDeg(atan2(val.x, -val.y));
}

template<typename T> T lerp(T value1, T value2, float interpolation)
{
    return (value1 * (1.0f - interpolation)) + (value2 * interpolation);
}

template <typename T> int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}
