#!/usr/bin/env python3
"""
Asset Pipeline Script for CRayEngine v2.3 (Final Specification)
================================================================
Converts raw PNG textures into a single texture atlas and generates
C header/source files for the game engine.

Features:
    - Shelf Packing algorithm for efficient atlas layout
    - Automatic sprite trimming with offset preservation
    - Edge Extrusion to prevent bleeding artifacts
    - Content-based deduplication with pixel normalization
    - Animation sequence detection with gap validation
    - Name collision protection for C identifiers
    - Debug HTML visualization
    - Atlas density metrics and fill efficiency logging
    - Fail-fast architecture (no silent failures)
    - Memory-safe context managers for image handling
    - Trimmed dimensions (w, h) in SpriteMeta for rendering

Usage:
    python build_assets.py [--input DIR] [--output DIR] [--size SIZE] [--padding N]

Author: Asset Pipeline Tool v2.3
"""

import argparse
import hashlib
import logging
import os
import re
import sys
import traceback
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

try:
    from PIL import Image
except ImportError:
    print("ERROR: Pillow library is required. Install with: pip install Pillow")
    sys.exit(1)


# =============================================================================
# Logging Configuration
# =============================================================================

def setup_logging(verbose: bool = False) -> logging.Logger:
    """Configure logging with appropriate level and format."""
    logger = logging.getLogger("AssetPipeline")
    logger.setLevel(logging.DEBUG if verbose else logging.INFO)
    
    # Console handler
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(logging.DEBUG if verbose else logging.INFO)
    
    # Format with timestamps for debugging
    if verbose:
        formatter = logging.Formatter(
            '%(asctime)s [%(levelname)s] %(message)s',
            datefmt='%H:%M:%S'
        )
    else:
        formatter = logging.Formatter('%(message)s')
    
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)
    
    return logger


# Global logger instance
log: logging.Logger = None  # type: ignore


# =============================================================================
# Configuration & Data Structures
# =============================================================================

@dataclass
class SpriteData:
    """Holds metadata for a single sprite after processing."""
    name: str                    # Original filename without extension
    original_path: Path          # Full path to source file
    original_width: int          # Original image width
    original_height: int         # Original image height
    trimmed_image: Image.Image   # The cropped/trimmed PIL Image
    trimmed_width: int           # Width after trimming
    trimmed_height: int          # Height after trimming
    offset_x: int                # X offset from original top-left to trimmed top-left
    offset_y: int                # Y offset from original top-left to trimmed top-left
    atlas_x: int = 0             # Final X position in atlas (of extruded image)
    atlas_y: int = 0             # Final Y position in atlas (of extruded image)
    sprite_id: int = 0           # Assigned sprite ID
    content_hash: str = ""       # MD5 hash of normalized pixel data for deduplication
    is_duplicate: bool = False   # Whether this sprite is a duplicate of another
    duplicate_of: Optional[int] = None  # Sprite ID of the original if this is a duplicate
    extruded_image: Optional[Image.Image] = None  # Image with edge extrusion applied
    c_identifier: str = ""       # The sanitized C identifier (e.g., "SPR_ORC")


@dataclass
class AnimationDef:
    """Holds metadata for an animation group."""
    name: str                    # Animation name (e.g., "soldier_run")
    start_sprite_id: int         # ID of first frame
    frame_count: int             # Number of frames
    default_speed: float = 0.1   # Default animation speed (seconds per frame)
    loop: bool = True            # Whether animation loops


class ShelfPacker:
    """
    Simple Shelf Packing Algorithm for 2D bin packing.
    
    Places rectangles left-to-right in horizontal "shelves".
    When a rectangle doesn't fit on the current shelf, starts a new shelf.
    """
    
    def __init__(self, atlas_width: int, atlas_height: int, padding: int = 2):
        self.atlas_width = atlas_width
        self.atlas_height = atlas_height
        self.padding = padding
        
        # Current position tracking
        self.current_x = padding
        self.current_y = padding
        self.current_shelf_height = 0
        
        # Statistics
        self.total_sprites_packed = 0
    
    def pack(self, width: int, height: int) -> Optional[tuple[int, int]]:
        """
        Attempt to pack a rectangle of given dimensions.
        
        Returns:
            (x, y) position if successful, None if doesn't fit.
        """
        padded_width = width + self.padding
        padded_height = height + self.padding
        
        # Check if it fits on current shelf
        if self.current_x + padded_width <= self.atlas_width:
            # Fits on current shelf
            x, y = self.current_x, self.current_y
            self.current_x += padded_width
            self.current_shelf_height = max(self.current_shelf_height, padded_height)
            self.total_sprites_packed += 1
            return (x, y)
        
        # Need a new shelf
        new_shelf_y = self.current_y + self.current_shelf_height
        
        # Check if new shelf fits vertically
        if new_shelf_y + padded_height > self.atlas_height:
            return None  # Doesn't fit
        
        # Check if it fits horizontally on new shelf
        if padded_width > self.atlas_width:
            return None  # Too wide for atlas
        
        # Start new shelf
        self.current_x = self.padding + padded_width
        self.current_y = new_shelf_y
        self.current_shelf_height = padded_height
        self.total_sprites_packed += 1
        
        return (self.padding, new_shelf_y)


# =============================================================================
# Core Functions
# =============================================================================

def find_png_files(input_dir: Path) -> list[Path]:
    """Find all PNG files in the input directory."""
    if not input_dir.exists():
        log.error(f"Input directory does not exist: {input_dir}")
        sys.exit(1)
    
    png_files = sorted(input_dir.glob("*.png"))
    
    if not png_files:
        log.warning(f"No PNG files found in {input_dir}")
    
    return png_files


