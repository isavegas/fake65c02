#include "ppu.h"
#include "machine.h"
#include "ppu_color_palette.h"
#include "ppu_registers.h"
#include "util.h"
#include <stdlib.h>

ppu_t *new_ppu(struct machine *machine, uint32_t *framebuffer) {
  if (framebuffer == NULL) {
    printf("null framebuffer");
    return NULL;
  }
  ppu_t *ppu = calloc(1, sizeof(ppu_t));
  ppu->machine = machine;
  ppu->framebuffer = framebuffer;
  return ppu;
}

void reset_ppu(ppu_t *ppu) {
  ppu->scanline = 0;
  ppu->mask = 0;
  SET_BIT(ppu->mask, PPU_MASK_SPRITE);
  SET_BIT(ppu->mask, PPU_MASK_BACKGROUND);
  SET_BIT(ppu->mask, PPU_MASK_SPRITE_LEFT);
  SET_BIT(ppu->mask, PPU_MASK_BACKGROUND_LEFT);
  ppu->ctrl = 0;
  ppu->status = 0;
}

void free_ppu(ppu_t *ppu) {
  if (ppu->machine != NULL && ppu->machine->ppu == ppu) {
    ppu->machine->ppu = NULL;
  }
  free(ppu);
}

#define TILE_COLOR_INDEX(high, low, x)                                         \
  ((((low) >> (7 - (x))) & 0x01) | (((high) >> (7 - (x))) & 0x01) << 1)
int render_background(ppu_t *ppu) {
  int x = 0;
  if (CHECK_BIT(ppu->mask, PPU_MASK_BACKGROUND_LEFT) == 0) {
    x += 8;
  }
  for (; x < (SCANLINE_DOTS/8); x++) {
    // TODO: Implement loopy
    int name_table_address =
        NAMETABLE_START + ((ppu->ctrl & PPU_CTRL_NT_MASK) * NAMETABLE_SIZE);
    // int metablock_index = ((ppu->scanline / 32) * 8) + (row / 4);
    //  uint8_t block =
    // ppu->vram[name_table_address + PATTERN_TABLE_OFFSET + metablock_index];
    // int palette_id =
    //  (block >> (((ppu->scanline % 32) / 16) * 2 + ((row % 32) / 16) * 2))
    //  & 0b00000011;

    int pattern_table_address =
        CHECK_BIT(ppu->ctrl, PPU_CTRL_BACKGROUND_PT) * PATTERN_TABLE_SIZE;
    if (ppu->machine == NULL)
      return 0;
    uint8_t tile_id =
        ppu->vram[name_table_address + ((ppu->scanline / 8) * 32) + (x/8)];
    uint8_t tile_address =
        pattern_table_address + (tile_id * 16) + (ppu->scanline % 8);
    uint8_t low_plane = ppu->vram[tile_address];
    uint8_t high_plane = ppu->vram[tile_address + 8];

    for (int tile_offset_x = 0; tile_offset_x < 8; tile_offset_x++) {
      plot_ppu(ppu, x + tile_offset_x, ppu->scanline,
               TILE_COLOR_INDEX(low_plane, high_plane, tile_offset_x));
    }
  }
  return 1;
}

int render_sprites(ppu_t *ppu) { return 0; }

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
int plot_ppu(ppu_t *ppu, unsigned int x, unsigned int y,
             unsigned int color_index) {
  if (color_index > sizeof(color_palette) || color_index < 0 ) {
    printf("Color index out of bounds: %i!\n", color_index);
    return 1;
  }
  if (x > SCANLINE_DOTS || x < 0) {
    printf("x out of bounds: %i!\n", x);
    return 0;
  }
  if (y > SCANLINES || y < 0) {
    printf("y out of bounds: %i!\n", y);
    return 0;
  }
  uint32_t color = color_palette[color_index];
  if (CHECK_BIT(ppu->mask, PPU_MASK_GRAYSCALE)) {
    color = grayscale_palette[color_index % 4];
  }
  if (ppu->framebuffer == NULL) {
    printf("[plot_ppu] No framebuffer present!\n");
    return 0;
  }
  ppu->framebuffer[(y * SCANLINE_DOTS) + x] = color;
  return 1;
}

