#include "Entity.hpp"

Entity::Entity(b2World& world, float x, float y, float w, float h, EntityType type) 
    : width(w), height(h) {
    
    // 1. Define Body
    b2BodyDef bodyDef;
    bodyDef.position.Set(x / SCALE, y / SCALE);

    if (type == EntityType::GROUND) {
        bodyDef.type = b2_staticBody; // Static: Doesn't move 
    } else {
        bodyDef.type = b2_dynamicBody; // Dynamic: Affected by gravity/forces
    }
    
    body = world.CreateBody(&bodyDef);

    // 2. Define Shape
    b2PolygonShape boxShape;
    boxShape.SetAsBox((w / 2.0f) / SCALE, (h / 2.0f) / SCALE);

    // 3. Define Fixture (Physical properties like friction/density)
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;

    // Set specialized properties based on material [cite: 116]
    if (type == EntityType::BIRD) fixtureDef.restitution = 0.5f; // Bouncy
    if (type == EntityType::WOOD) fixtureDef.density = 0.8f;     // Light

    body->CreateFixture(&fixtureDef);

    // 4. Setup SFML Graphics
    shape.setSize(sf::Vector2f(w, h));
    shape.setOrigin(w / 2.0f, h / 2.0f);
    
    if (type == EntityType::BIRD) shape.setFillColor(sf::Color::Red);
    else if (type == EntityType::WOOD) shape.setFillColor(sf::Color(139, 69, 19)); // Brown
    else shape.setFillColor(sf::Color::Green);
}

void Entity::Render(sf::RenderWindow& window) {
    // Sync Graphics with Physics
    b2Vec2 position = body->GetPosition();
    float angle = body->GetAngle(); // Box2D is in radians

    shape.setPosition(position.x * SCALE, position.y * SCALE);
    shape.setRotation(angle * 180.0f / b2_pi); // Convert to degrees for SFML

    window.draw(shape);
}