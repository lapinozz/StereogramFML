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
        void renderMouse();

        void update(float dt);
        void updateView(float dt);
        void updatePhysic(float dt);
        void updatePlayer(float dt);
        void updateEnemy(float dt);

        void mapUpdated();

        void handleCollision();
        void handleDamage(LifeEntity& entity, float dmgd);
        void handleDamage(int32_t id1, int32_t id2, float velocity = 0.f);

        void createPlayer(sf::Vector2f pos);
        void createEnemy(sf::Vector2f pos);
        void createBullet(sf::Vector2f pos, float angle, float speed, sf::Vector2f initialVelocity = {});

        void deleteEntity(int32_t id);

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

        sf::Sprite cursor;

        sf::View view;
        sf::Vector2f viewTarget = {0, 0};

        TileAdaptor adaptor;
        Pathfinder pathfinder = {adaptor};

        MappedVector<Entiy> mEntitys;
        MappedVector<RenderEntity> mRenderEntitys;
        MappedVector<PhysicEntity> mPhysicEntitys;
        MappedVector<EnemyController> mEnemyControllers;
        MappedVector<LifeEntity> mLifeEntitys;

        TileMap<std::unordered_set<int32_t>> collisionGrid;

        TileMap<bool> map;
        sf::VertexArray mapVertex;

        int32_t mPlayerId;
        DirectionResolver directionSolver;

        const int TILE_SIZE = 64;
        const int GRID_SIZE = TILE_SIZE*2;

        const int PLAYER_SIZE = TILE_SIZE/2;
        const int ENEMY_SIZE = TILE_SIZE/2;
        const int BULLET_SIZE = TILE_SIZE/4;

        const char* texturePaths[TEXTURE_COUNT] = {"", "./res/body.png", "./res/mouse-cursor.png"};
        sf::Texture textures[TEXTURE_COUNT];

        template<typename T>
        typename MappedVector<T>::iterator findEntity(MappedVector<T>& container, int32_t id)
        {
            return find_member_if_equal(container.begin(), container.end(), &T::entityId, id);
        }

};

#endif // GAME_H
