#include "Game.hpp"
#include <cmath>

Game::Game() 
    : window(sf::VideoMode(1280, 720), "FeatherForge Engine"), 
      slingshotPos(200, 550), 
      isDragging(false),
      birdIsActive(false),
      birdsRemaining(3), 
      score(0), // Initialize score to 0
      m_currentState(GameState::Menu), // Start at the menu
      m_currentLevel(1)
{
    window.setFramerateLimit(60);
    
    worldView.setSize(1280, 720);
    worldView.setCenter(640, 360);
    uiView = window.getDefaultView(); 

    LoadAssets();

    // Ground is persistent across levels
    ground = std::make_unique<Entity>(*physics.GetWorld(), 640, 700, 1280, 40, EntityType::GROUND, &groundTex);
    
    // Notice we do NOT LoadLevel() or SpawnBird() here anymore. 
    // That happens when the user clicks a level!
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

// --- 2. PROCESS EVENTS ---
void Game::ProcessEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
            
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePosUI = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

            if (m_currentState == GameState::Menu) {
                // Play Button (Raw Math bypasses SFML 3 Rect changes)
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 300 && mousePosUI.y <= 360) m_currentState = GameState::LevelSelect;
                // Exit Button
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 400 && mousePosUI.y <= 460) window.close();
            } 
            else if (m_currentState == GameState::LevelSelect) {
                for (int i = 0; i < MAX_LEVELS; i++) {
                    float x = 340.f + (i % 3) * 220.0f;
                    float y = 250.f + (i / 3) * 150.0f;
                    if (mousePosUI.x >= x && mousePosUI.x <= x + 100 && mousePosUI.y >= y && mousePosUI.y <= y + 100) {
                        ResetLevel(i + 1); 
                    }
                }
            }
            else if (m_currentState == GameState::LevelComplete) {
                // Next Level Button
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 350 && mousePosUI.y <= 410) {
                    if (m_currentLevel < MAX_LEVELS) ResetLevel(m_currentLevel + 1);
                    else m_currentState = GameState::Menu; 
                }
                // Menu Button
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 450 && mousePosUI.y <= 510) {
                    m_currentState = GameState::Menu;
                }
            }
            else if (m_currentState == GameState::Playing) {
                if (!birdIsActive && bird) {
                    sf::Vector2f mouseWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window), worldView);
                    b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
                    sf::Vector2f birdPos{b2Pos.x * SCALE, b2Pos.y * SCALE};

                    if (std::abs(mouseWorld.x - birdPos.x) < 50.f && std::abs(mouseWorld.y - birdPos.y) < 50.f) {
                        isDragging = true;
                    }
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (m_currentState == GameState::Playing && isDragging && bird) {
                isDragging = false;
                birdIsActive = true;
                birdsRemaining--;

                bird->GetBody()->SetType(b2_dynamicBody);
                bird->GetBody()->SetAwake(true);
                
                b2Vec2 force((slingshotPos.x - bird->GetBody()->GetPosition().x * SCALE) * 0.6f, 
                             (slingshotPos.y - bird->GetBody()->GetPosition().y * SCALE) * 0.6f);
                bird->GetBody()->ApplyLinearImpulseToCenter(force, true);
            }
        }
    }

    if (m_currentState == GameState::Playing && isDragging && bird) {
        sf::Vector2f dragPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), worldView);
        sf::Vector2f pullVec = dragPos - slingshotPos;
        float dist = std::sqrt(pullVec.x * pullVec.x + pullVec.y * pullVec.y);
        float maxPull = 100.0f; 
        
        if (dist > maxPull) {
            pullVec = (pullVec / dist) * maxPull;
            dragPos = slingshotPos + pullVec;
        }
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

