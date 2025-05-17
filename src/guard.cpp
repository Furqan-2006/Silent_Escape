#include "guard.hpp"
#include <SFML/System.hpp>
#include <string>

Guard::Guard(const sf::Texture &tex) : sprite(std::make_unique<sf::Sprite>(tex))
{
    state = GuardState::Patrolling;
    velocity = {1.0f, 0.f};
    facingDir = velocity;

    sprite->setOrigin({tex.getSize().x / 2.f, (float)tex.getSize().y});
    sprite->setScale({32.f / 256.f, 32.f / 256.f});

    lastSeenMarker.setRadius(5.f);
    lastSeenMarker.setFillColor(sf::Color::Red);
    lastSeenMarker.setOrigin({5.f, 5.f}); // Center the marker
}
Guard::Guard(const sf::Texture &tex, const sf::Vector2f &startPos) : Guard(tex)
{
    setPos(startPos);

    // sprite = std::make_unique<sf::Sprite>(tex);
}
bool Guard::checkCollision(const sf::FloatRect &otherBounds) const
{
    return sprite->getGlobalBounds().findIntersection(otherBounds).has_value();
}

void Guard::patrol(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime)
{
    if (!generatedInitialPatrolPath)
    {
        if (patrolPath.empty())
        {
            std::cout << "[PATROL] No patrol path" << std::endl;
            return;
        }

        // Always recalculate path to current patrol target to handle dynamic obstacles
        patrolPathToTarget = pathfinder.findPath(sprite->getPosition(), patrolPath[currentPatrolIndex]);
        currentPatrolPathIndex = 0;
        generatedInitialPatrolPath = true;
        // std::cout << "[PATROL] Starting path to patrol point " << currentPatrolIndex << "\n";
    }

    if (currentPatrolPathIndex < patrolPathToTarget.size())
    {
        sf::Vector2f target = patrolPathToTarget[currentPatrolIndex];
        sf::Vector2f direction = target - sprite->getPosition();
        float distance = std::hypot(direction.x, direction.y);

        std::cout << "[PATROL] distance: " <<distance << "\n";

        if (distance < 2.f) // Reached this small step
        {
            currentPatrolPathIndex++;
        }
        else
        {
            direction /= distance;
            facingDir = direction;

            // Move with deltaTime * moveSpeed
            sprite->move(direction * moveSpeed * deltaTime);
            std::cout << "[PATROL] Starting path to patrol point " << currentPatrolIndex << "\n";

            // Check collision and revert if needed
            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds()))
                {
                    sprite->move(-direction * moveSpeed * deltaTime); // consistent step back
                    // Optional: try alternative path or stop movement
                    return;
                }
            }
        }
    }
    else
    {
        currentPatrolIndex = (currentPatrolIndex + 1) % patrolPath.size();
        generatedInitialPatrolPath = false;
    }
}

void Guard::alert()
{
    std::cout << "[LOG] Guard is alerted!" << std::endl;
}

void Guard::chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime)
{
    state = GuardState::Chasing;

    float distanceToPlayer = std::hypot(playerPos.x - sprite->getPosition().x, playerPos.y - sprite->getPosition().y);
    pathTimer += deltaTime;

    if (distanceToPlayer <= viewDistance)
    {
        sf::Vector2f dir = playerPos - sprite->getPosition();
        float length = std::hypot(dir.x, dir.y);
        if (length > 1.f)
        {
            dir /= length;
            sprite->move(dir * moveSpeed * deltaTime);

            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds()))
                {
                    sprite->move(-dir * moveSpeed * deltaTime);
                    break;
                }
            }
        }
        return;
    }

    if (pathTimer >= repathInterval || std::hypot(playerPos.x - lastKnownPlayerPosition.x, playerPos.y - lastKnownPlayerPosition.y) > 30.f)
    {
        currentPath = pathfinder.findPath(sprite->getPosition(), playerPos);
        lastKnownPlayerPosition = playerPos;
        pathTimer = 0.f;
    }

    if (!currentPath.empty())
    {
        sf::Vector2f nextTarget = currentPath[1];
        sf::Vector2f dir = nextTarget - sprite->getPosition();
        float length = std::hypot(dir.x, dir.y);

        if (length < 5.f)
        {
            currentPath.erase(currentPath.begin());
        }
        else
        {
            dir /= length;
            sprite->move(dir * moveSpeed * deltaTime);

            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds()))
                {
                    sprite->move(-dir * moveSpeed * deltaTime);
                    break;
                }
            }
        }
    }
}

