#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

struct Cloud { sf::Vector2f pos; float scale; float speed; };
struct Bird { sf::Vector2f pos; float speed; float flapSpeed; float phase; };

// --- UPGRADED: Leaf structure with rotation and dimensions ---
struct Leaf {
    sf::Vector2f offset;  
    sf::Color color;      
    float length;         // Length of the leaf from stem to tip
    float width;          // Width of the leaf
    float baseAngle;      // Natural rotation of the leaf
    float phaseOffset;    
};

struct Tree { 
    sf::Vector2f pos; 
    float scale; 
    float phase; 
    std::vector<Leaf> leaves;
};

struct Theme {
    sf::Color skyTop, skyBot, sunCore, sunGlow;
    sf::Color mountFar, mountNear;
    sf::Color treeTrunk, treeLeaves;
    bool isNight;
};

class EnvironmentSystem {
public:
    EnvironmentSystem() : m_time(0.0f) {
        m_themes.push_back({sf::Color{100, 180, 255}, sf::Color{200, 230, 255}, sf::Color{255, 240, 100}, sf::Color{255, 200, 50, 100}, sf::Color{150, 180, 200}, sf::Color{100, 150, 120}, sf::Color{80, 50, 30}, sf::Color{34, 139, 34}, false});
        m_themes.push_back({sf::Color{255, 120, 80}, sf::Color{255, 200, 100}, sf::Color{255, 150, 50}, sf::Color{255, 100, 0, 120}, sf::Color{180, 100, 100}, sf::Color{120, 60, 60}, sf::Color{60, 30, 20}, sf::Color{180, 80, 20}, false});
        m_themes.push_back({sf::Color{10, 15, 40}, sf::Color{30, 40, 80}, sf::Color{240, 240, 255}, sf::Color{200, 200, 255, 50}, sf::Color{30, 40, 60}, sf::Color{20, 30, 40}, sf::Color{15, 10, 10}, sf::Color{20, 60, 40}, true});
        m_themes.push_back({sf::Color{80, 60, 120}, sf::Color{255, 180, 150}, sf::Color{255, 220, 150}, sf::Color{255, 150, 100, 80}, sf::Color{100, 90, 130}, sf::Color{60, 70, 100}, sf::Color{40, 30, 30}, sf::Color{40, 100, 80}, false});
        m_themes.push_back({sf::Color{120, 130, 140}, sf::Color{180, 190, 200}, sf::Color{200, 200, 200}, sf::Color{150, 150, 150, 50}, sf::Color{100, 110, 120}, sf::Color{80, 90, 100}, sf::Color{50, 50, 50}, sf::Color{60, 100, 60}, false});
        m_themes.push_back({sf::Color{150, 180, 220}, sf::Color{255, 230, 150}, sf::Color{255, 200, 80}, sf::Color{255, 150, 0, 100}, sf::Color{180, 150, 120}, sf::Color{140, 100, 70}, sf::Color{80, 40, 20}, sf::Color{220, 120, 30}, false});
    }

