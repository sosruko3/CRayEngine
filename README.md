# CRayEngine 

Goal:Creating a custom high-performance 2D game engine in C11 using [Raylib](https://www.raylib.com/).

This project is currently on early development. Focusing on Structure of Arrays(SoA),Data-oriented Design, Decoupled Systems, SIMD(in hot parts), Multithreading(in the future),Clean and Safe code. 

NOTE: I am designing,planning and reviewing while using AI as an helper. AI's job is helping me in the process, not "vibecoding" whole project.
(Python script is %100 vibecoded. "not vibecoded" C version is planned in the future.)

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
- [x] Camera System (using Raylib)
- [x] Command System
- [x] Viewport System
- [ ] Profiler System
- [ ] Audio System(using Raylib)
- [ ] Particle System
- [ ] AI Systems
- [ ] Raycasting
- [ ] Save Manager 
- [ ] Time System (?)
- [ ] Post-processing/Shader System
- [ ] Lighting System
- [ ] Editor (using Nuklear)
- [ ] Live coding.
- [ ] Animation Engine (Procedural Animation)
- [ ] Modding tools
- [ ] Refactoring Audio System with miniaudio.

## Building

### Prerequisites
* **CMake** (3.14+)
* **C Compiler** (GCC, Clang, or MSVC)
* **Raylib** (Cmake fetches Raylib automatically)
* **Python3** (Required for Automated textureAtlas script)
* **Pip and Pillow**(If not included in Python3)

### Instructions
1. Clone the repository:
   ```bash
   git clone https://github.com/sosruko3/CRayEngine.git
   cd CRayEngine
2. Generate build files and compile it.
```bash
cmake -B build
cmake --build build
