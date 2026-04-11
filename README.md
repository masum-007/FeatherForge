# FeatherForge

> A 2D Game Engine & Physics-Based Destruction Game (C++)

## Overview
FeatherForge is a 2D game engine and physics-based destruction game developed entirely in C++ as part of a Computer Graphics project. 

Inspired by Angry Birds, this project goes beyond a simple demo and focuses on building a reusable mini game engine with realistic physics, destructible environments, and visually rich graphics.

## Features

### Core Gameplay
- Slingshot-based projectile launching
- Multiple enemy targets
- Destructible structures
- Score-based progression system

### Engine Systems

**Engine Architecture**
- Scene Management (menu, gameplay, transitions)
- Entity-Component System (ECS)
- Asset Management (textures, fonts, audio)
- Layered Rendering System
- Dynamic Camera (follow, zoom, screen shake)

**Physics System**
- Rigid body dynamics (gravity, friction, collision)
- Material-based destruction:
  - Wood (medium strength)
  - Glass (fragile)
  - Stone (strong)
- Progressive destruction (crack break fragments)

### Graphics & Visuals
- Parallax scrolling backgrounds (multi-layer depth)
- Animated environments (clouds, birds, etc.)
- Particle effects (dust, debris, trails)
- Smooth camera transitions and effects

## Tech Stack
- **Language:** C++
- **Graphics Library:** SFML
- **Physics Engine:** Box2D
- **Data Format:** JSON

## Gameplay Mechanics
1. Drag to aim using the slingshot
2. Release to launch the projectile
3. Camera dynamically follows the projectile
4. Destroy structures to eliminate targets
5. Earn scores based on performance
6. Complete levels with star ratings

## Project Structure

```text
FeatherForge/
├── src/
│   ├── engine/
│   ├── physics/
│   ├── rendering/
│   └── gameplay/
├── assets/
│   ├── textures/
│   ├── audio/
│   └── levels/
├── include/
├── main.cpp
└── README.md
```
## How to Run

### Requirements
* C++ Compiler (GCC/MSVC)
* SFML installed
* Box2D installed

### Steps

```text
git clone [https://github.com/masum-007/FeatherForge.git](https://github.com/masum-007/FeatherForge.git)
cd FeatherForge
```

1. Build the project using your compiler or IDE
2. Run the generated executable

## Key Concepts Implemented
* Game Loop Design
* Collision Detection & Response
* Physics Simulation (Box2D Integration)
* Rendering Pipeline
* Camera Systems
* Entity Component System (ECS)
* JSON-based Level Handling

## Project Achievements
* Built a mini 2D game engine from scratch
* Implemented realistic physics
* Designed destructible environments
* Developed multiple playable levels
* Applied advanced computer graphics concepts

## Author
**Masum Al Mahamud**
* CSE Undergraduate
* Software Developer | AI/ML Enthusiast

## Future Improvements
* GUI-based level editor
* Advanced shaders & post-processing
* More enemy types and abilities
* Sound and music enhancements
* Performance optimization

## License
This project is developed for academic and educational purposes.
