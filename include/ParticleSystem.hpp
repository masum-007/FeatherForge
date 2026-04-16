#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cstdint> 

struct Particle {
    sf::Vector2f velocity;
    float lifespan;
    float maxLifespan;
    float gravityScale;
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
public:
    ParticleSystem(unsigned int count = 3000) :
        m_particles(count),
        m_vertices(sf::PrimitiveType::Triangles, count * 6)
    {
        for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i) {
            m_vertices[i].color.a = 0; 
        }
    }

    void emitWood(sf::Vector2f pos) {
        emitCustom(pos, 30, sf::Color(139, 69, 19), 150.f, 400.f, 800.f, 0.5f, 1.0f);
    }

    void emitFeathers(sf::Vector2f pos) {
        emitCustom(pos, 20, sf::Color(230, 230, 230), 50.f, 150.f, 50.f, 1.0f, 2.5f);
    }

    void emitDust(sf::Vector2f pos) {
        emitCustom(pos, 15, sf::Color(180, 170, 150, 200), 20.f, 80.f, -30.f, 0.4f, 1.2f);
    }

    void emitIce(sf::Vector2f pos) {
        emitCustom(pos, 25, sf::Color(150, 255, 255, 220), 100.f, 300.f, 600.f, 0.4f, 0.8f);
    }

    // ---Massive intense flood effect ---
    void emitWater(sf::Vector2f pos) {
        emitCustom(pos, 100, sf::Color(50, 150, 255, 220), 200.f, 800.f, 400.f, 0.8f, 1.5f);
        emitCustom(pos, 50, sf::Color(200, 230, 255, 200), 50.f, 400.f, 200.f, 0.5f, 1.0f); // Foam
    }
    // --- NEW: Bird Flight Trails ---
    void emitFireTrail(sf::Vector2f pos) {
        emitCustom(pos, 2, sf::Color(255, 100, 0, 200), 10.f, 50.f, -50.f, 0.2f, 0.5f);
    }
    void emitIceTrail(sf::Vector2f pos) {
        emitCustom(pos, 2, sf::Color(150, 255, 255, 150), 10.f, 30.f, 10.f, 0.2f, 0.5f);
    }

    void update(float dt) {
        for (std::size_t i = 0; i < m_particles.size(); ++i) {
            Particle& p = m_particles[i];
            if (m_vertices[i * 6].color.a > 0) {
                p.lifespan -= dt;
                if (p.lifespan <= 0.f) {
                    for(int v = 0; v < 6; v++) m_vertices[i * 6 + v].color.a = 0;
                } else {
                    p.velocity.y += p.gravityScale * dt; 
                    for(int v = 0; v < 6; v++) m_vertices[i * 6 + v].position += p.velocity * dt;
                    float ratio = p.lifespan / p.maxLifespan;
                    std::uint8_t alpha = static_cast<std::uint8_t>(255 * std::max(ratio, 0.f)); 
                    for(int v = 0; v < 6; v++) m_vertices[i * 6 + v].color.a = alpha;
                }
            }
        }
    }

private:
    void emitCustom(sf::Vector2f position, int amount, sf::Color color, float minSpeed, float maxSpeed, float gravity, float minLife, float maxLife) {
        int spawned = 0;
        for (std::size_t i = 0; i < m_particles.size() && spawned < amount; ++i) {
            if (m_vertices[i * 6].color.a == 0) {
                float angle = (std::rand() % 360) * 3.14159f / 180.f;
                float speed = minSpeed + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX/(maxSpeed - minSpeed)));
                
                m_particles[i].velocity = sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed};
                m_particles[i].maxLifespan = minLife + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX/(maxLife - minLife)));
                m_particles[i].lifespan = m_particles[i].maxLifespan;
                m_particles[i].gravityScale = gravity;

                float size = 4.0f; 
                sf::Vector2f p1 = position + sf::Vector2f{-size, -size}; 
                sf::Vector2f p2 = position + sf::Vector2f{size, -size};  
                sf::Vector2f p3 = position + sf::Vector2f{size, size};   
                sf::Vector2f p4 = position + sf::Vector2f{-size, size};  

                m_vertices[i * 6 + 0].position = p1; m_vertices[i * 6 + 1].position = p2; m_vertices[i * 6 + 2].position = p3;
                m_vertices[i * 6 + 3].position = p1; m_vertices[i * 6 + 4].position = p3; m_vertices[i * 6 + 5].position = p4;

                for(int v = 0; v < 6; v++) m_vertices[i * 6 + v].color = color;
                spawned++;
            }
        }
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(m_vertices, states);
    }

    std::vector<Particle> m_particles;
    sf::VertexArray m_vertices;
};