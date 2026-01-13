# CRayEngine 

This project is currently on very early development. Plan is to create a custom 2D game engine built in C11 using [Raylib](https://www.raylib.com/).

![License](https://img.shields.io/github/license/sosruko3/CRayEngine)
![Top Language](https://img.shields.io/github/languages/top/sosruko3/CRayEngine)
![Code Size](https://img.shields.io/github/languages/code-size/sosruko3/CRayEngine)

## Features (Current)
- **Log System:** Thread-safe logging with file history, rotation, and multiple verbosity levels (`INFO`, `WARNING`, `ERROR`).
- **Scene Manager:**
- **Input Manager:** 
- **Configuration:** Externalized game settings via header config.
- **Cross-Platform:** CMake build system supporting Windows and Linux.

## TODO
- [x] Scene Manager (State Machine for Menu/Gameplay switching)
- [x] Input Manager
- [x] Config Manager
- [x] Entity System
- [x] Collision System
- [ ] Resource/Asset Manager
- [ ] Audio Mixer
- [ ] UI
- [ ] Integrate RayGUI library for GUI
- [ ] Integrate an Physics Engine(Chipmunk2D)
- [ ] Achievement system
- [ ] Shader System???

## Building

### Prerequisites
* **CMake** (3.10+)
* **C Compiler** (GCC, Clang, or MSVC)
* **Raylib** (Must be installed or linked)
* **Node.js and NPM** (Required for free-tex-packer-cli)

### Instructions
1. Clone the repository:
   ```bash
   git clone https://github.com/sosruko3/CRayEngine.git
   cd CRayEngine
2. Generate build files and compile it.
```bash
cmake -B build
cmake --build build
