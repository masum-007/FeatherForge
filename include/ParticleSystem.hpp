#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cstdint> // Added for std::uint8_t

struct Particle {
    sf::Vector2f velocity;
    float lifespan;
    float maxLifespan;
    float gravityScale;
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
public:
    ParticleSystem(unsigned int count = 2000) :
        m_particles(count),
        m_vertices(sf::PrimitiveType::Triangles, count * 6) // FIXED: Replaced Quads with Triangles (6 vertices per debris piece)
    {
        // Initialize all as invisible
        for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i) {
            m_vertices[i].color.a = 0; 
        }
    }

    void emitWood(sf::Vector2f pos) {
        // Brown splinters, fast, heavy gravity
        emitCustom(pos, 30, sf::Color(139, 69, 19), 150.f, 400.f, 800.f, 0.5f, 1.0f);
    }

    void emitFeathers(sf::Vector2f pos) {
        // White/Grey feathers, slow, low gravity (floating)
        emitCustom(pos, 20, sf::Color(230, 230, 230), 50.f, 150.f, 50.f, 1.0f, 2.5f);
    }

    void emitDust(sf::Vector2f pos) {
        // Tan dust, slow, slightly negative gravity (drifts up)
        emitCustom(pos, 15, sf::Color(180, 170, 150, 200), 20.f, 80.f, -30.f, 0.4f, 1.2f);
    }

    void update(float dt) {
        for (std::size_t i = 0; i < m_particles.size(); ++i) {
            Particle& p = m_particles[i];
            
            // If the particle is alive (alpha > 0)
            if (m_vertices[i * 6].color.a > 0) {
                p.lifespan -= dt;
                
                if (p.lifespan <= 0.f) {
                    // Hide all 6 vertices of the two triangles
                    for(int v = 0; v < 6; v++) m_vertices[i * 6 + v].color.a = 0;
                } else {
                    // Apply gravity
                    p.velocity.y += p.gravityScale * dt; 
                    
                    // Move the shape
                    for(int v = 0; v < 6; v++) {
                        m_vertices[i * 6 + v].position += p.velocity * dt;
                    }

                    // Fade out
                    float ratio = p.lifespan / p.maxLifespan;
                    // FIXED: Replaced sf::Uint8 with std::uint8_t
                    std::uint8_t alpha = static_cast<std::uint8_t>(255 * std::max(ratio, 0.f)); 
                    for(int v = 0; v < 6; v++) m_vertices[i * 6 + v].color.a = alpha;
                }
            }
        }
    }

private:
    void emitCustom(sf::Vector2f position, int amount, sf::Color color, 
                    float minSpeed, float maxSpeed, float gravity, 
                    float minLife, float maxLife) 
    {
        int spawned = 0;
        for (std::size_t i = 0; i < m_particles.size() && spawned < amount; ++i) {
            if (m_vertices[i * 6].color.a == 0) { // Find a dead particle
                
                float angle = (std::rand() % 360) * 3.14159f / 180.f;
                float speed = minSpeed + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX/(maxSpeed - minSpeed)));
                
                m_particles[i].velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
                m_particles[i].maxLifespan = minLife + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX/(maxLife - minLife)));
                m_particles[i].lifespan = m_particles[i].maxLifespan;
                m_particles[i].gravityScale = gravity;

                // Create a 4x4 square using 2 Triangles (6 vertices)
                float size = 4.0f; 
                sf::Vector2f p1 = position + sf::Vector2f(-size, -size); // Top-left
                sf::Vector2f p2 = position + sf::Vector2f(size, -size);  // Top-right
                sf::Vector2f p3 = position + sf::Vector2f(size, size);   // Bottom-right
                sf::Vector2f p4 = position + sf::Vector2f(-size, size);  // Bottom-left

                // Triangle 1
                m_vertices[i * 6 + 0].position = p1;
                m_vertices[i * 6 + 1].position = p2;
                m_vertices[i * 6 + 2].position = p3;
                
                // Triangle 2
                m_vertices[i * 6 + 3].position = p1;
                m_vertices[i * 6 + 4].position = p3;
                m_vertices[i * 6 + 5].position = p4;

                // Apply color to all vertices
                for(int v = 0; v < 6; v++) {
                    m_vertices[i * 6 + v].color = color;
                }
                
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