#include "Game.hpp"
#include <cmath>
#include <algorithm>

Game::Game() 
    : window(sf::VideoMode(1280, 720), "FeatherForge Engine"), 
      slingshotPos(200, 620), 
      isDragging(false),
      birdIsActive(false),
      score(0),
      m_currentState(GameState::Menu), 
      m_currentLevel(1)
{
    window.setFramerateLimit(60);
    worldView.setSize({1280.f, 720.f});
    worldView.setCenter({640.f, 360.f});
    uiView = window.getDefaultView(); 

    LoadAssets();
    ground = std::make_unique<Entity>(*physics.GetWorld(), 640, 700, 1280, 40, EntityType::GROUND, &groundTex);
}

void Game::LoadAssets() {
    birdTex.loadFromFile("assets/bird.png");
    birdFireTex.loadFromFile("assets/bird_fire.png");
    birdSlothTex.loadFromFile("assets/bird_sloth.png");
    birdFreezeTex.loadFromFile("assets/bird_freeze.png");
    birdWaterTex.loadFromFile("assets/bird_water.png");

    woodTex.loadFromFile("assets/wood.png");
    woodCrackedTex.loadFromFile("assets/wood_cracked.png");
    iceTex.loadFromFile("assets/ice.png"); 
    groundTex.loadFromFile("assets/ground.png");
    enemyTex.loadFromFile("assets/enemy.png");
    
    font.loadFromFile("assets/arial.ttf");
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition({20.f, 50.f});

    if (menuBgTex.loadFromFile("assets/Feather.png")) {
        menuBgSprite.setTexture(menuBgTex);
        menuBgSprite.setScale({1280.0f / menuBgTex.getSize().x, 720.0f / menuBgTex.getSize().y});
    }
    // --- NEW: Generate the Rune mathematically! ---
    runeTex = CustomGraphics::GenerateMagicRune();
    runeSprite.setTexture(runeTex);
    runeSprite.setOrigin({100.f, 100.f}); // Center the 200x200 image
}

void Game::LoadLevel(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    nlohmann::json levelData;
    file >> levelData;

    m_birdQueue.clear();
    if (levelData.contains("birds")) {
        for (const auto& b : levelData["birds"]) {
            if (b == "fire") m_birdQueue.push_back(BirdType::Fire);
            else if (b == "sloth") m_birdQueue.push_back(BirdType::Sloth);
            else if (b == "freeze") m_birdQueue.push_back(BirdType::Freeze);
            else if (b == "water") m_birdQueue.push_back(BirdType::Water);
            else m_birdQueue.push_back(BirdType::Normal);
        }
    } else {
        m_birdQueue.push_back(BirdType::Normal);
        m_birdQueue.push_back(BirdType::Normal);
        m_birdQueue.push_back(BirdType::Normal);
    }
    birdsRemaining = m_birdQueue.size();

    for (const auto& item : levelData["entities"]) {
        std::string typeStr = item["type"];
        float x = item["x"]; float y = item["y"];
        float w = item["width"]; float h = item["height"];

        if (typeStr == "wood") {
            blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), x, y, w, h, EntityType::WOOD, &woodTex));
        } else if (typeStr == "ice") {
            blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), x, y, w, h, EntityType::ICE, &iceTex));
        } else if (typeStr == "enemy") {
            blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), x, y, w, h, EntityType::ENEMY, &enemyTex));
        }
    }
}

void Game::SpawnBird() {
    if (!m_birdQueue.empty()) {
        m_currentBirdType = m_birdQueue.front();
        m_birdQueue.erase(m_birdQueue.begin());
        
        sf::Texture* texToUse = &birdTex;
        if (m_currentBirdType == BirdType::Fire) texToUse = &birdFireTex;
        else if (m_currentBirdType == BirdType::Sloth) texToUse = &birdSlothTex;
        else if (m_currentBirdType == BirdType::Freeze) texToUse = &birdFreezeTex;
        else if (m_currentBirdType == BirdType::Water) texToUse = &birdWaterTex;

        bird = std::make_unique<Entity>(*physics.GetWorld(), slingshotPos.x, slingshotPos.y, 30, 30, EntityType::BIRD, texToUse);
        bird->GetBody()->SetType(b2_kinematicBody); 
        birdIsActive = false;
        isDragging = false;
        m_birdAbilityUsed = false;
        m_birdActiveTimer = 0.0f;

        // Sloth is Heavy AND does Massive 1-Hit Kill Damage
        if (m_currentBirdType == BirdType::Sloth) {
            b2MassData mass; 
            bird->GetBody()->GetMassData(&mass);
            mass.mass *= 5.0f; 
            bird->GetBody()->SetMassData(&mass);
            bird->SetDamageMultiplier(50.0f); // 50x Damage!
        }
    }
}