def normalize_pixels_for_hash(image: Image.Image) -> bytes:
    """
    Normalize pixel data for robust deduplication hashing.
    
    Problem: Two invisible pixels might have different RGB data:
    RGBA(0,0,0,0) vs RGBA(255,255,255,0) - both are fully transparent
    but would hash differently.
    
    Solution: For any pixel where Alpha == 0, force RGB to (0,0,0).
    This ensures identical visual appearance = identical hash.
    """
    # Get pixel data as a mutable list
    pixels = list(image.getdata())
    normalized = []
    
    for r, g, b, a in pixels:
        if a == 0:
            # Fully transparent - normalize RGB to zero
            normalized.append((0, 0, 0, 0))
        else:
            normalized.append((r, g, b, a))
    
    # Convert back to bytes for hashing
    # Pack as RGBA bytes
    byte_data = bytearray()
    for r, g, b, a in normalized:
        byte_data.extend([r, g, b, a])
    
    return bytes(byte_data)


def load_and_trim_sprite(file_path: Path) -> SpriteData:
    """
    Load a PNG file, trim transparent edges, and calculate offsets.
    
    Uses image.getbbox() to find the non-transparent bounding box.
    Uses context manager for memory-safe image handling.
    Preserves original_width and original_height exactly from source file.
    
    Fail-fast: Any error aborts the build immediately.
    """
    log.debug(f"Loading sprite: {file_path}")
    
    # Use context manager for proper resource cleanup
    try:
        with Image.open(file_path) as raw_image:
            image = raw_image.convert("RGBA")
            # Preserve original dimensions exactly from source file
            original_width, original_height = raw_image.size
    except FileNotFoundError:
        log.error(f"[ERROR] File not found: {file_path}")
        log.error(f"  Aborting build - file missing")
        sys.exit(1)
    except Exception as e:
        log.error(f"[ERROR] Failed to open image: {file_path}")
        log.error(f"  Exception: {type(e).__name__}: {e}")
        traceback.print_exc()
        sys.exit(1)
    
    log.debug(f"  Original size: {original_width}x{original_height}")
    
    # Get bounding box of non-transparent content
    try:
        bbox = image.getbbox()
        log.debug(f"  Bounding box: {bbox}")
    except Exception as e:
        log.error(f"[ERROR] Failed to get bounding box: {file_path}")
        log.error(f"  Exception: {type(e).__name__}: {e}")
        traceback.print_exc()
        sys.exit(1)
    
    if bbox is None:
        # Completely transparent image - keep 1x1 pixel
        log.warning(f"[WARN] '{file_path.name}' is fully transparent, using 1x1 placeholder")
        trimmed_image = Image.new("RGBA", (1, 1), (0, 0, 0, 0))
        offset_x, offset_y = 0, 0
        trimmed_width, trimmed_height = 1, 1
    else:
        # Crop to bounding box
        try:
            trimmed_image = image.crop(bbox)
            offset_x = bbox[0]
            offset_y = bbox[1]
            trimmed_width = bbox[2] - bbox[0]
            trimmed_height = bbox[3] - bbox[1]
            log.debug(f"  Trimmed to: {trimmed_width}x{trimmed_height}, offset: ({offset_x}, {offset_y})")
        except Exception as e:
            log.error(f"[ERROR] Failed to crop image: {file_path}")
            log.error(f"  BBox: {bbox}")
            log.error(f"  Exception: {type(e).__name__}: {e}")
            traceback.print_exc()
            sys.exit(1)
    
    name = file_path.stem  # Filename without extension
    
    # Compute content hash with pixel normalization for robust deduplication
    try:
        normalized_bytes = normalize_pixels_for_hash(trimmed_image)
        content_hash = hashlib.md5(normalized_bytes).hexdigest()
        log.debug(f"  Content hash: {content_hash[:16]}...")
    except Exception as e:
        log.error(f"[ERROR] Failed to compute hash for: {file_path}")
        log.error(f"  Exception: {type(e).__name__}: {e}")
        traceback.print_exc()
        sys.exit(1)
    
    return SpriteData(
        name=name,
        original_path=file_path,
        original_width=original_width,
        original_height=original_height,
        trimmed_image=trimmed_image,
        trimmed_width=trimmed_width,
        trimmed_height=trimmed_height,
        offset_x=offset_x,
        offset_y=offset_y,
        content_hash=content_hash
    )


