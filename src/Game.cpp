#include "Game.hpp"
#include <iostream>

Game::Game() 
    : window(sf::VideoMode(1280, 720), "FeatherForge Engine"), 
      isDragging(false) 
{
    window.setFramerateLimit(60);

    // Initialize Objects
    // Ground 
    ground = std::make_unique<Entity>(*physics.GetWorld(), 640, 700, 1280, 40, EntityType::GROUND);

    // Physics Stack (The Tower) [cite: 116]
    blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), 900, 650, 50, 50, EntityType::WOOD));
    blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), 900, 600, 50, 50, EntityType::WOOD));
    blocks.push_back(std::make_unique<Entity>(*physics.GetWorld(), 900, 550, 50, 50, EntityType::WOOD));

    // The Bird
    bird = std::make_unique<Entity>(*physics.GetWorld(), 200, 500, 30, 30, EntityType::BIRD);
}

void Game::Run() {
    while (window.isOpen()) {
        ProcessEvents();
        Update();
        Render();
    }
}

void Game::ProcessEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
            
        // Slingshot Input Mechanics [cite: 54]
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            // Check if mouse is near bird to start drag
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            // 1. Get the position as a Box2D vector
            b2Vec2 b2Pos = bird->GetBody()->GetPosition(); 

            // 2. Convert it to an SFML vector and scale it to pixels at the same time
            sf::Vector2f birdPos(b2Pos.x * SCALE, b2Pos.y * SCALE);

            if (abs(mousePos.x - birdPos.x) < 50 && abs(mousePos.y - birdPos.y) < 50) {
                isDragging = true;
                dragStartPos = mousePos;
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (isDragging) {
                isDragging = false;
                // Launch Logic: Apply Impulse based on drag distance [cite: 84]
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                b2Vec2 force((dragStartPos.x - mousePos.x) * 0.5f, (dragStartPos.y - mousePos.y) * 0.5f);
                
                bird->GetBody()->SetAwake(true);
                bird->GetBody()->ApplyLinearImpulseToCenter(force, true);
            }
        }
    }
}

void Game::Update() {
    physics.Update(1.0f / 60.0f);
}

void Game::Render() {
    window.clear(sf::Color(135, 206, 235)); // Sky Blue Background

    // Draw Slingshot Line if dragging
    if (isDragging) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vertex line[] = {
            sf::Vertex(dragStartPos, sf::Color::Black),
            sf::Vertex(sf::Vector2f(mousePos.x, mousePos.y), sf::Color::Black)
        };
        window.draw(line, 2, sf::Lines);
    }

    ground->Render(window);
    bird->Render(window);
    for (auto& block : blocks) {
        block->Render(window);
    }

    window.display();
}