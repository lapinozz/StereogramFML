#include "game.h"

#include "utility.hpp"

void Game::loop()
{
    sf::VideoMode videoMode;
    videoMode = sf::VideoMode(640, 480);
//    videoMode = sf::VideoMode::getDesktopMode();

    window.create(videoMode, "FPS incoming");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    sf::Image patternImg;
    patternImg.loadFromFile("./res/color-hax-pattern.png");
//    patternImg.loadFromFile("./res/pattern.jpg");
    sf::Image depthImg;
    depthImg.loadFromFile("./res/depth.png");

    sf::Texture patternTexture;
    sf::Texture depthTexture;
    patternTexture.loadFromImage(patternImg);
//    patternTexture.loadFromFile("./res/channel0.jpg");
    depthTexture.loadFromImage(depthImg);

    st.setPatternTexture(patternTexture);
    st.create(window.getSize().x, window.getSize().y);
    st.useInternalTexture();
    st.setInvertedDepth(true, false);
    st.setAnimate(true);
    st.setRandomBackDrop(true, true);
    st.setDepthScale(16);
    st.setPixelRepeat(96);
    st.setShowDepthImage(true);
//        st.useExternalTexture(depthTexture);

    sf::Clock fpClock;
    int fps;

    sf::Clock deltaClock;

    loadRessources();
    createPlayer();

    map.setSize({(680/20)-2, 480/20});

    for(int x = 0; x < map.getSize().x; x++)
        map.setTile({x, 0}, true);

    for(int x = 0; x < map.getSize().y; x++)
        map.setTile({0, x}, true);

    for(int x = 0; x < map.getSize().x; x++)
        map.setTile({x, map.getSize().y-1}, true);

    for(int x = 0; x < map.getSize().y; x++)
        map.setTile({map.getSize().x-1, x}, true);

    map.setTile({5, 5}, true);
    map.setTile({4, 5}, true);
    map.setTile({3, 5}, true);
    map.setTile({2, 5}, true);
    map.setTile({5, 5}, true);
    map.setTile({5, 6}, true);
    map.setTile({5, 7}, true);
    map.setTile({5, 8}, true);
    map.setTile({6, 8}, true);
    map.setTile({7, 8}, true);
    map.setTile({8, 8}, true);
    map.setTile({9, 8}, true);
    map.setTile({10, 8}, true);
    map.setTile({11, 8}, true);
    map.setTile({12, 8}, true);
    map.setTile({8, 7}, true);
    map.setTile({8, 6}, true);

    mapVertex = map.toVertexArray({TILE_SIZE, TILE_SIZE});

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
            {
                window.close();
                return;
            }
            else
                handleEvent(event);
        }

        update(deltaClock.restart().asSeconds());
        st.setCurrentTime(fpClock.getElapsedTime().asSeconds());

        window.clear();
        st.clear();

        render();
        st.display();
        window.draw(st);

        window.display();

        fps++;
        if(fpClock.getElapsedTime().asSeconds() >= 1.f)
        {
             window.setTitle(std::to_string(fps));
            fps = 0;
            fpClock.restart();
        }
    }
}

void Game::update(float dt)
{
    updatePlayer(dt);

    updatePhysic(dt);
    handleCollision();
}

void Game::updatePhysic(float dt)
{
    for(auto& physicEntity : mPhysicEntitys)
    {
//        if(physicEntity.targetVelocity.x && sgn(physicEntity.targetVelocity.x) != sgn(physicEntity.velocity.x))
//            physicEntity.velocity.x *= 0.8f;
//        if(physicEntity.targetVelocity.y && sgn(physicEntity.targetVelocity.y) != sgn(physicEntity.velocity.y))
//            physicEntity.velocity.y *= 0.8f;

        physicEntity.velocity = lerp(physicEntity.velocity, physicEntity.targetVelocity, dt*3.f);
        mEntitys[physicEntity.entityId].position += physicEntity.velocity * dt;
    }
}