def apply_edge_extrusion(sprite: SpriteData) -> None:
    """
    Apply edge extrusion to prevent texture bleeding.
    
    Creates a new image that is (width+2) x (height+2), pastes the original
    sprite at (1, 1), and smears/repeats edge pixels into the 1px border.
    
    This prevents dark edges when rendering at sub-pixel coordinates or
    when the camera zooms/moves.
    
    Memory Safety: Discards trimmed_image after extrusion to free RAM.
    Fail-fast: Any error aborts the build immediately.
    """
    log.debug(f"Applying extrusion to: {sprite.name} ({sprite.trimmed_width}x{sprite.trimmed_height})")
    
    try:
        src = sprite.trimmed_image
        w, h = sprite.trimmed_width, sprite.trimmed_height
        
        # Create new image with 1px border on all sides
        extruded = Image.new("RGBA", (w + 2, h + 2), (0, 0, 0, 0))
        
        # Paste original sprite at center (1, 1)
        extruded.paste(src, (1, 1))
        
        # Get pixel access for efficient manipulation
        src_pixels = src.load()
        ext_pixels = extruded.load()
        
        # Smear top edge (copy row 0 to row -1, i.e., y=0 in extruded)
        for x in range(w):
            ext_pixels[x + 1, 0] = src_pixels[x, 0]
        
        # Smear bottom edge (copy last row to row below)
        for x in range(w):
            ext_pixels[x + 1, h + 1] = src_pixels[x, h - 1]
        
        # Smear left edge (copy column 0 to column -1)
        for y in range(h):
            ext_pixels[0, y + 1] = src_pixels[0, y]
        
        # Smear right edge (copy last column to column after)
        for y in range(h):
            ext_pixels[w + 1, y + 1] = src_pixels[w - 1, y]
        
        # Handle the 4 corners
        # Top-left corner: copy from (0, 0) of source
        ext_pixels[0, 0] = src_pixels[0, 0]
        # Top-right corner: copy from (w-1, 0) of source
        ext_pixels[w + 1, 0] = src_pixels[w - 1, 0]
        # Bottom-left corner: copy from (0, h-1) of source
        ext_pixels[0, h + 1] = src_pixels[0, h - 1]
        # Bottom-right corner: copy from (w-1, h-1) of source
        ext_pixels[w + 1, h + 1] = src_pixels[w - 1, h - 1]
        
        sprite.extruded_image = extruded
        log.debug(f"  Extruded size: {w + 2}x{h + 2}")
        
        # Memory Safety: Discard the trimmed_image to free RAM
        # The extruded_image now contains all needed data
        sprite.trimmed_image.close()
        sprite.trimmed_image = None
        
    except Exception as e:
        log.error(f"[ERROR] Failed to apply extrusion to: {sprite.name}")
        log.error(f"  Dimensions: {sprite.trimmed_width}x{sprite.trimmed_height}")
        log.error(f"  Path: {sprite.original_path}")
        log.error(f"  Exception: {type(e).__name__}: {e}")
        traceback.print_exc()
        sys.exit(1)


def deduplicate_sprites(sprites: list[SpriteData]) -> tuple[list[SpriteData], int]:
    """
    Identify and mark duplicate sprites based on content hash.
    
    Returns:
        Tuple of (list of unique sprites to pack, count of duplicates found)
    """
    hash_to_sprite: dict[str, int] = {}  # hash -> sprite_id of first occurrence
    unique_sprites = []
    duplicate_count = 0
    
    for sprite in sprites:
        if sprite.content_hash in hash_to_sprite:
            # This is a duplicate
            original_id = hash_to_sprite[sprite.content_hash]
            sprite.is_duplicate = True
            sprite.duplicate_of = original_id
            duplicate_count += 1
        else:
            # First occurrence of this content
            hash_to_sprite[sprite.content_hash] = sprite.sprite_id
            unique_sprites.append(sprite)
    
    return unique_sprites, duplicate_count


def detect_animations(sprites: list[SpriteData], strict: bool = True) -> tuple[list[AnimationDef], list[str]]:
    """
    Detect animation sequences from sprite names with strict gap validation.
    
    Naming convention: base_name_NN.png where NN is frame number.
    Examples:
        soldier_run_00.png, soldier_run_01.png -> animation "soldier_run"
        player_idle0.png, player_idle1.png -> animation "player_idle"
    
    Args:
        sprites: List of all sprites
        strict: If True, returns errors for frame gaps; if False, just warnings
    
    Returns:
        Tuple of (list of valid animations, list of error messages)
    """
    # Pattern to match: name ending with digits (with optional underscore)
    # Matches: "soldier_run_00", "player_idle0", "character_zombie_run2"
    pattern = re.compile(r'^(.+?)_?(\d+)$')
    
    # Group sprites by animation base name
    anim_groups: dict[str, list[tuple[int, int, str]]] = defaultdict(list)
    
    for sprite in sprites:
        match = pattern.match(sprite.name)
        if match:
            base_name = match.group(1)
            frame_num = int(match.group(2))
            anim_groups[base_name].append((frame_num, sprite.sprite_id, sprite.name))
    
    animations = []
    errors = []
    
    for base_name, frames in sorted(anim_groups.items()):
        if len(frames) < 2:
            # Single frame doesn't count as animation
            continue
        
        # Sort by frame number
        frames.sort(key=lambda x: x[0])
        frame_numbers = [f[0] for f in frames]
        sprite_ids = [f[1] for f in frames]
        sprite_names = [f[2] for f in frames]
        
        log.debug(f"Analyzing animation '{base_name}': frames {frame_numbers}")
        
        # Check for gaps in frame numbers
        expected_start = frame_numbers[0]
        missing_frames = []
        for i, expected in enumerate(range(expected_start, expected_start + len(frames))):
            if i < len(frame_numbers) and frame_numbers[i] != expected:
                # Find all missing frames up to current
                for missing in range(expected, frame_numbers[i]):
                    missing_frames.append(missing)
        
        # Also check for gaps by comparing consecutive frame numbers
        for i in range(1, len(frame_numbers)):
            if frame_numbers[i] != frame_numbers[i-1] + 1:
                for gap in range(frame_numbers[i-1] + 1, frame_numbers[i]):
                    if gap not in missing_frames:
                        missing_frames.append(gap)
        
        if missing_frames:
            missing_frames.sort()
            error_msg = (
                f"Animation '{base_name}' is missing frame(s): {missing_frames}. "
                f"Found frames: {frame_numbers}"
            )
            errors.append(error_msg)
            log.error(f"  ANIMATION GAP: {error_msg}")
            continue  # Skip this animation
        
        # Verify consecutive sprite IDs (they should be, since we sorted alphabetically)
        start_id = sprite_ids[0]
        is_consecutive = all(
            sprite_ids[i] == start_id + i 
            for i in range(len(sprite_ids))
        )
        
        if is_consecutive:
            animations.append(AnimationDef(
                name=base_name,
                start_sprite_id=start_id,
                frame_count=len(frames),
                default_speed=0.1,
                loop=True
            ))
            log.debug(f"  Valid animation: {base_name} ({len(frames)} frames, start_id={start_id})")
        else:
            error_msg = (
                f"Animation '{base_name}' has non-consecutive sprite IDs. "
                f"Frame names: {sprite_names}, IDs: {sprite_ids}"
            )
            errors.append(error_msg)
            log.warning(f"  {error_msg}")
    
    return animations, errors


