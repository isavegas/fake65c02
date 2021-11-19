#include <stddef.h>
#include <stdint.h>
// NOLINTNEXTLINE(llvmlibc-restrict-system-libc-headers)
#include <stdio.h>

#ifdef FAKE6502
#include "include/fake6502.h"
#endif
#ifdef FAKE65c02
#include "include/fake65c02.h"
#endif

#if !defined(FAKE6502) && !defined(FAKE65c02)
#error CPU type not provided
#endif

#define IO_OUT 0x0300
#define HALT 0x0301

#define ADDRESS_SPACE 65536

#define ROM_SIZE 32768
uint8_t ROM[ROM_SIZE]; // NOLINT

uint8_t STATE = 0b00000000;
uint8_t HALTED = 0b00000001;
uint8_t last_char = 0;

uint8_t read6502(uint16_t address) {
  switch (address) {
  default:
    // ROM is duplicated across first and second half of
    // the address space
    if (address > ADDRESS_SPACE - ROM_SIZE) {
      address -= ROM_SIZE;
    }
    return ROM[(unsigned int)address];
  }
}

void write6502(uint16_t address, uint8_t value) {
  switch (address) {
  case IO_OUT:
    printf("%c", value);
    last_char = value;
    break;
  case HALT:
    STATE |= HALTED;
    break;
  default:
    ROM[(int)address] = value;
  }
}

// Fill ROM with noop
void initialize_rom() {
  for (int i = 0; i < ROM_SIZE; i++) {
    ROM[i] = 0xea; // NOLINT
  }
}

const int BUFFER_SIZE = 4096;
int load_rom(char *path) {
  FILE *fp = fopen(path, "re");
  if (fp == NULL) {
    return 0;
  } else {
    unsigned char buffer[BUFFER_SIZE];
    unsigned int p = 0;
    size_t size = 0;
    while ((size = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
      for (int i = 0; i < size; i++) {
        ROM[p] = buffer[i];
        p++;
      }
    }
  }
  return 1;
}

void l() {
  printf("Clock: %i, Instructions: %i\n", clockticks6502, instructions);
}

#undef NES_CPU
#define UNDOCUMENTED

int main(int argc, char *argv[]) {
  initialize_rom();
  if (argc < 2) {
    printf("Please supply a rom\n");
    return 1;
  }
  if (load_rom(argv[1])) {
    reset6502();
    while ((STATE & HALTED) == 0) {
      step6502();
    }
    if (last_char != '\n') {
      printf("\n");
    }
    return 0;
  }
  printf("Unable to read rom\n");
  return 1;
}