void Guard::search(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, const sf::Vector2f &lastPlayerPos, float &tileSize, float &deltaTime)
{
    switch (currentPhase)
    {
    case SearchPhase::LookAround:
        if (!searchClockStarted)
        {
            searchClock.restart();
            searchClockStarted = true;
            std::cout << "[SEARCH] Look Around started\n";
        }

        facingDir = {
            std::cos(searchClock.getElapsedTime().asSeconds() * 2),
            std::sin(searchClock.getElapsedTime().asSeconds() * 2)};

        if (searchClock.getElapsedTime().asSeconds() > 2.f)
        {
            currentPhase = SearchPhase::Wander;
            searchClock.restart();
            searchClockStarted = false;
            std::cout << "[SEARCH] Switching to Wander\n";
        }
        break;

    case SearchPhase::Wander:
    {
        if (!searchClockStarted)
        {
            searchClock.restart();
            searchClockStarted = true;
            std::cout << "[SEARCH] Wandering started\n";

            wanderPath = pathfinder.findPath(sprite->getPosition(), lastKnownPlayerPosition);
            currentWanderPathIndex = 1;
            generatedInitialWanderPath = true;
            hasCurrentWanderPath = true;
        }

        if (generatedInitialWanderPath)
        {
            if (hasCurrentWanderPath && currentWanderPathIndex < wanderPath.size())
            {
                sf::Vector2f target = wanderPath[currentWanderPathIndex];
                sf::Vector2f direction = target - sprite->getPosition();
                float length = std::hypot(direction.x, direction.y);

                if (length < 2.f)
                {
                    currentWanderPathIndex++;
                }
                else
                {
                    direction /= length;
                    facingDir = direction;
                    sprite->move(direction * moveSpeed * deltaTime);

                    for (const auto &obj : obstacles)
                    {
                        if (checkCollision(obj.getBounds()))
                        {
                            sprite->move(-direction * moveSpeed * deltaTime);
                            break;
                        }
                    }
                }
            }
            else // Finished initial wander to last known position
            {
                if (wanderTargets.empty())
                {
                    wanderTargets.clear();
                    currentWanderIndex = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        float offSetX = static_cast<float>((rand() % 201) - 100);
                        float offSetY = static_cast<float>((rand() % 201) - 100);
                        sf::Vector2f candidate = lastKnownPlayerPosition + sf::Vector2f(offSetX, offSetY);

                        if (pathfinder.isWalkable(candidate))
                        {
                            wanderTargets.push_back(candidate);
                        }
                    }
                }

                if (!hasCurrentWanderPath && currentWanderIndex < wanderTargets.size())
                {
                    // Find a path to next wander target
                    wanderPath = pathfinder.findPath(sprite->getPosition(), wanderTargets[currentWanderIndex]);
                    currentWanderPathIndex = 1;
                    hasCurrentWanderPath = true;
                }

                if (hasCurrentWanderPath && currentWanderPathIndex < wanderPath.size())
                {
                    sf::Vector2f target = wanderPath[currentWanderPathIndex];
                    sf::Vector2f direction = target - sprite->getPosition();
                    float length = std::hypot(direction.x, direction.y);

                    if (length < 2.f)
                    {
                        currentWanderPathIndex++;
                    }
                    else
                    {
                        direction /= length;
                        facingDir = direction;
                        sprite->move(direction * moveSpeed * deltaTime);

                        for (const auto &obj : obstacles)
                        {
                            if (checkCollision(obj.getBounds()))
                            {
                                sprite->move(-direction * moveSpeed * deltaTime);
                                break;
                            }
                        }
                    }
                }
                else if (hasCurrentWanderPath && currentWanderPathIndex >= wanderPath.size())
                {
                    // Reached wander point
                    if (!isPausingAtTarget)
                    {
                        wanderPauseClock.restart();
                        isPausingAtTarget = true;
                        std::cout << "[SEARCH] Reached wander point. Pausing to look around.\n";
                    }

                    facingDir = sf::Vector2f(std::cos(wanderPauseClock.getElapsedTime().asSeconds() * 2),
                                             std::sin(wanderPauseClock.getElapsedTime().asSeconds() * 2));

                    if (wanderPauseClock.getElapsedTime().asSeconds() >= pauseDurationAtTarget)
                    {
                        currentWanderIndex++;
                        hasCurrentWanderPath = false;
                        isPausingAtTarget = false;
                    }
                }
            }
        }

        if (searchClock.getElapsedTime().asSeconds() > searchDuration)
        {
            std::cout << "[SEARCH] Switching to ToPatrol\n";
            currentPhase = SearchPhase::ReturnToPatrol;
            searchClock.restart();
            searchClockStarted = false;
            generatedInitialWanderPath = false;
            hasCurrentWanderPath = false;
            wanderTargets.clear();
        }

        break;
    }
    case SearchPhase::ReturnToPatrol:
    {
        if (!isReturningToPatrol)
        {
            std::cout << "[SEARCH] Calculating return path...\n";
            returnPath = pathfinder.findPath(sprite->getPosition(), initialPosition);
            returnPathIndex = 1;
            isReturningToPatrol = true;
        }

        if (returnPathIndex < returnPath.size())
        {
            sf::Vector2f target = returnPath[returnPathIndex];
            sf::Vector2f direction = target - sprite->getPosition();
            float distance = std::hypot(direction.x, direction.y);

            if (distance < 2.f)
            {
                returnPathIndex++;
            }
            else
            {
                direction /= distance;
                facingDir = direction;
                sprite->move(direction * moveSpeed * deltaTime);

                for (const auto &obj : obstacles)
                {
                    if (checkCollision(obj.getBounds()))
                    {
                        sprite->move(-direction * moveSpeed * deltaTime);
                        break;
                    }
                }
            }
        }
        else
        {
            // Finished returning to patrol
            std::cout << "[SEARCH] Successfully returned to patrol.\n";
            state = GuardState::Patrolling;
            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;
            isReturningToPatrol = false;
        }
    }
    }
}