void Game::CheckBirdState() {
    if (!birdIsActive || !bird) return;
    
    b2Vec2 pos = bird->GetBody()->GetPosition();
    bool offScreen = pos.x * SCALE > 2500.f || pos.x * SCALE < -500.f || pos.y * SCALE > 800.f;
    bool stopped = !bird->GetBody()->IsAwake();
    
    m_birdActiveTimer += (1.0f / 60.0f);
    bool timeOut = m_birdActiveTimer > 4.0f; // 4 seconds max flight
    
    if (offScreen || stopped || timeOut) {
        sf::Vector2f deathPos{pos.x * SCALE, pos.y * SCALE};
        m_particles.emitFeathers(deathPos);

        bird.reset(); // Safe Box2D cleanup
        birdIsActive = false;
        m_birdActiveTimer = 0.0f; 

        if (birdsRemaining > 0) SpawnBird();
    }
}

void Game::Update() {
    m_environment.update(1.0f / 60.0f);
    if (m_currentState != GameState::Playing) return;

    physics.Update(1.0f / 60.0f);
    CheckBirdState();

    if (bird && birdIsActive) {
        float birdX = bird->GetBody()->GetPosition().x * SCALE;
        worldView.setCenter({std::max(640.0f, birdX), 360.f}); 
        
        // --- NEW: Draw Flight Trails! ---
        sf::Vector2f pxPos{birdX, bird->GetBody()->GetPosition().y * SCALE};
        if (m_currentBirdType == BirdType::Fire) m_particles.emitFireTrail(pxPos);
        else if (m_currentBirdType == BirdType::Freeze) m_particles.emitIceTrail(pxPos);
    } else {
        worldView.setCenter({640.f, 360.f});
    }

    if (bird && birdIsActive && !m_birdAbilityUsed) {
        b2Vec2 bPos = bird->GetBody()->GetPosition();
        bool impacted = false;
        for (auto& block : blocks) {
            if ((bPos - block->GetBody()->GetPosition()).Length() < 1.5f) { impacted = true; break; }
        }

        if (impacted) {
            m_birdAbilityUsed = true;
            sf::Vector2f pxPos{bPos.x * SCALE, bPos.y * SCALE};

            if (m_currentBirdType == BirdType::Fire) {
                m_particles.emitWood(pxPos); 
                for (auto& block : blocks) {
                    if ((bPos - block->GetBody()->GetPosition()).Length() < 3.5f) {
                        m_burningBodies[block->GetBody()] = 0.0f; 
                        block->getSprite().setColor(sf::Color(255, 100, 100));
                    }
                }
            } else if (m_currentBirdType == BirdType::Freeze) {
                m_particles.emitIce(pxPos); 
                for (auto& block : blocks) {
                    if ((bPos - block->GetBody()->GetPosition()).Length() < 4.0f) {
                        m_frozenBodies.insert(block->GetBody()); 
                        block->getSprite().setColor(sf::Color(150, 255, 255));
                    }
                }
            } else if (m_currentBirdType == BirdType::Water) {
                // --- FIX: Intense Water Tidal Wave ---
                m_particles.emitWater(pxPos); 
                for (auto& block : blocks) {
                    b2Vec2 diff = block->GetBody()->GetPosition() - bPos;
                    float dist = diff.Length();
                    if (dist < 10.0f && dist > 0.1f) {
                        diff.Normalize();
                        diff *= (120.0f / dist); // Massive outward push
                        block->GetBody()->ApplyLinearImpulseToCenter(diff, true);
                        block->TakeDamage(30.0f); // Water pressure damages things
                    }
                }
            }
        }
    }

    // Keep the rest of Update() exactly the same (Fire ticks, cleanup, win/loss logic)...
    // Process Fire Damage Tick
    for (auto it = m_burningBodies.begin(); it != m_burningBodies.end(); ) {
        it->second += (1.0f / 60.0f);
        if (rand() % 15 == 0) m_particles.emitWood(sf::Vector2f{it->first->GetPosition().x * SCALE, it->first->GetPosition().y * SCALE});
        
        if (it->second > 1.5f) { 
            it->first->ApplyLinearImpulseToCenter(b2Vec2(0, 5000.f), true);
            it = m_burningBodies.erase(it);
        } else {
            ++it;
        }
    }
    for (auto& block : blocks) {
        if (block->GetType() == EntityType::WOOD && block->GetHealth() <= 50.0f && !block->isCracked) {
            block->SwapTexture(&woodCrackedTex);
            block->isCracked = true; 
        }
    }
    blocks.erase(
        std::remove_if(blocks.begin(), blocks.end(),
            [this](const std::unique_ptr<Entity>& e) { 
                float yPos = e->GetBody()->GetPosition().y * SCALE;
                if (e->IsDestroyed() || yPos > 800.f) {
                    sf::Vector2f deathPos{e->GetBody()->GetPosition().x * SCALE, std::min(yPos, 700.f)};
                    if (e->GetType() == EntityType::WOOD) m_particles.emitWood(deathPos);
                    else if (e->GetType() == EntityType::ICE) m_particles.emitIce(deathPos);
                    else if (e->GetType() == EntityType::BIRD) m_particles.emitFeathers(deathPos);
                    else m_particles.emitDust(deathPos); 
                    if (e->GetType() == EntityType::ENEMY) score += 500;
                    else score += 100;
                    m_burningBodies.erase(e->GetBody());
                    m_frozenBodies.erase(e->GetBody());
                    return true; 
                }
                return false;
            }),
        blocks.end()
    );
    m_particles.update(1.0f / 60.0f);
    bool enemiesAlive = false;
    for (auto& b : blocks) if (b->GetType() == EntityType::ENEMY) { enemiesAlive = true; break; }
    
    if (!enemiesAlive) {
        m_levelTransitionTimer += (1.0f / 60.0f);
        if (m_levelTransitionTimer > 2.0f) { m_currentState = GameState::LevelComplete; m_levelTransitionTimer = 0.0f; }
    } else if (birdsRemaining == 0 && !bird) {
        m_levelTransitionTimer += (1.0f / 60.0f);
        if (m_levelTransitionTimer > 2.0f) { m_currentState = GameState::GameOver; m_levelTransitionTimer = 0.0f; }
    } else {
        m_levelTransitionTimer = 0.0f; 
    }
}

