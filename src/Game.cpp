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

    m_cameraPos = sf::Vector2f(640.f, 360.f);

    // ---render texture buffer! ---
    m_renderTexture.create(1280, 720);

    worldView.setSize({1280.f, 720.f});
    worldView.setCenter({640.f, 360.f});
    uiView = window.getDefaultView(); 

    LoadAssets();
    
    // Increased height from 40 to 400 so the "dirt" goes deep into the zoom void
    ground = std::make_unique<Entity>(*physics.GetWorld(), 4000, 700, 8000, 40, EntityType::GROUND, &groundTex);
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
    
    groundTex.setRepeated(true);

    // ---Load Audio ---
    m_sbPull.loadFromFile("assets/audio/pull.wav");
    m_sbRelease.loadFromFile("assets/audio/release.wav");
    m_sbWood.loadFromFile("assets/audio/wood_snap.wav");
    m_sbIce.loadFromFile("assets/audio/ice_shatter.wav");
    m_sbExplode.loadFromFile("assets/audio/explosion.wav");
    m_sbThud.loadFromFile("assets/audio/thud.wav");
    m_sbBirdPoof.loadFromFile("assets/audio/poof.wav");
    m_sbEnemyKill.loadFromFile("assets/audio/enemy_kill.wav");
    
    // Create 16 independent speakers for overlapping SFX
    m_soundPool.resize(16);
    
    font.loadFromFile("assets/arial.ttf");
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition({20.f, 50.f});

    if (menuBgTex.loadFromFile("assets/Feather.png")) {
        menuBgSprite.setTexture(menuBgTex);
        menuBgSprite.setScale({1280.0f / menuBgTex.getSize().x, 720.0f / menuBgTex.getSize().y});
    }
    // ---Generate the Rune mathematically! ---
    runeTex = CustomGraphics::GenerateMagicRune();
    runeSprite.setTexture(runeTex);
    runeSprite.setOrigin({100.f, 100.f}); // Center the 200x200 image

    if (!m_postShader.loadFromFile("assets/postprocess.frag", sf::Shader::Fragment)) {
    printf("Warning: Could not load shader!\n");
}
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

    // ---Read Target Scores (with fallbacks if they are missing) ---
    m_targetScore2Star = levelData.value("star2", 1500); 
    m_targetScore3Star = levelData.value("star3", 3000);

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

        // ---Play the disappearing Poof! (Slightly randomized pitch) ---
        PlaySFX(m_sbBirdPoof, 100.f, 0.9f + (rand() % 20) / 100.f);

        bird.reset(); // Safe Box2D cleanup
        birdIsActive = false;
        m_birdActiveTimer = 0.0f; 

        if (birdsRemaining > 0) SpawnBird();
    }
}

