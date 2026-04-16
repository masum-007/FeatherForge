#include "PhysicsWorld.hpp"
#include "Entity.hpp"

void GameContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
    float maxImpulse = 0.0f;
    for (int i = 0; i < impulse->count; ++i) {
        maxImpulse = b2Max(maxImpulse, impulse->normalImpulses[i]);
    }

    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    Entity* entityA = reinterpret_cast<Entity*>(bodyA->GetUserData().pointer);
    Entity* entityB = reinterpret_cast<Entity*>(bodyB->GetUserData().pointer);

    //Damage Multipliers (Sloth does 50x damage) ---
    float dmgA = maxImpulse * (entityB ? entityB->GetDamageMultiplier() : 1.0f);
    float dmgB = maxImpulse * (entityA ? entityA->GetDamageMultiplier() : 1.0f);

    if (entityA) entityA->TakeDamage(dmgA);
    if (entityB) entityB->TakeDamage(dmgB);
}

PhysicsWorld::PhysicsWorld() 
    : gravity(0.0f, 9.8f), world(gravity), timeStep(1.0f / 60.0f), velocityIterations(6), positionIterations(2) 
{
    world.SetContactListener(&contactListener);
}

void PhysicsWorld::Update(float deltaTime) {
    world.Step(timeStep, velocityIterations, positionIterations);
}