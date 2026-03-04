#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

const float SCALE = 30.0f; // 30 pixels per meter

enum class EntityType { BIRD, WOOD, GROUND };

class Entity {
public:
    Entity(b2World& world, float x, float y, float width, float height, EntityType type);
    void Render(sf::RenderWindow& window);
    b2Body* GetBody() { return body; }

private:
    b2Body* body;
    sf::RectangleShape shape;
    float width, height;
};