void Game::Update() {

    // Freeze time if paused! ---
    if (m_currentState == GameState::Paused) return;

    // ---Hit-Stop Logic (Freezes physics for a split second on heavy impact) ---
    if (m_hitStopTimer > 0.0f) {
        m_hitStopTimer -= (1.0f / 60.0f);
        return; // Skip all physics and updates this frame!
    }

    // ---Camera Shake Timer ---
    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= (1.0f / 60.0f);
    }

    // ---Get bird velocity to create the interactive grass wake ---
    sf::Vector2f pPos{-9999.f, -9999.f};
    sf::Vector2f pVel{0.f, 0.f};
    if (bird && birdIsActive) {
        pPos = {bird->GetBody()->GetPosition().x * SCALE, bird->GetBody()->GetPosition().y * SCALE};
        pVel = {bird->GetBody()->GetLinearVelocity().x * SCALE, bird->GetBody()->GetLinearVelocity().y * SCALE};
    }
    
    // Pass the bird coordinates into the environment!
    m_environment.update(1.0f / 60.0f, pPos, pVel);

    if (m_currentState != GameState::Playing) return;

    physics.Update(1.0f / 60.0f);
    CheckBirdState();

    // ---Cinematic Camera (Smooth Lerp & Dynamic Zoom) ---
    float targetCamX = 640.f;
    float targetZoom = 1.0f;
    
    if (bird && birdIsActive) {
        // 1.Camera position
        targetCamX = std::max(640.0f, bird->GetBody()->GetPosition().x * SCALE);
        
        // 2. Dynamic Zoom: If bird flies fast, pull the camera out slightly to show more level!
        float speed = bird->GetBody()->GetLinearVelocity().Length();
        if (speed > 10.0f) {
            // The faster it goes, the more it zooms out (maximum 15% zoom out)
            targetZoom = 1.0f + std::min((speed - 10.0f) * 0.015f, 0.15f); 
        }
        
        // (Keep your particle trail logic here if you have it)
        sf::Vector2f pxPos{bird->GetBody()->GetPosition().x * SCALE, bird->GetBody()->GetPosition().y * SCALE};
        if (m_currentBirdType == BirdType::Fire) m_particles.emitFireTrail(pxPos);
        else if (m_currentBirdType == BirdType::Freeze) m_particles.emitIceTrail(pxPos);
    }
    
    // 3. The "Lerp" (Linear Interpolation) Math
    // This makes the camera smoothly glide towards the target instead of snapping instantly
    // The '5.0f' is the tracking tightness. The '2.0f' is the zoom speed.
    m_cameraPos.x += (targetCamX - m_cameraPos.x) * 5.0f * (1.0f / 60.0f);
    m_cameraZoom += (targetZoom - m_cameraZoom) * 2.0f * (1.0f / 60.0f);


    worldView.setSize({1280.f * m_cameraZoom, 720.f * m_cameraZoom});
    worldView.setCenter({m_cameraPos.x, 360.f}); // Lock Y for stability

    if (bird && birdIsActive && !m_birdAbilityUsed) {
        b2Vec2 bPos = bird->GetBody()->GetPosition();
        bool impacted = false;
        for (auto& block : blocks) {
            if ((bPos - block->GetBody()->GetPosition()).Length() < 1.5f) { impacted = true; break; }
        }

        if (impacted) {
            m_birdAbilityUsed = true;
            sf::Vector2f pxPos{bPos.x * SCALE, bPos.y * SCALE};

            // --- NEW: Play a satisfying 'Thud' on every impact! ---
            PlaySFX(m_sbThud, 90.f, 0.9f + (rand() % 20) / 100.f);
            
            if (m_currentBirdType == BirdType::Fire) {
                m_particles.emitWood(pxPos); 
                PlaySFX(m_sbExplode, 100.f, 0.8f); // BOOM! Deep pitch for heavy explosion
                
                // --- REFINED: Small, soft, realistic soot stain ---
                sf::CircleShape scorch(12.f); // Much smaller radius
                scorch.setOrigin({12.f, 12.f});
                scorch.setFillColor(sf::Color(25, 20, 20, 160)); // Soft, semi-transparent ash
                
                if (pxPos.y > 640.f) { 
                    // Flatten it into an oval on the ground
                    scorch.setScale({3.0f, 0.35f});
                    scorch.setPosition({pxPos.x, 695.f}); 
                } else {
                    scorch.setPosition(pxPos); // Mid-air explosion
                }
                m_scorchMarks.push_back(scorch);

                for (auto& block : blocks) {
                    if ((bPos - block->GetBody()->GetPosition()).Length() < 3.5f) {
                        m_burningBodies[block->GetBody()] = 0.0f; 
                        block->getSprite().setColor(sf::Color(255, 100, 100));
                    }
                }
            } else if (m_currentBirdType == BirdType::Freeze) {
                m_particles.emitIce(pxPos); 
                PlaySFX(m_sbIce, 100.f, 1.2f); // Sharp, high-pitch shatter for the freeze spell
                for (auto& block : blocks) {
                    if ((bPos - block->GetBody()->GetPosition()).Length() < 4.0f) {
                        m_frozenBodies.insert(block->GetBody()); 
                        block->getSprite().setColor(sf::Color(150, 255, 255));
                    }
                }
            } else if (m_currentBirdType == BirdType::Water) {
                // Intense Water Tidal Wave ---
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

    // effects!
    blocks.erase(
        std::remove_if(blocks.begin(), blocks.end(),
            [this](const std::unique_ptr<Entity>& e) { 
                // 1. Grab the X position
                float xPos = e->GetBody()->GetPosition().x * SCALE;
                float yPos = e->GetBody()->GetPosition().y * SCALE;
                
                // 2. Add the X bounds to the kill condition
                if (e->IsDestroyed() || yPos > 800.f || xPos < -200.f || xPos > 1500.f) {
                    
                    sf::Vector2f deathPos{xPos, std::min(yPos, 700.f)};
                    
                    // Randomize pitch slightly so multiple blocks breaking sounds organic!
                    float randomPitch = 0.85f + (rand() % 30) / 100.f; 

                    // --- CONTEXT-AWARE SOUNDS & SCORCH MARKS ---
                    if (m_burningBodies.count(e->GetBody())) {
                        PlaySFX(m_sbExplode, 80.f, 1.3f + (rand() % 20) / 100.f); // Pop/Explosion for fire
                        
                        sf::CircleShape scorch(10.f);
                        scorch.setOrigin({10.f, 10.f});
                        scorch.setFillColor(sf::Color(20, 15, 15, 140)); 
                        scorch.setScale({2.5f, 0.3f});
                        scorch.setPosition({deathPos.x, 695.f});
                        m_scorchMarks.push_back(scorch);
                    }
                    else if (e->GetType() == EntityType::ICE) {
                        PlaySFX(m_sbIce, 100.f, randomPitch); // Ice Shatter Sound
                        TriggerHitStop(0.04f); // Tiny freeze for glass shattering
                    }
                    else if (e->GetType() == EntityType::WOOD) {
                        PlaySFX(m_sbWood, 100.f, randomPitch); // Wood Crunch Sound
                    }

                    // --- VISUAL PARTICLES ---
                    if (e->GetType() == EntityType::WOOD) m_particles.emitWood(deathPos);
                    else if (e->GetType() == EntityType::ICE) m_particles.emitIce(deathPos);
                    else if (e->GetType() == EntityType::BIRD) m_particles.emitFeathers(deathPos);
                    else m_particles.emitDust(deathPos); 

                    // --- SCORING & ENEMY EFFECTS ---
                    if (e->GetType() == EntityType::ENEMY) {
                        // --- Enemy Defeat Sound! ---
                        PlaySFX(m_sbEnemyKill, 100.f, randomPitch);
                        score += 500;
                        TriggerHitStop(0.1f); // Massive pause for killing an enemy
                        TriggerShake(0.25f, 15.0f); // Screen shake!
                    }
                    else {
                        score += 100;
                    }

                    // --- CLEANUP ---
                    m_burningBodies.erase(e->GetBody());
                    m_frozenBodies.erase(e->GetBody());
                    
                    return true; // ALL LOGIC DONE -> Now delete the entity!
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
// ---CLEAR SCORCH MARKS ---
    m_scorchMarks.clear();

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
    UpdateThemeMusic(); // Start the correct background track
}

void Game::DrawSlingshot() {
    slingshotPos = sf::Vector2f(200.f, 620.f); 

    // --- Draw the Magical Rune behind everything ---
    runeSprite.setPosition(slingshotPos);
    runeRotation += 0.5f; 
    runeSprite.setRotation(runeRotation);
    
    if (isDragging) {
        runeSprite.setColor(sf::Color(255, 255, 255, 255));
        float pulse = 1.2f + (std::sin(runeRotation * 0.1f) * 0.05f); 
        runeSprite.setScale(pulse, pulse);
    } else {
        runeSprite.setColor(sf::Color(255, 255, 255, 80));
        runeSprite.setScale(1.0f, 1.0f);
    }
    m_renderTexture.draw(runeSprite); 

    // 1. Dark Main Stem 
    sf::RectangleShape stem(sf::Vector2f(16.f, 60.f));
    stem.setFillColor(sf::Color(90, 50, 20)); 
    stem.setOrigin(8.f, 0.f);
    stem.setPosition(slingshotPos.x, slingshotPos.y + 10.f);
    m_renderTexture.draw(stem);

    // 2. Back Fork
    sf::RectangleShape backFork(sf::Vector2f(12.f, 40.f));
    backFork.setFillColor(sf::Color(70, 40, 15)); 
    backFork.setOrigin(6.f, 40.f);
    backFork.setPosition(slingshotPos.x + 8.f, slingshotPos.y + 15.f);
    backFork.setRotation(25.f);
    m_renderTexture.draw(backFork);

    if (bird && !birdIsActive) {
        b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
        sf::Vertex backBand[] = {
            sf::Vertex(sf::Vector2f(slingshotPos.x + 18.f, slingshotPos.y - 15.f), sf::Color(40, 40, 40)),
            sf::Vertex(sf::Vector2f(b2Pos.x * SCALE, b2Pos.y * SCALE), sf::Color(40, 40, 40))
        };
        m_renderTexture.draw(backBand, 2, sf::Lines);
    }

    // 3. Front Fork 
    sf::RectangleShape frontFork(sf::Vector2f(12.f, 40.f));
    frontFork.setFillColor(sf::Color(110, 70, 30)); 
    frontFork.setOrigin(6.f, 40.f);
    frontFork.setPosition(slingshotPos.x - 8.f, slingshotPos.y + 15.f);
    frontFork.setRotation(-25.f);
    m_renderTexture.draw(frontFork);

    if (bird && !birdIsActive) {
        b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
        sf::Vertex frontBand[] = {
            sf::Vertex(sf::Vector2f(slingshotPos.x - 18.f, slingshotPos.y - 15.f), sf::Color(60, 60, 60)),
            sf::Vertex(sf::Vector2f(b2Pos.x * SCALE, b2Pos.y * SCALE), sf::Color(60, 60, 60))
        };
        m_renderTexture.draw(frontBand, 2, sf::Lines);
    }
}

void Game::DrawTrajectory() {
    if (!bird) return;

    b2Vec2 startPos = bird->GetBody()->GetPosition();
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
        dot.setOrigin(1.5f, 1.5f); 
        dot.setPosition(stepPos.x * SCALE, stepPos.y * SCALE); 
        m_renderTexture.draw(dot); 
    }
}

void Game::ProcessEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
            
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePosUI = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

            if (m_currentState == GameState::Menu) {
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 300 && mousePosUI.y <= 360) m_currentState = GameState::LevelSelect;
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 390 && mousePosUI.y <= 450) {
                    m_previousState = GameState::Menu; // Remember we came from Menu!
                    m_currentState = GameState::Options;
                }
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 480 && mousePosUI.y <= 540) window.close();
            } 
            else if (m_currentState == GameState::Options) {
                // MUSIC MINUS (-)
                if (mousePosUI.x >= 430 && mousePosUI.x <= 490 && mousePosUI.y >= 280 && mousePosUI.y <= 330) {
                    m_musicVolume = std::max(0.f, m_musicVolume - 10.f);
                    m_bgMusic.setVolume(m_musicVolume);
                    if (m_musicVolume > 0 && m_bgMusic.getStatus() != sf::Music::Playing) UpdateThemeMusic();
                }
                // MUSIC PLUS (+)
                if (mousePosUI.x >= 790 && mousePosUI.x <= 850 && mousePosUI.y >= 280 && mousePosUI.y <= 330) {
                    m_musicVolume = std::min(100.f, m_musicVolume + 10.f);
                    m_bgMusic.setVolume(m_musicVolume);
                    if (m_bgMusic.getStatus() != sf::Music::Playing) UpdateThemeMusic();
                }
                
                // SFX MINUS (-)
                if (mousePosUI.x >= 430 && mousePosUI.x <= 490 && mousePosUI.y >= 360 && mousePosUI.y <= 410) {
                    m_sfxVolume = std::max(0.f, m_sfxVolume - 10.f);
                    PlaySFX(m_sbThud, 100.f, 1.0f); // Play test sound!
                }
                // SFX PLUS (+)
                if (mousePosUI.x >= 790 && mousePosUI.x <= 850 && mousePosUI.y >= 360 && mousePosUI.y <= 410) {
                    m_sfxVolume = std::min(100.f, m_sfxVolume + 10.f);
                    PlaySFX(m_sbThud, 100.f, 1.0f); // Play test sound!
                }

                // Back Button
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 480 && mousePosUI.y <= 540) m_currentState = m_previousState;
            }
            else if (m_currentState == GameState::LevelSelect) {
                // Back Button
                if (mousePosUI.x >= 50 && mousePosUI.x <= 200 && mousePosUI.y >= 50 && mousePosUI.y <= 100) m_currentState = GameState::Menu;
                
                for (int i = 0; i < MAX_LEVELS; i++) {
                    float x = 210.f + (i % 5) * 175.0f; 
                    float y = 250.f + (i / 5) * 150.0f; 
                    if (mousePosUI.x >= x && mousePosUI.x <= x + 100 && mousePosUI.y >= y && mousePosUI.y <= y + 100) {
                        ResetLevel(i + 1); 
                    }
                }
            }
            // ---PAUSED STATE CLICKS ---
            else if (m_currentState == GameState::Paused) {
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 250 && mousePosUI.y <= 310) m_currentState = GameState::Playing; // RESUME
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 340 && mousePosUI.y <= 400) ResetLevel(m_currentLevel); // RETRY
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 430 && mousePosUI.y <= 490) {
                    m_previousState = GameState::Paused; // Remember the previous state(pause)
                    m_currentState = GameState::Options; // OPTIONS
                }
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 520 && mousePosUI.y <= 580) m_currentState = GameState::Menu; // MAIN MENU
            }
            else if (m_currentState == GameState::LevelComplete) {
                // NEXT LEVEL Button 
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 410 && mousePosUI.y <= 470) {
                    if (m_currentLevel < MAX_LEVELS) ResetLevel(m_currentLevel + 1);
                    else m_currentState = GameState::Menu; 
                }
                // MAIN MENU Button 
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 500 && mousePosUI.y <= 560) {
                    m_currentState = GameState::Menu;
                }
            }
            else if (m_currentState == GameState::GameOver) {
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 360 && mousePosUI.y <= 420) ResetLevel(m_currentLevel);
                if (mousePosUI.x >= 540 && mousePosUI.x <= 740 && mousePosUI.y >= 450 && mousePosUI.y <= 510) m_currentState = GameState::Menu;
            }
            else if (m_currentState == GameState::Playing) {
                // ---Check Pause Button Click (Top Right Corner) ---
                if (mousePosUI.x >= 1200 && mousePosUI.x <= 1260 && mousePosUI.y >= 10 && mousePosUI.y <= 70) {
                    m_currentState = GameState::Paused;
                }
                else if (!birdIsActive && bird) {
                    sf::Vector2f mouseWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window), worldView);
                    b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 
                    sf::Vector2f birdPos{b2Pos.x * SCALE, b2Pos.y * SCALE};

                    if (std::abs(mouseWorld.x - birdPos.x) < 50.f && std::abs(mouseWorld.y - birdPos.y) < 50.f) {
                        isDragging = true;
                        PlaySFX(m_sbPull, 80.f, 1.0f); // PLAY PULL SOUND
                    }
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (m_currentState == GameState::Playing && isDragging && bird) {
                isDragging = false;
                birdIsActive = true;
                birdsRemaining--;

                PlaySFX(m_sbRelease, 100.f, 1.0f + (rand() % 20 - 10) * 0.01f); // PLAY RELEASE SOUND (slight random pitch)

                bird->GetBody()->SetType(b2_dynamicBody);
                bird->GetBody()->SetAwake(true);
                
                float speedMult = 0.6f;
                if (m_currentBirdType == BirdType::Fire) speedMult = 0.85f; 
                else if (m_currentBirdType == BirdType::Sloth) speedMult = 0.35f; 
                else if (m_currentBirdType == BirdType::Freeze) speedMult = 0.45f; 

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
    // 1. Clear our off-screen buffer
    m_renderTexture.clear(sf::Color(135, 206, 235)); 

    if (m_currentState == GameState::Playing || m_currentState == GameState::LevelComplete || m_currentState == GameState::GameOver) {
        float currentCamX = worldView.getCenter().x;
        float currentCamY = worldView.getCenter().y;
        
        // Camera Shake logic
        if (m_shakeTimer > 0.0f) {
            float offsetX = (rand() % 100 - 50) * 0.01f * m_shakeMagnitude;
            float offsetY = (rand() % 100 - 50) * 0.01f * m_shakeMagnitude;
            worldView.setCenter(currentCamX + offsetX, currentCamY + offsetY);
        }

        m_renderTexture.setView(worldView);
        
        // Draw World to the Render Texture
        // ---Pass worldView.getSize() to make background responsive ---
        m_environment.render(m_renderTexture, currentCamX, worldView.getSize());
        DrawSlingshot();
        ground->Render(m_renderTexture);
        // --- NEW: Draw permanent scorch marks on top of ground, behind crates ---
        for (auto& scorch : m_scorchMarks) m_renderTexture.draw(scorch);

        for (auto& block : blocks) block->Render(m_renderTexture);
        if (isDragging) DrawTrajectory();
        if (bird) bird->Render(m_renderTexture);
        m_renderTexture.draw(m_particles);
        m_environment.renderForeground(m_renderTexture, currentCamX);

        // Reset camera so it doesn't drift permanently
        worldView.setCenter(currentCamX, currentCamY);
    }

    m_renderTexture.display();

    // 2. Draw the rendered texture to the ACTUAL window using our GLSL Shader
    window.clear();
    
    m_renderSprite.setTexture(m_renderTexture.getTexture());
    
    m_renderSprite.setTextureRect(sf::IntRect(0, 0, 1280, 720)); 

    // Only use shader if it loaded properly to prevent black screen
    if (m_postShader.getNativeHandle() != 0) {
        m_postShader.setUniform("time", m_shakeTimer); 
        window.draw(m_renderSprite, &m_postShader);
    } else {
        window.draw(m_renderSprite);
    }

    // 3. Draw Beautiful In-Game HUD
    window.setView(uiView);
    if (m_currentState == GameState::Playing || m_currentState == GameState::Paused || m_currentState == GameState::LevelComplete || m_currentState == GameState::GameOver) {
        
        sf::RectangleShape topBar(sf::Vector2f(1280.f, 60.f));
        topBar.setFillColor(sf::Color(10, 15, 25, 120));
        window.draw(topBar);

        for (size_t i = 0; i < m_birdQueue.size(); i++) {
            sf::Sprite uiBird;
            if (m_birdQueue[i] == BirdType::Normal) uiBird.setTexture(birdTex);
            else if (m_birdQueue[i] == BirdType::Fire) uiBird.setTexture(birdFireTex);
            else if (m_birdQueue[i] == BirdType::Sloth) uiBird.setTexture(birdSlothTex);
            else if (m_birdQueue[i] == BirdType::Freeze) uiBird.setTexture(birdFreezeTex);
            else if (m_birdQueue[i] == BirdType::Water) uiBird.setTexture(birdWaterTex);
            
            if (uiBird.getTexture()) {
                float sX = 24.0f / uiBird.getTexture()->getSize().x;
                float sY = 24.0f / uiBird.getTexture()->getSize().y;
                uiBird.setScale(sX, sY);
            }
            uiBird.setPosition(20.f + (i * 35.f), 18.f);
            window.draw(uiBird);
        }

        std::string scoreStr = "SCORE: " + std::to_string(score);
        sf::Text scoreShadow(scoreStr, font, 28);
        scoreShadow.setFillColor(sf::Color(0, 0, 0, 200));
        scoreShadow.setPosition({1002.f, 17.f}); 
        window.draw(scoreShadow);

        scoreText.setString(scoreStr);
        scoreText.setCharacterSize(28);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition({1000.f, 15.f});
        window.draw(scoreText);

        // ---Draw Procedural Pause Button in Top Right (Only when playing or paused) ---
        if (m_currentState == GameState::Playing || m_currentState == GameState::Paused) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);
            bool hoverPause = (mousePos.x >= 1200 && mousePos.x <= 1260 && mousePos.y >= 10 && mousePos.y <= 70);
            
            sf::RectangleShape pBox(sf::Vector2f(60.f, 60.f));
            pBox.setPosition({1200.f, 10.f});
            pBox.setFillColor(hoverPause ? sf::Color(60, 80, 120, 220) : sf::Color(20, 30, 50, 180));
            pBox.setOutlineThickness(2.f);
            pBox.setOutlineColor(hoverPause ? sf::Color(200, 220, 255, 255) : sf::Color(100, 120, 150, 100));
            window.draw(pBox);

            // The two vertical pause lines
            sf::RectangleShape bar1(sf::Vector2f(8.f, 26.f));
            bar1.setPosition({1218.f, 27.f});
            bar1.setFillColor(hoverPause ? sf::Color::White : sf::Color(200, 200, 200));
            window.draw(bar1);

            sf::RectangleShape bar2(sf::Vector2f(8.f, 26.f));
            bar2.setPosition({1234.f, 27.f});
            bar2.setFillColor(hoverPause ? sf::Color::White : sf::Color(200, 200, 200));
            window.draw(bar2);
        }
    }

    if (m_currentState == GameState::Menu) DrawMenu();
    else if (m_currentState == GameState::Options) DrawOptions();
    else if (m_currentState == GameState::LevelSelect) DrawLevelSelect();
    else if (m_currentState == GameState::Paused) DrawPauseMenu(); // <-- NEW
    else if (m_currentState == GameState::LevelComplete) DrawLevelComplete();
    else if (m_currentState == GameState::GameOver) DrawGameOver();

    window.display();
}
//RENDERING FUNCTIONS ---