def pack_sprites(sprites: list[SpriteData], atlas_size: int, padding: int) -> Image.Image:
    """
    Pack all sprites into an atlas using shelf packing algorithm.
    
    Uses extruded images (with edge extrusion) for packing.
    The extruded image is (trimmed_width + 2) x (trimmed_height + 2).
    
    Returns the completed atlas image.
    """
    log.debug(f"Packing {len(sprites)} sprites into {atlas_size}x{atlas_size} atlas")
    
    # Sort by extruded height (descending) for better shelf packing efficiency
    # But maintain original order for ID assignment
    pack_order = sorted(
        range(len(sprites)),
        key=lambda i: sprites[i].trimmed_height + 2,  # +2 for extrusion
        reverse=True
    )
    
    packer = ShelfPacker(atlas_size, atlas_size, padding)
    
    # Create transparent atlas
    atlas = Image.new("RGBA", (atlas_size, atlas_size), (0, 0, 0, 0))
    
    failed_sprites = []
    
    for pack_idx, idx in enumerate(pack_order):
        sprite = sprites[idx]
        
        # Pack the extruded dimensions
        extruded_w = sprite.trimmed_width + 2
        extruded_h = sprite.trimmed_height + 2
        
        log.debug(f"  [{pack_idx+1}/{len(sprites)}] Packing '{sprite.name}' ({extruded_w}x{extruded_h})")
        
        position = packer.pack(extruded_w, extruded_h)
        
        if position is None:
            log.error(f"  FAILED to pack: {sprite.name} ({extruded_w}x{extruded_h})")
            log.error(f"    Current shelf: x={packer.current_x}, y={packer.current_y}, height={packer.current_shelf_height}")
            failed_sprites.append(sprite)
            continue
        
        sprite.atlas_x = position[0]
        sprite.atlas_y = position[1]
        log.debug(f"    Placed at: ({sprite.atlas_x}, {sprite.atlas_y})")
        
        # Paste the extruded sprite into atlas
        try:
            atlas.paste(sprite.extruded_image, (sprite.atlas_x, sprite.atlas_y))
        except Exception as e:
            log.error(f"Failed to paste sprite into atlas: {sprite.name}")
            log.error(f"  Position: ({sprite.atlas_x}, {sprite.atlas_y})")
            log.error(f"  Extruded size: {extruded_w}x{extruded_h}")
            log.error(f"  Error: {e}")
            raise
    
    if failed_sprites:
        log.error(f"\nFailed to pack {len(failed_sprites)} sprites (atlas too small):")
        for sprite in failed_sprites[:10]:
            log.error(f"  - {sprite.name} ({sprite.trimmed_width + 2}x{sprite.trimmed_height + 2})")
        if len(failed_sprites) > 10:
            log.error(f"  ... and {len(failed_sprites) - 10} more")
        sys.exit(1)
    
    return atlas


def calculate_space_savings(sprites: list[SpriteData]) -> tuple[int, int, float]:
    """Calculate how much space was saved by trimming."""
    original_total = sum(s.original_width * s.original_height for s in sprites)
    trimmed_total = sum(s.trimmed_width * s.trimmed_height for s in sprites)
    
    if original_total == 0:
        return 0, 0, 0.0
    
    saved = original_total - trimmed_total
    percentage = (saved / original_total) * 100
    
    return original_total, trimmed_total, percentage


def sanitize_name_for_c(name: str) -> str:
    """Convert a sprite name to a valid C identifier."""
    # Replace non-alphanumeric chars with underscore
    sanitized = re.sub(r'[^a-zA-Z0-9]', '_', name)
    # Ensure it starts with a letter or underscore
    if sanitized and sanitized[0].isdigit():
        sanitized = '_' + sanitized
    return sanitized.upper()


def check_name_collisions(sprites: list[SpriteData]) -> list[str]:
    """
    Check for C identifier collisions and assign c_identifier to each sprite.
    
    Problem: If we have assets/units/orc.png and assets/bosses/orc.png,
    both would generate SPR_ORC, causing C compiler errors.
    
    Returns:
        List of error messages (empty if no collisions)
    """
    identifier_registry: dict[str, list[SpriteData]] = defaultdict(list)
    errors = []
    
    for sprite in sprites:
        c_name = sanitize_name_for_c(sprite.name)
        full_identifier = f"SPR_{c_name}"
        sprite.c_identifier = full_identifier
        identifier_registry[full_identifier].append(sprite)
    
    # Check for collisions
    for identifier, sprite_list in identifier_registry.items():
        if len(sprite_list) > 1:
            paths = [str(s.original_path) for s in sprite_list]
            error_msg = (
                f"Name Collision! '{identifier}' is generated by multiple files:\n" +
                "\n".join(f"    - {p}" for p in paths) +
                "\n  Please rename one of these files."
            )
            errors.append(error_msg)
    
    return errors


def calculate_atlas_density(unique_sprites: list[SpriteData], atlas_size: int) -> tuple[float, float, int]:
    """
    Calculate the atlas fill efficiency.
    
    Returns:
        Tuple of (density_percent, wasted_percent, total_extruded_pixels)
    """
    # Calculate total extruded pixels used
    total_extruded_px = sum(
        (s.trimmed_width + 2) * (s.trimmed_height + 2)
        for s in unique_sprites
    )
    
    atlas_total_px = atlas_size * atlas_size
    density = (total_extruded_px / atlas_total_px) * 100
    wasted = 100 - density
    
    return density, wasted, total_extruded_px


