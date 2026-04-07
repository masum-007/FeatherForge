#include "PhysicsWorld.hpp"
#include "Entity.hpp"

// NEW: The collision logic
void GameContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
    // Get the maximum impulse force from the collision
    float maxImpulse = 0.0f;
    for (int i = 0; i < impulse->count; ++i) {
        maxImpulse = b2Max(maxImpulse, impulse->normalImpulses[i]);
    }

    // Get the two bodies that crashed
    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    // Retrieve our custom Entity objects from the Box2D bodies
    Entity* entityA = reinterpret_cast<Entity*>(bodyA->GetUserData().pointer);
    Entity* entityB = reinterpret_cast<Entity*>(bodyB->GetUserData().pointer);

    // Deal damage to both
    if (entityA) entityA->TakeDamage(maxImpulse);
    if (entityB) entityB->TakeDamage(maxImpulse);
}

PhysicsWorld::PhysicsWorld() 
    : gravity(0.0f, 9.8f), 
      world(gravity),
      timeStep(1.0f / 60.0f),
      velocityIterations(6),
      positionIterations(2) 
{
    // NEW: Tell Box2D to use our listener
    world.SetContactListener(&contactListener);
}

void PhysicsWorld::Update(float deltaTime) {
    world.Step(timeStep, velocityIterations, positionIterations);
}