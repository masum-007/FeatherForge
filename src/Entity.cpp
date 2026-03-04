#include "Entity.hpp"

Entity::Entity(b2World& world, float x, float y, float w, float h, EntityType type, sf::Texture* texture) 
    : width(w), height(h) {
    
    // 1. Define Body
    b2BodyDef bodyDef;
    bodyDef.position.Set(x / SCALE, y / SCALE);

    if (type == EntityType::GROUND) {
        bodyDef.type = b2_staticBody; 
    } else {
        bodyDef.type = b2_dynamicBody; 
    }
    
    body = world.CreateBody(&bodyDef);

    // 2. Define Shape & Fixture
    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;

    b2PolygonShape boxShape;
    b2CircleShape circleShape; // Box2D circle math

    if (type == EntityType::ENEMY || type == EntityType::BIRD) {
        circleShape.m_radius = (w / 2.0f) / SCALE;
        fixtureDef.shape = &circleShape;
        fixtureDef.restitution = 0.4f; // slightly bouncy
    } else {
        boxShape.SetAsBox((w / 2.0f) / SCALE, (h / 2.0f) / SCALE);
        fixtureDef.shape = &boxShape;
        if (type == EntityType::WOOD) fixtureDef.density = 0.8f;
    }

    body->CreateFixture(&fixtureDef);

    // 3. Setup SFML Graphics
    shape.setSize(sf::Vector2f(w, h));
    shape.setOrigin(w / 2.0f, h / 2.0f);
    
    // FIX: Check if texture exists AND actually loaded an image (size > 0)
    if (texture && texture->getSize().x > 0) {
        shape.setTexture(texture); 
    } else {
        if (type == EntityType::BIRD) shape.setFillColor(sf::Color::Red);
        else if (type == EntityType::WOOD) shape.setFillColor(sf::Color(139, 69, 19)); 
        else if (type == EntityType::ENEMY) shape.setFillColor(sf::Color::Red); // Now it will be red!
        else shape.setFillColor(sf::Color(34, 139, 34)); // Ground
    }

    // 3. Setup SFML Graphics
    shape.setSize(sf::Vector2f(w, h));
    shape.setOrigin(w / 2.0f, h / 2.0f);
    
    // --- NEW TEXTURE LOGIC ---
    if (texture) {
        shape.setTexture(texture); // Map the image to the box!
    } else {
        // Fallback to solid colors if the image file is missing
        if (type == EntityType::BIRD) shape.setFillColor(sf::Color::Red);
        else if (type == EntityType::WOOD) shape.setFillColor(sf::Color(139, 69, 19)); 
        else shape.setFillColor(sf::Color::Green);
    }
}

// ... Keep your Entity::Render function exactly the same ...
void Entity::Render(sf::RenderWindow& window) {
    // Sync Graphics with Physics
    b2Vec2 position = body->GetPosition();
    float angle = body->GetAngle(); // Box2D is in radians

    shape.setPosition(position.x * SCALE, position.y * SCALE);
    shape.setRotation(angle * 180.0f / b2_pi); // Convert to degrees for SFML

    window.draw(shape);
}