#include "machine.h"
#include "ppu.h"
#include "ppu_registers.h"
#include "util.h"
#include "raylib.h"
#include <stdint.h>

const unsigned int scale = 4;

int main(int argc, char **argv) {
  /*if (argc < 2) {
      printf("Please provide a rom.\n");
      return 1;
  }*/
  machine_t *machine = new_machine();
  int dots = SCANLINE_DOTS * SCANLINES;
  uint32_t *framebuffer = calloc((unsigned long long)(dots), sizeof(uint32_t));
  if (framebuffer == NULL) {
    printf("Unable to allocate framebuffer\n");
    return 1;
  }
  //printf("%i\n", framebuffer[10]);
  machine->ppu = new_ppu(machine, framebuffer);
  if (machine->ppu == NULL){
    printf("Unable to create ppu\n");
    return 1;
  }
  reset_ppu(machine->ppu);
  CLEAR_BIT(machine->ppu->mask, PPU_MASK_SPRITE);
  SET_BIT(machine->ppu->mask, PPU_MASK_GRAYSCALE);

  uint8_t basic_tile[16] = {0x41, 0xC2, 0x44, 0x48, 0x10, 0x20, 0x40, 0x80,
                            0x01, 0x02, 0x04, 0x08, 0x16, 0x21, 0x42, 0x87};
  for (int i = 0; i < sizeof(basic_tile); i++) {
    machine->ppu->vram[i + 16] = basic_tile[i];
  }
  for (int i = 0; i < 16; i++) {
    machine->ppu->vram[i + 32] = 0xff;
  }
  for (int i = 0; i < (32 * 30); i++) {
    if (i % 3 == 0) {
      machine->ppu->vram[NAMETABLE_START + i] = 0x02;
    } else {
      machine->ppu->vram[NAMETABLE_START + i] = 0x01;
    }
  }

  SetTraceLogLevel(LOG_ERROR);
  InitWindow(SCANLINE_DOTS * scale, SCANLINES * scale, "fake65c02");
  SetExitKey(KEY_F4);
  SetTargetFPS(60);

  // TODO: Crop using VISIBLE_SCANLINES, VISIBLE_SCANLINE_DOTS, and HORIZONTAL_OFFSET
  Image bg_image = GenImageColor(SCANLINE_DOTS, SCANLINES, BLACK);
  Texture2D ppu_texture = LoadTextureFromImage(bg_image);
  UnloadImage(bg_image);

  SetTextureFilter(ppu_texture, TEXTURE_FILTER_POINT);
  int frame = 0;
  while (!WindowShouldClose()) {
    do {
      if (!step_ppu(machine->ppu)) {
        printf("PPU Errored on frame %i, %i!\n", frame, machine->ppu->scanline);
        break;
      }
    } while(machine->ppu->scanline > 0);
    for (int x = HORIZONTAL_OFFSET; x < VISIBLE_SCANLINE_DOTS; x++)
      for (int y = 0; y < VISIBLE_SCANLINES; y++)
        plot_ppu(machine->ppu, x, y, 1);

    if (machine->ppu->framebuffer == NULL) {
      printf("Framebuffer vanished after frame %i!\n", frame);
      break;
    }
    UpdateTexture(ppu_texture, machine->ppu->framebuffer);

    BeginDrawing();
      ClearBackground(BLACK);
      DrawTextureEx(ppu_texture, (Vector2){0.0f, 0.0f}, 0.0f, (float)scale, WHITE);
    EndDrawing();

    #ifdef VCPKG_FIX
    SwapScreenBuffer();
    PollInputEvents();
    #endif
    frame++;
  }

  UnloadTexture(ppu_texture);
  CloseWindow();

  free(framebuffer);
  free_machine(machine);

  return 0;
}