void Game::ResetLevel(int level) {
    blocks.clear();
    bird.reset();
    m_burningBodies.clear();
    m_frozenBodies.clear();

    m_currentLevel = level;
    score = 0;
    birdsRemaining = 3;
    birdIsActive = false;
    isDragging = false;
    m_levelTransitionTimer = 0.0f;

    m_environment.loadLevel(level);

    std::string path = "assets/level" + std::to_string(level) + ".json";
    LoadLevel(path);
    SpawnBird();
    
    m_currentState = GameState::Playing;
}

void Game::DrawSlingshot() {
    slingshotPos = sf::Vector2f{200.f, 620.f}; // Grounded

    // --- Draw the Magical Rune behind everything ---
    runeSprite.setPosition(slingshotPos);
    runeRotation += 0.5f; // Slowly rotate it every frame
    runeSprite.setRotation(runeRotation);
    
    if (isDragging) {
        // When pulling back, the rune glows brightly and pulses!
        runeSprite.setColor(sf::Color(255, 255, 255, 255));
        
        // --- FIX: Use runeRotation to drive the pulse math instead of m_time ---
        float pulse = 1.2f + (std::sin(runeRotation * 0.1f) * 0.05f); 
        runeSprite.setScale({pulse, pulse});
    } else {
        // When idle, it is faded and dormant
        runeSprite.setColor(sf::Color(255, 255, 255, 80));
        runeSprite.setScale({1.0f, 1.0f});
    }
    window.draw(runeSprite);

    sf::RectangleShape stem(sf::Vector2f{16.f, 60.f});
    stem.setFillColor(sf::Color{90, 50, 20}); 
    stem.setOrigin({8.f, 0.f});
    stem.setPosition({slingshotPos.x, slingshotPos.y + 10.f});
    window.draw(stem);

    sf::RectangleShape backFork(sf::Vector2f{12.f, 40.f});
    backFork.setFillColor(sf::Color{70, 40, 15}); 
    backFork.setOrigin({6.f, 40.f});
    backFork.setPosition({slingshotPos.x + 8.f, slingshotPos.y + 15.f});
    backFork.setRotation(25.f);
    window.draw(backFork);

    if (bird && !birdIsActive) {
        b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
        sf::Vertex backBand[] = {
            sf::Vertex{sf::Vector2f{slingshotPos.x + 18.f, slingshotPos.y - 15.f}, sf::Color{40, 40, 40}},
            sf::Vertex{sf::Vector2f{b2Pos.x * SCALE, b2Pos.y * SCALE}, sf::Color{40, 40, 40}}
        };
        window.draw(backBand, 2, sf::PrimitiveType::Lines);
    }

    sf::RectangleShape frontFork(sf::Vector2f{12.f, 40.f});
    frontFork.setFillColor(sf::Color{110, 70, 30}); 
    frontFork.setOrigin({6.f, 40.f});
    frontFork.setPosition({slingshotPos.x - 8.f, slingshotPos.y + 15.f});
    frontFork.setRotation(-25.f);
    window.draw(frontFork);

    if (bird && !birdIsActive) {
        b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
        sf::Vertex frontBand[] = {
            sf::Vertex{sf::Vector2f{slingshotPos.x - 18.f, slingshotPos.y - 15.f}, sf::Color{60, 60, 60}},
            sf::Vertex{sf::Vector2f{b2Pos.x * SCALE, b2Pos.y * SCALE}, sf::Color{60, 60, 60}}
        };
        window.draw(frontBand, 2, sf::PrimitiveType::Lines);
    }
}