    void loadLevel(int level) {
        m_theme = m_themes[(level - 1) % m_themes.size()]; 
        m_time = 0.0f;
        m_clouds.clear(); m_trees.clear(); m_birds.clear();
        
        for (int i = 0; i < 20; i++) {
            m_clouds.push_back({sf::Vector2f{-1000.f + (rand() % 6000), 50.f + (rand() % 200)}, 0.5f + (rand() % 10)/10.f, 10.f + (rand() % 20)});
        }
        
        for (int i = 0; i < 30; i++) {
            Tree t;
            // 1. SMALLER OVERALL TREES (0.6x to 0.9x scale)
            t.scale = 0.6f + (rand() % 4) / 10.f; 
            t.pos = sf::Vector2f{-800.f + (rand() % 5000), 680.f};
            t.phase = (rand() % 314) / 100.f;

            // 2. EXTREMELY DENSE CANOPY (200-300 leaves per tree)
            int numLeaves = 200 + (rand() % 100);
            for (int j = 0; j < numLeaves; ++j) {
                Leaf l;
                
                // 3. ORGANIC BRANCH CLUSTERING (Top, Left, Right)
                int branch = rand() % 3;
                sf::Vector2f branchCenter;
                if (branch == 0) branchCenter = {0.f, -110.f * t.scale};       // Top Branch
                else if (branch == 1) branchCenter = {-35.f * t.scale, -75.f * t.scale}; // Left Branch
                else branchCenter = {35.f * t.scale, -75.f * t.scale};         // Right Branch

                float spreadAngle = (rand() % 360) * 3.14159f / 180.f;
                float dist = std::sqrt((rand() % 100) / 100.f); 
                float radius = 45.f * t.scale * dist; // Tighter clusters to prevent floating leaves
                
                l.offset = branchCenter + sf::Vector2f{std::cos(spreadAngle) * radius, std::sin(spreadAngle) * radius};
                
                // 4. LEAF DIMENSIONS
                l.length = (8.f + (rand() % 5)) * t.scale; // Longer
                l.width = (4.f + (rand() % 3)) * t.scale;  // Thinner
                l.baseAngle = (rand() % 360) * 3.14159f / 180.f; // Random leaf rotation

                // Color Variation
                int colorVar = (rand() % 30) - 15; 
                sf::Color c = m_theme.treeLeaves;
                c.r = static_cast<std::uint8_t>(std::clamp((int)c.r + colorVar, 0, 255));
                c.g = static_cast<std::uint8_t>(std::clamp((int)c.g + colorVar, 0, 255));
                c.b = static_cast<std::uint8_t>(std::clamp((int)c.b + colorVar, 0, 255));
                
                // Darken leaves that are at the bottom of the clusters
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

        generateMountains(m_mountainsFar, m_theme.mountFar, 400.f, 150.f);
        generateMountains(m_mountainsNear, m_theme.mountNear, 550.f, 100.f);
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
    }

    void render(sf::RenderWindow& window, float cameraX) {
        sf::VertexArray sky(sf::PrimitiveType::TriangleStrip, 4);
        float cx = cameraX - 640.f; 
        
        sky[0].position = sf::Vector2f{cx, 0.f};               sky[0].color = m_theme.skyTop;
        sky[1].position = sf::Vector2f{cx, 720.f};             sky[1].color = m_theme.skyBot;
        sky[2].position = sf::Vector2f{cx + 1280.f, 0.f};      sky[2].color = m_theme.skyTop;
        sky[3].position = sf::Vector2f{cx + 1280.f, 720.f};    sky[3].color = m_theme.skyBot;
        window.draw(sky);

        sf::CircleShape sunCore(40.f), sunGlow(80.f);
        float sunX = cx + 900.f - (cameraX * 0.05f); 
        sunCore.setOrigin({40.f, 40.f}); sunCore.setPosition({sunX, 150.f}); sunCore.setFillColor(m_theme.sunCore);
        sunGlow.setOrigin({80.f, 80.f}); sunGlow.setPosition({sunX, 150.f}); sunGlow.setFillColor(m_theme.sunGlow);
        window.draw(sunGlow); window.draw(sunCore);

        drawParallaxLayer(window, m_mountainsFar, cameraX, 0.2f);
        drawParallaxLayer(window, m_mountainsNear, cameraX, 0.4f);

        for (auto& cloud : m_clouds) drawCloud(window, cloud, cameraX, 0.15f);
        for (auto& tree : m_trees) drawTree(window, tree, cameraX, 0.8f);
        for (auto& bird : m_birds) drawBird(window, bird, cameraX, 0.6f);
    }

private:
    float m_time;
    Theme m_theme;
    std::vector<Theme> m_themes;
    std::vector<Cloud> m_clouds;
    std::vector<Tree> m_trees;
    std::vector<Bird> m_birds;
    sf::VertexArray m_mountainsFar, m_mountainsNear;

    void generateMountains(sf::VertexArray& va, sf::Color color, float baseHeight, float variance) {
        va.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
        va.resize(0);
        for (float x = -2000.f; x <= 6000.f; x += 150.f) {
            float height = baseHeight - (std::sin(x * 0.01f) * variance) - (std::cos(x * 0.03f) * (variance * 0.5f));
            va.append(sf::Vertex{sf::Vector2f{x, height}, color});
            va.append(sf::Vertex{sf::Vector2f{x, 800.f}, color});
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

        // Draw Trunk
        sf::ConvexShape trunk(4);
        trunk.setPoint(0, sf::Vector2f{-4.f * t.scale, -120.f * t.scale}); 
        trunk.setPoint(1, sf::Vector2f{4.f * t.scale, -120.f * t.scale});  
        trunk.setPoint(2, sf::Vector2f{14.f * t.scale, 0.f});              
        trunk.setPoint(3, sf::Vector2f{-14.f * t.scale, 0.f});             
        trunk.setFillColor(m_theme.treeTrunk);
        trunk.setPosition({drawX, t.pos.y});
        win.draw(trunk);

        // We use 6 vertices (2 Triangles) per leaf to make a diamond shape
        sf::VertexArray leafArray(sf::PrimitiveType::Triangles, t.leaves.size() * 6);

        float globalWind = std::sin(m_time * 2.0f + t.phase);

        for (size_t i = 0; i < t.leaves.size(); ++i) {
            const Leaf& l = t.leaves[i];

            // Wind calculation
            float swayAmount = globalWind * (std::abs(l.offset.y) * 0.08f);
            float flutter = std::sin(m_time * 6.0f + l.phaseOffset) * 2.0f;
            
            // Final position of the leaf center
            sf::Vector2f center = sf::Vector2f{drawX + l.offset.x + swayAmount + flutter, t.pos.y + l.offset.y};

            // 5. DIAMOND LEAF MATH
            // Calculate rotation for this specific leaf
            float currentAngle = l.baseAngle + (flutter * 0.1f); 
            float cosA = std::cos(currentAngle);
            float sinA = std::sin(currentAngle);
            
            auto rotate = [&](float x, float y) {
                return sf::Vector2f{x * cosA - y * sinA, x * sinA + y * cosA};
            };

            // Diamond points relative to center
            sf::Vector2f top = center + rotate(0.f, -l.length);
            sf::Vector2f bottom = center + rotate(0.f, l.length);
            sf::Vector2f left = center + rotate(-l.width, 0.f);
            sf::Vector2f right = center + rotate(l.width, 0.f);

            // Create a darker shade for the bottom tip of the leaf to give it 3D texture
            sf::Color darkTip = l.color;
            darkTip.r = static_cast<std::uint8_t>(std::max(0, (int)darkTip.r - 30));
            darkTip.g = static_cast<std::uint8_t>(std::max(0, (int)darkTip.g - 30));
            darkTip.b = static_cast<std::uint8_t>(std::max(0, (int)darkTip.b - 30));

            // Triangle 1: Top Half of the leaf
            leafArray[i*6 + 0] = sf::Vertex{top, l.color};
            leafArray[i*6 + 1] = sf::Vertex{left, l.color};
            leafArray[i*6 + 2] = sf::Vertex{right, l.color};

            // Triangle 2: Bottom Half of the leaf (using darkTip for gradient)
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