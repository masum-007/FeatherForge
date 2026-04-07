#include "Game.hpp"
#include <cmath>

Game::Game() 
    : window(sf::VideoMode(1280, 720), "FeatherForge Engine"), 
      slingshotPos(200, 550), 
      isDragging(false),
      birdIsActive(false),
      birdsRemaining(3), 
      score(0) // Initialize score to 0
{
    window.setFramerateLimit(60);
    
    worldView.setSize(1280, 720);
    worldView.setCenter(640, 360);
    uiView = window.getDefaultView(); 

    LoadAssets();

    // Ground is still hardcoded as the foundation
    ground = std::make_unique<Entity>(*physics.GetWorld(), 640, 700, 1280, 40, EntityType::GROUND, &groundTex);

    // --- LOAD THE JSON LEVEL INSTEAD OF HARDCODING BLOCKS ---
    LoadLevel("assets/level1.json");

    SpawnBird();
}

void Game::SpawnBird() {
    if (birdsRemaining > 0) {
        // Don't forget to pass the bird texture here too!
        bird = std::make_unique<Entity>(*physics.GetWorld(), slingshotPos.x, slingshotPos.y, 30, 30, EntityType::BIRD, &birdTex);
        bird->GetBody()->SetType(b2_kinematicBody); 
        birdIsActive = false;
        isDragging = false;
    }
}

void Game::Run() {
    while (window.isOpen()) {
        ProcessEvents();
        Update();
        Render();
    }
}

void Game::ProcessEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
            
        // Start Drag
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (!birdIsActive && bird) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
                sf::Vector2f birdPos(b2Pos.x * SCALE, b2Pos.y * SCALE);

                if (abs(mousePos.x - birdPos.x) < 50 && abs(mousePos.y - birdPos.y) < 50) {
                    isDragging = true;
                }
            }
        }

        /// Release to Launch
        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (isDragging && bird) {
                isDragging = false;
                birdIsActive = true;
                birdsRemaining--;

                bird->GetBody()->SetType(b2_dynamicBody);
                bird->GetBody()->SetAwake(true);

                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                
                // FIX 2: Increased multiplier from 0.15f to 0.6f for a strong punch
                b2Vec2 force((slingshotPos.x - bird->GetBody()->GetPosition().x * SCALE) * 0.6f, 
                             (slingshotPos.y - bird->GetBody()->GetPosition().y * SCALE) * 0.6f);
                
                bird->GetBody()->ApplyLinearImpulseToCenter(force, true);
            }
        }
    }

    // Handle Dragging Math
    if (isDragging && bird) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f dragPos(mousePos.x, mousePos.y);
        
        // Calculate pull vector
        sf::Vector2f pullVec = dragPos - slingshotPos;
        float dist = std::sqrt(pullVec.x * pullVec.x + pullVec.y * pullVec.y);
        float maxPull = 100.0f; // Max rubber band stretch
        
        // Clamp the drag distance
        if (dist > maxPull) {
            pullVec = (pullVec / dist) * maxPull;
            dragPos = slingshotPos + pullVec;
        }
        
        // Move physics body with mouse
        bird->GetBody()->SetTransform(b2Vec2(dragPos.x / SCALE, dragPos.y / SCALE), 0);
    }
}

void Game::CheckBirdState() {
    if (!birdIsActive || !bird) return;
    
    b2Vec2 pos = bird->GetBody()->GetPosition();
    
    // 1. Check if the bird flew completely off the screen
    bool offScreen = pos.x * SCALE > 1300 || pos.x * SCALE < -100 || pos.y * SCALE > 800;

    // 2. Check if the bird has come to a complete rest anywhere (ground OR on wood)
    // Box2D sets IsAwake() to false automatically when an object stops moving!
    bool stopped = !bird->GetBody()->IsAwake();

    if (offScreen || stopped) {
        SpawnBird(); // Reload the next bird
    }
}

void Game::Update() {
    physics.Update(1.0f / 60.0f);
    CheckBirdState();

    if (bird && birdIsActive) {
        float birdX = bird->GetBody()->GetPosition().x * SCALE;
        float cameraX = std::max(640.0f, birdX);
        worldView.setCenter(cameraX, 360); 
    } else {
        worldView.setCenter(640, 360);
    }

    // --- UPDATED CLEANUP & SCORING ---
    blocks.erase(
        std::remove_if(blocks.begin(), blocks.end(),
            [this](const std::unique_ptr<Entity>& e) { 
                if (e->IsDestroyed()) {
                    // Add to score before deleting
                    if (e->GetType() == EntityType::ENEMY) score += 500;
                    else score += 100;
                    return true;
                }
                return false;
            }),
        blocks.end()
    );
}

