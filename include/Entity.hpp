#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

const float SCALE = 30.0f;

enum class EntityType { BIRD, WOOD, GROUND, ENEMY };

class Entity {
public:
    Entity(b2World& world, float x, float y, float width, float height, EntityType type, sf::Texture* texture = nullptr);
    ~Entity(); // Need a destructor to clean up physics bodies properly

    void Render(sf::RenderWindow& window);
    void TakeDamage(float impact); // NEW

    b2Body* GetBody() { return body; }
    EntityType GetType() { return type; } // NEW
    bool IsDestroyed() { return isDestroyed; } // NEW

private:
    b2Body* body;
    sf::RectangleShape shape;
    float width, height;
    
    EntityType type; // NEW
    float health;    // NEW
    bool isDestroyed;// NEW
};