// Helper lambda for gorgeous interactive hover buttons
auto drawButton = [](sf::RenderWindow& win, sf::Font& font, const std::string& text, float x, float y, float w, float h) {
    sf::Vector2f mousePos = win.mapPixelToCoords(sf::Mouse::getPosition(win), win.getDefaultView());
    bool isHovered = (mousePos.x >= x && mousePos.x <= x + w && mousePos.y >= y && mousePos.y <= y + h);

    sf::RectangleShape btn(sf::Vector2f(w, h));
    btn.setPosition({x, y});
    // Glassmorphism look: Semi-transparent fill, bright border on hover
    btn.setFillColor(isHovered ? sf::Color(60, 80, 120, 220) : sf::Color(20, 30, 50, 180));
    btn.setOutlineThickness(2.f);
    btn.setOutlineColor(isHovered ? sf::Color(200, 220, 255, 255) : sf::Color(100, 120, 150, 100));
    win.draw(btn);

    sf::Text txt(text, font, 24);
    sf::FloatRect bounds = txt.getLocalBounds();
    txt.setOrigin({bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f});
    txt.setPosition({x + w / 2.0f, y + h / 2.0f});
    txt.setFillColor(isHovered ? sf::Color::White : sf::Color(200, 200, 200));
    win.draw(txt);
};

