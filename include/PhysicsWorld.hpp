#pragma once
#include <box2d/box2d.h>

class PhysicsWorld {
public:
    PhysicsWorld();
    void Update(float deltaTime);
    b2World* GetWorld() { return &world; }

private:
    b2Vec2 gravity;
    b2World world;
    float timeStep;
    int32 velocityIterations;
    int32 positionIterations;
};