#include "guard.hpp"
#include <SFML/System.hpp>
#include <string>

// Constructor to initialize the guard with a texture and default state
Guard::Guard(const sf::Texture &tex) : sprite(std::make_unique<sf::Sprite>(tex))
{
    state = GuardState::Patrolling; // Set the initial state to patrolling
    velocity = {1.0f, 0.f};         // Set initial movement velocity to right
    facingDir = velocity;           // Initially, the guard faces the direction of movement

    sprite->setOrigin({tex.getSize().x / 2.f, (float)tex.getSize().y}); // Set sprite origin to the center-bottom
    sprite->setScale({32.f / 256.f, 32.f / 256.f});                     // Scale the sprite

    // Initialize the marker for the last seen position of the player
    lastSeenMarker.setRadius(5.f);
    lastSeenMarker.setFillColor(sf::Color::Red); // Color the marker red
    lastSeenMarker.setOrigin({5.f, 5.f});        // Set the marker's origin to its center
}

// Constructor to initialize the guard with a texture and a start position
Guard::Guard(const sf::Texture &tex, const sf::Vector2f &startPos) : Guard(tex)
{
    setPos(startPos); // Set the initial position of the guard
}

// Function to check if the guard's sprite collides with another bounding box
bool Guard::checkCollision(const sf::FloatRect &otherBounds) const
{
    return sprite->getGlobalBounds().findIntersection(otherBounds).has_value(); // Check intersection
}

// Function for patrolling, navigating through a path and avoiding obstacles
void Guard::patrol(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime)
{
    // If the patrol path has not been generated yet
    if (!generatedInitialPatrolPath)
    {
        if (patrolPath.empty())
        {
            std::cout << "[PATROL] No patrol path" << std::endl;
            return; // Exit if no patrol path exists
        }

        // Recalculate the patrol path to handle dynamic obstacles
        patrolPathToTarget = pathfinder.findPath(sprite->getPosition(), patrolPath[currentPatrolIndex]);
        currentPatrolPathIndex = 0;
        generatedInitialPatrolPath = true;
    }

    // If there are more patrol points in the path
    if (currentPatrolPathIndex < patrolPathToTarget.size())
    {
        sf::Vector2f target = patrolPathToTarget[currentPatrolPathIndex];
        sf::Vector2f direction = target - sprite->getPosition();
        float distance = std::hypot(direction.x, direction.y); // Calculate the distance to the target

        if (distance < 2.f) // If the guard is close to the patrol point
        {
            currentPatrolPathIndex++; // Move to the next patrol point
        }
        else
        {
            direction /= distance; // Normalize direction vector
            facingDir = direction; // Update the direction the guard is facing

            sprite->move(direction * moveSpeed * deltaTime); // Move the sprite

            // Check for collisions with obstacles
            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds())) // If collision occurs
                {
                    sprite->move(-direction * moveSpeed * deltaTime); // Step back
                    return;                                           // Exit to avoid collision
                }
            }
        }
    }
    else
    {
        // If the guard reaches the last patrol point, cycle through the patrol path
        currentPatrolIndex = (currentPatrolIndex + 1) % patrolPath.size();
        generatedInitialPatrolPath = false;
    }
}

// Function to handle the alert state of the guard
void Guard::alert()
{
    std::cout << "[LOG] Guard is alerted!" << std::endl;
}

