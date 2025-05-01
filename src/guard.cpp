#include "guard.hpp"
#include <SFML/System.hpp>
#include <string>

Guard::Guard()
{
    circle.setRadius(10.f);
    circle.setFillColor(sf::Color::Cyan);
    state = GuardState::Patrolling;
    velocity = {1.0f, 0.f};
    facingDir = velocity;

    lastSeenMarker.setRadius(5.f);
    lastSeenMarker.setFillColor(sf::Color::Red);
    lastSeenMarker.setOrigin({5.f, 5.f}); // Center the marker
}
Guard::Guard(const sf::Vector2f &startPos) : Guard()
{
    setPosition(startPos);
}
bool Guard::checkCollision(const sf::FloatRect &otherBounds) const
{
    return circle.getGlobalBounds().findIntersection(otherBounds).has_value();
}

void Guard::patrol(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime)
{
    for (const auto &path : patrolPath)
    {
        std::cout << path.x << ", " << path.y << std::endl;
    }
    // sf::sleep(sf::seconds(3));

    if (!generatedInitialPatrolPath)
    {
        if (patrolPath.empty())
            return;

        patrolPathToTarget = pathfinder.findPath(circle.getPosition(), patrolPath[currentPatrolIndex]);
        currentPatrolPathIndex = 0;
        generatedInitialPatrolPath = true;
        std::cout << "[PATROL] Starting path to patrol point " << currentPatrolIndex << "\n";
    }

    if (currentPatrolPathIndex < patrolPathToTarget.size())
    {
        sf::Vector2f target = patrolPathToTarget[currentPatrolPathIndex];
        sf::Vector2f direction = target - circle.getPosition();
        float distance = std::hypot(direction.x, direction.y);

        if (distance < 2.f) // Reached this small step
        {
            currentPatrolPathIndex++;
        }
        else
        {
            direction /= distance;
            facingDir = direction;
            circle.move(direction * moveSpeed * deltaTime);

            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds()))
                {
                    circle.move(-direction * moveSpeed * deltaTime);
                    break;
                }
            }
        }
    }
    else
    {
        // Finished reaching patrol point
        currentPatrolIndex = (currentPatrolIndex + 1) % patrolPath.size();
        generatedInitialPatrolPath = false;
    }

    std::cout << "[LOG] Guard is patrolling\n";
}

void Guard::alert()
{
    std::cout << "[LOG] Guard is alerted!" << std::endl;
}