void Game::DrawMenu() {
    window.draw(menuBgSprite);
    
    sf::Text titleShadow("FEATHER FORGE", font, 64);
    titleShadow.setFillColor(sf::Color(0, 0, 0, 150)); 
    titleShadow.setPosition({374.f, 124.f});
    window.draw(titleShadow);

    sf::Text title("FEATHER FORGE", font, 64);
    title.setFillColor(sf::Color(255, 230, 100)); 
    title.setPosition({370.f, 120.f});
    window.draw(title);

    drawButton(window, font, "START", 540.f, 300.f, 200.f, 60.f);
    drawButton(window, font, "OPTIONS", 540.f, 390.f, 200.f, 60.f);
    drawButton(window, font, "EXIT", 540.f, 480.f, 200.f, 60.f);
}

void Game::DrawOptions() {
    window.draw(menuBgSprite);
    
    sf::RectangleShape panel(sf::Vector2f{500.f, 400.f});
    panel.setFillColor(sf::Color(10, 15, 25, 220));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(100, 120, 150, 100));
    panel.setPosition({390.f, 180.f});
    window.draw(panel);

    sf::Text title("OPTIONS", font, 48);
    title.setFillColor(sf::Color::White);
    sf::FloatRect b = title.getLocalBounds();
    title.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
    title.setPosition({640.f, 220.f});
    window.draw(title);

    // --- MUSIC VOLUME ROW ---
    drawButton(window, font, "-", 430.f, 280.f, 60.f, 50.f);
    sf::Text musTxt("MUSIC: " + std::to_string((int)m_musicVolume) + "%", font, 28);
    b = musTxt.getLocalBounds();
    musTxt.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
    musTxt.setPosition({640.f, 305.f});
    musTxt.setFillColor(sf::Color::White);
    window.draw(musTxt);
    drawButton(window, font, "+", 790.f, 280.f, 60.f, 50.f);

    // --- SFX VOLUME ROW ---
    drawButton(window, font, "-", 430.f, 360.f, 60.f, 50.f);
    sf::Text sfxTxt("SFX: " + std::to_string((int)m_sfxVolume) + "%", font, 28);
    b = sfxTxt.getLocalBounds();
    sfxTxt.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
    sfxTxt.setPosition({640.f, 385.f});
    sfxTxt.setFillColor(sf::Color::White);
    window.draw(sfxTxt);
    drawButton(window, font, "+", 790.f, 360.f, 60.f, 50.f);

    // Back Button
    drawButton(window, font, "BACK", 540.f, 480.f, 200.f, 60.f);
}

