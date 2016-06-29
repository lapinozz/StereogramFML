#include "game.h"

#include "utility.hpp"

void Game::loop()
{
    sf::VideoMode videoMode;
    videoMode = sf::VideoMode(640, 480);
//    videoMode = sf::VideoMode::getDesktopMode();
    videoMode = sf::VideoMode::getFullscreenModes()[0];

    window.create(videoMode, "FPS incoming", sf::Style::Default | sf::Style::Fullscreen);
    window.setMouseCursorVisible(false);
//    window.create(videoMode, "FPS incoming");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    view = window.getDefaultView();
    viewTarget = view.getCenter();

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
    st.setDepthScale(16);
    st.setPixelRepeat(210);
//    st.setAddColour(true);
//    st.setShowDepthImage(true);
//        st.useExternalTexture(depthTexture);

    sf::Clock fpClock;
    int fps = 0;

    sf::Clock deltaClock;

    sf::Vector2f mapSize = {videoMode.width, videoMode.height};
    mapSize *= 3.f;

    collisionGrid.setSize({(mapSize.x/GRID_SIZE), mapSize.y/GRID_SIZE});
    map.setSize({(mapSize.x/TILE_SIZE)-2, mapSize.y/TILE_SIZE});

    loadRessources();
    createPlayer({100, 100});

    cursor.setTexture(textures[CURSOR]);
    cursor.setScale({12, 12});
    cursor.setOrigin((sf::Vector2f)(cursor.getTexture()->getSize()/2u));

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

    adaptor = TileAdaptor(map.getSize(), [this](const sf::Vector2i& pos){return !map.getTile(pos);});

    mapUpdated();

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

        st.setCurrentTime(fpClock.getElapsedTime().asSeconds());
//
//        window.clear();
//        st.clear();
        st.setView(view);

        render();
        update(deltaClock.restart().asSeconds());
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
    updateEnemy(dt);
    updateView(dt);

    updatePhysic(dt);
    updateCollisionGrid();
    handleCollision();
}


void Game::updateView(float dt)
{
    viewTarget = mEntitys[mPlayerId].position;

    viewTarget.x = std::max(viewTarget.x, window.getSize().x/2.f);
    viewTarget.y = std::max(viewTarget.y, window.getSize().y/2.f);

    viewTarget.x = std::min(viewTarget.x, TILE_SIZE*map.getSize().x - window.getSize().x/2.f);
    viewTarget.y = std::min(viewTarget.y, TILE_SIZE*map.getSize().y - window.getSize().y/2.f);

    view.setCenter(lerp(view.getCenter(), viewTarget, 3*dt));
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
    auto mousePos = getMousePos();

//    for(auto id : getEntityAtCollisionGrid({mousePos.x, mousePos.y, 0, 0}))
//        std::cout << id << std::endl;

    auto& entity = mEntitys[mPlayerId];

    auto path = pathfinder.search(toMapPos(mousePos), toMapPos(entity.position),
                                        {
                                            [this](const auto id)
                                            {
                                                return adaptor.idToPos(id);
                                            }
                                        },
                                        {
                                            [this](const auto& data)
                                            {
                                                return adaptor.posToId(data);
                                            }
                                        });

    sf::RectangleShape rect({TILE_SIZE, TILE_SIZE});
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineThickness(2);

    rect.setOutlineColor(sf::Color::Blue);
    rect.setPosition((sf::Vector2f)((((sf::Vector2i)entity.position)/TILE_SIZE)*TILE_SIZE));
    st.draw(rect);

    rect.setOutlineColor(sf::Color::Green);
    rect.setPosition((sf::Vector2f)((((sf::Vector2i)mousePos)/TILE_SIZE)*TILE_SIZE));
    st.draw(rect);

    rect.setOutlineColor(sf::Color::Red);
    for(auto pos : path)
    {
        rect.setPosition(pos.x * TILE_SIZE, pos.y * TILE_SIZE);
        st.draw(rect);
    }


    entity.rotation = vectorToAngle((sf::Vector2f)mousePos - entity.position);
}

void Game::updateEnemy(float dt)
{
    const auto& player = mEntitys[mPlayerId];

    for(const auto& controller : mEnemyControllers)
    {
        auto& enemy = mEntitys[controller.entityId];
        auto& enemyPhysic = *find_member_if_equal(mPhysicEntitys.begin(), mPhysicEntitys.end(), &PhysicEntity::entityId, controller.entityId);

        auto path = pathfinder.search(toMapPos(enemy.position), toMapPos(player.position),
                                        {
                                            [this](const auto id)
                                            {
                                                return adaptor.idToPos(id);
                                            }
                                        },
                                        {
                                            [this](const auto& data)
                                            {
                                                return adaptor.posToId(data);
                                            }
                                        });

        if(path.size() > 1)
        {
            auto targetPos = toWorldPos(path[1]);

            enemyPhysic.targetVelocity = normalize(targetPos - enemy.position) * 200.f;
            enemy.rotation = vectorToAngle(targetPos - enemy.position);
        }
    }
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

        if(event.key.code == sf::Keyboard::F1)
        {
            st.setShowDepthImage(true);
            st.setAddColour(true);
        }
        else if(event.key.code == sf::Keyboard::F2)
        {
            st.setShowDepthImage(false);
            st.setAddColour(false);
        }
    }
    else if(event.type == sf::Event::KeyReleased)
    {
        directionSolver.upMaybeInactive(event.key.code == UP);
        directionSolver.downMaybeInactive(event.key.code == DOWN);
        directionSolver.leftMaybeInactive(event.key.code == LEFT);
        directionSolver.rightMaybeInactive(event.key.code == RIGHT);
    }
    else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        const auto& entity = mEntitys[mPlayerId];

        auto pos = entity.position + angleToVector(entity.rotation) * (float(PLAYER_SIZE + BULLET_SIZE ));

        createBullet(pos, entity.rotation, 600);
    }
    else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
    {
        const sf::Vector2i mousePos = (sf::Vector2i)getMousePos()/TILE_SIZE;

        map.setTile(mousePos, !map.getTile(mousePos));

        mapUpdated();
    }
    else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Middle)
    {
        createEnemy(getMousePos());
    }

    auto it = find_member_if_equal(mPhysicEntitys.begin(), mPhysicEntitys.end(), &PhysicEntity::entityId, mPlayerId);
    if(it != mPhysicEntitys.end())
        it->targetVelocity = normalize(directionSolver.getDirection()) * 500.f;
}