void Guard::capture(GameState &gameState, const sf::Vector2f &playerPos, sf::Clock &gameOverClock)
{
    std::cout << "[LOG] Guard captured the player!" << std::endl;
    std::cout << "[LOG] player Pos: " << playerPos.x << ", " << playerPos.y << std::endl;

    gameState = GameState::GAME_OVER;
    gameOverClock.restart();
}

bool Guard::canSeePlayer(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f toPlayer = playerPos - sprite->getPosition();
    float distance = std::hypot(toPlayer.x, toPlayer.y);

    if (distance > viewDistance)
        return false;

    sf::Vector2f dir = toPlayer / distance;
    sf::Vector2f current = sprite->getPosition();

    for (float t = 0.f; t < distance; t += 5.f)
    {
        sf::Vector2f point = current + dir * t;
        sf::FloatRect pointRect(point, {2.f, 2.f});

        for (const auto &ob : obstacles)
        {
            if (ob.getType() == ObjectType::Wall || ob.getType() == ObjectType::Box)
            {
                if (ob.getBounds().findIntersection(pointRect).has_value())
                    return false;
            }
        }
    }

    if (distance != 0)
        facingDir = dir;
    float dot = std::clamp(facingDir.x * dir.x + facingDir.y * dir.y, -1.f, 1.f);
    float angle = std::acos(dot) * 180.f / PI;

    return angle <= (fieldOfView / 2.f);
}

void Guard::drawSightCone(sf::RenderWindow &window)
{
    if (!sightCone)
        return;

    const int segments = 30;
    float angledRad = fieldOfView * PI / 180.f;
    float startAngle = std::atan2(facingDir.y, facingDir.x) - angledRad / 2.f;

    sf::VertexArray cone(sf::PrimitiveType::TriangleFan, segments + 2);
    // sf::Vector2f origin = sprite->getPosition() + sf::Vector2f(sprite->getRadius(), sprite->getRadius());
    sf::Vector2f origin = sprite->getPosition();

    cone[0].position = origin;
    cone[0].color = sightColor;

    for (int i = 0; i <= segments; i++)
    {
        float theta = startAngle + (i / static_cast<float>(segments)) * angledRad;
        sf::Vector2f point = origin + sf::Vector2f(std::cos(theta), std::sin(theta)) * viewDistance;
        cone[i + 1].position = point;
        cone[i + 1].color = sightColor;
    }
    window.draw(cone);
}

