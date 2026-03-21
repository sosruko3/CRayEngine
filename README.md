# CRayEngine 

Goal:Creating a custom high-performance 2D game engine in C++20 using [Raylib](https://www.raylib.com/).

This project is currently on early development. Focusing on Structure of Arrays(SoA),Data-oriented Design, Decoupled Systems, SIMD(in hot parts), Multithreading(in the future),Clean and Safe code. 

NOTE: I have switched to C++20 recently, i will be using "Orthodox C++20" rules, planning on focusing C-like code with some C++ features.
I will be using C++20 features that are zero-cost, or have very low costs, and provide significant benefits in terms of code clarity, safety, or maintainability over their C counterparts.

NOTE: AI Agent is being used as an helper for development, it is responsible for speeding up my coding process, not for any decision making in the engine's design. I am reviewing every code it has created, making sure it does exactly what i want, and doesn't decide on its own. I am the one who is responsible for the design and implementation of the engine, not the AI Agent.


![License](https://img.shields.io/github/license/sosruko3/CRayEngine)
![Top Language](https://img.shields.io/github/languages/top/sosruko3/CRayEngine)
![Code Size](https://img.shields.io/github/languages/code-size/sosruko3/CRayEngine)

## Features (Current)
//TODO

## Planned features
- [x] Log System.(integrates Raylib's logs too.)
- [x] Scene Manager (State Machine for Menu/Gameplay switching)
- [x] Input Manager
- [x] Config Manager
- [x] Entity System (ECS-Lite,SoA structure)
- [x] Collision System (AABB,Circle)
- [x] Spatial Hash (Dual-Layered(Static/Dynamic) ,Index-based object pool)
- [x] Physics System
- [x] Asset Manager (using TextureAtlas)
- [x] Animation System
- [x] Render System (using Raylib)
- [x] Camera System (ECS Component)
- [x] Command System
- [x] Viewport System
- [x] Profiler System
- [x] Audio System(using miniaudio)
- [x] Switching to C++20, using "Orthodox C++".

- [ ] Arena Allocator (For one-time memory allocation and splitting that to domains for required systems.)
- [ ] Asset Loader (will be using stb_image for loading textures, and custom loaders for other asset types.)
- [ ] Asset Registry (for keeping track of loaded assets and their references)
- [ ] Updating LogSystem with fmt library for better formatting and performance.
- [ ] Accumulator for fixed timestep updates in the main game loop.
- [ ] Switching Raylib with SDL3 and SDL3_GPU for better performance and more control over rendering and input.
- [ ] Switching to Box2D v3, why: It is better on every aspect and it uses my exact philosophy for physics engine design, and it is also more stable and has more features than my current physics system. It would take +6 month to reach that stage with my current physics system.

- [ ] Particle System
- [ ] AI System
- [ ] Save Manager 
- [ ] Post-processing/Shader System
- [ ] Lighting System (using Radiance Cascades)
- [ ] Editor (using Dear ImGui)
- [ ] Live coding feature for the editor.
- [ ] Animation Engine (Procedural Animation)
- [ ] Modding tools

## Building

### Prerequisites
* **CMake** (3.14+)
* C++ compiler that supprts C++20 (GCC 10+, Clang 10+, MSVC 19.28+)
* **Raylib** (Cmake fetches Raylib automatically)
* **Python3** (Required for Automated TextureAtlas script)
* **Pip and Pillow**(If not included in Python3)

### Instructions
1. Clone the repository:
   ```bash
   git clone https://github.com/sosruko3/CRayEngine.git
   cd CRayEngine
2. Generate build files and compile it.
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
