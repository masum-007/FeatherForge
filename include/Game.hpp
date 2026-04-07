// Add these at the very top of Game.hpp
#include <fstream>
#include <nlohmann/json.hpp>
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
    void LoadAssets(); // The new asset loader
    void LoadLevel(const std::string& filepath); // NEW FUNCTION
    // --- NEW CAMERA VARIABLES ---
    sf::View worldView;
    sf::View uiView;
    // Textures stored in memory
    sf::Texture birdTex;
    sf::Texture woodTex;
    sf::Texture groundTex;
    sf::Texture enemyTex; // NEW TEXTURE
// --- NEW UI VARIABLES ---
    sf::Font font;
    sf::Text scoreText;
    int score;

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