# =============================================================================
# Code Generation
# =============================================================================

def generate_header(sprites: list[SpriteData], animations: list[AnimationDef]) -> str:
    """Generate the atlas_data.h header file content."""
    lines = [
        "// ============================================================================",
        "// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY",
        "// Generated by build_assets.py v2.3",
        "// ============================================================================",
        "",
        "#ifndef ATLAS_DATA_H",
        "#define ATLAS_DATA_H",
        "",
        "#include <stdint.h>",
        "",
        "// ---------------------------------------------------------------------------",
        "// Sprite Metadata Structure",
        "// ---------------------------------------------------------------------------",
        "",
        "typedef struct {",
        "    uint16_t x, y;         // Position in the atlas (clean pixels, inside extrusion)",
        "    uint16_t w, h;         // Trimmed dimensions (visible pixels to render)",
        "    uint16_t offX, offY;   // Offset from original top-left (after trimming)",
        "    uint16_t origW, origH; // Original image dimensions before trimming",
        "} SpriteMeta;",
        "",
        "// ---------------------------------------------------------------------------",
        "// Animation Definition Structure",
        "// ---------------------------------------------------------------------------",
        "",
        "typedef struct {",
        "    uint16_t startSpriteID;  // ID of first frame sprite",
        "    uint16_t frameCount;     // Number of frames in animation",
        "    float defaultSpeed;      // Seconds per frame",
        "    uint8_t loop;            // Whether animation loops (1=yes, 0=no)",
        "} AnimDef;",
        "",
        "// ---------------------------------------------------------------------------",
        "// Sprite ID Enumeration",
        "// ---------------------------------------------------------------------------",
        "",
        f"#define SPRITE_COUNT {len(sprites)}",
        f"#define ANIM_COUNT {len(animations)}",
        "",
        "typedef enum {",
    ]
    
    # Generate sprite IDs (use pre-computed c_identifier)
    for i, sprite in enumerate(sprites):
        comma = "," if i < len(sprites) - 1 else ""
        lines.append(f"    {sprite.c_identifier} = {i}{comma}")
    
    lines.append("} SpriteID;")
    lines.append("")
    
    # Generate animation IDs
    if animations:
        lines.append("// ---------------------------------------------------------------------------")
        lines.append("// Animation ID Enumeration")
        lines.append("// ---------------------------------------------------------------------------")
        lines.append("")
        lines.append("typedef enum {")
        
        for i, anim in enumerate(animations):
            c_name = sanitize_name_for_c(anim.name)
            comma = "," if i < len(animations) - 1 else ""
            lines.append(f"    ANIM_{c_name} = {i}{comma}")
        
        lines.append("} AnimID;")
        lines.append("")
    
    # External declarations
    lines.extend([
        "// ---------------------------------------------------------------------------",
        "// External Data Arrays (defined in atlas_data.c)",
        "// ---------------------------------------------------------------------------",
        "",
        "extern const SpriteMeta ASSET_SPRITES[SPRITE_COUNT];",
    ])
    
    if animations:
        lines.append("extern const AnimDef ASSET_ANIMS[ANIM_COUNT];")
    
    lines.extend([
        "",
        "#endif // ATLAS_DATA_H",
        ""
    ])
    
    return "\n".join(lines)


def generate_source(sprites: list[SpriteData], animations: list[AnimationDef]) -> str:
    """
    Generate the atlas_data.c source file content.
    
    IMPORTANT: For edge extrusion, we output (atlas_x + 1, atlas_y + 1) so the
    engine points to the clean pixels, not the smeared border.
    
    For duplicates, we copy the data from the original sprite they alias.
    """
    lines = [
        "// ============================================================================",
        "// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY",
        "// Generated by build_assets.py v2.3",
        "// ============================================================================",
        "",
        '#include "atlas_data.h"',
        "",
        "// ---------------------------------------------------------------------------",
        "// Sprite Metadata Array",
        "// Format: { x, y, w, h, offX, offY, origW, origH }",
        "// Note: x, y point to clean pixels (inside the extruded border)",
        "// Note: w, h are the trimmed dimensions (visible pixels to render)",
        "// ---------------------------------------------------------------------------",
        "",
        "const SpriteMeta ASSET_SPRITES[SPRITE_COUNT] = {",
    ]
    
    for i, sprite in enumerate(sprites):
        # Use pre-computed c_identifier (without SPR_ prefix for display)
        c_name = sprite.c_identifier[4:] if sprite.c_identifier.startswith("SPR_") else sprite.c_identifier
        comma = "," if i < len(sprites) - 1 else ""
        
        if sprite.is_duplicate and sprite.duplicate_of is not None:
            # Get data from the original sprite
            original = sprites[sprite.duplicate_of]
            # Output original's position (+1 for extrusion offset)
            x_pos = original.atlas_x + 1
            y_pos = original.atlas_y + 1
            trim_w = original.trimmed_width
            trim_h = original.trimmed_height
            off_x = original.offset_x
            off_y = original.offset_y
            orig_w = original.original_width
            orig_h = original.original_height
            dup_marker = " (DUP)"
        else:
            # Output this sprite's position (+1 for extrusion offset)
            x_pos = sprite.atlas_x + 1
            y_pos = sprite.atlas_y + 1
            trim_w = sprite.trimmed_width
            trim_h = sprite.trimmed_height
            off_x = sprite.offset_x
            off_y = sprite.offset_y
            orig_w = sprite.original_width
            orig_h = sprite.original_height
            dup_marker = ""
        
        lines.append(
            f"    /* [{i:3d}] SPR_{c_name:30s}{dup_marker:6s} */ "
            f"{{ {x_pos:4d}, {y_pos:4d}, {trim_w:4d}, {trim_h:4d}, "
            f"{off_x:3d}, {off_y:3d}, "
            f"{orig_w:4d}, {orig_h:4d} }}{comma}"
        )
    
    lines.append("};")
    lines.append("")
    
    # Animation data
    if animations:
        lines.extend([
            "// ---------------------------------------------------------------------------",
            "// Animation Definitions Array",
            "// Format: { startSpriteID, frameCount, defaultSpeed, loop }",
            "// ---------------------------------------------------------------------------",
            "",
            "const AnimDef ASSET_ANIMS[ANIM_COUNT] = {",
        ])
        
        for i, anim in enumerate(animations):
            c_name = sanitize_name_for_c(anim.name)
            loop_val = 1 if anim.loop else 0
            comma = "," if i < len(animations) - 1 else ""
            lines.append(
                f"    /* [{i:2d}] ANIM_{c_name:25s} */ "
                f"{{ {anim.start_sprite_id:3d}, {anim.frame_count:2d}, "
                f"{anim.default_speed:.2f}f, {loop_val} }}{comma}"
            )
        
        lines.append("};")
    
    lines.append("")
    
    return "\n".join(lines)