void Game::updatePlayer(float dt)
{
    auto mousePos = sf::Mouse::getPosition(window);

    auto& entity = mEntitys[mPlayerId];

    entity.rotation = vectorToAngle((sf::Vector2f)mousePos - entity.position);
}

void Game::handleEvent(const sf::Event& event)
{
    static const sf::Keyboard::Key UP = sf::Keyboard::W;
    static const sf::Keyboard::Key DOWN = sf::Keyboard::S;
    static const sf::Keyboard::Key LEFT = sf::Keyboard::A;
    static const sf::Keyboard::Key RIGHT = sf::Keyboard::D;

    if(event.type == sf::Event::KeyPressed)
    {
        directionSolver.upMaybeActive(event.key.code == UP);
        directionSolver.downMaybeActive(event.key.code == DOWN);
        directionSolver.leftMaybeActive(event.key.code == LEFT);
        directionSolver.rightMaybeActive(event.key.code == RIGHT);
    }
    else if(event.type == sf::Event::KeyReleased)
    {
        directionSolver.upMaybeInactive(event.key.code == UP);
        directionSolver.downMaybeInactive(event.key.code == DOWN);
        directionSolver.leftMaybeInactive(event.key.code == LEFT);
        directionSolver.rightMaybeInactive(event.key.code == RIGHT);
    }

    auto it = find_member_if_equal(mPhysicEntitys.begin(), mPhysicEntitys.end(), &PhysicEntity::entityId, mPlayerId);
    if(it != mPhysicEntitys.end())
        it->targetVelocity = normalize(directionSolver.getDirection()) * 500.f;
}

void Game::handleCollision()
{
    for(auto& physicEntity : mPhysicEntitys)
    {
        auto& entity = mEntitys[physicEntity.entityId];

        for(size_t x = (entity.position.x - entity.radius) / TILE_SIZE; x < (entity.position.x + entity.radius) / TILE_SIZE; x++)
        {
            for(size_t y = (entity.position.y - entity.radius) / TILE_SIZE; y < (entity.position.y + entity.radius); y++)
            {
                if(!map.getTile({x, y}))
                    continue;

                auto manifold = CircleVsRect(entity.position, entity.radius, TileMap::tileToRect({x, y}, {TILE_SIZE, TILE_SIZE}));
                if(manifold.collide)
                {
                    entity.position += manifold.normal * manifold.depth;

                    //bounce code
//                    physicEntity.velocity -=  2.f * dot(physicEntity.velocity, manifold.normal) * manifold.normal;
//                    physicEntity.velocity *= 0.8f;

                    //velocity negation code
                    physicEntity.velocity -= dot(physicEntity.velocity, manifold.normal) * manifold.normal;
                }
            }
        }
    }
}

void Game::render()
{
    renderTileMap();
    renderEntitys();
}

void Game::renderTileMap()
{
    st.draw(mapVertex);
}

void Game::renderEntitys()
{
    sf::CircleShape shape;

    for(auto renderer : mRenderEntitys)
    {
        const auto& entity = mEntitys[renderer.entityId];

        shape.setTexture(renderer.texture == Textures::NONE ? nullptr : &textures[renderer.texture], true);
        shape.setPosition(entity.position);
        shape.setRotation(entity.rotation);
        shape.setRadius(entity.radius);
        shape.setOrigin(entity.radius, entity.radius);
        st.draw(shape);
    }
}

void Game::createPlayer()
{
    auto id = mEntitys.insert({sf::Vector2f(100, 100), 0, TILE_SIZE});
    mRenderEntitys.insert({id, Textures::PLAYER});
    mPhysicEntitys.insert({id});

    mPlayerId = id;
}

void Game::loadRessources()
{
    // load textures
    // x = 1 because we are skipping NONE which equal 0
    // actually probably could have used 0 and do a check to skip none and the compiler would have optimized
    for(size_t x = 1; x < Textures::TEXTURE_COUNT; x++)
        textures[x].loadFromFile(texturePaths[x]);
}
