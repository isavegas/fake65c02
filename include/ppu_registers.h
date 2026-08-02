#ifndef PPU_REGISTERS_H
#define PPU_REGISTERS_H

#include <stdint.h>

#define PPU_CTRL_NMI_VBLANK (1 << 7)
#define PPU_CTRL_TALL_SPRITES (1 << 5)
#define PPU_CTRL_BACKGROUND_PT (1 << 4)
#define PPU_CTRL_SPRITE_PT (1 << 3)
#define PPU_CTRL_VRAM_INCR_32 (1 << 2)
#define PPU_CTRL_NT_MASK (0b00000011)

#define PPU_MASK_EMPHASIS_B (1 << 7)
#define PPU_MASK_EMPHASIS_G (1 << 6)
#define PPU_MASK_EMPHASIS_R (1 << 5)
#define PPU_MASK_SPRITE (1 << 4)
#define PPU_MASK_BACKGROUND (1 << 3)
#define PPU_MASK_SPRITE_LEFT (1 << 2)
#define PPU_MASK_BACKGROUND_LEFT (1 << 1)
#define PPU_MASK_GRAYSCALE (1 << 0)

#define PPU_STATUS_VBLANK (1 << 7)
#define PPU_STATUS_SPRITE_HIT (1 << 6)
#define PPU_STATUS_SPRITE_OVERFLOW (1 << 5)

#endif