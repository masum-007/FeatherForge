#include "Entity.hpp"

Entity::Entity(b2World& world, float x, float y, float w, float h, EntityType type, sf::Texture* texture) 
    : width(w), height(h), type(type), isDestroyed(false), outgoingDamageMultiplier(1.0f) { 
    
    // ---Ice is extremely fragile---
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
        // Ice is lighter, Wood is heavier ---
        if (type == EntityType::WOOD) fixtureDef.density = 0.8f;
        else if (type == EntityType::ICE) fixtureDef.density = 0.3f; 
    }

    body->CreateFixture(&fixtureDef);

    // texture logic :
    if (texture) {
        sprite.setTexture(*texture);
        
        if (type == EntityType::GROUND) {
            // 1. Tell the sprite to repeat the texture until it hits the full width (w)
            sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(w), static_cast<int>(texture->getSize().y)));
            
            // 2. Fix the Origin: It must be half of the NEW width (w), not the texture width
            sprite.setOrigin({ w / 2.0f, texture->getSize().y / 2.0f });
            
            // 3. Scale only the height to match 'h', leave width at 1.0 because TextureRect handles it
            baseScale = { 1.0f, h / texture->getSize().y };
        } else {
            // Normal scaling for birds and blocks
            baseScale = { w / texture->getSize().x, h / texture->getSize().y };
            sprite.setOrigin({ texture->getSize().x / 2.0f, texture->getSize().y / 2.0f });
        }
        
        sprite.setScale(baseScale);
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

void Entity::Render(sf::RenderTarget& window) {
    if (isDestroyed) return;
    b2Vec2 position = body->GetPosition();
    float angle = body->GetAngle();

    sprite.setPosition({position.x * SCALE, position.y * SCALE});

    // --- NEW: Soft-Body Squash and Stretch for Birds ---
    if (type == EntityType::BIRD && body->IsAwake()) {
        b2Vec2 vel = body->GetLinearVelocity();
        float speed = vel.Length();
        
        if (speed > 2.0f) {
            // Stretch along the velocity axis, squash perpendicular to it
            float stretch = 1.0f + (speed * 0.015f);
            float squash = 1.0f / stretch;
            
            // Clamp so it doesn't turn into a spaghetti noodle
            stretch = std::min(stretch, 1.5f);
            squash = std::max(squash, 0.5f);

            // Rotate the bird to literally face its movement trajectory
            float moveAngle = std::atan2(vel.y, vel.x);
            sprite.setRotation(moveAngle * 180.0f / 3.14159f);
            sprite.setScale({baseScale.x * stretch, baseScale.y * squash});
        } else {
            // Idle/Slow: Return to normal size and Box2D rotation
            sprite.setRotation(angle * 180.0f / 3.14159f);
            sprite.setScale(baseScale);
        }
    } else {
        // Wood, Ice, Enemies, Ground
        sprite.setRotation(angle * 180.0f / 3.14159f);
        sprite.setScale(baseScale);
    }

    window.draw(sprite);
}