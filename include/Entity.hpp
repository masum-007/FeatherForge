#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

const float SCALE = 30.0f;
enum class EntityType { BIRD, WOOD, ICE, GROUND, ENEMY };

class Entity {
public:
    Entity(b2World& world, float x, float y, float width, float height, EntityType type, sf::Texture* texture = nullptr);
    ~Entity(); 

    void Render(sf::RenderTarget& window); // Updated from sf::RenderWindow&
    void TakeDamage(float impact); 

    b2Body* GetBody() { return body; }
    EntityType GetType() { return type; } 
    bool IsDestroyed() { return isDestroyed; } 
    sf::Sprite& getSprite() { return sprite; } 

    float GetHealth() { return health; } 
    void SwapTexture(sf::Texture* newTexture); 
    bool isCracked = false;

    // --- NEW: Custom Damage System for Sloth ---
    void SetDamageMultiplier(float mult) { outgoingDamageMultiplier = mult; }
    float GetDamageMultiplier() { return outgoingDamageMultiplier; }

private:
    b2Body* body;
    sf::RectangleShape shape;
    sf::Sprite sprite; 
    float width, height;
    
    EntityType type; 
    float health;    
    bool isDestroyed;
    float outgoingDamageMultiplier = 1.0f; // NEW
    // ---For Soft-Body Physics ---
    sf::Vector2f baseScale;
};