#include "game.h"

#include "utility.hpp"

void Game::loop()
{
    sf::VideoMode videoMode;
    videoMode = sf::VideoMode(640, 480);
    videoMode = sf::VideoMode::getDesktopMode();

    window.create(videoMode, "FPS incoming");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    sf::Image patternImg;
    patternImg.loadFromFile("./res/color-hax-pattern.png");
    patternImg.loadFromFile("./res/pattern.jpg");
    sf::Image depthImg;
    depthImg.loadFromFile("./res/depth.png");

    sf::Texture patternTexture;
    sf::Texture depthTexture;
    patternTexture.loadFromImage(patternImg);
//    patternTexture.loadFromFile("./res/channel0.jpg");
    patternTexture.loadFromFile("./res/channel1.jpg");
    depthTexture.loadFromImage(depthImg);

    st.setPatternTexture(patternTexture);
    st.create(window.getSize().x, window.getSize().y);
    st.useInternalTexture();
    st.setInvertedDepth(true, false);
//    st.setAnimate(true);
    st.setRandomBackDrop(true, false);
    st.setDepthScale(10);
    st.setPixelRepeat(100);
//    st.setAddColour(true);
    st.setShowDepthImage(true);
//        st.useExternalTexture(depthTexture);

    sf::Clock fpClock;
    int fps = 0;

    sf::Clock deltaClock;

    collisionGrid.setSize({(videoMode.width/GRID_SIZE), videoMode.height/GRID_SIZE});
    map.setSize({(videoMode.width/TILE_SIZE)-2, videoMode.height/TILE_SIZE});

    loadRessources();
    createPlayer({100, 100});

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
    updateCollisionGrid();
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
        if(physicEntity.updateVelocity)
            physicEntity.velocity = lerp(physicEntity.velocity, physicEntity.targetVelocity, dt*3.f);

        mEntitys[physicEntity.entityId].position += physicEntity.velocity * dt;
    }
}

void Game::updatePlayer(float dt)
{
    auto mousePos = sf::Mouse::getPosition(window);

    for(auto id : getEntityAtCollisionGrid({mousePos.x, mousePos.y, 0, 0}))
        std::cout << id << std::endl;


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
    else if(event.type == sf::Event::MouseButtonPressed)
    {
        const auto& entity = mEntitys[mPlayerId];

        createBullet(entity.position, entity.rotation, 600);
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
            for(size_t y = (entity.position.y - entity.radius) / TILE_SIZE; y < (entity.position.y + entity.radius) / TILE_SIZE; y++)
            {
                if(!map.getTile({x, y}))
                    continue;

                auto manifold = CircleVsRect(entity.position, entity.radius, TileMap<bool>::tileToRect({x, y}, {TILE_SIZE, TILE_SIZE}));
                if(manifold.collide)
                {
                    entity.position += manifold.normal * manifold.depth;

                    //bounce code
                    if(physicEntity.bounce)
                    {
                        physicEntity.velocity -=  2.f * dot(physicEntity.velocity, manifold.normal) * manifold.normal;
                        physicEntity.velocity *= 0.8f;
                    }
                    else
                    //velocity negation code
                        physicEntity.velocity -= dot(physicEntity.velocity, manifold.normal) * manifold.normal;
                }
            }
        }

        for(auto id : getEntityAtCollisionGrid(getEntityRect(physicEntity.entityId)))
        {
//            if(physicEntity.entityId == id || id == mPlayerId || physicEntity.entityId == mPlayerId)
            if(physicEntity.entityId == id)
                continue;

            auto& other = mEntitys[id];

            auto manifold = CircleVsCircle(entity.position, entity.radius, other.position, other.radius);
            if(manifold.collide)
            {
                entity.position += manifold.normal * manifold.depth * 0.5f;
                other.position -= manifold.normal * manifold.depth * 0.5f;

                auto& otherPhysicEntity = *find_member_if_equal(mPhysicEntitys.begin(), mPhysicEntitys.end(), &PhysicEntity::entityId, id);

                float mass1 = pow(M_PI * entity.radius, 2);
                float mass2 = pow(M_PI * other.radius, 2);

                sf::Vector2f tangent = {-manifold.normal.y, manifold.normal.x};

                auto scalaNorm1 = dot(manifold.normal, physicEntity.velocity);
                auto scalaNorm2 = dot(manifold.normal, otherPhysicEntity.velocity);

                auto scalaNormAfter1 = (scalaNorm1 * (mass1 - mass2) + 2.f * mass2 * scalaNorm2) / (mass1 + mass2);
                auto scalaNormAfter2 = (scalaNorm2 * (mass2 - mass1) + 2.f * mass1 * scalaNorm1) / (mass1 + mass2);

                physicEntity.velocity = manifold.normal * scalaNormAfter1 + tangent * dot(tangent, physicEntity.velocity);
                otherPhysicEntity.velocity = manifold.normal * scalaNormAfter2 + tangent * dot(tangent, otherPhysicEntity.velocity);
            }
        }
    }
}