// Render one scanline
int step_ppu(ppu_t *ppu) {
  if (ppu->framebuffer == NULL) {
    printf("missing ppu->framebuffer in step_ppu scanline %i\n",
           ppu->scanline);
    return 0;
  }
  if (ppu->scanline != 261) {
    if (CHECK_BIT(ppu->mask, PPU_MASK_BACKGROUND)) {
      render_background(ppu);
    }
    if (CHECK_BIT(ppu->mask, PPU_MASK_SPRITE)) {
      render_sprites(ppu);
    }
  }
  if (ppu->scanline == 241) {
    ppu->status = 0;
  }
  ppu->scanline = (ppu->scanline + 1) % SCANLINES;
  return 1;
}

uint8_t ppu_read_register(ppu_t *ppu, uint16_t address) {
  switch (address) {
  case PPU_CTRL_ADDRESS:
    return ppu->ctrl;
  case PPU_MASK_ADDRESS:
    return ppu->mask;
  case PPU_STATUS_ADDRESS:
    ppu->latch = 0;
    return ppu->status;
  case OAM_ADDR_ADDRESS:
    return ppu->oam_addr;
  case OAM_DATA_ADDRESS:
    return ppu->oam_data;
  case PPU_SCROLL_ADDRESS:
    return 0;
  case PPU_ADDR_ADDRESS:
    if (!ppu->latch) {
      return ppu->addr & 0x0F;
    } else {
      return ppu->addr >> 8;
    }
  case PPU_DATA_ADDRESS:
    return ppu->data;
  case OAM_DMA_ADDRESS:
    return ppu->oam_dma;
  default:
    return 0;
  }
}

#define LATCHED_WRITE_REGISTER(register, latch, value)                         \
  {                                                                            \
    if (!(latch)) {                                                            \
      (register) = ((uint16_t)(value) << 8) | (ppu->addr & 0x0F);              \
    } else {                                                                   \
      (register) = (ppu->addr & 0xF0) | (uint16_t)(value);                     \
    }                                                                          \
    (latch) = !(latch);                                                        \
  }

void ppu_scroll_write(ppu_t *ppu, uint8_t value) {
  // TODO: Implement Loopy scrolling
  LATCHED_WRITE_REGISTER(ppu->scroll, ppu->latch, value);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
int ppu_write_register(ppu_t *ppu, uint16_t address, uint8_t value) {
  switch (address) {
  case PPU_CTRL_ADDRESS:
    ppu->ctrl = value;
    return 1;
  case PPU_MASK_ADDRESS:
    ppu->mask = value;
    return 1;
  case PPU_STATUS_ADDRESS:
    return 1;
  case OAM_ADDR_ADDRESS:
    ppu->oam_addr = value;
    return 1;
  case OAM_DATA_ADDRESS:
    ppu->oam_data = value;
    return 1;
  case PPU_SCROLL_ADDRESS:
    ppu_scroll_write(ppu, value);
    return 1;
  case PPU_ADDR_ADDRESS:
    LATCHED_WRITE_REGISTER(ppu->addr, ppu->latch, value)
    return 1;
  case PPU_DATA_ADDRESS:
    ppu->data = value;
    return 1;
  case OAM_DMA_ADDRESS:
    ppu->oam_dma = value;
    return 1;
  default:
    return 0;
  }
}

// TODO: mirroring
// https://www.nesdev.org/wiki/PPU_registers
int ppu_is_register(uint16_t address) {
  switch (address) {
  case PPU_CTRL_ADDRESS:
  case PPU_MASK_ADDRESS:
  case PPU_STATUS_ADDRESS:
  case OAM_ADDR_ADDRESS:
  case OAM_DATA_ADDRESS:
  case PPU_SCROLL_ADDRESS:
  case PPU_ADDR_ADDRESS:
  case PPU_DATA_ADDRESS:
  case OAM_DMA_ADDRESS:
    return 1;
  default:
    return 0;
  }
}