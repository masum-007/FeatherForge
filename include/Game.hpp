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
    void HandleSlingshotInput();

    sf::RenderWindow window;
    PhysicsWorld physics;
    
    // Game Objects
    std::unique_ptr<Entity> bird;
    std::vector<std::unique_ptr<Entity>> blocks;
    std::unique_ptr<Entity> ground;

    // Slingshot State
    bool isDragging;
    sf::Vector2f dragStartPos;
};