void Game::DrawTrajectory() {
    if (!bird) return;

    b2Vec2 startPos = bird->GetBody()->GetPosition();

    // --- FIX: Aiming dots must match the new dynamic speeds! ---
    float speedMult = 0.6f;
    if (m_currentBirdType == BirdType::Fire) speedMult = 0.85f; 
    else if (m_currentBirdType == BirdType::Sloth) speedMult = 0.35f; 
    else if (m_currentBirdType == BirdType::Freeze) speedMult = 0.45f; 

    b2Vec2 force((slingshotPos.x - startPos.x * SCALE) * speedMult, 
                 (slingshotPos.y - startPos.y * SCALE) * speedMult);

    float mass = bird->GetBody()->GetMass();
    if (mass == 0.0f) mass = 1.0f; 
    
    b2Vec2 initialVelocity = b2Vec2(force.x / mass, force.y / mass);
    b2Vec2 gravity = physics.GetWorld()->GetGravity();

    for (float t = 0.0f; t < 1.5f; t += 0.05f) { 
        b2Vec2 stepPos;
        stepPos.x = startPos.x + initialVelocity.x * t + 0.5f * gravity.x * t * t;
        stepPos.y = startPos.y + initialVelocity.y * t + 0.5f * gravity.y * t * t;

        sf::CircleShape dot(3.f); 
        dot.setFillColor(sf::Color::White); 
        dot.setOrigin({1.5f, 1.5f}); 
        dot.setPosition({stepPos.x * SCALE, stepPos.y * SCALE}); 
        window.draw(dot);
    }
}

void Game::ProcessEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
            
        // ... Keep all menu UI click logic exactly the same ...
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePosUI = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

            if (m_currentState == GameState::Menu) {
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 300 && mousePosUI.y <= 360) m_currentState = GameState::LevelSelect;
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 400 && mousePosUI.y <= 460) window.close();
            } 
            else if (m_currentState == GameState::LevelSelect) {
                for (int i = 0; i < MAX_LEVELS; i++) {
                    // --- FIX: Match the exact math from DrawLevelSelect (5x2 grid) ---
                    float x = 210.f + (i % 5) * 175.0f; 
                    float y = 250.f + (i / 5) * 150.0f; 
                    
                    if (mousePosUI.x >= x && mousePosUI.x <= x + 100 && mousePosUI.y >= y && mousePosUI.y <= y + 100) {
                        ResetLevel(i + 1); 
                    }
                }
            }
            else if (m_currentState == GameState::LevelComplete) {
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 350 && mousePosUI.y <= 410) {
                    if (m_currentLevel < MAX_LEVELS) ResetLevel(m_currentLevel + 1);
                    else m_currentState = GameState::Menu; 
                }
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 450 && mousePosUI.y <= 510) m_currentState = GameState::Menu;
            }
            else if (m_currentState == GameState::GameOver) {
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 350 && mousePosUI.y <= 410) ResetLevel(m_currentLevel);
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 450 && mousePosUI.y <= 510) m_currentState = GameState::Menu;
            }
            else if (m_currentState == GameState::Playing) {
                if (!birdIsActive && bird) {
                    sf::Vector2f mouseWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window), worldView);
                    b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
                    sf::Vector2f birdPos{b2Pos.x * SCALE, b2Pos.y * SCALE};

                    if (std::abs(mouseWorld.x - birdPos.x) < 50.f && std::abs(mouseWorld.y - birdPos.y) < 50.f) isDragging = true;
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
                
                // --- FIX: Dynamic Flight Speeds! ---
                float speedMult = 0.6f;
                if (m_currentBirdType == BirdType::Fire) speedMult = 0.85f; // Very fast
                else if (m_currentBirdType == BirdType::Sloth) speedMult = 0.35f; // Very slow and heavy
                else if (m_currentBirdType == BirdType::Freeze) speedMult = 0.45f; // Slow, floaty

                b2Vec2 force((slingshotPos.x - bird->GetBody()->GetPosition().x * SCALE) * speedMult, 
                             (slingshotPos.y - bird->GetBody()->GetPosition().y * SCALE) * speedMult);
                bird->GetBody()->ApplyLinearImpulseToCenter(force, true);
            }
        }
    }

    if (m_currentState == GameState::Playing && isDragging && bird) {
        sf::Vector2f dragPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), worldView);
        sf::Vector2f pullVec = dragPos - slingshotPos;
        float dist = std::sqrt(pullVec.x * pullVec.x + pullVec.y * pullVec.y);
        if (dist > 100.0f) {
            pullVec = (pullVec / dist) * 100.0f;
            dragPos = slingshotPos + pullVec;
        }
        bird->GetBody()->SetTransform(b2Vec2(dragPos.x / SCALE, dragPos.y / SCALE), 0);
    }
}