void Game::DrawLevelSelect() {
    window.draw(menuBgSprite); 
    
    sf::RectangleShape panel(sf::Vector2f{1040.f, 500.f}); 
    panel.setFillColor(sf::Color(10, 15, 25, 200));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(100, 120, 150, 80));
    panel.setPosition({120.f, 150.f});
    window.draw(panel);

    sf::Text title("SELECT LEVEL", font, 48);
    title.setFillColor(sf::Color::Black);
    title.setPosition({460.f, 70.f});
    window.draw(title);

    drawButton(window, font, "BACK", 50.f, 50.f, 150.f, 50.f);

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);
    for (int i = 0; i < MAX_LEVELS; i++) {
        float x = 210.f + (i % 5) * 175.0f; 
        float y = 250.f + (i / 5) * 150.0f; 
        bool isHovered = (mousePos.x >= x && mousePos.x <= x + 100 && mousePos.y >= y && mousePos.y <= y + 100);

        sf::RectangleShape box(sf::Vector2f{100.f, 100.f});
        box.setPosition({x, y});
        box.setFillColor(isHovered ? sf::Color(60, 80, 120, 220) : sf::Color(30, 40, 60, 180));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(isHovered ? sf::Color(200, 220, 255, 255) : sf::Color(100, 120, 150, 100));
        window.draw(box);

        sf::Text lvlTxt(std::to_string(i + 1), font, 36);
        sf::FloatRect bounds = lvlTxt.getLocalBounds();
        lvlTxt.setOrigin({bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f});
        lvlTxt.setPosition({x + 50.f, y + 50.f});
        lvlTxt.setFillColor(isHovered ? sf::Color::White : sf::Color(200, 200, 200));
        window.draw(lvlTxt);
    }
}

