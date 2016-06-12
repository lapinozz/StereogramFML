#pragma once

#include <SFML/System.hpp>

enum Textures {NONE = 0, PLAYER, TEXTURE_COUNT};

struct Entiy
{
    sf::Vector2f position;
    float rotation;

    float radius;
};

struct RenderEntity
{
    const int32_t entityId;

    Textures texture;
};

struct PhysicEntity
{
    const int32_t entityId;

    sf::Vector2f velocity;
    sf::Vector2f targetVelocity;
    bool updateVelocity;

    bool bounce;

//    sf::Vector2f maxVelocity = {200, 200};

//    float deceleration = 200;

};

struct EnemyController
{
    const int32_t entityId;
};
