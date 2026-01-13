# CRayEngine 

This project is currently on very early development. Plan is to create a custom 2D game engine built in C11 using [Raylib](https://www.raylib.com/).

![License](https://img.shields.io/github/license/sosruko3/CRayEngine)
![Top Language](https://img.shields.io/github/languages/top/sosruko3/CRayEngine)
![Code Size](https://img.shields.io/github/languages/code-size/sosruko3/CRayEngine)

## Features (Current)
- **Core Engine:** 
  - Written in **C11**, prioritizing performance and simplicity.
  - **Gen-Index Entity System:** Uses a Data-Oriented pooling with generational indices (Handles) for safe compliance and cache-friendly access.
  - **Component Support:** Hybrid architecture supporting dedicated component arrays (like Animation components) parallel to the main entity store.
  - **Scene Management:** Finite State Machine (FSM) for handling Game, Menu, and Pause states seamlessly.
  - **Event/Logging System:** Thread-safe logging with multiple verbosity levels (`INFO`, `WARNING`, `ERROR`).

- **Animation System:**
  - Frame-based sprite animation built on top of the texture atlas.
  - Support for looping, one-shot, and variable playback speeds.
  - Automatic state flipping (flipX) based on velocity.

- **Physics Engine:**
  - **Spatial Hash Grid:** Broad-phase collision detection optimized for high entity counts.
  - **Sleep Optimization:** Entities automatically "sleep" when idle to reduce CPU usage, with intelligent "wake-up" logic on collision.
  - **Sub-stepping:** Physics simulation runs multiple steps per frame for stability at high velocities.
  - **Collision Layers:** Bit-mask filtering (Layers & Masks) to control which entities interact (e.g., Player vs Enemy, but not Enemy vs Enemy).
- **Automated Asset Pipeline:**
  - **Custom Asset Packer:** Node.js-based tool integrated directly into the build process.
  - **Automatic Generation:** Detects changes in `assets/raw_textures/` and rebuilds the texture atlas (`atlas.png`) and coordinate headers (`atlas_data.h`) automatically.
  - **CMake Integration:** Just run `cmake --build` and assets are handled for you.

- **Build System:**
  - Uses **CMake** for robust cross-platform support (Linux/Windows).
  - **Dependency Management:** Automatically fetches and builds **Raylib 5.5** from source using `FetchContent`.


## TODO
- [x] Scene Manager (State Machine for Menu/Gameplay switching)
- [x] Input Manager
- [x] Config Manager
- [x] Entity System
- [x] Collision System
- [x] Resource/Asset Manager
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
