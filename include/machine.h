#ifndef MACHINE_H
#define MACHINE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __WIN32__
#include <errno.h>
#endif

#ifndef _MSC_VER
#include <unistd.h>
#endif

// Opcodes
#define OP_JSR 0x20
#define OP_RTS 0x60

// #define BANK_SIZE 0x2000
// #define BANK_COUNT 8

#define RAM_SIZE 0x8000
#define ROM_SIZE 0x8000
#define BUFFER_SIZE 4096

#define VECTORS_LOCATION 0xfffc

#define ALIGNMENT 128

#define RAM_LOCATION 0x0000
#define ROM_LOCATION 0x8000

typedef struct {
  // Features
  uint8_t serial_enabled: 1;
  uint8_t cmd_enabled: 1;

  uint8_t writable_rom: 1;
  uint8_t writable_vectors: 1;

  // Runtime state
  uint8_t cpu_halted: 1;
  uint8_t pending_interrupt: 1;
  uint8_t serial_written: 1;
  uint8_t cmd_pending: 1;
} machine_state_t;

typedef enum {
  interrupt_irq,
  interrupt_nmi,
} interrupt_t;

#include "fake65c02.h"
#include "ppu.h"

typedef struct machine machine_t;
struct machine {
  fake65c02_t *cpu;
  ppu_t *ppu;
  machine_state_t state;

  uint8_t cmd;
  uint16_t cmd_address;
  uint16_t cmd_data_address;
  uint8_t call_level;
  uint8_t serial_data;
  uint16_t serial_address;
  uint8_t exit_code;
  uint8_t irq_request;
  uint8_t irq_delay;
  uint8_t irq_wait;
  uint8_t char_request;
  uint8_t char_sync;

  uint8_t debug_steps;
  uint8_t hooked_call;

  //  uint8_t bank_map[BANK_COUNT];

  uint8_t ram_start;
  uint8_t ram[RAM_SIZE];
  uint8_t rom_start;
  uint8_t rom[ROM_SIZE];
} __attribute__((aligned(ALIGNMENT))) __attribute__((packed));

// Public memory modification functions
uint8_t read_memory(machine_t *machine, uint16_t address);
void write_memory(machine_t *machine, uint16_t address, uint8_t value);

size_t load_rom(machine_t *machine, char *path);
void unload_rom(machine_t* machine);

int step_machine(machine_t *machine);
void hook_machine(machine_t *machine);
void unhook_machine(machine_t *machine);
void interrupt_machine(machine_t* machine, interrupt_t interrupt);

machine_t* new_machine();
void reset_machine(machine_t* machine);

#endif