void Game::Render() {
    window.clear(sf::Color(135, 206, 235)); 

    if (m_currentState == GameState::Playing || m_currentState == GameState::LevelComplete || m_currentState == GameState::GameOver) {
        window.setView(worldView);
        float camX = worldView.getCenter().x;
        m_environment.render(window, camX);
        
        DrawSlingshot();
        ground->Render(window);
        for (auto& block : blocks) block->Render(window);
        if (isDragging) DrawTrajectory();
        if (bird) bird->Render(window);
        window.draw(m_particles);

        window.setView(uiView);
        
        // Draw upcoming birds using PNGs
        for (size_t i = 0; i < m_birdQueue.size(); i++) {
            sf::Sprite uiBird;
            if (m_birdQueue[i] == BirdType::Normal) uiBird.setTexture(birdTex);
            else if (m_birdQueue[i] == BirdType::Fire) uiBird.setTexture(birdFireTex);
            else if (m_birdQueue[i] == BirdType::Sloth) uiBird.setTexture(birdSlothTex);
            else if (m_birdQueue[i] == BirdType::Freeze) uiBird.setTexture(birdFreezeTex);
            else if (m_birdQueue[i] == BirdType::Water) uiBird.setTexture(birdWaterTex);
            
            if (uiBird.getTexture()) {
                float sX = 20.0f / uiBird.getTexture()->getSize().x;
                float sY = 20.0f / uiBird.getTexture()->getSize().y;
                uiBird.setScale({sX, sY});
            }
            uiBird.setPosition({20.f + (i * 30.f), 20.f});
            window.draw(uiBird);
        }
        scoreText.setString("SCORE: " + std::to_string(score));
        window.draw(scoreText);
    }

    window.setView(uiView);
    if (m_currentState == GameState::Menu) DrawMenu();
    else if (m_currentState == GameState::LevelSelect) DrawLevelSelect();
    else if (m_currentState == GameState::LevelComplete) DrawLevelComplete();
    else if (m_currentState == GameState::GameOver) DrawGameOver();

    window.display();
}

void Game::DrawMenu() {
    window.draw(menuBgSprite);
    sf::RectangleShape panel(sf::Vector2f{400.f, 350.f});
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setPosition({440.f, 150.f});
    window.draw(panel);

    sf::Text title("FEATHER FORGE", font, 43);
    title.setFillColor(sf::Color(255, 215, 0)); 
    title.setPosition({455.f, 180.f});
    window.draw(title);

    sf::RectangleShape startBtn(sf::Vector2f{200.f, 60.f});
    startBtn.setPosition({540.f, 300.f});
    startBtn.setFillColor(sf::Color(46, 204, 113)); 
    window.draw(startBtn);
    sf::Text startTxt("START", font, 30);
    startTxt.setFillColor(sf::Color::White);
    startTxt.setPosition({590.f, 310.f});
    window.draw(startTxt);

    sf::RectangleShape exitBtn(sf::Vector2f{200.f, 60.f});
    exitBtn.setPosition({540.f, 400.f});
    exitBtn.setFillColor(sf::Color(231, 76, 60)); 
    window.draw(exitBtn);
    sf::Text exitTxt("EXIT", font, 30);
    exitTxt.setFillColor(sf::Color::White);
    exitTxt.setPosition({605.f, 410.f});
    window.draw(exitTxt);
}