void Guard::update(Player &player, const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, GameState &gameState, PathFinder &pathfinder, sf::RenderWindow &window, float &tileSize, float &deltaTime, sf::Clock &gameOverClock)
{
    float dist = std::hypot(playerPos.x - sprite->getPosition().x, playerPos.y - sprite->getPosition().y);
    bool seesPlayer = canSeePlayer(playerPos, obstacles);

    switch (state)
    {
    case GuardState::Patrolling:
    {
        if (seesPlayer)
        {
            std::cout << "[FSM] switching to alerted from patrolling.\n";
            state = GuardState::Alerted;
        }
        else
        {
            patrol(obstacles, pathfinder, deltaTime);
        }
        break;
    }
    case GuardState::Alerted:
    {
        if (!seesPlayer)
        {
            // If the player disappears mid-alert, forget about alert
            std::cout << "[FSM] lost player during alert! switching to Searching\n";
            lastKnownPlayerPosition = playerPos;
            lastSeenMarker.setPosition(lastKnownPlayerPosition);

            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;
            alertClockStarted = false;
            state = GuardState::Searching;

            break;
        }

        if (!alertClockStarted)
        {
            alertClock.restart();
            alertClockStarted = true;
            std::cout << "[FSM] Guard is alerted and pausing before chasing.\n";
        }

        if (alertClock.getElapsedTime().asSeconds() >= alertDuration)
        {
            std::cout << "[FSM] Alert duration passed. Switching to Chasing.\n";
            alertClockStarted = false;
            state = GuardState::Chasing;
        }
        break;
    }
    case GuardState::Chasing:
    {
        if (!seesPlayer)
        {
            std::cout << "[FSM] lost player! Staying alerted\n";
            lastKnownPlayerPosition = playerPos;
            lastSeenMarker.setPosition(lastKnownPlayerPosition);

            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;

            state = GuardState::Searching;
        }
        else
        {
            if (sprite->getGlobalBounds().findIntersection(player.getBounds()).has_value())
            {
                capture(gameState, playerPos, gameOverClock);
            }
            else
            {
                chase(playerPos, obstacles, pathfinder, deltaTime);
            }
        }
        break;
    }
    case GuardState::Searching:
        search(obstacles, pathfinder, playerPos, tileSize, deltaTime);
        if (canSeePlayer(playerPos, obstacles))
        {
            std::cout << "[FSM] Player found again! switching to Alerted\n";
            state = GuardState::Alerted;
            alertClock.restart();
        }

        break;
    }
}
void Guard::draw(sf::RenderWindow &window)
{
    // Convert guard's position to grid coordinates
    sf::Vector2f guardPos = sprite->getPosition();
    // int gridX = static_cast<int>(guardPos.x / tileSize);
    // int gridY = static_cast<int>(guardPos.y / tileSize);

    // Create diamond shape for tile highlight
    sf::ConvexShape diamond;
    diamond.setPointCount(4);

    float tileWidth = static_cast<float>(tileSize);
    float tileHeight = tileWidth / 2.f;

    diamond.setPoint(0, sf::Vector2f(guardPos.x, guardPos.y + tileHeight / 2.f));             // left
    diamond.setPoint(1, sf::Vector2f(guardPos.x + tileWidth / 2.f, guardPos.y));              // top
    diamond.setPoint(2, sf::Vector2f(guardPos.x + tileWidth, guardPos.y + tileHeight / 2.f)); // right
    diamond.setPoint(3, sf::Vector2f(guardPos.x + tileWidth / 2.f, guardPos.y + tileHeight)); // bottom

    diamond.setFillColor(sf::Color(255, 255, 0, 100)); // Yellow with transparency

    // Draw tile highlight diamond under the guard
    window.draw(diamond);

    // Draw the guard sprite and last seen marker on top
    window.draw(*sprite);
    window.draw(lastSeenMarker);
}

void Guard::setPos(const sf::Vector2f &position)
{
    sprite->setPosition(position);
    initialPosition = position;
}

void Guard::setVelocity(const sf::Vector2f &dir)
{
    velocity = dir;
    if (velocity != sf::Vector2f({0.f, 0.f}))
    {
        if (std::hypot(velocity.x, velocity.y) > 0)
        {

            facingDir = velocity / std::hypot(velocity.x, velocity.y);
        }
        else
        {
            std::cerr << "[Error] Guard velocity is zero!" << std::endl;
        }
    }
}
void Guard::setPatrolPath(const std::vector<sf::Vector2f> &path)
{
    patrolPath = path;
}
sf::FloatRect Guard::getBounds() const
{
    return sprite->getGlobalBounds();
}
void Guard::resetState()
{
    state = GuardState::Patrolling;
}