void Guard::chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime)
{
    state = GuardState::Chasing;
    circle.setFillColor(sf::Color::Magenta);

    float distanceToPlayer = std::hypot(playerPos.x - circle.getPosition().x, playerPos.y - circle.getPosition().y);
    pathTimer += deltaTime;

    if (distanceToPlayer < 40.f)
    {
        sf::Vector2f dir = playerPos - circle.getPosition();
        float length = std::hypot(dir.x, dir.y);
        if (length > 1.f)
        {
            dir /= length;
            circle.move(dir * moveSpeed * deltaTime);

            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds()))
                {
                    circle.move(-dir * 1.f);
                    break;
                }
            }
        }
        return;
    }

    if (pathTimer >= repathInterval || std::hypot(playerPos.x - lastKnownPlayerPosition.x, playerPos.y - lastKnownPlayerPosition.y) > 30.f)
    {
        currentPath = pathfinder.findPath(circle.getPosition(), playerPos);
        lastKnownPlayerPosition = playerPos;
        pathTimer = 0.f;
    }

    if (!currentPath.empty())
    {
        sf::Vector2f nextTarget = currentPath[1];
        sf::Vector2f dir = nextTarget - circle.getPosition();
        float length = std::hypot(dir.x, dir.y);

        if (length < 5.f)
        {
            currentPath.erase(currentPath.begin());
        }
        else
        {
            dir /= length;
            circle.move(dir * moveSpeed * deltaTime);

            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds()))
                {
                    circle.move(-dir * 1.f);
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

            wanderPath = pathfinder.findPath(circle.getPosition(), lastKnownPlayerPosition);
            currentWanderPathIndex = 1;
            generatedInitialWanderPath = true;
            hasCurrentWanderPath = true;
        }

        if (generatedInitialWanderPath)
        {
            if (hasCurrentWanderPath && currentWanderPathIndex < wanderPath.size())
            {
                sf::Vector2f target = wanderPath[currentWanderPathIndex];
                sf::Vector2f direction = target - circle.getPosition();
                float length = std::hypot(direction.x, direction.y);

                if (length < 2.f)
                {
                    currentWanderPathIndex++;
                }
                else
                {
                    direction /= length;
                    facingDir = direction;
                    circle.move(direction * moveSpeed * deltaTime);

                    for (const auto &obj : obstacles)
                    {
                        if (checkCollision(obj.getBounds()))
                        {
                            circle.move(-direction * 1.f);
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
                    wanderPath = pathfinder.findPath(circle.getPosition(), wanderTargets[currentWanderIndex]);
                    currentWanderPathIndex = 1;
                    hasCurrentWanderPath = true;
                }

                if (hasCurrentWanderPath && currentWanderPathIndex < wanderPath.size())
                {
                    sf::Vector2f target = wanderPath[currentWanderPathIndex];
                    sf::Vector2f direction = target - circle.getPosition();
                    float length = std::hypot(direction.x, direction.y);

                    if (length < 2.f)
                    {
                        currentWanderPathIndex++;
                    }
                    else
                    {
                        direction /= length;
                        facingDir = direction;
                        circle.move(direction * moveSpeed * deltaTime);

                        for (const auto &obj : obstacles)
                        {
                            if (checkCollision(obj.getBounds()))
                            {
                                circle.move(-direction * 1.f);
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
            returnPath = pathfinder.findPath(circle.getPosition(), initialPosition);
            returnPathIndex = 1;
            isReturningToPatrol = true;
        }

        if (returnPathIndex < returnPath.size())
        {
            sf::Vector2f target = returnPath[returnPathIndex];
            sf::Vector2f direction = target - circle.getPosition();
            float distance = std::hypot(direction.x, direction.y);

            if (distance < 2.f)
            {
                returnPathIndex++;
            }
            else
            {
                direction /= distance;
                facingDir = direction;
                circle.move(direction * moveSpeed * deltaTime);

                for (const auto &obj : obstacles)
                {
                    if (checkCollision(obj.getBounds()))
                    {
                        circle.move(-direction * moveSpeed * deltaTime);
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
            circle.setFillColor(sf::Color::Cyan);
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

    circle.setFillColor(sf::Color::White);
    gameState = GameState::GAME_OVER;
    gameOverClock.restart();
}

bool Guard::canSeePlayer(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f toPlayer = playerPos - circle.getPosition();
    float distance = std::hypot(toPlayer.x, toPlayer.y);

    if (distance > viewDistance)
        return false;

    sf::Vector2f dir = toPlayer / distance;
    sf::Vector2f current = circle.getPosition();

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
    sf::Vector2f origin = circle.getPosition() + sf::Vector2f(circle.getRadius(), circle.getRadius());

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
    float dist = std::hypot(playerPos.x - circle.getPosition().x, playerPos.y - circle.getPosition().y);
    bool seesPlayer = canSeePlayer(playerPos, obstacles);

    switch (state)
    {
    case GuardState::Patrolling:
    {
        if (seesPlayer)
        {
            std::cout << "[FSM] switching to alerted from patrolling.\n";
            state = GuardState::Alerted;
            circle.setFillColor(sf::Color::Yellow);
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
            circle.setFillColor(sf::Color::Red);
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
            circle.setFillColor(sf::Color::Magenta);
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
            circle.setFillColor(sf::Color::Red);
        }
        else
        {
            if (circle.getGlobalBounds().findIntersection(player.getBounds()).has_value())
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
            circle.setFillColor(sf::Color::Yellow);
        }

        break;
    }
}

void Guard::draw(sf::RenderWindow &window)
{
    sf::RectangleShape highlight;
    highlight.setSize(sf::Vector2f(tileSize, tileSize));
    highlight.setFillColor(sf::Color(255, 255, 0, 100)); // Yellow with transparency

    // Calculate which tile the guard is on
    sf::Vector2f guardPos = circle.getPosition();
    int tileX = static_cast<int>(guardPos.x) / tileSize;
    int tileY = static_cast<int>(guardPos.y) / tileSize;

    highlight.setPosition(sf::Vector2f(tileX * tileSize, tileY * tileSize));

    window.draw(highlight);
    window.draw(circle);
    window.draw(lastSeenMarker);
}

void Guard::setPosition(const sf::Vector2f &position)
{
    circle.setPosition(position);
    initialPosition = position;
}

void Guard::setVelocity(const sf::Vector2f &dir)
{
    velocity = dir;
    if (velocity != sf::Vector2f({0.f, 0.f}))
    {
        facingDir = velocity / std::hypot(velocity.x, velocity.y);
    }
}
void Guard::setPatrolPath(const std::vector<sf::Vector2f> &path)
{
    patrolPath = path;
}
sf::FloatRect Guard::getBounds() const
{
    return circle.getGlobalBounds();
}
void Guard::resetState()
{
    state = GuardState::Patrolling;
    circle.setFillColor(sf::Color::Cyan);
}