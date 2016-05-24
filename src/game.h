#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>

#include <unordered_set>
#include <iostream>

#include "mappedvector.h"
#include "stereograme.h"
#include "directionresolver.h"
#include "datas.hpp"
#include "tilemap.hpp"
#include "collision.hpp"
#include "grid.hpp"

class Game
{
    public:
//        Game();
//        virtual ~Game();
        void loop();

        void loadRessources();

        void render();
        void renderEntitys();
        void renderTileMap();

        void update(float dt);
        void updatePhysic(float dt);
        void updatePlayer(float dt);

        void handleCollision();

        void createPlayer(sf::Vector2f pos);
        void createBullet(sf::Vector2f pos, float angle, float speed, sf::Vector2f initialVelocity = {});

        sf::FloatRect getEntityRect(int32_t id);

        void handleEvent(const sf::Event& event);

        void updateCollisionGrid();
        void updateCollisionGrid(int32_t id);
        std::unordered_set<int32_t> getEntityAtCollisionGrid(const sf::FloatRect& region);
        void addEntityToCollisionGrid(int32_t id);
        void removeEntityFromCollisionGrid(int32_t id);

    private:
        sf::RenderWindow window;
        Stereograme st;

        MappedVector<Entiy> mEntitys;
        MappedVector<RenderEntity> mRenderEntitys;
        MappedVector<PhysicEntity> mPhysicEntitys;

        TileMap<std::unordered_set<int32_t>> collisionGrid;

        TileMap<bool> map;
        sf::VertexArray mapVertex;

        int32_t mPlayerId;
        DirectionResolver directionSolver;

        const int TILE_SIZE = 64;
        const int GRID_SIZE = TILE_SIZE*2;

        const char* texturePaths[TEXTURE_COUNT] = {"", "./res/body.png"};
        sf::Texture textures[TEXTURE_COUNT];

};

#endif // GAME_H
