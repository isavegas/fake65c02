#include "include/main.h"

#define IO_OUT 0x7001
#define HALT 0x7002

#define ADDRESS_SPACE 65536

#define RAM_SIZE 0x8000
const uint16_t RAM_LOCATION = 0x0000;
uint8_t RAM[RAM_SIZE]; //NOLINT

#define ROM_SIZE 0x8000
const uint16_t ROM_LOCATION = 0x8000;
uint8_t ROM[ROM_SIZE]; // NOLINT

uint8_t STATE = 0b00000000;
uint8_t HALTED = 0b00000001;

uint8_t last_char = 0;
uint8_t exit_code = 0;

uint8_t read6502(uint16_t address) {
  switch (address) {
  default:
    if (address >= RAM_LOCATION && address < RAM_LOCATION + RAM_SIZE) {
      /*
      unsigned int addr = address - RAM_LOCATION;
      printf("read ram $%04x -> $%04x :: $%02x\n", address, addr, RAM[addr]);
      */
      return RAM[(unsigned int)(address - RAM_LOCATION)];
    }
    if (address >= ROM_LOCATION && address < ROM_LOCATION + ROM_SIZE) {
      /*
      unsigned int addr = address - ROM_LOCATION;
      printf("read rom $%04x -> $%04x :: $%02x\n", address, addr, ROM[addr]);
      */
      return ROM[(unsigned int)(address - ROM_LOCATION)];
    }
    return 0xea;
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
    exit_code = value;
    break;
  default:
    if (address >= RAM_LOCATION && address < RAM_LOCATION + RAM_SIZE) {
      //printf("write $%04x -> $%04x\n", address, (unsigned int)(address - RAM_LOCATION));
      RAM[(unsigned int)(address - RAM_LOCATION)] = value;
    }
    if (address >= ROM_LOCATION && address < ROM_LOCATION + ROM_SIZE) {
#ifdef WRITABLE_ROM
      //printf("write $%04x -> $%04x\n", address, (unsigned int)(address - ROM_LOCATION));
      ROM[(unsigned int)(address - ROM_LOCATION)] = value;
#else
      // Nothing happens. Perhaps throw error down the line?
#endif
    }
    }
}

// Fill with noop
void initialize(uint8_t* bytes, int size) {
  for (int i = 0; i < size; i++) {
    bytes[i] = 0xea; // NOLINT
  }
}

const int BUFFER_SIZE = 4096;
int load_rom(char *path, unsigned int rom_size) {
  FILE *fp = fopen(path, "re");
  if (fp == NULL) {
    return 0;
  } else {
    unsigned char buffer[BUFFER_SIZE];
    unsigned int p = 0;
    size_t size = 0;
    while ((size = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
      for (int i = 0; i < size; i++) {
        if (p < rom_size) {
          ROM[p] = buffer[i];
          p++;
        } else {
          return 1;
        }
      }
    }
  }
  return 1;
}

void l() {
//  printf("Clock: %i, Instructions: %i, PC: $%04x, OP: $%02x\n", clockticks6502, instructions, pc, opcode);
}

int main(int argc, char *argv[]) {
  initialize(ROM, ROM_SIZE);
  initialize(RAM, RAM_SIZE);
  if (argc < 2) {
    printf("Please supply a rom\n");
    return 1;
  }
  if (load_rom(argv[1], ROM_SIZE)) {
//    printf("$%02x, $%02x :: $%02x, $%02x\n", ROM[0x7ffc], ROM[0x7ffd], read6502(0xfffc), read6502(0xfffd));
//    printf("$0000: $%02x, $0001: $%02x :: $8000: $%02x, $8001: $%02x\n", ROM[0x0000], ROM[0x0001], read6502(0x8000), read6502(0x8001));
    l();
    reset6502(); l();
    while ((STATE & HALTED) == 0) {
      step6502(); l();
    }
    if (last_char != '\n') {
      printf("\n");
    }
    if (exit_code > 0) {
        printf("Exited with code: %i\n", exit_code);
    }
    return exit_code;
  }
  printf("Unable to read rom\n");
  return 1;
}