// Function to handle the chasing behavior of the guard when it sees the player
void Guard::chase(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, PathFinder &pathfinder, float &deltaTime)
{
    state = GuardState::Chasing; // Set the guard state to chasing

    float distanceToPlayer = std::hypot(playerPos.x - sprite->getPosition().x, playerPos.y - sprite->getPosition().y);
    pathTimer += deltaTime; // Increase the path timer

    // If the player is within view distance, pursue them
    if (distanceToPlayer <= viewDistance)
    {
        sf::Vector2f dir = playerPos - sprite->getPosition();
        float length = std::hypot(dir.x, dir.y);
        if (length > 1.f)
        {
            dir /= length;                             // Normalize the direction to the player
            sprite->move(dir * moveSpeed * deltaTime); // Move toward the player

            // Check for collisions while moving
            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds())) // If collision occurs
                {
                    sprite->move(-dir * moveSpeed * deltaTime); // Step back
                    break;                                      // Exit to avoid collision
                }
            }
        }
        return;
    }

    // Recalculate the path to the player if necessary
    if (pathTimer >= repathInterval || std::hypot(playerPos.x - lastKnownPlayerPosition.x, playerPos.y - lastKnownPlayerPosition.y) > 30.f)
    {
        currentPath = pathfinder.findPath(sprite->getPosition(), playerPos);
        lastKnownPlayerPosition = playerPos; // Update the last known position of the player
        pathTimer = 0.f;
    }

    // Follow the path towards the player
    if (!currentPath.empty())
    {
        sf::Vector2f nextTarget = currentPath[1];
        sf::Vector2f dir = nextTarget - sprite->getPosition();
        float length = std::hypot(dir.x, dir.y);

        if (length < 5.f)
        {
            currentPath.erase(currentPath.begin()); // Remove the reached target
        }
        else
        {
            dir /= length;                             // Normalize the direction
            sprite->move(dir * moveSpeed * deltaTime); // Move along the path

            // Check for collisions with obstacles
            for (const auto &obj : obstacles)
            {
                if (checkCollision(obj.getBounds())) // If collision occurs
                {
                    sprite->move(-dir * moveSpeed * deltaTime); // Step back
                    break;                                      // Exit to avoid collision
                }
            }
        }
    }
}

// Function to handle the searching behavior when the player is lost
void Guard::search(const std::vector<GameObject> &obstacles, PathFinder &pathfinder, const sf::Vector2f &lastPlayerPos, float &tileSize, float &deltaTime)
{
    switch (currentPhase)
    {
    case SearchPhase::LookAround:
        if (!searchClockStarted)
        {
            searchClock.restart(); // Start the search timer
            searchClockStarted = true;
            std::cout << "[SEARCH] Look Around started\n";
        }

        // Rotate the guard to look around
        facingDir = {
            std::cos(searchClock.getElapsedTime().asSeconds() * 2),
            std::sin(searchClock.getElapsedTime().asSeconds() * 2)};

        // If the look around phase has lasted long enough, switch to wandering
        if (searchClock.getElapsedTime().asSeconds() > 2.f)
        {
            currentPhase = SearchPhase::Wander;
            searchClock.restart();
            searchClockStarted = false;
            std::cout << "[SEARCH] Switching to Wander\n";
        }
        break;

    // Case for wandering to find the last known player position
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

        // Move along the wander path
        if (generatedInitialWanderPath)
        {
            if (hasCurrentWanderPath && currentWanderPathIndex < wanderPath.size())
            {
                sf::Vector2f target = wanderPath[currentWanderPathIndex];
                sf::Vector2f direction = target - sprite->getPosition();
                float length = std::hypot(direction.x, direction.y);

                if (length < 2.f)
                {
                    currentWanderPathIndex++; // Move to next point
                }
                else
                {
                    direction /= length;
                    facingDir = direction;
                    sprite->move(direction * moveSpeed * deltaTime);

                    // Check for collisions with obstacles
                    for (const auto &obj : obstacles)
                    {
                        if (checkCollision(obj.getBounds())) // If collision occurs
                        {
                            sprite->move(-direction * moveSpeed * deltaTime); // Step back
                            break;                                            // Exit to avoid collision
                        }
                    }
                }
            }
            else // Finished wandering to the last known position
            {
                // Continue wandering with new random targets if needed
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

                // If there are wander targets, move towards the next one
                if (!hasCurrentWanderPath && currentWanderIndex < wanderTargets.size())
                {
                    wanderPath = pathfinder.findPath(sprite->getPosition(), wanderTargets[currentWanderIndex]);
                    currentWanderPathIndex = 1;
                    hasCurrentWanderPath = true;
                }

                // Continue wandering
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

                        // Check for collisions with obstacles
                        for (const auto &obj : obstacles)
                        {
                            if (checkCollision(obj.getBounds())) // If collision occurs
                            {
                                sprite->move(-direction * moveSpeed * deltaTime); // Step back
                                break;
                            }
                        }
                    }
                }
                else if (hasCurrentWanderPath && currentWanderPathIndex >= wanderPath.size())
                {
                    // Pause for a while after reaching a wander target
                    if (!isPausingAtTarget)
                    {
                        wanderPauseClock.restart();
                        isPausingAtTarget = true;
                        std::cout << "[SEARCH] Reached wander point. Pausing to look around.\n";
                    }

                    facingDir = sf::Vector2f(std::cos(wanderPauseClock.getElapsedTime().asSeconds() * 2),
                                             std::sin(wanderPauseClock.getElapsedTime().asSeconds() * 2));

                    // If pause duration is over, go to the next target
                    if (wanderPauseClock.getElapsedTime().asSeconds() >= pauseDurationAtTarget)
                    {
                        currentWanderIndex++;
                        hasCurrentWanderPath = false;
                        isPausingAtTarget = false;
                    }
                }
            }
        }

        // After a while, return to patrolling state
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

    // Case for returning to the patrol route after searching
    case SearchPhase::ReturnToPatrol:
    {
        if (!isReturningToPatrol)
        {
            std::cout << "[SEARCH] Calculating return path...\n";
            returnPath = pathfinder.findPath(sprite->getPosition(), initialPosition);
            returnPathIndex = 1;
            isReturningToPatrol = true;
        }

        // Move back to the patrol position
        if (returnPathIndex < returnPath.size())
        {
            sf::Vector2f target = returnPath[returnPathIndex];
            sf::Vector2f direction = target - sprite->getPosition();
            float distance = std::hypot(direction.x, direction.y);

            if (distance < 2.f)
            {
                returnPathIndex++; // Move to next path point
            }
            else
            {
                direction /= distance;
                facingDir = direction;
                sprite->move(direction * moveSpeed * deltaTime);

                // Check for collisions while moving
                for (const auto &obj : obstacles)
                {
                    if (checkCollision(obj.getBounds())) // If collision occurs
                    {
                        sprite->move(-direction * moveSpeed * deltaTime); // Step back
                        break;
                    }
                }
            }
        }
        else
        {
            // Successfully returned to patrol
            std::cout << "[SEARCH] Successfully returned to patrol.\n";
            state = GuardState::Patrolling;
            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;
            isReturningToPatrol = false;
        }
    }
    }
}

