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
#include "pathfinding.hpp"
#include "tileadaptor.hpp"

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
        void updateView(float dt);
        void updatePhysic(float dt);
        void updatePlayer(float dt);
        void updateEnemy(float dt);

        void mapUpdated();

        void handleCollision();

        void createPlayer(sf::Vector2f pos);
        void createEnemy(sf::Vector2f pos);
        void createBullet(sf::Vector2f pos, float angle, float speed, sf::Vector2f initialVelocity = {});

        sf::FloatRect getEntityRect(int32_t id);

        void handleEvent(const sf::Event& event);

        sf::Vector2f getMousePos();

        sf::Vector2i toMapPos(sf::Vector2f worldPos);
        sf::Vector2f toWorldPos(sf::Vector2i mapPos);

        void updateCollisionGrid();
        void updateCollisionGrid(int32_t id);
        std::unordered_set<int32_t> getEntityAtCollisionGrid(const sf::FloatRect& region);
        void addEntityToCollisionGrid(int32_t id);
        void removeEntityFromCollisionGrid(int32_t id);

    private:
        sf::RenderWindow window;
        Stereograme st;

        sf::View view;
        sf::Vector2f viewTarget = {0, 0};

        TileAdaptor adaptor;
        Pathfinder pathfinder = {adaptor};

        MappedVector<Entiy> mEntitys;
        MappedVector<RenderEntity> mRenderEntitys;
        MappedVector<PhysicEntity> mPhysicEntitys;
        MappedVector<EnemyController> mEnemyControllers;

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
