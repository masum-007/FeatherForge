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
};

class EnvironmentSystem {
public:
    EnvironmentSystem() : m_time(0.0f), m_lightningFlash(0.0f), m_nextLightning(5.0f) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); 

        // --- 10 HAND-CRAFTED CINEMATIC LEVEL THEMES ---

        // Level 1: Classic Day (Calm, bright blue skies)
        m_themes.push_back({sf::Color{120, 180, 240}, sf::Color{210, 235, 255}, sf::Color{255, 245, 180}, sf::Color{255, 230, 150, 100}, sf::Color{140, 175, 205}, sf::Color{100, 140, 120}, sf::Color{80, 55, 40}, sf::Color{45, 140, 75}, false, false, false});
        
        // Level 2: Heavy Thunderstorm (Requested: Rain, lightning, moody dark greys)
        m_themes.push_back({sf::Color{50, 60, 75}, sf::Color{90, 100, 110}, sf::Color{200, 200, 210}, sf::Color{150, 150, 160, 10}, sf::Color{65, 75, 85}, sf::Color{40, 50, 60}, sf::Color{25, 30, 35}, sf::Color{35, 60, 45}, false, true, true});
        
        // Level 3: Sunset (Deep oranges, purples, silhouettes)
        m_themes.push_back({sf::Color{180, 90, 120}, sf::Color{255, 170, 100}, sf::Color{255, 130, 80}, sf::Color{255, 100, 50, 120}, sf::Color{140, 80, 90}, sf::Color{90, 50, 60}, sf::Color{50, 30, 25}, sf::Color{140, 60, 40}, false, false, false});
        
        // Level 4: Clear Starry Night (Deep navy, twinkling stars, glowing moon)
        m_themes.push_back({sf::Color{10, 15, 35}, sf::Color{30, 45, 80}, sf::Color{240, 245, 255}, sf::Color{200, 210, 255, 40}, sf::Color{35, 45, 65}, sf::Color{20, 30, 45}, sf::Color{15, 15, 20}, sf::Color{25, 60, 45}, true, false, false});
        
        // Level 5: Dawn Misty (Soft lavender, peach, morning fog)
        m_themes.push_back({sf::Color{90, 80, 140}, sf::Color{255, 190, 160}, sf::Color{255, 220, 180}, sf::Color{255, 180, 120, 80}, sf::Color{110, 100, 140}, sf::Color{70, 75, 105}, sf::Color{45, 35, 40}, sf::Color{50, 100, 85}, false, false, false});
        
        // Level 6: Midnight Rain (Starry night but with gentle rainfall)
        m_themes.push_back({sf::Color{15, 20, 45}, sf::Color{40, 55, 90}, sf::Color{240, 245, 255}, sf::Color{200, 210, 255, 20}, sf::Color{45, 55, 75}, sf::Color{25, 40, 55}, sf::Color{15, 15, 25}, sf::Color{30, 70, 55}, true, false, true});
        
        // Level 7: Autumn Golden Hour (Warm golds, browns, beautiful sun)
        m_themes.push_back({sf::Color{140, 175, 215}, sf::Color{255, 230, 170}, sf::Color{255, 210, 100}, sf::Color{255, 160, 50, 100}, sf::Color{170, 145, 120}, sf::Color{130, 95, 70}, sf::Color{70, 40, 25}, sf::Color{200, 110, 40}, false, false, false});
        
        // Level 8: Sunshower (Bright day, shining sun, light rain)
        m_themes.push_back({sf::Color{100, 160, 220}, sf::Color{190, 215, 235}, sf::Color{255, 240, 150}, sf::Color{255, 220, 100, 80}, sf::Color{120, 155, 185}, sf::Color{80, 120, 100}, sf::Color{70, 45, 30}, sf::Color{35, 130, 65}, false, false, true});
        
        // Level 9: Crimson Eclipse (Intense deep red, dramatic boss-level feel)
        m_themes.push_back({sf::Color{80, 10, 15}, sf::Color{180, 40, 30}, sf::Color{255, 200, 200}, sf::Color{255, 50, 50, 150}, sf::Color{100, 20, 25}, sf::Color{50, 10, 15}, sf::Color{20, 5, 10}, sf::Color{80, 20, 25}, false, false, false});
        
        // Level 10: The Emperor's Storm (Epic finale: Dark purple, fierce lightning, stars)
        m_themes.push_back({sf::Color{20, 10, 35}, sf::Color{50, 20, 60}, sf::Color{255, 255, 255}, sf::Color{200, 150, 255, 80}, sf::Color{30, 15, 45}, sf::Color{15, 10, 25}, sf::Color{10, 5, 15}, sf::Color{20, 15, 35}, true, true, true});
    }

    void loadLevel(int level) {
        // --- FIX: Specific Themes locked to Specific Levels ---
        m_theme = m_themes[(level - 1) % m_themes.size()]; 
        
        m_time = 0.0f;
        m_clouds.clear(); m_trees.clear(); m_birds.clear(); m_stars.clear(); m_raindrops.clear();
        
        for (int i = 0; i < 20; i++) {
            m_clouds.push_back({sf::Vector2f{-1000.f + (rand() % 6000), 50.f + (rand() % 200)}, 0.5f + (rand() % 10)/10.f, 10.f + (rand() % 20)});
        }
        
        for (int i = 0; i < 30; i++) {
            Tree t;
            t.scale = 0.6f + (rand() % 4) / 10.f; 
            t.pos = sf::Vector2f{-800.f + (rand() % 5000), 680.f};
            t.phase = (rand() % 314) / 100.f;

            int numLeaves = 200 + (rand() % 100);
            for (int j = 0; j < numLeaves; ++j) {
                Leaf l;
                int branch = rand() % 3;
                sf::Vector2f branchCenter;
                if (branch == 0) branchCenter = {0.f, -110.f * t.scale};       
                else if (branch == 1) branchCenter = {-35.f * t.scale, -75.f * t.scale}; 
                else branchCenter = {35.f * t.scale, -75.f * t.scale};         

                float spreadAngle = (rand() % 360) * 3.14159f / 180.f;
                float dist = std::sqrt((rand() % 100) / 100.f); 
                float radius = 45.f * t.scale * dist; 
                
                l.offset = branchCenter + sf::Vector2f{std::cos(spreadAngle) * radius, std::sin(spreadAngle) * radius};
                l.length = (8.f + (rand() % 5)) * t.scale; 
                l.width = (4.f + (rand() % 3)) * t.scale;  
                l.baseAngle = (rand() % 360) * 3.14159f / 180.f; 

                int colorVar = (rand() % 30) - 15; 
                sf::Color c = m_theme.treeLeaves;
                c.r = static_cast<std::uint8_t>(std::clamp((int)c.r + colorVar, 0, 255));
                c.g = static_cast<std::uint8_t>(std::clamp((int)c.g + colorVar, 0, 255));
                c.b = static_cast<std::uint8_t>(std::clamp((int)c.b + colorVar, 0, 255));
                
                if (l.offset.y > branchCenter.y + 10.f) {
                    c.r = static_cast<std::uint8_t>(std::max(0, (int)c.r - 40));
                    c.g = static_cast<std::uint8_t>(std::max(0, (int)c.g - 40));
                    c.b = static_cast<std::uint8_t>(std::max(0, (int)c.b - 40));
                }

                l.color = c;
                l.phaseOffset = (rand() % 314) / 100.f; 
                t.leaves.push_back(l);
            }
            m_trees.push_back(t);
        }

        for (int i = 0; i < 15; i++) { 
            m_birds.push_back({sf::Vector2f{-500.f + (rand() % 4000), 100.f + (rand() % 300)}, 80.f + (rand() % 40), 10.f + (rand() % 5), (rand() % 314) / 100.f});
        }

        if (m_theme.isNight) {
            for (int i = 0; i < 250; i++) {
                m_stars.push_back({
                    sf::Vector2f{static_cast<float>(rand() % 1280), static_cast<float>(rand() % 450)}, 
                    1.0f + (rand() % 20) / 10.f, 
                    static_cast<float>(rand() % 314) / 100.f
                });
            }
        }

        // --- UPGRADED: Softer, more natural rain ---
        if (m_theme.isRaining) {
            for (int i = 0; i < 350; i++) { 
                m_raindrops.push_back({
                    sf::Vector2f{-1000.f + (rand() % 4000), -500.f + (rand() % 1500)}, 
                    600.f + (rand() % 400), // Much slower, natural falling speed
                    10.f + (rand() % 15)    // Shorter, softer droplets
                });
            }
        }

        m_lightningFlash = 0.0f;
        m_nextLightning = 2.0f + (rand() % 50) / 10.f;

        sf::Color backTop = m_theme.mountFar;
        sf::Color backBot = m_theme.skyBot; 
        generateMountains(m_mountainsBack, backTop, backBot, 400.f, 250.f, 0.f, true);

        sf::Color midTop;
        midTop.r = static_cast<std::uint8_t>((m_theme.mountFar.r + m_theme.mountNear.r) / 2);
        midTop.g = static_cast<std::uint8_t>((m_theme.mountFar.g + m_theme.mountNear.g) / 2);
        midTop.b = static_cast<std::uint8_t>((m_theme.mountFar.b + m_theme.mountNear.b) / 2);
        sf::Color midBot = backTop; 
        generateMountains(m_mountainsMid, midTop, midBot, 500.f, 180.f, 5000.f, true);

        sf::Color frontTop = m_theme.mountNear;
        sf::Color frontBot = m_theme.treeTrunk; 
        generateMountains(m_mountainsNear, frontTop, frontBot, 620.f, 100.f, 12000.f, false);
    }

    void update(float dt) {
        m_time += dt;
        for (auto& cloud : m_clouds) {
            cloud.pos.x += cloud.speed * dt;
            if (cloud.pos.x > 5000.f) cloud.pos.x = -1000.f;
        }
        for (auto& bird : m_birds) {
            bird.pos.x -= bird.speed * dt; 
            if (bird.pos.x < -1000.f) bird.pos.x = 5000.f;
        }

        if (m_theme.isRaining) {
            for (auto& drop : m_raindrops) {
                drop.pos.y += drop.speed * dt;
                drop.pos.x += (drop.speed * 0.08f) * dt; // Softer, more realistic wind slant
                
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

    void render(sf::RenderWindow& window, float cameraX) {
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

                starArray[i*6 + 0] = sf::Vertex{sp + sf::Vector2f{-s, -s}, c};
                starArray[i*6 + 1] = sf::Vertex{sp + sf::Vector2f{s, -s}, c};
                starArray[i*6 + 2] = sf::Vertex{sp + sf::Vector2f{s, s}, c};
                starArray[i*6 + 3] = sf::Vertex{sp + sf::Vector2f{-s, -s}, c};
                starArray[i*6 + 4] = sf::Vertex{sp + sf::Vector2f{s, s}, c};
                starArray[i*6 + 5] = sf::Vertex{sp + sf::Vector2f{-s, s}, c};
            }
            window.draw(starArray);
        }

        if (m_theme.isStormy && m_lightningFlash > 0.0f) {
            if (m_lightningFlash > 0.6f) { 
                sf::Transform t;
                t.translate({cx, 0.f});
                window.draw(m_lightningBolt, t);
                t.translate({1.f, 0.f}); window.draw(m_lightningBolt, t);
                t.translate({-2.f, 0.f}); window.draw(m_lightningBolt, t);
            }
            sf::VertexArray flash(sf::PrimitiveType::TriangleStrip, 4);
            sf::Color flashCol{255, 255, 255, static_cast<std::uint8_t>(m_lightningFlash * 150)};
            flash[0].position = {cx, 0.f};               flash[0].color = flashCol;
            flash[1].position = {cx, 720.f};             flash[1].color = flashCol;
            flash[2].position = {cx + 1280.f, 0.f};      flash[2].color = flashCol;
            flash[3].position = {cx + 1280.f, 720.f};    flash[3].color = flashCol;
            window.draw(flash);
        }

        sf::CircleShape sunCore(40.f), sunGlow(120.f); 
        float sunX = cx + 900.f - (cameraX * 0.05f); 
        sunCore.setOrigin({40.f, 40.f}); sunCore.setPosition({sunX, 150.f}); sunCore.setFillColor(m_theme.sunCore);
        sunGlow.setOrigin({120.f, 120.f}); sunGlow.setPosition({sunX, 150.f}); sunGlow.setFillColor(m_theme.sunGlow);
        window.draw(sunGlow); window.draw(sunCore);

        drawParallaxLayer(window, m_mountainsBack, cameraX, 0.1f);
        drawParallaxLayer(window, m_mountainsMid, cameraX, 0.25f);
        drawParallaxLayer(window, m_mountainsNear, cameraX, 0.45f);

        for (auto& cloud : m_clouds) drawCloud(window, cloud, cameraX, 0.15f);
        for (auto& tree : m_trees) drawTree(window, tree, cameraX, 0.8f);
        for (auto& bird : m_birds) drawBird(window, bird, cameraX, 0.6f);

        if (m_theme.isRaining) {
            sf::VertexArray rainArray(sf::PrimitiveType::Lines, m_raindrops.size() * 2);
            sf::Color rainCol{220, 230, 255, 100}; // Softer, less obtrusive rain color
            for (size_t i = 0; i < m_raindrops.size(); i++) {
                sf::Vector2f dropPos = m_raindrops[i].pos + sf::Vector2f{cameraX, 0.f}; 
                
                rainArray[i*2 + 0] = sf::Vertex{dropPos, rainCol};
                rainArray[i*2 + 1] = sf::Vertex{dropPos + sf::Vector2f{-m_raindrops[i].length * 0.08f, -m_raindrops[i].length}, rainCol};
            }
            window.draw(rainArray);
        }
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
    sf::VertexArray m_mountainsBack, m_mountainsMid, m_mountainsNear;
    
    float m_lightningFlash;
    float m_nextLightning;
    sf::VertexArray m_lightningBolt;

    void generateLightningBolt() {
        m_lightningBolt.clear();
        m_lightningBolt.setPrimitiveType(sf::PrimitiveType::LineStrip);
        float lx = 200.f + (rand() % 880); 
        float ly = 0.f;
        while (ly < 600.f) { 
            m_lightningBolt.append(sf::Vertex{sf::Vector2f{lx, ly}, sf::Color::White});
            lx += (rand() % 100) - 50.f; 
            ly += 30.f + (rand() % 50);  
        }
    }

    void generateMountains(sf::VertexArray& va, sf::Color topColor, sf::Color bottomColor, float baseHeight, float variance, float seedOffset, bool hasSnow) {
        va.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
        va.resize(0);
        
        sf::Color snowColor = m_theme.isNight ? sf::Color{130, 130, 150} : sf::Color{255, 255, 255};

        for (float x = -4000.f; x <= 8000.f; x += 15.f) {
            float nx = x + 15000.f + seedOffset; 

            float n1 = std::sin(nx * 0.0008f) * 1.2f;        
            float n2 = std::sin(nx * 0.0025f + 1.0f) * 0.5f; 
            float n3 = std::abs(std::sin(nx * 0.006f)) * 0.3f; 
            float n4 = std::sin(nx * 0.015f + 2.0f) * 0.1f;  

            float combinedNoise = (n1 + n2 - n3 + n4);
            float height = baseHeight - (combinedNoise * variance);

            sf::Color peakColor = topColor;

            if (hasSnow) {
                float snowLine = baseHeight - (variance * 0.2f); 
                if (height < snowLine) { 
                    float blend = (snowLine - height) / (variance * 0.5f); 
                    blend = std::clamp(blend, 0.0f, 1.0f);
                    
                    peakColor.r = static_cast<std::uint8_t>(topColor.r + (snowColor.r - topColor.r) * blend);
                    peakColor.g = static_cast<std::uint8_t>(topColor.g + (snowColor.g - topColor.g) * blend);
                    peakColor.b = static_cast<std::uint8_t>(topColor.b + (snowColor.b - topColor.b) * blend);
                }
            }

            va.append(sf::Vertex{sf::Vector2f{x, 800.f}, bottomColor});
            va.append(sf::Vertex{sf::Vector2f{x, height}, peakColor});
        }
    }

    void drawParallaxLayer(sf::RenderWindow& win, const sf::VertexArray& va, float camX, float parallax) {
        sf::Transform t;
        t.translate({camX * (1.0f - parallax), 0.f});
        win.draw(va, t);
    }

    void drawCloud(sf::RenderWindow& win, const Cloud& c, float camX, float parallax) {
        float drawX = c.pos.x + camX * (1.0f - parallax);
        if (drawX < camX - 800.f || drawX > camX + 1480.f) return; 
        
        sf::Color baseCol = m_theme.isNight ? sf::Color{90, 90, 110, 220} : sf::Color{255, 255, 255, 240};
        sf::Color shadowCol = m_theme.isNight ? sf::Color{60, 60, 80, 220} : sf::Color{210, 210, 225, 240};

        if (m_theme.isStormy) {
            baseCol = sf::Color{100, 110, 120, 240};
            shadowCol = sf::Color{70, 80, 90, 240};
        }

        auto drawPuff = [&](float offsetX, float offsetY, float radius, sf::Color col) {
            sf::CircleShape puff(radius);
            puff.setOrigin({radius, radius});
            puff.setPosition({drawX + offsetX * c.scale, c.pos.y + offsetY * c.scale});
            puff.setScale({1.3f, 0.8f}); 
            puff.setFillColor(col);
            win.draw(puff);
        };

        drawPuff(0.f, 15.f, 35.f, shadowCol);
        drawPuff(50.f, 10.f, 45.f, shadowCol);
        drawPuff(100.f, 20.f, 30.f, shadowCol);

        drawPuff(-5.f, 0.f, 35.f, baseCol);
        drawPuff(45.f, -10.f, 45.f, baseCol);
        drawPuff(95.f, 5.f, 30.f, baseCol);
        drawPuff(25.f, 15.f, 25.f, baseCol);
        drawPuff(75.f, 10.f, 25.f, baseCol);
    }

    void drawTree(sf::RenderWindow& win, const Tree& t, float camX, float parallax) {
        float drawX = t.pos.x + camX * (1.0f - parallax);
        if (drawX < camX - 800.f || drawX > camX + 1280.f) return; 

        sf::ConvexShape trunk(4);
        trunk.setPoint(0, sf::Vector2f{-4.f * t.scale, -120.f * t.scale}); 
        trunk.setPoint(1, sf::Vector2f{4.f * t.scale, -120.f * t.scale});  
        trunk.setPoint(2, sf::Vector2f{14.f * t.scale, 0.f});              
        trunk.setPoint(3, sf::Vector2f{-14.f * t.scale, 0.f});             
        trunk.setFillColor(m_theme.treeTrunk);
        trunk.setPosition({drawX, t.pos.y});
        win.draw(trunk);

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
            float cosA = std::cos(currentAngle);
            float sinA = std::sin(currentAngle);
            
            auto rotate = [&](float x, float y) {
                return sf::Vector2f{x * cosA - y * sinA, x * sinA + y * cosA};
            };

            sf::Vector2f top = center + rotate(0.f, -l.length);
            sf::Vector2f bottom = center + rotate(0.f, l.length);
            sf::Vector2f left = center + rotate(-l.width, 0.f);
            sf::Vector2f right = center + rotate(l.width, 0.f);

            sf::Color darkTip = l.color;
            darkTip.r = static_cast<std::uint8_t>(std::max(0, (int)darkTip.r - 30));
            darkTip.g = static_cast<std::uint8_t>(std::max(0, (int)darkTip.g - 30));
            darkTip.b = static_cast<std::uint8_t>(std::max(0, (int)darkTip.b - 30));

            leafArray[i*6 + 0] = sf::Vertex{top, l.color};
            leafArray[i*6 + 1] = sf::Vertex{left, l.color};
            leafArray[i*6 + 2] = sf::Vertex{right, l.color};

            leafArray[i*6 + 3] = sf::Vertex{bottom, darkTip};
            leafArray[i*6 + 4] = sf::Vertex{right, l.color};
            leafArray[i*6 + 5] = sf::Vertex{left, l.color};
        }
        win.draw(leafArray);
    }

    void drawBird(sf::RenderWindow& win, const Bird& b, float camX, float parallax) {
        float drawX = b.pos.x + camX * (1.0f - parallax);
        if (drawX < camX - 800.f || drawX > camX + 1280.f) return;

        float wingY = std::sin(m_time * b.flapSpeed + b.phase) * 10.0f;

        sf::VertexArray v(sf::PrimitiveType::LineStrip, 3);
        sf::Color bCol = m_theme.isNight ? sf::Color{50, 50, 50} : sf::Color::Black;
        v[0] = sf::Vertex{sf::Vector2f{drawX - 10.f, b.pos.y + wingY}, bCol};
        v[1] = sf::Vertex{sf::Vector2f{drawX, b.pos.y}, bCol};
        v[2] = sf::Vertex{sf::Vector2f{drawX + 15.f, b.pos.y + wingY}, bCol};
        win.draw(v);
    }
};