// Function to handle player capture
void Guard::capture(GameState &gameState, const sf::Vector2f &playerPos, sf::Clock &gameOverClock)
{
    std::cout << "[LOG] Guard captured the player!" << std::endl;
    std::cout << "[LOG] player Pos: " << playerPos.x << ", " << playerPos.y << std::endl;

    gameState = GameState::GAME_OVER; // Change game state to game over
    gameOverClock.restart();          // Start the game over timer
}

// Function to check if the guard can see the player based on distance and obstacles
bool Guard::canSeePlayer(const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles)
{
    sf::Vector2f toPlayer = playerPos - sprite->getPosition();
    float distance = std::hypot(toPlayer.x, toPlayer.y);

    if (distance > viewDistance)
        return false; // If the player is too far, return false

    // Check if there are any obstacles blocking the line of sight
    sf::Vector2f dir = toPlayer / distance;
    sf::Vector2f current = sprite->getPosition();

    for (float t = 0.f; t < distance; t += 5.f)
    {
        sf::Vector2f point = current + dir * t;
        sf::FloatRect pointRect(point, {2.f, 2.f}); // Small area to check along the line of sight

        for (const auto &ob : obstacles)
        {
            if (ob.getType() == ObjectType::Wall || ob.getType() == ObjectType::Box)
            {
                if (ob.getBounds().findIntersection(pointRect).has_value()) // If any obstacle blocks the sight
                    return false;                                           // The guard cannot see the player
            }
        }
    }

    // Check if the player is within the guard's field of view
    if (distance != 0)
        facingDir = dir;

    float dot = std::clamp(facingDir.x * dir.x + facingDir.y * dir.y, -1.f, 1.f);
    float angle = std::acos(dot) * 180.f / PI;

    return angle <= (fieldOfView / 2.f); // Check if the player is within the cone of vision
}

// Function to draw the guard's sight cone on the window
void Guard::drawSightCone(sf::RenderWindow &window)
{
    if (!sightCone)
        return; // If sight cone is not enabled, return

    const int segments = 30;
    float angledRad = fieldOfView * PI / 180.f;
    float startAngle = std::atan2(facingDir.y, facingDir.x) - angledRad / 2.f;

    sf::VertexArray cone(sf::PrimitiveType::TriangleFan, segments + 2);
    sf::Vector2f origin = sprite->getPosition();

    cone[0].position = origin;
    cone[0].color = sightColor;

    // Draw the cone with the segments
    for (int i = 0; i <= segments; i++)
    {
        float theta = startAngle + (i / static_cast<float>(segments)) * angledRad;
        sf::Vector2f point = origin + sf::Vector2f(std::cos(theta), std::sin(theta)) * viewDistance;
        cone[i + 1].position = point;
        cone[i + 1].color = sightColor;
    }

    window.draw(cone); // Draw the sight cone on the window
}

