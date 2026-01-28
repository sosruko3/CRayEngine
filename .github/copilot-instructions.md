# CRayEngine - AI Coding Agent Instructions

## Project Overview
A custom 2D game engine written in **C11** using **Raylib 5.5**. Prioritizes data-oriented design, cache efficiency, and high entity counts. Currently implements a Snake game demo.

## Architecture

### Entity-Component System (Data-Oriented)
- **SoA Layout**: Entity data stored in parallel arrays (`pos_x[]`, `pos_y[]`, `vel_x[]`, etc.) for SIMD-friendliness
- **Generational Indices**: `Entity` handles contain `{id, generation}` - always validate with `EntityManager_IsValid(e)` before access
- **Component Masks**: 64-bit masks define entity capabilities (`COMP_POSITION | COMP_VELOCITY | COMP_SPRITE`)
- **State Flags**: Behavioral flags + collision layer/mask packed into `uint64_t state_flags[]`
  ```

### Core Modules (`src/engine/core/`)
| Module | Purpose |
|--------|---------|
| `entity_manager` | O(1) entity creation/destruction via FreeList |
| `scene_manager` | FSM for scene transitions (Menu → Game → GameOver) |
| `cre_renderer` | Virtual canvas with atlas-based sprite rendering |
| `cre_camera` | Camera with viewport culling |
| `animationSystem` | Frame-based sprite animation from atlas |
| `input` | Action-based input with remappable bindings |
| `logger` | Thread-safe logging (`LOG_LVL_INFO/WARNING/ERROR/DEBUG`) |

### Physics (`src/engine/physics/`)
- **Spatial Hash Grid**: Broad-phase collision in 128px buckets
- **Sleep Optimization**: Idle entities skip physics via `FLAG_SLEEPING`
- **Collision Layers**: Bit-mask filtering (`L_PLAYER`, `L_ENEMY`, `L_BULLET`, etc.)

## Build System

```bash
# Standard build
cmake -B build && cmake --build build

# Run executable
./build/CRayEngine
```

- **CMake 3.14+** with `FetchContent` auto-downloads Raylib
- **Python3 required** for asset pipeline (auto-runs during build)
- **No in-source builds** - use `-B build` flag

## Asset Pipeline

Raw PNGs in `assets/raw_textures/` are automatically packed into a texture atlas:

1. Place sprites: `assets/raw_textures/enemy_idle.png`
2. Animation sequences: `character_zombie_run0.png`, `character_zombie_run1.png`, `character_zombie_run2.png`
3. Build triggers `tools/build_assets.py` → generates:
   - `build/generated/atlas_data.h` - `SpriteID` and `AnimID` enums
   - `build/generated/atlas_data.c` - sprite metadata arrays
   - `build/generated/atlas.png` - packed texture

Use generated enums: `SPR_ENEMY_IDLE`, `ANIM_CHARACTER_ZOMBIE_RUN`

## Key Patterns

### Adding New Scenes
1. Create `src/game/my_scene.c` with `MyScene_Init/Update/Draw/Unload`
2. Add state to `game_scenes.h` enum
3. Register in `Game_GetScene()` switch in [game_scenes.c](src/game/game_scenes.c)

## Configuration Files
- `src/engine/core/config.h` - Engine constants (`MAX_ENTITIES`, grid sizes, physics params)
- `src/game_config.h` - Game-specific settings (speeds, text, paths)
- `assets/config/settings.cfg` - Runtime config (input bindings)

## Conventions
- **Prefix functions** with module name: `EntityManager_`, `SceneManager_`, `creRenderer_`
- **Use `Log()`** instead of printf: `Log(LOG_LVL_INFO, "Spawned %d entities", count)`
- **64-byte alignment** for hot data arrays (cache line optimization)
- **Check entity validity** before accessing: `if (data) { ... }`
- **Direct SoA access** preferred in hot loops over `EntityManager_Get()`
