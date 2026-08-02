#ifndef PPU_H
#define PPU_H

#include <stdint.h>

// PPU Output resolution
#define SCANLINES 262
#define SCANLINE_DOTS 304

#define VISIBLE_SCANLINES 240
#define VISIBLE_SCANLINE_DOTS 256

#define HORIZONTAL_OFFSET 16

// PPU Register Addresses
#define PPU_CTRL_ADDRESS 0x2000
#define PPU_MASK_ADDRESS 0x2001
#define PPU_STATUS_ADDRESS 0x2002
#define OAM_ADDR_ADDRESS 0x2003
#define OAM_DATA_ADDRESS 0x2004
#define PPU_SCROLL_ADDRESS 0x2005
#define PPU_ADDR_ADDRESS 0x2006
#define PPU_DATA_ADDRESS 0x2007
#define OAM_DMA_ADDRESS 0x4014

// Various constants
#define NAMETABLE_START 0x2000
#define NAMETABLE_SIZE 0x400
#define PATTERN_TABLE_OFFSET 0x03c0
#define PATTERN_TABLE_SIZE 0x1000

// Forward declaration to fix circular includes
struct machine;

typedef struct ppu ppu_t;
struct ppu {
    struct machine *machine;
    uint32_t* framebuffer;
    int scanline;

    uint8_t latch;

    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oam_addr;
    uint8_t oam_data;
    uint16_t scroll;
    uint16_t addr;
    uint8_t data;
    uint8_t oam_dma;

    uint8_t vram[0x4000];
};

ppu_t* new_ppu(struct machine *machine, uint32_t* framebuffer);
void reset_ppu(ppu_t *ppu);
void free_ppu(ppu_t *ppu);

int render_background(ppu_t *ppu);
int render_sprites(ppu_t *ppu);
int step_ppu(ppu_t *ppu);

int plot_ppu(ppu_t *ppu, unsigned int x, unsigned int y, unsigned int color_index);

int ppu_is_register(uint16_t address);
int ppu_write_register(ppu_t *ppu, uint16_t address, uint8_t value);
uint8_t ppu_read_register(ppu_t *ppu, uint16_t address);

#endif