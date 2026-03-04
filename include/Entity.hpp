#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

const float SCALE = 30.0f; // 30 pixels per meter

enum class EntityType { BIRD, WOOD, GROUND, ENEMY }; // Added ENEMY

class Entity {
public:
    // UPDATE THIS LINE: Add the sf::Texture pointer at the end
    Entity(b2World& world, float x, float y, float width, float height, EntityType type, sf::Texture* texture = nullptr);
    
    void Render(sf::RenderWindow& window);
    b2Body* GetBody() { return body; }

private:
    b2Body* body;
    sf::RectangleShape shape;
    float width, height;
};