// --- 3. UPDATE ---
void Game::Update() {
    // FIX: Always update the environment so clouds animate in menus!
    m_environment.update(1.0f / 60.0f);

    if (m_currentState != GameState::Playing) return;

    physics.Update(1.0f / 60.0f);
    CheckBirdState();

    if (bird && birdIsActive) {
        float birdX = bird->GetBody()->GetPosition().x * SCALE;
        float cameraX = std::max(640.0f, birdX);
        worldView.setCenter({cameraX, 360.f}); 
    } else {
        worldView.setCenter({640.f, 360.f});
    }

    for (auto& block : blocks) {
        if (block->GetType() == EntityType::WOOD && block->GetHealth() <= 50.0f && !block->isCracked) {
            block->SwapTexture(&woodCrackedTex);
            block->isCracked = true; 
        }
    }

    // Safely erase entities
    blocks.erase(
        std::remove_if(blocks.begin(), blocks.end(),
            [this](const std::unique_ptr<Entity>& e) { 
                if (e->IsDestroyed()) {
                    sf::Vector2f deathPos{e->GetBody()->GetPosition().x * SCALE, e->GetBody()->GetPosition().y * SCALE};
                    
                    if (e->GetType() == EntityType::WOOD) m_particles.emitWood(deathPos);
                    else if (e->GetType() == EntityType::BIRD) m_particles.emitFeathers(deathPos);
                    else m_particles.emitDust(deathPos); 

                    if (e->GetType() == EntityType::ENEMY) score += 500;
                    else score += 100;
                    
                    return true; // The ~Entity destructor handles Box2D safely!
                }
                return false;
            }),
        blocks.end()
    );

    m_particles.update(1.0f / 60.0f);

    // Timer check
    bool enemiesAlive = false;
    for (auto& b : blocks) {
        if (b->GetType() == EntityType::ENEMY) { enemiesAlive = true; break; }
    }
    
    if (!enemiesAlive) {
        m_levelTransitionTimer += (1.0f / 60.0f);
        if (m_levelTransitionTimer > 2.0f) { 
            m_currentState = GameState::LevelComplete; 
            m_levelTransitionTimer = 0.0f;
        }
    } else {
        m_levelTransitionTimer = 0.0f; 
    }
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
    birdTex.loadFromFile("assets/bird.png");
    woodTex.loadFromFile("assets/wood.png");
    groundTex.loadFromFile("assets/ground.png");
    enemyTex.loadFromFile("assets/enemy.png");
    woodCrackedTex.loadFromFile("assets/wood_cracked.png");
    font.loadFromFile("assets/arial.ttf");
    
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(20, 50);

    // --- NEW: Load your beautiful background ---
    if (menuBgTex.loadFromFile("assets/Feather.png")) {
        menuBgSprite.setTexture(menuBgTex);
        // Scale it to perfectly fit the 1280x720 window
        menuBgSprite.setScale(1280.0f / menuBgTex.getSize().x, 720.0f / menuBgTex.getSize().y);
    }
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

// --- 4. RENDER ---
void Game::Render() {
    window.clear(sf::Color(135, 206, 235)); // Sky background

    if (m_currentState == GameState::Playing || m_currentState == GameState::LevelComplete) {
        // Draw game world
        window.setView(worldView);

        // --- NEW: Render the dynamic parallax background ---
        float camX = worldView.getCenter().x;
        m_environment.render(window, camX);

        DrawSlingshot();
        ground->Render(window);
        for (auto& block : blocks) block->Render(window);
        if (isDragging) DrawTrajectory();
        if (bird) bird->Render(window);
        window.draw(m_particles);

        // Draw HUD overlay
        window.setView(uiView);
        for (int i = 0; i < birdsRemaining; i++) {
            sf::CircleShape uiBird(10);
            uiBird.setFillColor(sf::Color::Red);
            uiBird.setPosition(20 + (i * 30), 20);
            window.draw(uiBird);
        }
        scoreText.setString("SCORE: " + std::to_string(score));
        window.draw(scoreText);
    }

    // Draw UI Panels over everything
    window.setView(uiView);
    if (m_currentState == GameState::Menu) DrawMenu();
    else if (m_currentState == GameState::LevelSelect) DrawLevelSelect();
    else if (m_currentState == GameState::LevelComplete) DrawLevelComplete();

    window.display();
}

// --- NEW HELPER FUNCTIONS (Paste these at the bottom of Game.cpp) ---

void Game::ResetLevel(int level) {
    // FIX: 100% rely on your Entity destructors to prevent the Box2D double-free crash.
    blocks.clear();
    bird.reset();

    m_currentLevel = level;
    score = 0;
    birdsRemaining = 3;
    birdIsActive = false;
    isDragging = false;
    m_levelTransitionTimer = 0.0f;

    // Load the background theme
    m_environment.loadLevel(level);

    std::string path = "assets/level" + std::to_string(level) + ".json";
    LoadLevel(path);
    SpawnBird();
    
    m_currentState = GameState::Playing;
}

void Game::DrawMenu() {
    // Draw your uploaded background first
    window.draw(menuBgSprite);

    // Draw a dark transparent box so text is readable over the image
    sf::RectangleShape panel(sf::Vector2f(400, 350));
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setPosition(440, 150);
    window.draw(panel);

    sf::Text title("FEATHER FORGE", font, 45);
    title.setFillColor(sf::Color(255, 215, 0)); // Gold title
    title.setPosition(455, 180);
    window.draw(title);

    // Play Button
    sf::RectangleShape startBtn(sf::Vector2f(200, 60));
    startBtn.setPosition(540, 300);
    startBtn.setFillColor(sf::Color(46, 204, 113)); // Bright green
    startBtn.setOutlineThickness(2);
    startBtn.setOutlineColor(sf::Color::White);
    window.draw(startBtn);

    sf::Text startTxt("START", font, 30);
    startTxt.setFillColor(sf::Color::White);
    startTxt.setPosition(590, 310);
    window.draw(startTxt);

    // Exit Button
    sf::RectangleShape exitBtn(sf::Vector2f(200, 60));
    exitBtn.setPosition(540, 400);
    exitBtn.setFillColor(sf::Color(231, 76, 60)); // Bright red
    exitBtn.setOutlineThickness(2);
    exitBtn.setOutlineColor(sf::Color::White);
    window.draw(exitBtn);

    sf::Text exitTxt("EXIT", font, 30);
    exitTxt.setFillColor(sf::Color::White);
    exitTxt.setPosition(605, 410);
    window.draw(exitTxt);
}

void Game::DrawLevelSelect() {
    sf::Text title("SELECT LEVEL", font, 48);
    title.setFillColor(sf::Color::Black);
    title.setPosition(470, 100);
    window.draw(title);

    for (int i = 0; i < MAX_LEVELS; i++) {
        float x = 340 + (i % 3) * 220.0f;
        float y = 250 + (i / 3) * 150.0f;

        sf::RectangleShape box(sf::Vector2f(100, 100));
        box.setPosition(x, y);
        box.setFillColor(sf::Color(150, 150, 150));
        window.draw(box);

        sf::Text lvlTxt(std::to_string(i + 1), font, 40);
        lvlTxt.setFillColor(sf::Color::White);
        lvlTxt.setPosition(x + 35, y + 25);
        window.draw(lvlTxt);
    }
}

void Game::DrawLevelComplete() {
    // Semi-transparent dark overlay
    sf::RectangleShape overlay(sf::Vector2f(1280, 720));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    sf::Text title("LEVEL COMPLETE!", font, 64);
    title.setFillColor(sf::Color::Yellow);
    title.setPosition(380, 150);
    window.draw(title);

    sf::Text scoreDisplay("FINAL SCORE: " + std::to_string(score), font, 40);
    scoreDisplay.setFillColor(sf::Color::White);
    scoreDisplay.setPosition(480, 250);
    window.draw(scoreDisplay);

    sf::RectangleShape nextBtn(sf::Vector2f(200, 60));
    nextBtn.setPosition(540, 350);
    nextBtn.setFillColor(sf::Color(100, 200, 100));
    window.draw(nextBtn);

    sf::Text nextTxt("NEXT LEVEL", font, 24);
    nextTxt.setFillColor(sf::Color::White);
    nextTxt.setPosition(565, 365);
    window.draw(nextTxt);

    sf::RectangleShape menuBtn(sf::Vector2f(200, 60));
    menuBtn.setPosition(540, 450);
    menuBtn.setFillColor(sf::Color(100, 100, 200));
    window.draw(menuBtn);

    sf::Text menuTxt("MAIN MENU", font, 24);
    menuTxt.setFillColor(sf::Color::White);
    menuTxt.setPosition(570, 465);
    window.draw(menuTxt);
}