void Game::DrawLevelComplete() {
    sf::RectangleShape overlay(sf::Vector2f{1280.f, 720.f});
    overlay.setFillColor(sf::Color(0, 5, 10, 200)); 
    window.draw(overlay);

    sf::Text title("LEVEL COMPLETE", font, 64);
    title.setFillColor(sf::Color(255, 230, 100));
    sf::FloatRect b = title.getLocalBounds();
    title.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
    title.setPosition({640.f, 130.f});
    window.draw(title);

    // ---Calculate Stars Based on JSON Targets ---
    int starsEarned = 1; 
    if (score >= m_targetScore2Star) starsEarned = 2; 
    if (score >= m_targetScore3Star) starsEarned = 3;

    // ---Procedural Star Drawing Lambda ---
    auto drawStar = [](sf::RenderWindow& win, float x, float y, float radius, bool earned) {
        sf::ConvexShape star(10);
        for (int i = 0; i < 10; ++i) {
            // Math to calculate the 5 points and 5 inner dips of a perfect star
            float angle = i * 36.0f * 3.14159f / 180.0f - (3.14159f / 2.0f);
            float r = (i % 2 == 0) ? radius : radius / 2.5f;
            star.setPoint(i, sf::Vector2f(std::cos(angle) * r, std::sin(angle) * r));
        }
        star.setPosition({x, y});
        
        // Glowing Gold if earned, Dark Glass if not earned
        star.setFillColor(earned ? sf::Color(255, 215, 0) : sf::Color(30, 40, 60, 180));
        star.setOutlineThickness(3.f);
        star.setOutlineColor(earned ? sf::Color(255, 255, 200) : sf::Color(100, 120, 150, 100));
        win.draw(star);
    };

    // Draw the 3 Stars (Middle one is slightly higher and larger)
    drawStar(window, 540.f, 250.f, 40.f, starsEarned >= 1);
    drawStar(window, 640.f, 230.f, 50.f, starsEarned >= 2);
    drawStar(window, 740.f, 250.f, 40.f, starsEarned >= 3);

    // Score Display
    sf::Text scoreDisplay("FINAL SCORE: " + std::to_string(score), font, 36);
    scoreDisplay.setFillColor(sf::Color::White);
    b = scoreDisplay.getLocalBounds();
    scoreDisplay.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
    scoreDisplay.setPosition({640.f, 330.f});
    window.draw(scoreDisplay);

    // Buttons
    std::string nextText = (m_currentLevel < MAX_LEVELS) ? "NEXT LEVEL" : "FINISH";
    drawButton(window, font, nextText, 540.f, 410.f, 200.f, 60.f);
    drawButton(window, font, "MAIN MENU", 540.f, 500.f, 200.f, 60.f);
}

