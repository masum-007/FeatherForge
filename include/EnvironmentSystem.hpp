#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstdint>

struct Cloud { sf::Vector2f pos; float scale; float speed; };
struct Bird { sf::Vector2f pos; float speed; float flapSpeed; float phase; };
struct Star { sf::Vector2f pos; float size; float twinklePhase; }; 

struct GrassBlade {
    sf::Vector2f pos;
    float length;
    float baseAngle;
    float currentAngle;
    sf::Color color;
};

struct Leaf {
    sf::Vector2f offset;  
    sf::Color color;      
    float length;         
    float width;          
    float baseAngle;      
    float phaseOffset;    
};

struct Tree { 
    sf::Vector2f pos; 
    float scale; 
    float phase; 
    std::vector<Leaf> leaves;
};

struct Raindrop {
    sf::Vector2f pos;
    float speed;
    float length;
};

struct Theme {
    sf::Color skyTop, skyBot, sunCore, sunGlow;
    sf::Color mountFar, mountNear;
    sf::Color treeTrunk, treeLeaves;
    bool isNight;
    bool isStormy; 
    bool isRaining; 
    int moonPhase; // 0 = Full, 1 = Crescent, 2 = Half
};

class EnvironmentSystem {
public:
    EnvironmentSystem() : m_time(0.0f), m_lightningFlash(0.0f), m_nextLightning(5.0f) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); 
        
        // --- CREATE THE HIDDEN MOON CANVAS ---
        m_moonBuffer.create(300, 300);

        // --- 5 BEAUTIFUL DAY/SUNSET SCENARIOS ---
        m_themes.push_back({sf::Color{120, 180, 240}, sf::Color{210, 235, 255}, sf::Color{255, 230, 0}, sf::Color{255, 240, 150, 150}, sf::Color{140, 175, 205}, sf::Color{100, 140, 120}, sf::Color{80, 55, 40}, sf::Color{45, 140, 75}, false, false, false, 0});
        m_themes.push_back({sf::Color{50, 60, 75}, sf::Color{90, 100, 110}, sf::Color{200, 200, 210}, sf::Color{150, 150, 160, 10}, sf::Color{65, 75, 85}, sf::Color{40, 50, 60}, sf::Color{25, 30, 35}, sf::Color{35, 60, 45}, false, true, true, 0});
        m_themes.push_back({sf::Color{180, 90, 120}, sf::Color{255, 170, 100}, sf::Color{255, 230, 180}, sf::Color{255, 100, 50, 150}, sf::Color{140, 80, 90}, sf::Color{90, 50, 60}, sf::Color{50, 30, 25}, sf::Color{140, 60, 40}, false, false, false, 0});
        m_themes.push_back({sf::Color{90, 80, 140}, sf::Color{255, 190, 160}, sf::Color{255, 240, 220}, sf::Color{255, 180, 120, 100}, sf::Color{110, 100, 140}, sf::Color{70, 75, 105}, sf::Color{45, 35, 40}, sf::Color{50, 100, 85}, false, false, false, 0});
        m_themes.push_back({sf::Color{140, 175, 215}, sf::Color{255, 230, 170}, sf::Color{255, 245, 200}, sf::Color{255, 180, 80, 120}, sf::Color{170, 145, 120}, sf::Color{130, 95, 70}, sf::Color{70, 40, 25}, sf::Color{200, 110, 40}, false, false, false, 0});

        // --- 5 BREATHTAKING NIGHT SCENARIOS ---
        // Night 1: Majestic Clear Full Moon (Deep Navy)
        m_themes.push_back({sf::Color{10, 15, 35}, sf::Color{30, 45, 80}, sf::Color{255, 255, 245}, sf::Color{150, 180, 255, 120}, sf::Color{35, 45, 65}, sf::Color{20, 30, 45}, sf::Color{15, 15, 20}, sf::Color{25, 60, 45}, true, false, false, 0});
        // Night 2: Quiet Tilted Crescent Moon (Deep Space Purple, highly starry)
        m_themes.push_back({sf::Color{8, 5, 15}, sf::Color{25, 15, 45}, sf::Color{255, 250, 240}, sf::Color{180, 160, 255, 100}, sf::Color{25, 20, 40}, sf::Color{15, 10, 25}, sf::Color{10, 5, 15}, sf::Color{15, 30, 25}, true, false, false, 1});
        // Night 3: Eerie Tilted Half Moon (Foggy Teal/Grey)
        m_themes.push_back({sf::Color{15, 25, 30}, sf::Color{40, 60, 70}, sf::Color{220, 240, 230}, sf::Color{120, 180, 200, 100}, sf::Color{35, 50, 60}, sf::Color{25, 35, 45}, sf::Color{15, 20, 25}, sf::Color{20, 45, 35}, true, false, false, 2});
        // Night 4: Midnight Rain (Glowing Full Moon with intense storm)
        m_themes.push_back({sf::Color{15, 20, 45}, sf::Color{40, 55, 90}, sf::Color{240, 245, 255}, sf::Color{150, 170, 255, 100}, sf::Color{45, 55, 75}, sf::Color{25, 40, 55}, sf::Color{15, 15, 25}, sf::Color{30, 70, 55}, true, false, true, 0});
        // Night 5: The Emperor's Storm (Epic purple lightning, sharp Crescent Moon)
        m_themes.push_back({sf::Color{20, 10, 35}, sf::Color{50, 20, 60}, sf::Color{255, 255, 255}, sf::Color{200, 150, 255, 120}, sf::Color{30, 15, 45}, sf::Color{15, 10, 25}, sf::Color{10, 5, 15}, sf::Color{20, 15, 35}, true, true, true, 1});
    }

    void loadLevel(int level) {
        m_theme = m_themes[(level - 1) % m_themes.size()]; 
        m_time = 0.0f;
        m_clouds.clear(); m_trees.clear(); m_birds.clear(); m_stars.clear(); m_raindrops.clear(); m_grass.clear();
        
        for (float x = -1000.f; x <= 6000.f; x += 2.0f) { 
            GrassBlade gb;
            gb.pos = {x, 680.f};
            gb.length = 5.f + (rand() % 6); 
            gb.baseAngle = -1.57f + ((rand() % 20 - 10) * 0.03f); 
            gb.currentAngle = gb.baseAngle;
            
            sf::Color c = m_theme.treeLeaves;
            int var = (rand() % 50) - 25; 
            c.r = static_cast<std::uint8_t>(std::clamp((int)c.r + var, 0, 255));
            c.g = static_cast<std::uint8_t>(std::clamp((int)c.g + var, 0, 255));
            c.b = static_cast<std::uint8_t>(std::clamp((int)c.b + var, 0, 255));
            gb.color = c;
            m_grass.push_back(gb);
        }

        for (int i = 0; i < 20; i++) m_clouds.push_back({sf::Vector2f{-1000.f + (rand() % 6000), 50.f + (rand() % 200)}, 0.5f + (rand() % 10)/10.f, 10.f + (rand() % 20)});
        
        for (int i = 0; i < 30; i++) {
            Tree t; t.scale = 0.6f + (rand() % 4) / 10.f; t.pos = sf::Vector2f{-800.f + (rand() % 5000), 680.f}; t.phase = (rand() % 314) / 100.f;
            int numLeaves = 200 + (rand() % 100);
            for (int j = 0; j < numLeaves; ++j) {
                Leaf l; int branch = rand() % 3; sf::Vector2f branchCenter;
                if (branch == 0) branchCenter = {0.f, -110.f * t.scale};       
                else if (branch == 1) branchCenter = {-35.f * t.scale, -75.f * t.scale}; 
                else branchCenter = {35.f * t.scale, -75.f * t.scale};         
                float sprAngle = (rand() % 360) * 3.14159f / 180.f; float dist = std::sqrt((rand() % 100) / 100.f); float radius = 45.f * t.scale * dist; 
                l.offset = branchCenter + sf::Vector2f{std::cos(sprAngle) * radius, std::sin(sprAngle) * radius};
                l.length = (8.f + (rand() % 5)) * t.scale; l.width = (4.f + (rand() % 3)) * t.scale; l.baseAngle = (rand() % 360) * 3.14159f / 180.f; 
                int colorVar = (rand() % 30) - 15; sf::Color c = m_theme.treeLeaves;
                c.r = static_cast<std::uint8_t>(std::clamp((int)c.r + colorVar, 0, 255)); c.g = static_cast<std::uint8_t>(std::clamp((int)c.g + colorVar, 0, 255)); c.b = static_cast<std::uint8_t>(std::clamp((int)c.b + colorVar, 0, 255));
                if (l.offset.y > branchCenter.y + 10.f) { c.r = static_cast<std::uint8_t>(std::max(0, (int)c.r - 40)); c.g = static_cast<std::uint8_t>(std::max(0, (int)c.g - 40)); c.b = static_cast<std::uint8_t>(std::max(0, (int)c.b - 40)); }
                l.color = c; l.phaseOffset = (rand() % 314) / 100.f; t.leaves.push_back(l);
            }
            m_trees.push_back(t);
        }

        for (int i = 0; i < 15; i++) m_birds.push_back({sf::Vector2f{-500.f + (rand() % 4000), 100.f + (rand() % 300)}, 80.f + (rand() % 40), 10.f + (rand() % 5), (rand() % 314) / 100.f});

        if (m_theme.isNight) {
            for (int i = 0; i < 300; i++) m_stars.push_back({sf::Vector2f{static_cast<float>(rand() % 1280), static_cast<float>(rand() % 450)}, 1.0f + (rand() % 20) / 10.f, static_cast<float>(rand() % 314) / 100.f});
        }

        if (m_theme.isRaining) {
            for (int i = 0; i < 350; i++) m_raindrops.push_back({sf::Vector2f{-1000.f + (rand() % 4000), -500.f + (rand() % 1500)}, 600.f + (rand() % 400), 10.f + (rand() % 15)});
        }

        m_lightningFlash = 0.0f;
        m_nextLightning = 2.0f + (rand() % 50) / 10.f;

        generateMountains(m_mountainsBack, m_theme.mountFar, m_theme.skyBot, 400.f, 250.f, 0.f, true);
        
        sf::Color midTop;
        midTop.r = static_cast<std::uint8_t>((m_theme.mountFar.r + m_theme.mountNear.r) / 2);
        midTop.g = static_cast<std::uint8_t>((m_theme.mountFar.g + m_theme.mountNear.g) / 2);
        midTop.b = static_cast<std::uint8_t>((m_theme.mountFar.b + m_theme.mountNear.b) / 2);
        generateMountains(m_mountainsMid, midTop, m_theme.mountFar, 500.f, 180.f, 5000.f, true);
        
        generateMountains(m_mountainsNear, m_theme.mountNear, m_theme.treeTrunk, 620.f, 100.f, 12000.f, false);
        generateMountains(m_mountainsForeground, sf::Color(10, 10, 15, 255), sf::Color(10, 10, 15, 255), 1000.f, 150.f, 25000.f, false);
    }

    void update(float dt, sf::Vector2f playerPos = {-9999.f, -9999.f}, sf::Vector2f playerVel = {0.f, 0.f}) {
        m_time += dt;
        for (auto& cloud : m_clouds) {
            cloud.pos.x += cloud.speed * dt;
            if (cloud.pos.x > 5000.f) cloud.pos.x = -1000.f;
        }
        for (auto& bird : m_birds) {
            bird.pos.x -= bird.speed * dt; 
            if (bird.pos.x < -1000.f) bird.pos.x = 5000.f;
        }

        float globalWind = std::sin(m_time * 2.0f) * 0.2f;
        if (m_theme.isStormy) globalWind *= 3.0f;

        for (auto& gb : m_grass) {
            float targetAngle = gb.baseAngle + globalWind + std::sin(m_time * 4.0f + gb.pos.x * 0.01f) * 0.08f;
            
            float dist = std::sqrt(std::pow(gb.pos.x - playerPos.x, 2) + std::pow(gb.pos.y - playerPos.y, 2));
            if (dist < 100.f) {
                float force = (100.f - dist) / 100.f; 
                float bendDir = (playerVel.x > 0) ? 1.0f : -1.0f; 
                float speedForce = std::min(std::abs(playerVel.x) * 0.008f, 1.2f); 
                targetAngle += bendDir * force * speedForce;
            }
            
            gb.currentAngle += (targetAngle - gb.currentAngle) * dt * 8.0f;
        }

        if (m_theme.isRaining) {
            for (auto& drop : m_raindrops) {
                drop.pos.y += drop.speed * dt;
                drop.pos.x += (drop.speed * 0.08f) * dt; 
                if (drop.pos.y > 900.f) { 
                    drop.pos.y = -100.f - (rand() % 500);
                    drop.pos.x = -500.f + (rand() % 3000); 
                }
            }
        }

        if (m_theme.isStormy) {
            m_nextLightning -= dt;
            if (m_nextLightning <= 0.0f) {
                m_lightningFlash = 1.0f; 
                m_nextLightning = 3.0f + (rand() % 70) / 10.f; 
                generateLightningBolt();
            }
            if (m_lightningFlash > 0.0f) {
                m_lightningFlash -= dt * 1.5f; 
                if (m_lightningFlash < 0.0f) m_lightningFlash = 0.0f;
            }
        }
    }

    void render(sf::RenderTarget& window, float cameraX) {
        float cx = cameraX - 640.f; 
        
        sf::VertexArray sky(sf::PrimitiveType::TriangleStrip, 4);
        sky[0].position = sf::Vector2f{cx, 0.f};               sky[0].color = m_theme.skyTop;
        sky[1].position = sf::Vector2f{cx, 720.f};             sky[1].color = m_theme.skyBot;
        sky[2].position = sf::Vector2f{cx + 1280.f, 0.f};      sky[2].color = m_theme.skyTop;
        sky[3].position = sf::Vector2f{cx + 1280.f, 720.f};    sky[3].color = m_theme.skyBot;
        window.draw(sky);

        if (m_theme.isNight) {
            sf::VertexArray starArray(sf::PrimitiveType::Triangles, m_stars.size() * 6);
            for (size_t i = 0; i < m_stars.size(); i++) {
                float twinkle = (std::sin(m_time * 4.0f + m_stars[i].twinklePhase) + 1.0f) * 0.5f; 
                sf::Color c = sf::Color{255, 255, 255, static_cast<std::uint8_t>(50 + 200 * twinkle)};
                sf::Vector2f sp = sf::Vector2f{cx + m_stars[i].pos.x, m_stars[i].pos.y};
                float s = m_stars[i].size;
                starArray[i*6+0] = sf::Vertex{sp + sf::Vector2f{-s,-s}, c}; starArray[i*6+1] = sf::Vertex{sp + sf::Vector2f{s,-s}, c}; starArray[i*6+2] = sf::Vertex{sp + sf::Vector2f{s,s}, c};
                starArray[i*6+3] = sf::Vertex{sp + sf::Vector2f{-s,-s}, c}; starArray[i*6+4] = sf::Vertex{sp + sf::Vector2f{s,s}, c}; starArray[i*6+5] = sf::Vertex{sp + sf::Vector2f{-s,s}, c};
            }
            window.draw(starArray);
        }

        if (m_theme.isStormy && m_lightningFlash > 0.0f) {
            if (m_lightningFlash > 0.6f) { 
                sf::Transform t; t.translate({cx, 0.f});
                window.draw(m_lightningBolt, t); t.translate({1.f, 0.f}); window.draw(m_lightningBolt, t); t.translate({-2.f, 0.f}); window.draw(m_lightningBolt, t);
            }
            sf::VertexArray flash(sf::PrimitiveType::TriangleStrip, 4);
            sf::Color flashCol{255, 255, 255, static_cast<std::uint8_t>(m_lightningFlash * 150)};
            flash[0].position = {cx, 0.f}; flash[0].color = flashCol; flash[1].position = {cx, 720.f}; flash[1].color = flashCol;
            flash[2].position = {cx+1280.f, 0.f}; flash[2].color = flashCol; flash[3].position = {cx+1280.f, 720.f}; flash[3].color = flashCol;
            window.draw(flash);
        }

        // --- UPGRADED: MAJESTIC 12-SECOND CELESTIAL ANIMATION ---
        float animProgress = std::clamp(m_time / 12.0f, 0.0f, 1.0f);
        float easeOut = 1.0f - std::pow(1.0f - animProgress, 4.0f);
        float celestialY = 750.f - (600.f * easeOut); 
        float sunX = cx + 900.f - (cameraX * 0.05f); 

        if (!m_theme.isNight) {
            // BEAUTIFUL GRADIENT SUN
            sf::Color baseGlow = m_theme.sunGlow;
            for(int i = 10; i >= 1; --i) {
                float r = 18.f * i;
                sf::CircleShape g(r);
                g.setOrigin({r, r}); g.setPosition({sunX, celestialY});
                sf::Color c = baseGlow; c.a = 255 / (i * i); 
                g.setFillColor(c); window.draw(g);
            }
            
            // Intense Inner Corona
            sf::CircleShape corona(50.f);
            corona.setOrigin({50.f, 50.f}); corona.setPosition({sunX, celestialY});
            sf::Color coronaCol = m_theme.sunCore; coronaCol.a = 200;
            corona.setFillColor(coronaCol); window.draw(corona);
            
            // Blinding White Core
            sf::CircleShape core(30.f);
            core.setOrigin({30.f, 30.f}); core.setPosition({sunX, celestialY});
            core.setFillColor(sf::Color(255, 255, 245)); window.draw(core);

        } else {
            // --- FLAWLESS STENCIL MOON ---
            m_moonBuffer.clear(sf::Color::Transparent);

            // 1. Draw Beautiful Soft Glow to the isolated buffer
            sf::Color baseGlow = m_theme.sunGlow;
            for(int i = 6; i >= 1; --i) {
                float r = 20.f * i;
                sf::CircleShape g(r);
                g.setOrigin({r, r}); g.setPosition({150.f, 150.f});
                sf::Color c = baseGlow; c.a = 150 / (i * i);
                g.setFillColor(c); m_moonBuffer.draw(g);
            }

            // 2. Draw Solid Moon Core
            sf::CircleShape core(30.f);
            core.setOrigin({30.f, 30.f}); core.setPosition({150.f, 150.f});
            core.setFillColor(m_theme.sunCore); m_moonBuffer.draw(core);

            // 3. Draw Craters
            sf::Color craterCol(0, 0, 0, 30); 
            sf::CircleShape c1(6.f); c1.setOrigin({6.f, 6.f}); c1.setPosition({138.f, 138.f}); c1.setFillColor(craterCol); m_moonBuffer.draw(c1);
            sf::CircleShape c2(9.f); c2.setOrigin({9.f, 9.f}); c2.setPosition({165.f, 155.f}); c2.setFillColor(craterCol); m_moonBuffer.draw(c2);
            sf::CircleShape c3(4.f); c3.setOrigin({4.f, 4.f}); c3.setPosition({140.f, 168.f}); c3.setFillColor(craterCol); m_moonBuffer.draw(c3);

            // 4. APPLY THE ERASER MASK (This perfectly deletes the dark side AND its glow!)
            sf::RenderStates eraseState(sf::BlendNone);
            
            if (m_theme.moonPhase == 1) { // Crescent Moon
                // Erase the left half of the canvas (gets rid of the left glow)
                sf::RectangleShape halfMask(sf::Vector2f(150.f, 300.f));
                halfMask.setPosition({0.f, 0.f});
                halfMask.setFillColor(sf::Color::Transparent);
                m_moonBuffer.draw(halfMask, eraseState);

                // Take a precise bite out of the core to form a mathematical crescent
                sf::CircleShape bite(30.f); // Same exact radius as the core!
                bite.setOrigin({30.f, 30.f});
                bite.setPosition({138.f, 150.f}); // Shifted just 12 pixels left
                bite.setFillColor(sf::Color::Transparent);
                m_moonBuffer.draw(bite, eraseState);
            } 
            else if (m_theme.moonPhase == 2) { // Half Moon
                // Perfectly cut exactly down the middle
                sf::RectangleShape mask(sf::Vector2f(150.f, 300.f));
                mask.setPosition({0.f, 0.f}); // X=0 to 150
                mask.setFillColor(sf::Color::Transparent);
                m_moonBuffer.draw(mask, eraseState);
            }
            m_moonBuffer.display();

            // 5. Draw the perfected moon onto the actual game screen
            sf::Sprite moonSprite(m_moonBuffer.getTexture());
            moonSprite.setOrigin({150.f, 150.f});
            moonSprite.setPosition({sunX, celestialY});
            
            // --- BEAUTIFUL TILTING ---
            if (m_theme.moonPhase == 1) moonSprite.setRotation(125.f); // Tilt Crescent
            else if (m_theme.moonPhase == 2) moonSprite.setRotation(130.f); // Tilt Half Moon
            else moonSprite.setRotation(-15.f); // Slight natural tilt for full moon

            window.draw(moonSprite);
        }

        drawParallaxLayer(window, m_mountainsBack, cameraX, 0.1f);
        drawParallaxLayer(window, m_mountainsMid, cameraX, 0.25f);
        drawParallaxLayer(window, m_mountainsNear, cameraX, 0.45f);

        for (auto& cloud : m_clouds) drawCloud(window, cloud, cameraX, 0.15f);
        for (auto& tree : m_trees) drawTree(window, tree, cameraX, 0.8f);
        for (auto& bird : m_birds) drawBird(window, bird, cameraX, 0.6f);

        sf::VertexArray grassArray(sf::PrimitiveType::Lines, m_grass.size() * 2);
        for (size_t i = 0; i < m_grass.size(); ++i) {
            const auto& gb = m_grass[i];
            sf::Vector2f tip = {gb.pos.x + std::cos(gb.currentAngle) * gb.length, gb.pos.y + std::sin(gb.currentAngle) * gb.length};
            
            sf::Color shadow = gb.color; 
            shadow.r /= 2; shadow.g /= 2; shadow.b /= 2; 
            
            grassArray[i*2 + 0] = sf::Vertex{gb.pos, shadow};
            grassArray[i*2 + 1] = sf::Vertex{tip, gb.color};
        }
        window.draw(grassArray);

        if (m_theme.isRaining) {
            sf::VertexArray rainArray(sf::PrimitiveType::Lines, m_raindrops.size() * 2);
            sf::Color rainCol{220, 230, 255, 100}; 
            for (size_t i = 0; i < m_raindrops.size(); i++) {
                sf::Vector2f dropPos = m_raindrops[i].pos + sf::Vector2f{cameraX, 0.f}; 
                rainArray[i*2 + 0] = sf::Vertex{dropPos, rainCol};
                rainArray[i*2 + 1] = sf::Vertex{dropPos + sf::Vector2f{-m_raindrops[i].length * 0.08f, -m_raindrops[i].length}, rainCol};
            }
            window.draw(rainArray);
        }
    }

    void renderForeground(sf::RenderTarget& window, float cameraX) {
        drawParallaxLayer(window, m_mountainsForeground, cameraX, 1.4f);
    }