void Game::mapUpdated()
{
    mapVertex = map.toVertexArray({TILE_SIZE, TILE_SIZE});
    pathfinder.generateNodes();
}

sf::Vector2f Game::getMousePos()
{
//    return view.getTransform().transformPoint((sf::Vector2f));
//    return (sf::Vector2f)sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
}

sf::Vector2i Game::toMapPos(sf::Vector2f worldPos)
{
    return (sf::Vector2i)(worldPos/(float)TILE_SIZE);
}

sf::Vector2f Game::toWorldPos(sf::Vector2i mapPos)
{
    return (sf::Vector2f)(mapPos*TILE_SIZE);
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

                    handleDamage(physicEntity.entityId, -1, length(physicEntity.velocity));

                    //bounce code
                    if(physicEntity.bounce)
                    {
                        physicEntity.velocity -=  2.f * dot(physicEntity.velocity, manifold.normal) * manifold.normal;
//                        physicEntity.velocity *= 0.8f;
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
                handleDamage(physicEntity.entityId, id, 0);

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

void Game::handleDamage(LifeEntity& entity, float dmg)
{
    dmg *= 1.f - entity.armor;

    dmg = std::max(dmg, (float)entity.minDmgTaken);

    if(entity.maxDmgTaken != -1)
        dmg = std::min(dmg, (float)entity.maxDmgTaken);

    entity.life -= dmg;

    if(entity.entityId == 0)
        std::cout << entity.life << std::endl;

    if(entity.life <= 0)
        deleteEntity(entity.entityId);
}

void Game::handleDamage(int32_t id1, int32_t id2, float velocity)
{
    auto it1 = find_member_if_equal(mLifeEntitys.begin(), mLifeEntitys.end(), &LifeEntity::entityId, id1);
    if(it1 == mLifeEntitys.end())
        return;

    //if wall
    if(id2 == -1)
    {
        float dmg = velocity * it1->dmgOnWall;
        handleDamage(*it1, dmg);
    }
    else
    {
        auto it2 = find_member_if_equal(mLifeEntitys.begin(), mLifeEntitys.end(), &LifeEntity::entityId, id2);
        if(it2 == mLifeEntitys.end())
            return;

        auto dmg1 = it1->dmg;
        auto dmg2 = it2->dmg;

        handleDamage(*it1, dmg2);
        handleDamage(*it2, dmg1);
    }
}

void Game::render()
{
    renderTileMap();
    renderEntitys();
    renderMouse();

//    sf::RectangleShape rect({GRID_SIZE, GRID_SIZE});
//    rect.setFillColor(sf::Color::Transparent);
//    rect.setOutlineColor(sf::Color::Red);
//    rect.setOutlineThickness(2);

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

    shape.setFillColor(white(255/2));

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

void Game::renderMouse()
{
    cursor.setPosition(getMousePos());
    st.draw(cursor);
}

void Game::createPlayer(sf::Vector2f pos)
{
    auto id = mEntitys.insert({pos, 0, PLAYER_SIZE});
    mRenderEntitys.insert({id, Textures::PLAYER});
    mPhysicEntitys.insert({id, {}, {}, true, false});
    mLifeEntitys.insert({id, 100, 0.f, 0, 0, 0, 0.f});
//    mLifeEntitys.insert({id, 100, 0.f, 0, -1, 0, 0.f});

    addEntityToCollisionGrid(id);

    mPlayerId = id;
}

void Game::createEnemy(sf::Vector2f pos)
{
    auto id = mEntitys.insert({pos, 0, ENEMY_SIZE});
    mRenderEntitys.insert({id, Textures::PLAYER});
    mPhysicEntitys.insert({id, {}, {}, true, false});
    mLifeEntitys.insert({id, 50, 0.f, 0, -1, 0, 0.f});

    mEnemyControllers.insert({id});

    addEntityToCollisionGrid(id);
}

void Game::createBullet(sf::Vector2f pos, float angle, float speed, sf::Vector2f initialVelocity)
{
    auto id = mEntitys.insert({pos, angle, BULLET_SIZE});
    mRenderEntitys.insert({id, Textures::NONE});
    mPhysicEntitys.insert({id, initialVelocity + normalize(angleToVector(angle))*speed, {}, false, true});
    mLifeEntitys.insert({id, 3, 0.f, 25, 1, 1, 0.1f});

    addEntityToCollisionGrid(id);
}

void Game::deleteEntity(int32_t id)
{
    removeEntityFromCollisionGrid(id);

    mEntitys.remove(id);
    mRenderEntitys.remove(findEntity(mRenderEntitys, id));
    mPhysicEntitys.remove(findEntity(mPhysicEntitys, id));
    mEnemyControllers.remove(findEntity(mEnemyControllers, id));
    mLifeEntitys.remove(findEntity(mLifeEntitys, id));
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