void Game::DrawGameOver() {
    sf::RectangleShape overlay(sf::Vector2f{1280.f, 720.f});
    overlay.setFillColor(sf::Color(20, 0, 0, 200)); 
    window.draw(overlay);

    sf::Text title("OUT OF BIRDS", font, 64);
    title.setFillColor(sf::Color(255, 100, 100)); 
    sf::FloatRect b = title.getLocalBounds();
    title.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
    title.setPosition({640.f, 200.f});
    window.draw(title);

    drawButton(window, font, "TRY AGAIN", 540.f, 360.f, 200.f, 60.f);
    drawButton(window, font, "MAIN MENU", 540.f, 450.f, 200.f, 60.f);
}

void Game::PlaySFX(const sf::SoundBuffer& buffer, float volume, float pitch) {
    if (m_sfxVolume <= 0.f) return;
    
    // Convert 0-100 percentage to a 0.0 - 1.0 multiplier
    float masterSfxMultiplier = m_sfxVolume / 100.f; 
    
    for (auto& sound : m_soundPool) {
        if (sound.getStatus() != sf::Sound::Playing) {
            sound.setBuffer(buffer);
            // Multiply the requested volume by your master slider
            sound.setVolume(volume * masterSfxMultiplier); 
            sound.setPitch(pitch);
            sound.play();
            return;
        }
    }
}