void Game::DrawLevelSelect() {
    window.draw(menuBgSprite); 
    sf::RectangleShape panel(sf::Vector2f{1000.f, 550.f}); // Made panel wider
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setPosition({140.f, 80.f});
    window.draw(panel);

    sf::Text title("SELECT LEVEL", font, 48);
    title.setFillColor(sf::Color::White);
    title.setPosition({470.f, 100.f});
    window.draw(title);

    // --- FIX: 5x2 Grid to fit all 10 Levels cleanly ---
    for (int i = 0; i < MAX_LEVELS; i++) {
        float x = 210.f + (i % 5) * 175.0f; // 5 columns
        float y = 250.f + (i / 5) * 150.0f; // 2 rows

        sf::RectangleShape box(sf::Vector2f{100.f, 100.f});
        box.setPosition({x, y});
        box.setFillColor(sf::Color(150, 150, 150));
        window.draw(box);

        sf::Text lvlTxt(std::to_string(i + 1), font, 40);
        lvlTxt.setFillColor(sf::Color::White);
        
        // Center single vs double digit numbers perfectly
        float textOffsetX = (i >= 9) ? 25.f : 35.f; 
        lvlTxt.setPosition({x + textOffsetX, y + 25.f});
        window.draw(lvlTxt);

        // Click detection logic is still handled in ProcessEvents!
    }
}

void Game::DrawLevelComplete() {
    sf::RectangleShape overlay(sf::Vector2f{1280.f, 720.f});
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    sf::Text title("LEVEL COMPLETE!", font, 64);
    title.setFillColor(sf::Color(255, 215, 0));
    title.setPosition({380.f, 150.f});
    window.draw(title);

    sf::Text scoreDisplay("FINAL SCORE: " + std::to_string(score), font, 40);
    scoreDisplay.setFillColor(sf::Color::White);
    scoreDisplay.setPosition({480.f, 250.f});
    window.draw(scoreDisplay);

    sf::RectangleShape nextBtn(sf::Vector2f{200.f, 60.f});
    nextBtn.setPosition({540.f, 350.f});
    nextBtn.setFillColor(sf::Color(46, 204, 113));
    window.draw(nextBtn);
    sf::Text nextTxt("NEXT LEVEL", font, 24);
    nextTxt.setFillColor(sf::Color::White);
    nextTxt.setPosition({565.f, 365.f});
    window.draw(nextTxt);

    sf::RectangleShape menuBtn(sf::Vector2f{200.f, 60.f});
    menuBtn.setPosition({540.f, 450.f});
    menuBtn.setFillColor(sf::Color(100, 100, 200));
    window.draw(menuBtn);
    sf::Text menuTxt("MAIN MENU", font, 24);
    menuTxt.setFillColor(sf::Color::White);
    menuTxt.setPosition({570.f, 465.f});
    window.draw(menuTxt);
}

void Game::DrawGameOver() {
    sf::RectangleShape overlay(sf::Vector2f{1280.f, 720.f});
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    sf::Text title("OUT OF BIRDS!", font, 64);
    title.setFillColor(sf::Color(231, 76, 60)); 
    title.setPosition({410.f, 150.f});
    window.draw(title);

    sf::RectangleShape retryBtn(sf::Vector2f{200.f, 60.f});
    retryBtn.setPosition({540.f, 350.f});
    retryBtn.setFillColor(sf::Color(46, 204, 113));
    window.draw(retryBtn);
    sf::Text retryTxt("TRY AGAIN", font, 24);
    retryTxt.setFillColor(sf::Color::White);
    retryTxt.setPosition({575.f, 365.f});
    window.draw(retryTxt);

    sf::RectangleShape menuBtn(sf::Vector2f{200.f, 60.f});
    menuBtn.setPosition({540.f, 450.f});
    menuBtn.setFillColor(sf::Color(100, 100, 200));
    window.draw(menuBtn);
    sf::Text menuTxt("MAIN MENU", font, 24);
    menuTxt.setFillColor(sf::Color::White);
    menuTxt.setPosition({570.f, 465.f});
    window.draw(menuTxt);
}

void Game::Run() {
    while (window.isOpen()) {
        ProcessEvents();
        Update();
        Render();
    }
}