def generate_debug_html(sprites: list[SpriteData], atlas_size: int) -> str:
    """
    Generate an HTML file for visual debugging of the atlas.
    
    Displays the atlas.png as a background with semi-transparent red boxes
    overlaid on each sprite region, with tooltips showing sprite names.
    """
    # Build sprite boxes HTML
    sprite_boxes = []
    
    for sprite in sprites:
        if sprite.is_duplicate:
            # Skip duplicates - they don't have their own atlas position
            continue
        
        # Position inside the extruded border (+1 offset)
        x = sprite.atlas_x + 1
        y = sprite.atlas_y + 1
        w = sprite.trimmed_width
        h = sprite.trimmed_height
        
        # Determine color based on whether it's part of an animation
        # (different color helps visualize animation frames)
        color = "rgba(255, 0, 0, 0.3)"  # Default red
        border_color = "rgba(255, 0, 0, 0.8)"
        
        sprite_boxes.append(f'''        <div class="sprite-box" 
             style="left: {x}px; top: {y}px; width: {w}px; height: {h}px;
                    background: {color}; border: 1px solid {border_color};"
             title="{sprite.name} (ID: {sprite.sprite_id})&#10;Atlas: ({x}, {y})&#10;Size: {w}x{h}&#10;Offset: ({sprite.offset_x}, {sprite.offset_y})&#10;Original: {sprite.original_width}x{sprite.original_height}">
            <span class="label">{sprite.sprite_id}</span>
        </div>''')
    
    # Also show boxes for the full extruded region (optional, in different color)
    extruded_boxes = []
    for sprite in sprites:
        if sprite.is_duplicate:
            continue
        
        x = sprite.atlas_x
        y = sprite.atlas_y
        w = sprite.trimmed_width + 2
        h = sprite.trimmed_height + 2
        
        extruded_boxes.append(f'''        <div class="extruded-box" 
             style="left: {x}px; top: {y}px; width: {w}px; height: {h}px;">
        </div>''')
    
    html = f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Atlas Debug View - build_assets.py v2.3</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        body {{
            background: #1a1a2e;
            color: #eee;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            padding: 20px;
        }}
        h1 {{
            margin-bottom: 10px;
            color: #00d4ff;
        }}
        .controls {{
            margin-bottom: 20px;
            padding: 15px;
            background: #16213e;
            border-radius: 8px;
        }}
        .controls label {{
            margin-right: 20px;
            cursor: pointer;
        }}
        .controls input[type="checkbox"] {{
            margin-right: 5px;
        }}
        .stats {{
            margin-bottom: 20px;
            padding: 10px 15px;
            background: #0f3460;
            border-radius: 8px;
            display: inline-block;
        }}
        .atlas-container {{
            position: relative;
            display: inline-block;
            background: repeating-conic-gradient(#333 0% 25%, #222 0% 50%) 50% / 20px 20px;
            border: 2px solid #00d4ff;
            border-radius: 4px;
            overflow: auto;
            max-width: 100%;
            max-height: 80vh;
        }}
        .atlas-wrapper {{
            position: relative;
            width: {atlas_size}px;
            height: {atlas_size}px;
        }}
        .atlas-image {{
            position: absolute;
            top: 0;
            left: 0;
            width: {atlas_size}px;
            height: {atlas_size}px;
            image-rendering: pixelated;
        }}
        .sprite-box {{
            position: absolute;
            cursor: pointer;
            transition: all 0.15s ease;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        .sprite-box:hover {{
            background: rgba(0, 255, 0, 0.5) !important;
            border-color: rgba(0, 255, 0, 1) !important;
            z-index: 1000;
        }}
        .sprite-box .label {{
            font-size: 9px;
            color: white;
            text-shadow: 1px 1px 1px black, -1px -1px 1px black;
            pointer-events: none;
            opacity: 0;
        }}
        .show-labels .sprite-box .label {{
            opacity: 1;
        }}
        .extruded-box {{
            position: absolute;
            border: 1px dashed rgba(0, 200, 255, 0.4);
            pointer-events: none;
            display: none;
        }}
        .show-extrusion .extruded-box {{
            display: block;
        }}
        .hide-boxes .sprite-box {{
            display: none;
        }}
        .info-panel {{
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: #16213e;
            padding: 15px;
            border-radius: 8px;
            min-width: 200px;
            border: 1px solid #0f3460;
        }}
        .info-panel h3 {{
            color: #00d4ff;
            margin-bottom: 10px;
        }}
    </style>
</head>
<body>
    <h1>Atlas Debug View</h1>
    
    <div class="stats">
        <strong>Sprites:</strong> {len([s for s in sprites if not s.is_duplicate])} unique, 
        {len([s for s in sprites if s.is_duplicate])} duplicates | 
        <strong>Atlas:</strong> {atlas_size}x{atlas_size}px
    </div>
    
    <div class="controls">
        <label>
            <input type="checkbox" id="toggleBoxes" checked onchange="toggleClass('hide-boxes', !this.checked)">
            Show Sprite Boxes
        </label>
        <label>
            <input type="checkbox" id="toggleLabels" onchange="toggleClass('show-labels', this.checked)">
            Show ID Labels
        </label>
        <label>
            <input type="checkbox" id="toggleExtrusion" onchange="toggleClass('show-extrusion', this.checked)">
            Show Extrusion Borders
        </label>
    </div>
    
    <div class="atlas-container">
        <div class="atlas-wrapper" id="atlasWrapper">
            <img src="atlas.png" alt="Texture Atlas" class="atlas-image">
            
            <!-- Extruded region boxes (shown optionally) -->
{chr(10).join(extruded_boxes)}
            
            <!-- Sprite boxes -->
{chr(10).join(sprite_boxes)}
        </div>
    </div>
    
    <script>
        function toggleClass(className, add) {{
            const wrapper = document.getElementById('atlasWrapper');
            if (add) {{
                wrapper.classList.add(className);
            }} else {{
                wrapper.classList.remove(className);
            }}
        }}
    </script>
</body>
</html>
'''
    return html


# =============================================================================
# Main Entry Point
# =============================================================================

def main():
    global log
    
    parser = argparse.ArgumentParser(
        description="Asset Pipeline v2.3: Generate texture atlas and C data files",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Features:
    - Edge Extrusion: Prevents texture bleeding at sub-pixel rendering
    - Deduplication: Identical frames share atlas space (with pixel normalization)
    - Name Collision Protection: Detects duplicate C identifiers
    - Animation Validation: Strict checking for missing frames
    - Debug HTML: Visual debugging overlay for the atlas
    - Fill Efficiency: Atlas density metrics and warnings

Examples:
    python build_assets.py
    python build_assets.py --size 2048 --padding 1
    python build_assets.py --input ./my_textures --output ./build
    python build_assets.py --no-strict  # Allow animation gaps
    python build_assets.py --verbose    # Enable debug logging
        """
    )
    
    parser.add_argument(
        "--input", "-i",
        type=Path,
        default=None,
        help="Input directory containing raw PNG files (default: ../assets/raw_textures)"
    )
    
    parser.add_argument(
        "--output", "-o",
        type=Path,
        default=None,
        help="Output directory for atlas and C files (default: ../assets/build)"
    )
    
    parser.add_argument(
        "--size", "-s",
        type=int,
        default=4096,
        choices=[256, 512, 1024, 2048, 4096, 8192],
        help="Atlas size in pixels (must be power of 2, default: 4096)"
    )
    
    parser.add_argument(
        "--padding", "-p",
        type=int,
        default=2,
        help="Padding between sprites in pixels (default: 2)"
    )
    
    parser.add_argument(
        "--header-dir",
        type=Path,
        default=None,
        help="Directory for C header/source files (default: same as --output)"
    )
    
    parser.add_argument(
        "--no-debug",
        action="store_true",
        help="Skip generating debug HTML file"
    )
    
    parser.add_argument(
        "--no-dedup",
        action="store_true",
        help="Disable deduplication (pack all sprites even if identical)"
    )
    
    parser.add_argument(
        "--no-strict",
        action="store_true",
        help="Disable strict mode (allow animation gaps without failing)"
    )
    
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable verbose debug logging"
    )
    
    args = parser.parse_args()
    
    # Setup logging
    log = setup_logging(args.verbose)
    
    # Resolve paths relative to script location
    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent
    
    input_dir = args.input or (project_root / "assets" / "raw_textures")
    output_dir = args.output or (project_root / "assets" / "build")
    header_dir = args.header_dir or output_dir
    
    strict_mode = not args.no_strict
    
    # Ensure output directories exist
    output_dir.mkdir(parents=True, exist_ok=True)
    header_dir.mkdir(parents=True, exist_ok=True)
    
    print("=" * 70)
    print("  ASSET PIPELINE v2.3 - Texture Atlas Generator")
    print("=" * 70)
    print(f"  Input:       {input_dir}")
    print(f"  Output:      {output_dir}")
    print(f"  Atlas Size:  {args.size}x{args.size}")
    print(f"  Padding:     {args.padding}px")
    print(f"  Extrusion:   1px (edge smearing enabled)")
    print(f"  Dedup:       {'Disabled' if args.no_dedup else 'Enabled (normalized)'}")
    print(f"  Strict:      {'Enabled' if strict_mode else 'Disabled'}")
    print(f"  Debug HTML:  {'Disabled' if args.no_debug else 'Enabled'}")
    print(f"  Verbose:     {'Enabled' if args.verbose else 'Disabled'}")
    print("=" * 70)
    print()
    
    build_failed = False
    
    try:
        # Step 1: Find PNG files
        print("[1/8] Scanning for PNG files...")
        png_files = find_png_files(input_dir)
        print(f"      Found {len(png_files)} PNG files")
        
        if not png_files:
            print("\nNo PNG files to process. Exiting.")
            return 0
        
        # Step 2: Load and trim sprites
        print("\n[2/8] Loading and trimming sprites...")
        sprites: list[SpriteData] = []
        
        for i, file_path in enumerate(png_files):
            # load_and_trim_sprite uses fail-fast - will sys.exit(1) on any error
            sprite = load_and_trim_sprite(file_path)
            sprite.sprite_id = i  # Assign ID based on alphabetical order
            sprites.append(sprite)
            
            # Progress indicator for large sets
            if (i + 1) % 50 == 0:
                print(f"      Processed {i + 1}/{len(png_files)} sprites...")
        
        # Calculate space savings from trimming
        original_px, trimmed_px, saved_pct = calculate_space_savings(sprites)
        print(f"      Trimmed {saved_pct:.1f}% of transparent space")
        print(f"      ({original_px:,} px -> {trimmed_px:,} px)")
        
        # Step 3: Check for name collisions (The Guardian)
        print("\n[3/8] Checking for C identifier collisions...")
        collision_errors = check_name_collisions(sprites)
        if collision_errors:
            print("      [ERROR] NAME COLLISION DETECTED!")
            for error in collision_errors:
                print(f"\n      ERROR: {error}")
            print("\n      Build aborted. Please rename conflicting files.")
            return 1
        print("      [OK] No collisions detected")
        
        # Step 4: Deduplicate sprites
        print("\n[4/8] Deduplicating identical sprites (with pixel normalization)...")
        if args.no_dedup:
            unique_sprites = sprites
            duplicate_count = 0
            print("      Deduplication disabled")
        else:
            unique_sprites, duplicate_count = deduplicate_sprites(sprites)
            if duplicate_count > 0:
                print(f"      Found {duplicate_count} duplicate sprite(s)")
                print(f"      Packing {len(unique_sprites)} unique sprites (of {len(sprites)} total)")
                # Show which sprites are duplicates
                for sprite in sprites:
                    if sprite.is_duplicate:
                        original = sprites[sprite.duplicate_of]
                        print(f"        - '{sprite.name}' -> alias of '{original.name}'")
            else:
                print("      No duplicates found")
        
        # Step 5: Apply edge extrusion to unique sprites
        print("\n[5/8] Applying edge extrusion (1px border)...")
        for sprite in unique_sprites:
            # apply_edge_extrusion uses fail-fast - will sys.exit(1) on any error
            apply_edge_extrusion(sprite)
        print(f"      Applied extrusion to {len(unique_sprites)} sprites")
        
        # Step 6: Pack sprites into atlas
        print("\n[6/8] Packing sprites into atlas...")
        atlas = pack_sprites(unique_sprites, args.size, args.padding)
        print(f"      Successfully packed {len(unique_sprites)} unique sprites")
        
        # Step 7: Detect animations with strict validation
        print("\n[7/8] Detecting animation sequences...")
        animations, anim_errors = detect_animations(sprites, strict=strict_mode)
        
        if anim_errors:
            print(f"      [WARN] Animation validation issues:")
            for error in anim_errors:
                print(f"        - {error}")
            
            if strict_mode:
                print("\n      Build aborted due to animation errors (use --no-strict to ignore)")
                return 1
            else:
                print("      (Continuing in non-strict mode)")
        
        if animations:
            print(f"      Found {len(animations)} valid animations:")
            for anim in animations:
                print(f"        - {anim.name} ({anim.frame_count} frames)")
        else:
            print("      No multi-frame animations detected")
        
        # Step 8: Save outputs
        print("\n[8/8] Saving output files...")
        
        # Save atlas image
        atlas_path = output_dir / "atlas.png"
        atlas.save(atlas_path, "PNG", optimize=True)
        atlas_size_kb = atlas_path.stat().st_size / 1024
        print(f"      Saved: {atlas_path} ({atlas_size_kb:.1f} KB)")
        
        # Save C header (uses ALL sprites for enum, even duplicates)
        header_content = generate_header(sprites, animations)
        header_path = header_dir / "atlas_data.h"
        header_path.write_text(header_content)
        print(f"      Saved: {header_path}")
        
        # Save C source (uses ALL sprites, duplicates reference originals)
        source_content = generate_source(sprites, animations)
        source_path = header_dir / "atlas_data.c"
        source_path.write_text(source_content)
        print(f"      Saved: {source_path}")
        
        # Save debug HTML
        if not args.no_debug:
            html_content = generate_debug_html(sprites, args.size)
            html_path = output_dir / "atlas_debug.html"
            html_path.write_text(html_content)
            print(f"      Saved: {html_path}")
        
        # Calculate atlas density
        density, wasted, total_extruded = calculate_atlas_density(unique_sprites, args.size)
        
        # Summary
        print()
        print("=" * 70)
        print("  BUILD COMPLETE")
        print("=" * 70)
        print(f"  Total Sprites:   {len(sprites)}")
        print(f"  Unique Packed:   {len(unique_sprites)}")
        print(f"  Duplicates:      {duplicate_count}")
        print(f"  Animations:      {len(animations)}")
        print(f"  Atlas:           {args.size}x{args.size} px")
        print(f"  Trimmed:         {saved_pct:.1f}% space saved")
        print()
        print(f"  Atlas Density:   {density:.1f}% (Wasted Space: {wasted:.1f}%)")
        print(f"  Pixels Used:     {total_extruded:,} / {args.size * args.size:,}")
        
        # Warning if atlas is getting too full
        if density > 90:
            print()
            print("  [WARN] Atlas is >90% full!")
            print("  Consider increasing atlas size or splitting into multiple atlases.")
        elif density > 75:
            print()
            print("  [INFO] Atlas is >75% full. Monitor capacity as you add more sprites.")
        
        print("=" * 70)
        
        return 0
        
    except Exception as e:
        log.error(f"")
        log.error(f"[ERROR] BUILD FAILED with unexpected error:")
        log.error(f"  Exception: {type(e).__name__}: {e}")
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