void Game::UpdateThemeMusic() {
    if (m_musicVolume <= 0.f) {
        m_bgMusic.pause();
        return;
    }
    
    if (m_bgMusic.getStatus() != sf::Music::Playing) {
        std::string musicFile = "assets/audio/bgm_day.ogg"; 
        
        if (m_currentLevel >= 6) musicFile = "assets/audio/bgm_night.ogg"; 
        
        if (m_bgMusic.openFromFile(musicFile)) {
            m_bgMusic.setLoop(true);
            m_bgMusic.setVolume(m_musicVolume); 
            m_bgMusic.play();
        }
    } else {
        // Just update the volume if it's already playing
        m_bgMusic.setVolume(m_musicVolume);
    }
}

void Game::DrawPauseMenu() {
    sf::RectangleShape overlay(sf::Vector2f{1280.f, 720.f});
    overlay.setFillColor(sf::Color(0, 5, 10, 150)); 
    window.draw(overlay);

    sf::RectangleShape panel(sf::Vector2f{400.f, 480.f});
    panel.setFillColor(sf::Color(10, 15, 25, 220));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(100, 120, 150, 100));
    panel.setPosition({440.f, 120.f});
    window.draw(panel);

    sf::Text title("PAUSED", font, 48);
    title.setFillColor(sf::Color::White);
    sf::FloatRect b = title.getLocalBounds();
    title.setOrigin({b.left + b.width/2.f, b.top + b.height/2.f});
    title.setPosition({640.f, 170.f});
    window.draw(title);

    drawButton(window, font, "RESUME", 540.f, 250.f, 200.f, 60.f);
    drawButton(window, font, "RETRY", 540.f, 340.f, 200.f, 60.f);
    drawButton(window, font, "OPTIONS", 540.f, 430.f, 200.f, 60.f);
    drawButton(window, font, "MAIN MENU", 540.f, 520.f, 200.f, 60.f);
}

void Game::Run() {
    while (window.isOpen()) {
        ProcessEvents();
        Update();
        Render();
    }
}

void Game::TriggerShake(float duration, float magnitude) {
    m_shakeTimer = duration;
    m_shakeMagnitude = magnitude;
}

void Game::TriggerHitStop(float duration) {
    m_hitStopTimer = duration;
}