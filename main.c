#include "include/main.h"

// Ports
#define IO_IN 0x7fff
#define IO_CMD 0x8000
#define IO_OUT 0x8001

// Messages
#define IO_HALT 0x01
#define IO_HOOK 0xff
#define IO_HOOK_FUNC 0xfe
#define IO_HOOK_CALL 0xfd

#define NOOP 0xea

#define SERIAL_OUT 0xffff

#define ADDRESS_SPACE 65536

#define RAM_SIZE 0x8000
const uint16_t RAM_LOCATION = 0x0000;
uint8_t RAM[RAM_SIZE]; // NOLINT

#define ROM_SIZE 0x8000
const uint16_t ROM_LOCATION = 0x8000;
uint8_t ROM[ROM_SIZE]; // NOLINT

uint8_t STATE = 0b00000000; // NOLINT
const uint8_t HALTED = 0b00000001;
const uint8_t STOPPED = 0b00000010;
const uint8_t WAITING = 0b00000100;

uint8_t io_in = 0;          // NOLINT
uint8_t io_out = 0;         // NOLINT
uint8_t io_cmd = 0;         // NOLINT
uint8_t hooked_call = 0;    // NOLINT
uint8_t call_level = 0;     // NOLINT
uint8_t serial_last = 0;    // NOLINT
uint8_t serial = 0;         // NOLINT
uint8_t serial_written = 0; // NOLINT
uint8_t exit_code = 0;      // NOLINT
uint8_t debug_steps = 0;

uint8_t read6502(uint16_t address) {
  if (address == IO_IN) {
    return io_in;
  }
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
  return NOOP;
}

void write6502(uint16_t address, uint8_t value) {
  switch (address) {
  case SERIAL_OUT:
    if (!serial_written) serial_written = 1;
    printf("%c", value);
#ifdef DEBUG
    if (value == '\n' || value == '\0') // Only flush on \n in release build
#endif
        fflush(stdout); // Flush every byte in debug build
    serial_last = serial;
    serial = value;
    break;
  case IO_OUT:
    io_out = value;
    break;
  case IO_CMD:
    io_cmd = value;
    switch (io_cmd) {
    case IO_HOOK:
      debug_steps = io_out;
      io_cmd = 0;
      break;
    case IO_HOOK_CALL:
      hooked_call = 1;
      io_cmd = 0;
      break;
    case IO_HALT:
      STATE |= HALTED;
      exit_code = io_out;
      break;
    }
    break;
  default:
    if (address >= RAM_LOCATION && address < RAM_LOCATION + RAM_SIZE) {
      // printf("write $%04x -> $%04x\n", address, (unsigned int)(address -
      // RAM_LOCATION));
      RAM[(unsigned int)(address - RAM_LOCATION)] = value;
    }
    if (address >= ROM_LOCATION && address < ROM_LOCATION + ROM_SIZE) {
      unsigned int addr = (unsigned int)(address - ROM_LOCATION);
#ifdef WRITABLE_ROM
      // printf("write $%04x -> $%04x\n", address, (unsigned int)(address -
      // ROM_LOCATION));
      ROM[addr] = value;
#else
#ifdef WRITABLE_VECTORS
      // Most of ROM is not writable, but we allow vectors to be written
      if (addr >= 0xfffc) {
        ROM[addr] = value;
      }
#endif
#endif
    }
  }
}

#define FLAG_CARRY 0x01
#define FLAG_ZERO 0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL 0x08
#define FLAG_BREAK 0x10
#define FLAG_CONSTANT 0x20
#define FLAG_OVERFLOW 0x40
#define FLAG_SIGN 0x80

void debug_hook() {
    printf(" [debug] A: $%02x, X: $%02x, Y: $%02x, Z: %i, C: %i, $00: $%02x, $01: $%02x\n",
        a, x, y, (status & FLAG_ZERO) > 0, (status & FLAG_CARRY) > 0, read6502(0), read6502(1)
    );
    printf("         PC: $%04x, EA: $%04x ::: $%02x $%02x $%02x $%02x\n",
        pc, ea,
        read6502(pc), read6502(pc+1), read6502(pc+2), read6502(pc+3)
    );
    fflush(stdout);
}

void hook() {
  if (debug_steps > 0) {
    debug_steps--;
    debug_hook();
  }
  if (hooked_call) {
    if (opcode == 0x20) { // jsr
      call_level++;
    } else if (opcode == 0x60) { // rts
      call_level--;
      if (call_level == 0)
        hooked_call = 0;
    }
    if (call_level > 0) {
      debug_hook();
    }
  }
}

// Fill with noop
void initialize(uint8_t *bytes, int size) {
  for (int i = 0; i < size; i++) {
    bytes[i] = NOOP; // NOLINT
  }
}

const int BUFFER_SIZE = 4096;
int load_rom(char *path, unsigned int rom_size) {
  FILE *fp = fopen(path, "re");
  if (fp == NULL) {
    return 0;
  }
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
  return 1;
}

int main(int argc, char *argv[]) {
  initialize(ROM, ROM_SIZE);
  initialize(RAM, RAM_SIZE);
  if (argc < 2) {
    printf("Please supply a rom\n");
    return 1;
  }
  if (load_rom(argv[1], ROM_SIZE)) {
    //    printf("$%02x, $%02x :: $%02x, $%02x\n", ROM[0x7ffc], ROM[0x7ffd],
    //    read6502(0xfffc), read6502(0xfffd)); printf("$0000: $%02x, $0001:
    //    $%02x :: $8000: $%02x, $8001: $%02x\n", ROM[0x0000], ROM[0x0001],
    //    read6502(0x8000), read6502(0x8001));
    hookexternal(*hook);
    reset6502();
    int d = 0;
    while ((STATE & HALTED) == 0) {
      step6502();
    }
    if (serial != '\n' && (serial == 0 && serial_last != '\n') && serial_written == 1) {
      printf("\n");
      fflush(stdout);
    }
    if (exit_code > 0) {
      printf("Exited with code: %i\n", exit_code);
    }
    return exit_code;
  }
  printf("Unable to read rom\n");
  return 1;
}
