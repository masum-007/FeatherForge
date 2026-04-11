#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <set>
#include "PhysicsWorld.hpp"
#include "Entity.hpp"
#include "ParticleSystem.hpp"
#include "EnvironmentSystem.hpp"
#include <nlohmann/json.hpp>
#include "CustomGraphics.hpp"

// --- ALL GAME STATES ---
enum class GameState { Menu, LevelSelect, Playing, LevelComplete, GameOver };
// --- ALL BIRD ABILITIES ---
enum class BirdType { Normal, Fire, Sloth, Freeze, Water };

class Game {
public:
    Game();
    void Run();

private:
    void ProcessEvents();
    void Update();
    void Render();
    
    void LoadAssets();
    void LoadLevel(const std::string& filepath);
    void SpawnBird();
    void CheckBirdState();
    
    void DrawSlingshot();
    void DrawTrajectory();
    void ResetLevel(int level);
    void DrawMenu();
    void DrawLevelSelect();
    void DrawLevelComplete();
    void DrawGameOver();


    // --- NEW: Post-Processing & Rendering ---
    sf::RenderTexture m_renderTexture;
    sf::Sprite m_renderSprite;
    sf::Shader m_postShader;

    // --- NEW: Camera Kinematics (Game Feel) ---
    float m_shakeTimer = 0.0f;
    float m_shakeMagnitude = 0.0f;
    float m_hitStopTimer = 0.0f;
    // --- NEW: Permanent Environmental Decals ---
    std::vector<sf::CircleShape> m_scorchMarks;

    void TriggerShake(float duration, float magnitude);
    void TriggerHitStop(float duration);

    sf::RenderWindow window;
    sf::View worldView;
    sf::View uiView;
    PhysicsWorld physics;
    
    std::unique_ptr<Entity> ground;
    std::vector<std::unique_ptr<Entity>> blocks;
    std::unique_ptr<Entity> bird;
    
    sf::Vector2f slingshotPos;
    bool isDragging;
    bool birdIsActive;
    int score;

    // --- ALL TEXTURES REQUIRED ---
    sf::Texture birdTex, birdFireTex, birdSlothTex, birdFreezeTex, birdWaterTex;
    sf::Texture woodTex, woodCrackedTex, iceTex, groundTex, enemyTex;
    sf::Texture menuBgTex;
    sf::Sprite menuBgSprite;
    sf::Font font;
    sf::Text scoreText;

    // --- NEW RUNE VARIABLES ---
    sf::Texture runeTex;
    sf::Sprite runeSprite;
    float runeRotation = 0.0f;

    ParticleSystem m_particles;
    EnvironmentSystem m_environment;

    GameState m_currentState = GameState::Menu;
    int m_currentLevel = 1;
    // Change this line in your private variables:
    const int MAX_LEVELS = 10;
    float m_levelTransitionTimer = 0.0f; 
    float m_birdActiveTimer = 0.0f;

    // --- ABILITY TRACKERS ---
    std::vector<BirdType> m_birdQueue;
    BirdType m_currentBirdType;
    bool m_birdAbilityUsed = false;
    int birdsRemaining;

    std::map<b2Body*, float> m_burningBodies;
    std::set<b2Body*> m_frozenBodies;
};