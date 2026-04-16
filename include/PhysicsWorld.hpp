#pragma once
#include <box2d/box2d.h>

// Listens for Box2D crashes
class GameContactListener : public b2ContactListener {
public:
    // PostSolve happens right after Box2D calculates how hard they hit
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
};

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
    
    GameContactListener contactListener; 
};