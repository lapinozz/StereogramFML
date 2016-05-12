#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>

#include "mappedvector.h"
#include "stereograme.h"
#include "directionresolver.h"
#include "datas.hpp"
#include "tilemap.hpp"
#include "collision.hpp"

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

        void createPlayer();

        void handleEvent(const sf::Event& event);

    private:
        sf::RenderWindow window;
        Stereograme st;

        MappedVector<Entiy> mEntitys;
        MappedVector<RenderEntity> mRenderEntitys;
        MappedVector<PhysicEntity> mPhysicEntitys;

        TileMap map;
        sf::VertexArray mapVertex;

        int32_t mPlayerId;
        DirectionResolver directionSolver;

        const int TILE_SIZE = 20;

        const char* texturePaths[TEXTURE_COUNT] = {"", "./res/body.png"};
        sf::Texture textures[TEXTURE_COUNT];

};

#endif // GAME_H
