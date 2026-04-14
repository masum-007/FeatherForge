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
#include <SFML/Audio.hpp>


// 1. Add 'Options' to the GameState enum
enum class GameState { Menu, LevelSelect, Options, Playing, Paused, LevelComplete, GameOver };
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

// ... inside private variables ...
    bool m_musicEnabled = true;
    bool m_sfxEnabled = true;

    // --- NEW: Audio System ---
    sf::Music m_bgMusic;
    
    // Sound Buffers (The audio files stored in RAM)
    sf::SoundBuffer m_sbPull;
    sf::SoundBuffer m_sbRelease;
    sf::SoundBuffer m_sbWood;
    sf::SoundBuffer m_sbIce;
    sf::SoundBuffer m_sbExplode;

    // Sound Pool (The "speakers" that play the audio)
    std::vector<sf::Sound> m_soundPool;
    
    // Helper function to play overlapping sounds
    void PlaySFX(const sf::SoundBuffer& buffer, float volume = 100.f, float pitch = 1.0f);
    void UpdateThemeMusic();

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
    // --- NEW: Dynamic Star Scores ---
    int m_targetScore2Star = 1500;
    int m_targetScore3Star = 3000;
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

    // --- NEW: Cinematic Camera Variables ---
    sf::Vector2f m_cameraPos;
    float m_cameraZoom = 1.0f;

    sf::SoundBuffer m_sbThud;
    sf::SoundBuffer m_sbBirdPoof;
    sf::SoundBuffer m_sbEnemyKill;
    // --- NEW: Track where we came from when opening Options ---
    GameState m_previousState = GameState::Menu;
    // --- NEW: Pause Menu Draw Function ---
    void DrawPauseMenu();
    
    // --- NEW: Options Draw Function ---
    void DrawOptions();
};
