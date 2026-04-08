// Add these at the very top of Game.hpp
#include <fstream>
#include <nlohmann/json.hpp>
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "PhysicsWorld.hpp"
#include "Entity.hpp"
#include "ParticleSystem.hpp"
#include "EnvironmentSystem.hpp"

// --- NEW: Define Game States ---
enum class GameState {
    Menu,
    LevelSelect,
    Playing,
    LevelComplete
};
class Game {
public:
    Game();
    void Run();

private:
    void ProcessEvents();
    void Update();
    void Render();
    ParticleSystem m_particles;
    // New Features
    void SpawnBird();
    void CheckBirdState();
    void DrawSlingshot();
    void DrawTrajectory(); // <--- ADD THIS LINE
    void LoadAssets(); // The new asset loader
    void LoadLevel(const std::string& filepath); // NEW FUNCTION

    // --- NEW: Level & UI Functions ---
    void ResetLevel(int level);
    void DrawMenu();
    void DrawLevelSelect();
    void DrawLevelComplete();

    // --- NEW CAMERA VARIABLES ---
    sf::View worldView;
    sf::View uiView;
    // Textures stored in memory
    sf::Texture birdTex;
    sf::Texture woodTex;
    sf::Texture groundTex;
    sf::Texture enemyTex; // NEW TEXTURE
    sf::Texture woodCrackedTex; // NEW TEXTURE

// --- NEW UI VARIABLES ---
    sf::Font font;
    sf::Text scoreText;
    int score;
    // --- NEW ASSET VARIABLES ---
    sf::Texture menuBgTex; // Added menuBgTex
    sf::Sprite menuBgSprite; // Added Sprite for the background
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

    EnvironmentSystem m_environment;
    // --- NEW: State Variables ---
    GameState m_currentState = GameState::Menu;
    int m_currentLevel = 1;
    const int MAX_LEVELS = 6;

    // --- NEW TIMING VARIABLE ---
    float m_levelTransitionTimer = 0.0f; // Tracks time after last enemy dies
    
};