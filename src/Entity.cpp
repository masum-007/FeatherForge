#include "Entity.hpp"

Entity::Entity(b2World& world, float x, float y, float w, float h, EntityType type, sf::Texture* texture) 
    : width(w), height(h), type(type), isDestroyed(false), outgoingDamageMultiplier(1.0f) { 
    
    // --- FIX: Ice is extremely fragile now ---
    if (type == EntityType::WOOD) health = 100.0f;
    else if (type == EntityType::ICE) health = 10.0f; // Shatters instantly!
    else if (type == EntityType::ENEMY) health = 50.0f;
    else health = 9999.0f; 

    b2BodyDef bodyDef;
    bodyDef.position.Set(x / SCALE, y / SCALE);
    bodyDef.type = (type == EntityType::GROUND) ? b2_staticBody : b2_dynamicBody; 
    body = world.CreateBody(&bodyDef);

    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;

    b2PolygonShape boxShape;
    b2CircleShape circleShape;

    if (type == EntityType::ENEMY || type == EntityType::BIRD) {
        circleShape.m_radius = (w / 2.0f) / SCALE;
        fixtureDef.shape = &circleShape;
        fixtureDef.restitution = 0.4f; 
    } else {
        boxShape.SetAsBox((w / 2.0f) / SCALE, (h / 2.0f) / SCALE);
        fixtureDef.shape = &boxShape;
        // --- FIX: Ice is lighter, Wood is heavier ---
        if (type == EntityType::WOOD) fixtureDef.density = 0.8f;
        else if (type == EntityType::ICE) fixtureDef.density = 0.3f; 
    }

    body->CreateFixture(&fixtureDef);

    if (texture) {
        sprite.setTexture(*texture);
        sprite.setScale({w / texture->getSize().x, h / texture->getSize().y});
        sprite.setOrigin({texture->getSize().x / 2.0f, texture->getSize().y / 2.0f});
    }

    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
}

Entity::~Entity() {
    if (body && body->GetWorld()) body->GetWorld()->DestroyBody(body);
}

void Entity::TakeDamage(float impact) {
    if (isDestroyed || type == EntityType::GROUND || type == EntityType::BIRD) return;

    if (impact > 5.0f) { // Lowered impact threshold so soft hits break Ice
        health -= impact; 
        if (health <= 0) isDestroyed = true;
    }
}

void Entity::SwapTexture(sf::Texture* newTexture) {
    if (newTexture && newTexture->getSize().x > 0) sprite.setTexture(*newTexture);
}

// Change the function signature in Entity.cpp:
void Entity::Render(sf::RenderTarget& window) {
    if (isDestroyed) return;
    b2Vec2 position = body->GetPosition();
    float angle = body->GetAngle();

    sprite.setPosition({position.x * SCALE, position.y * SCALE});
    sprite.setRotation(angle * 180.0f / 3.14159f);

    window.draw(sprite);
}