void Game::render()
{
    renderTileMap();
    renderEntitys();

//    sf::RectangleShape rect({GRID_SIZE, GRID_SIZE});
//    rect.setFillColor(sf::Color::Transparent);
//    rect.setOutlineColor(sf::Color::Red);
//    rect.setOutlineThickness(2);
//
//    for(int x = 0; x < collisionGrid.getSize().x; x++)
//    {
//        for(int y = 0; y < collisionGrid.getSize().y; y++)
//        {
//            rect.setPosition(x * GRID_SIZE, y * GRID_SIZE);
//            rect.setOutlineColor(collisionGrid.at({x, y}).size() ? sf::Color::Red : sf::Color::Green);
//
//            st.draw(rect);
//        }
//    }
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

void Game::createPlayer(sf::Vector2f pos)
{
    auto id = mEntitys.insert({pos, 0, TILE_SIZE/2});
    mRenderEntitys.insert({id, Textures::PLAYER});
    mPhysicEntitys.insert({id, {}, {}, true, false});

    addEntityToCollisionGrid(id);

    mPlayerId = id;
}

void Game::createBullet(sf::Vector2f pos, float angle, float speed, sf::Vector2f initialVelocity)
{
    auto id = mEntitys.insert({pos, angle, TILE_SIZE/4});
    mRenderEntitys.insert({id, Textures::NONE});
    mPhysicEntitys.insert({id, initialVelocity + normalize(angleToVector(angle))*speed, {}, false, true});

    addEntityToCollisionGrid(id);
}

sf::FloatRect Game::getEntityRect(int32_t id)
{
    const auto& entity = mEntitys[id];

    return {entity.position.x - entity.radius, entity.position.y - entity.radius, entity.radius * 2.f, entity.radius * 2.f};
}

void Game::loadRessources()
{
    // load textures
    // x = 1 because we are skipping NONE which equal 0
    // actually probably could have used 0 and do a check to skip none and the compiler would have optimized
    for(size_t x = 1; x < Textures::TEXTURE_COUNT; x++)
        textures[x].loadFromFile(texturePaths[x]);
}

void Game::updateCollisionGrid()
{
    for(int x = 0; x < collisionGrid.getSize().x; x++)
    {
        for(int y = 0; y < collisionGrid.getSize().y; y++)
        {
            std::vector<int32_t> toRemove;

            for(const auto id : collisionGrid.at({x, y}))
            {
                const auto entityRect = getEntityRect(id);

                if(!entityRect.intersects(collisionGrid.tileToRect({x, y}, {GRID_SIZE, GRID_SIZE})))
                    toRemove.push_back(id);


                ///NOTE: if entity can overlap more than 4 cell at the same time then remove those <<else>>

                if(x != 0 && entityRect.intersects(collisionGrid.tileToRect({x - 1, y}, {GRID_SIZE, GRID_SIZE})))
                {
                    collisionGrid.at({x - 1, y}).insert(id);

                    if(y != 0 && entityRect.intersects(collisionGrid.tileToRect({x - 1, y - 1}, {GRID_SIZE, GRID_SIZE})))
                        collisionGrid.at({x - 1, y - 1}).insert(id);
                    else if(y != collisionGrid.getSize().y - 1 && entityRect.intersects(collisionGrid.tileToRect({x - 1, y + 1}, {GRID_SIZE, GRID_SIZE})))
                        collisionGrid.at({x - 1, y + 1}).insert(id);
                }
                else if(x != collisionGrid.getSize().x - 1 && entityRect.intersects(collisionGrid.tileToRect({x + 1, y}, {GRID_SIZE, GRID_SIZE})))
                {
                    collisionGrid.at({x + 1, y}).insert(id);

                    if(y != 0 && entityRect.intersects(collisionGrid.tileToRect({x + 1, y - 1}, {GRID_SIZE, GRID_SIZE})))
                        collisionGrid.at({x + 1, y - 1}).insert(id);
                    else if(y != collisionGrid.getSize().y - 1 && entityRect.intersects(collisionGrid.tileToRect({x + 1, y + 1}, {GRID_SIZE, GRID_SIZE})))
                        collisionGrid.at({x + 1, y + 1}).insert(id);
                }

                if(y != 0 && entityRect.intersects(collisionGrid.tileToRect({x, y - 1}, {GRID_SIZE, GRID_SIZE})))
                   collisionGrid.at({x, y - 1}).insert(id);
                else if(y != collisionGrid.getSize().y - 1 && entityRect.intersects(collisionGrid.tileToRect({x, y + 1}, {GRID_SIZE, GRID_SIZE})))
                   collisionGrid.at({x, y + 1}).insert(id);
            }

            for(auto remove : toRemove)
                collisionGrid.at({x, y}).erase(remove);
        }
    }
}

void Game::updateCollisionGrid(int32_t id)
{

}

std::unordered_set<int32_t> Game::getEntityAtCollisionGrid(const sf::FloatRect& region)
{
    std::unordered_set<int32_t> ids;

    for(size_t x = region.left / GRID_SIZE; x < (region.left + region.width) / GRID_SIZE; x++)
    {
        for(size_t y = region.top / GRID_SIZE; y < (region.top + region.height) / GRID_SIZE; y++)
        {
            const auto& cellIds = collisionGrid.at({x, y});

            ids.insert(cellIds.begin(), cellIds.end());
        }
    }

    return ids;
}

void Game::addEntityToCollisionGrid(int32_t id)
{
    const auto& entity = mEntitys[id];

    for(size_t x = (entity.position.x - entity.radius) / GRID_SIZE; x < (entity.position.x + entity.radius) / GRID_SIZE; x++)
    {
        for(size_t y = (entity.position.y - entity.radius) / GRID_SIZE; y < (entity.position.y + entity.radius) / GRID_SIZE; y++)
        {
            collisionGrid.at({x, y}).insert(id);
        }
    }
}

void Game::removeEntityFromCollisionGrid(int32_t id)
{
    const auto& entity = mEntitys[id];

    for(size_t x = (entity.position.x - entity.radius) / GRID_SIZE; x < (entity.position.x + entity.radius) / GRID_SIZE; x++)
    {
        for(size_t y = (entity.position.y - entity.radius) / GRID_SIZE; y < (entity.position.y + entity.radius) / GRID_SIZE; y++)
        {
            collisionGrid.at({x, y}).erase(id);
        }
    }
}
