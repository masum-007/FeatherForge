#include "PhysicsWorld.hpp"

PhysicsWorld::PhysicsWorld() 
    : gravity(0.0f, 9.8f), // Standard earth gravity
      world(gravity),
      timeStep(1.0f / 60.0f),
      velocityIterations(6),
      positionIterations(2) 
{
    // Initialize world
}

void PhysicsWorld::Update(float deltaTime) {
    // Box2D steps at a fixed rate for stability
    world.Step(timeStep, velocityIterations, positionIterations);
}