private:
    float m_time;
    Theme m_theme;
    std::vector<Theme> m_themes;
    std::vector<Cloud> m_clouds;
    std::vector<Tree> m_trees;
    std::vector<Bird> m_birds;
    std::vector<Star> m_stars;
    std::vector<Raindrop> m_raindrops;
    std::vector<GrassBlade> m_grass; 
    sf::VertexArray m_mountainsBack, m_mountainsMid, m_mountainsNear, m_mountainsForeground;
    
    float m_lightningFlash;
    float m_nextLightning;
    sf::VertexArray m_lightningBolt;
    
    // --- HIDDEN CANVAS FOR FLAWLESS STENCIL MOONS ---
    sf::RenderTexture m_moonBuffer;

    void generateLightningBolt() {
        m_lightningBolt.clear(); m_lightningBolt.setPrimitiveType(sf::PrimitiveType::LineStrip);
        float lx = 200.f + (rand() % 880); float ly = 0.f;
        while (ly < 600.f) { 
            m_lightningBolt.append(sf::Vertex{sf::Vector2f{lx, ly}, sf::Color::White});
            lx += (rand() % 100) - 50.f; ly += 30.f + (rand() % 50);  
        }
    }

    void generateMountains(sf::VertexArray& va, sf::Color topColor, sf::Color bottomColor, float baseHeight, float variance, float seedOffset, bool hasSnow) {
        va.setPrimitiveType(sf::PrimitiveType::TriangleStrip); va.resize(0);
        sf::Color snowColor = m_theme.isNight ? sf::Color{130, 130, 150} : sf::Color{255, 255, 255};
        for (float x = -4000.f; x <= 8000.f; x += 15.f) {
            float nx = x + 15000.f + seedOffset; 
            float combinedNoise = (std::sin(nx * 0.0008f) * 1.2f) + (std::sin(nx * 0.0025f + 1.0f) * 0.5f) - (std::abs(std::sin(nx * 0.006f)) * 0.3f) + (std::sin(nx * 0.015f + 2.0f) * 0.1f);
            float height = baseHeight - (combinedNoise * variance);
            sf::Color peakColor = topColor;
            if (hasSnow) {
                float snowLine = baseHeight - (variance * 0.2f); 
                if (height < snowLine) { 
                    float blend = std::clamp((snowLine - height) / (variance * 0.5f), 0.0f, 1.0f);
                    peakColor.r = static_cast<std::uint8_t>(topColor.r + (snowColor.r - topColor.r) * blend);
                    peakColor.g = static_cast<std::uint8_t>(topColor.g + (snowColor.g - topColor.g) * blend);
                    peakColor.b = static_cast<std::uint8_t>(topColor.b + (snowColor.b - topColor.b) * blend);
                }
            }
            va.append(sf::Vertex{sf::Vector2f{x, 800.f}, bottomColor}); va.append(sf::Vertex{sf::Vector2f{x, height}, peakColor});
        }
    }

    void drawParallaxLayer(sf::RenderTarget& win, const sf::VertexArray& va, float camX, float parallax) {
        sf::Transform t; t.translate({camX * (1.0f - parallax), 0.f}); win.draw(va, t);
    }

    void drawCloud(sf::RenderTarget& win, const Cloud& c, float camX, float parallax) {
        float drawX = c.pos.x + camX * (1.0f - parallax);
        if (drawX < camX - 800.f || drawX > camX + 1480.f) return; 
        sf::Color baseCol = m_theme.isNight ? sf::Color{90, 90, 110, 220} : sf::Color{255, 255, 255, 240};
        sf::Color shadowCol = m_theme.isNight ? sf::Color{60, 60, 80, 220} : sf::Color{210, 210, 225, 240};
        if (m_theme.isStormy) { baseCol = sf::Color{100, 110, 120, 240}; shadowCol = sf::Color{70, 80, 90, 240}; }
        auto drawPuff = [&](float offsetX, float offsetY, float radius, sf::Color col) {
            sf::CircleShape puff(radius); puff.setOrigin({radius, radius}); puff.setPosition({drawX + offsetX * c.scale, c.pos.y + offsetY * c.scale});
            puff.setScale({1.3f, 0.8f}); puff.setFillColor(col); win.draw(puff);
        };
        drawPuff(0.f, 15.f, 35.f, shadowCol); drawPuff(50.f, 10.f, 45.f, shadowCol); drawPuff(100.f, 20.f, 30.f, shadowCol);
        drawPuff(-5.f, 0.f, 35.f, baseCol); drawPuff(45.f, -10.f, 45.f, baseCol); drawPuff(95.f, 5.f, 30.f, baseCol); drawPuff(25.f, 15.f, 25.f, baseCol); drawPuff(75.f, 10.f, 25.f, baseCol);
    }

    void drawTree(sf::RenderTarget& win, const Tree& t, float camX, float parallax) {
        float drawX = t.pos.x + camX * (1.0f - parallax);
        if (drawX < camX - 800.f || drawX > camX + 1280.f) return; 
        sf::ConvexShape trunk(4);
        trunk.setPoint(0, sf::Vector2f{-4.f * t.scale, -120.f * t.scale}); trunk.setPoint(1, sf::Vector2f{4.f * t.scale, -120.f * t.scale});  
        trunk.setPoint(2, sf::Vector2f{14.f * t.scale, 0.f}); trunk.setPoint(3, sf::Vector2f{-14.f * t.scale, 0.f});             
        trunk.setFillColor(m_theme.treeTrunk); trunk.setPosition({drawX, t.pos.y}); win.draw(trunk);

        sf::VertexArray leafArray(sf::PrimitiveType::Triangles, t.leaves.size() * 6);
        float globalWind = std::sin(m_time * 2.0f + t.phase);
        if (m_theme.isStormy) globalWind *= 3.0f;

        for (size_t i = 0; i < t.leaves.size(); ++i) {
            const Leaf& l = t.leaves[i];
            float swayAmount = globalWind * (std::abs(l.offset.y) * 0.08f);
            float flutter = std::sin(m_time * 6.0f + l.phaseOffset) * 2.0f;
            if (m_theme.isStormy) flutter *= 2.0f; 
            
            sf::Vector2f center = sf::Vector2f{drawX + l.offset.x + swayAmount + flutter, t.pos.y + l.offset.y};
            float currentAngle = l.baseAngle + (flutter * 0.1f); 
            float cosA = std::cos(currentAngle); float sinA = std::sin(currentAngle);
            auto rotate = [&](float x, float y) { return sf::Vector2f{x * cosA - y * sinA, x * sinA + y * cosA}; };

            sf::Vector2f top = center + rotate(0.f, -l.length); sf::Vector2f bottom = center + rotate(0.f, l.length);
            sf::Vector2f left = center + rotate(-l.width, 0.f); sf::Vector2f right = center + rotate(l.width, 0.f);

            sf::Color darkTip = l.color;
            darkTip.r = static_cast<std::uint8_t>(std::max(0, (int)darkTip.r - 30)); darkTip.g = static_cast<std::uint8_t>(std::max(0, (int)darkTip.g - 30)); darkTip.b = static_cast<std::uint8_t>(std::max(0, (int)darkTip.b - 30));

            leafArray[i*6+0] = sf::Vertex{top, l.color}; leafArray[i*6+1] = sf::Vertex{left, l.color}; leafArray[i*6+2] = sf::Vertex{right, l.color};
            leafArray[i*6+3] = sf::Vertex{bottom, darkTip}; leafArray[i*6+4] = sf::Vertex{right, l.color}; leafArray[i*6+5] = sf::Vertex{left, l.color};
        }
        win.draw(leafArray);
    }

    void drawBird(sf::RenderTarget& win, const Bird& b, float camX, float parallax) {
        float drawX = b.pos.x + camX * (1.0f - parallax);
        if (drawX < camX - 800.f || drawX > camX + 1280.f) return;
        float wingY = std::sin(m_time * b.flapSpeed + b.phase) * 10.0f;
        sf::VertexArray v(sf::PrimitiveType::LineStrip, 3);
        sf::Color bCol = m_theme.isNight ? sf::Color{50, 50, 50} : sf::Color::Black;
        v[0] = sf::Vertex{sf::Vector2f{drawX - 10.f, b.pos.y + wingY}, bCol}; v[1] = sf::Vertex{sf::Vector2f{drawX, b.pos.y}, bCol}; v[2] = sf::Vertex{sf::Vector2f{drawX + 15.f, b.pos.y + wingY}, bCol};
        win.draw(v);
    }
};