void Game::DrawEnvironment() {
    // 1. Draw Sky Layer
    window.clear(sf::Color(135, 206, 235)); 

    // 2. Draw Sun
    sf::CircleShape sun(40);
    sun.setFillColor(sf::Color(255, 223, 0));
    sun.setPosition(1000, 100);
    window.draw(sun);

    // 3. Draw Clouds
    sf::CircleShape cloudPart(30);
    cloudPart.setFillColor(sf::Color(255, 255, 255, 200));
    cloudPart.setPosition(200, 150);
    window.draw(cloudPart);
    cloudPart.setPosition(240, 130);
    window.draw(cloudPart);
    cloudPart.setPosition(280, 150);
    window.draw(cloudPart);

    // 4. Draw Mountains (Triangles)
    sf::ConvexShape mountain;
    mountain.setPointCount(3);
    mountain.setFillColor(sf::Color(105, 105, 105)); // Dark Gray
    mountain.setPoint(0, sf::Vector2f(400, 680));
    mountain.setPoint(1, sf::Vector2f(600, 300));
    mountain.setPoint(2, sf::Vector2f(800, 680));
    window.draw(mountain);

    // 5. Draw simple Tree
    sf::RectangleShape trunk(sf::Vector2f(20, 80));
    trunk.setFillColor(sf::Color(139, 69, 19));
    trunk.setPosition(100, 600);
    window.draw(trunk);
    sf::CircleShape leaves(40);
    leaves.setFillColor(sf::Color(34, 139, 34));
    leaves.setPosition(70, 540);
    window.draw(leaves);
}

void Game::DrawSlingshot() {
    // FIX: Changed height from 100 to 120 so it touches the Y=680 ground line
    sf::RectangleShape post(sf::Vector2f(15, 120)); 
    post.setFillColor(sf::Color(101, 67, 33)); 
    post.setOrigin(7.5f, 0);
    post.setPosition(slingshotPos.x, slingshotPos.y + 10);
    window.draw(post);

    // FIX 3: Only stretch the bands if the bird has NOT been launched
    if (bird && !birdIsActive) {
        b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
        sf::Vector2f birdPos(b2Pos.x * SCALE, b2Pos.y * SCALE);

        sf::Vertex leftBand[] = {
            sf::Vertex(sf::Vector2f(slingshotPos.x - 10, slingshotPos.y), sf::Color::Black),
            sf::Vertex(birdPos, sf::Color::Black)
        };
        sf::Vertex rightBand[] = {
            sf::Vertex(sf::Vector2f(slingshotPos.x + 10, slingshotPos.y), sf::Color::Black),
            sf::Vertex(birdPos, sf::Color::Black)
        };

        window.draw(leftBand, 2, sf::Lines);
        window.draw(rightBand, 2, sf::Lines);
    }
}
void Game::DrawTrajectory() {
    if (!bird) return;

    b2Vec2 startPos = bird->GetBody()->GetPosition();

    // Match the new 0.6f multiplier here
    b2Vec2 force((slingshotPos.x - startPos.x * SCALE) * 0.6f, 
                 (slingshotPos.y - startPos.y * SCALE) * 0.6f);

    // FIX 1: Override the 0 mass to prevent a divide-by-zero error
    float mass = 1.0f; 
    b2Vec2 initialVelocity = b2Vec2(force.x / mass, force.y / mass);
    b2Vec2 gravity = physics.GetWorld()->GetGravity();

    float maxTime = 1.5f; 
    
    for (float t = 0.0f; t < maxTime; t += 0.05f) { 
        b2Vec2 stepPos;
        
        stepPos.x = startPos.x + initialVelocity.x * t + 0.5f * gravity.x * t * t;
        stepPos.y = startPos.y + initialVelocity.y * t + 0.5f * gravity.y * t * t;

        sf::CircleShape dot(3); 
        dot.setFillColor(sf::Color::White); // Solid white so it's easy to see
        dot.setOrigin(1.5f, 1.5f); 
        dot.setPosition(stepPos.x * SCALE, stepPos.y * SCALE); 
        
        window.draw(dot);
    }
}

void Game::LoadAssets() {
    // These load the images from the assets folder.
    // If it fails, SFML will print an error to your terminal automatically.
    birdTex.loadFromFile("assets/bird.png");
    woodTex.loadFromFile("assets/wood.png");
    groundTex.loadFromFile("assets/ground.png");
    enemyTex.loadFromFile("assets/enemy.png");//added enemy
    // --- NEW FONT SETUP ---
    font.loadFromFile("assets/arial.ttf");
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(20, 50); // Place it below the red bird icons
}


void Game::LoadLevel(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        printf("Failed to open level file: %s\n", filepath.c_str());
        return;
    }

    nlohmann::json levelData;
    file >> levelData;

    // Loop through the "entities" array in the JSON
    for (const auto& item : levelData["entities"]) {
        std::string typeStr = item["type"];
        float x = item["x"];
        float y = item["y"];
        float w = item["width"];
        float h = item["height"];

        if (typeStr == "wood") {
            blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), x, y, w, h, EntityType::WOOD, &woodTex));
        } 
        else if (typeStr == "enemy") {
            blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), x, y, w, h, EntityType::ENEMY, &enemyTex));
        }
    }
}

void Game::Render() {
    // 1. APPLY THE GAME CAMERA
    window.setView(worldView);

    DrawEnvironment();
    DrawSlingshot();

    ground->Render(window);
    
    for (auto& block : blocks) {
        block->Render(window);
    }
    
    if (isDragging) {
        DrawTrajectory();
    }
    
    if (bird) {
        bird->Render(window);
    }

    // 2. SWITCH TO UI CAMERA (so the UI doesn't fly away with the bird)
    window.setView(uiView);

    // UI: Draw Remaining Birds counter
    for (int i = 0; i < birdsRemaining; i++) {
        sf::CircleShape uiBird(10);
        uiBird.setFillColor(sf::Color::Red);
        uiBird.setPosition(20 + (i * 30), 20);
        window.draw(uiBird);
    }
    // --- NEW: Draw the Score ---
    scoreText.setString("SCORE: " + std::to_string(score));
    window.draw(scoreText);

    window.display();
}