// Function to update the guard's state and behavior
void Guard::update(Player &player, const sf::Vector2f &playerPos, const std::vector<GameObject> &obstacles, GameState &gameState, PathFinder &pathfinder, sf::RenderWindow &window, float &tileSize, float &deltaTime, sf::Clock &gameOverClock)
{
    float dist = std::hypot(playerPos.x - sprite->getPosition().x, playerPos.y - sprite->getPosition().y);
    bool seesPlayer = canSeePlayer(playerPos, obstacles); // Check if the guard can see the player

    // Switch between different guard states based on current state and player interaction
    switch (state)
    {
    case GuardState::Patrolling:
    {
        if (seesPlayer)
        {
            std::cout << "[FSM] switching to alerted from patrolling.\n";
            state = GuardState::Alerted; // Switch to alerted state
        }
        else
        {
            patrol(obstacles, pathfinder, deltaTime); // Continue patrolling
        }
        break;
    }
    case GuardState::Alerted:
    {
        if (!seesPlayer)
        {
            // If the player is lost during the alert state, start searching
            std::cout << "[FSM] lost player during alert! switching to Searching\n";
            lastKnownPlayerPosition = playerPos;
            lastSeenMarker.setPosition(lastKnownPlayerPosition);

            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;
            alertClockStarted = false;
            state = GuardState::Searching;
        }

        // If the guard has been alerted for long enough, start chasing
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
            // If the player is lost while chasing, start searching again
            std::cout << "[FSM] lost player! Staying alerted\n";
            lastKnownPlayerPosition = playerPos;
            lastSeenMarker.setPosition(lastKnownPlayerPosition);

            currentPhase = SearchPhase::LookAround;
            searchClockStarted = false;

            state = GuardState::Searching;
        }
        else
        {
            // If the guard captures the player, end the game
            if (sprite->getGlobalBounds().findIntersection(player.getBounds()).has_value())
            {
                capture(gameState, playerPos, gameOverClock);
            }
            else
            {
                chase(playerPos, obstacles, pathfinder, deltaTime); // Continue chasing the player
            }
        }
        break;
    }
    case GuardState::Searching:
        search(obstacles, pathfinder, playerPos, tileSize, deltaTime); // Continue searching

        if (canSeePlayer(playerPos, obstacles))
        {
            std::cout << "[FSM] Player found again! switching to Alerted\n";
            state = GuardState::Alerted; // If the player is found, switch back to alerted state
            alertClock.restart();
        }

        break;
    }
}

// Function to draw the guard on the window
void Guard::draw(sf::RenderWindow &window)
{
    sf::Vector2f guardPos = sprite->getPosition(); // Get the current position of the guard

    // Create a diamond shape for highlighting the tile
    sf::ConvexShape diamond;
    diamond.setPointCount(4);

    float tileWidth = static_cast<float>(tileSize);
    float tileHeight = tileWidth / 2.f;

    diamond.setPoint(0, sf::Vector2f(guardPos.x, guardPos.y + tileHeight / 2.f));             // Left
    diamond.setPoint(1, sf::Vector2f(guardPos.x + tileWidth / 2.f, guardPos.y));              // Top
    diamond.setPoint(2, sf::Vector2f(guardPos.x + tileWidth, guardPos.y + tileHeight / 2.f)); // Right
    diamond.setPoint(3, sf::Vector2f(guardPos.x + tileWidth / 2.f, guardPos.y + tileHeight)); // Bottom

    diamond.setFillColor(sf::Color(255, 255, 0, 100)); // Set the diamond color to yellow with transparency

    window.draw(diamond); // Draw the tile highlight diamond on the window

    window.draw(*sprite);        // Draw the guard sprite on the window
    window.draw(lastSeenMarker); // Draw the last seen marker on the window
}

// Function to set the position of the guard
void Guard::setPos(const sf::Vector2f &position)
{
    sprite->setPosition(position);
    initialPosition = position; // Store the initial position
}

// Function to set the velocity/direction of the guard
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

// Function to set the patrol path for the guard
void Guard::setPatrolPath(const std::vector<sf::Vector2f> &path)
{
    patrolPath = path;
}

// Function to get the global bounds (collision box) of the guard's sprite
sf::FloatRect Guard::getBounds() const
{
    return sprite->getGlobalBounds();
}

// Function to reset the guard's state to patrolling
void Guard::resetState()
{
    state = GuardState::Patrolling;
}