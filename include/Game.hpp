#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "PhysicsWorld.hpp"
#include "Entity.hpp"

class Game {
public:
    Game();
    void Run();

private:
    void ProcessEvents();
    void Update();
    void Render();
    
    // New Features
    void SpawnBird();
    void CheckBirdState();
    void DrawEnvironment();
    void DrawSlingshot();
    void DrawTrajectory(); // <--- ADD THIS LINE
    sf::RenderWindow window;
    PhysicsWorld physics;
    
    // Game Objects
    std::unique_ptr<Entity> bird;
    std::vector<std::unique_ptr<Entity>> blocks;
    std::unique_ptr<Entity> ground;

    // Slingshot State
    sf::Vector2f slingshotPos;
    bool isDragging;
    bool birdIsActive;
    int birdsRemaining;
};