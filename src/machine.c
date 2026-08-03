#include "machine.h"
#include "messaging.h"
#include "ppu.h"

#include <stdio.h>

#ifndef __MINGW32__
#include <sys/mman.h>
#endif // __MINGW32__

uint8_t read_memory(machine_t *machine, uint16_t address) {
  if (machine->ppu != NULL && ppu_is_register(address) == 1) {
    return ppu_read_register(machine->ppu, address);
  }

  if (address >= RAM_LOCATION && address < RAM_LOCATION + RAM_SIZE) {
    return machine->ram[address - RAM_LOCATION];
  }
  
  if (address >= ROM_LOCATION) {
    return machine->rom[address - ROM_LOCATION];
  }

  return 0;
}

void write_memory(machine_t *machine, uint16_t address, uint8_t value) {
  if (machine->ppu != NULL && ppu_is_register(address) == 1) {
    ppu_write_register(machine->ppu, address, value);
    return;
  }

  if (machine->state.serial_enabled && address == machine->serial_address) {
    machine->serial_data = value;
    machine->state.serial_written = 1;
    return;
  }

  if (machine->state.cmd_enabled && address == machine->cmd_address) {
    machine->cmd = value;
    machine->state.cmd_pending = 1;
    return;
  }

  if (address >= RAM_LOCATION && address < RAM_LOCATION + RAM_SIZE) {
    machine->ram[address - RAM_LOCATION] = value;
    return;
  }

  if (machine->state.writable_rom && (address >= ROM_LOCATION)) {
    machine->rom[address - ROM_LOCATION] = value;
    return;
  }

  if ((machine->state.writable_vectors || machine->state.writable_rom)) {
    if (address >= ROM_LOCATION) {
      unsigned int addr = address - ROM_LOCATION;
      if (addr >= VECTORS_LOCATION) {
        machine->rom[addr] = value;
        return;
      }
    }
  }
}

#define FLAG_CARRY 0x01U
#define FLAG_ZERO 0x02U

// TODO: Implement a better memory + register view. NCurses?
void dump_state(fake65c02_t *cpu) {
  machine_t *machine = cpu->context;
  fprintf(stderr, " [debug] A: $%02x, X: $%02x, Y: $%02x, Z: %i, C: %i\n",
          cpu->a, cpu->x, cpu->y, (cpu->status & FLAG_ZERO) > 0,
          (cpu->status & FLAG_CARRY) > 0);
  fprintf(stderr, "         PC: $%04x, EA: $%04x ::: $%02x $%02x $%02x $%02x\n",
          cpu->pc, cpu->ea, read_memory(machine, cpu->pc),
          read_memory(machine, cpu->pc + 1), read_memory(machine, cpu->pc + 2),
          read_memory(machine, cpu->pc + 3));
  fflush(stdout);
}

void hook(fake65c02_t *cpu) {
  machine_t *machine = cpu->context;
  if (machine->debug_steps > 0) {
    machine->debug_steps--;
    dump_state(cpu);
  }
  if (machine->hooked_call) {
    if (cpu->opcode == OP_JSR) {
      machine->call_level++;
    } else if (cpu->opcode == OP_RTS) {
      machine->call_level--;
      if (machine->call_level == 0) {
        machine->hooked_call = 0;
      }
    }
    if (machine->call_level > 0) {
      dump_state(cpu);
    }
  }
}

// TODO: Figure out something better than implicit truncation
size_t load_rom(machine_t *machine, char *path) {
  #ifndef __MINGW32__
  FILE *rom_file = fopen(path, "rbe");
  if (rom_file == NULL) return 0;
  int fd = fileno(rom_file);
  void* loc = mmap(machine->rom, ROM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  fclose(rom_file);
  if (loc == MAP_FAILED) return 0;
  return 1;
  #else
  // mingw doesn't support rbe
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error opening file `%s`: %s\n", path, strerror(errno));
    return 0;
  }

  size_t index = 0;
  while (index < ROM_SIZE) {
    size_t chunk_size = fread(&(machine->rom)[index], sizeof(char),
                              MIN(ROM_SIZE - index, BUFFER_SIZE), file);
    if (chunk_size == 0) {
      break;
    }
    index += chunk_size;
  }
  fclose(file);
  // return how many bytes we read so caller can double-check
  return index;
  #endif // __MINGW32__
}

void unload_rom(machine_t *machine) {
  #ifndef __MINGW32__
  munmap(machine->rom, ROM_SIZE);
  #endif // __MINGW32__
  memset(machine->rom, 0, ROM_SIZE * sizeof(uint8_t));
}

// Private fake65c02 memory read/write functions
uint8_t read_memory_fake65c02(fake65c02_t *cpu, uint16_t address) {
  return read_memory(cpu->context, address);
}
void write_memory_fake65c02(fake65c02_t *cpu, uint16_t address, uint8_t value) {
  write_memory(cpu->context, address, value);
}

// TODO: Figure out how I want to handle configuration
machine_t* new_machine() {
  machine_t *machine = calloc(1, sizeof(struct machine));
  machine->cpu = new_fake65c02(machine);
  machine->cpu->read = read_memory_fake65c02;
  machine->cpu->write = write_memory_fake65c02;

  machine->state.cpu_halted = 1; // Start halted

  machine->serial_address = SERIAL_ADDRESS;
  machine->cmd_address = CMD_ADDRESS;
  machine->cmd_data_address = CMD_DATA_ADDRESS;

  return machine;
}

void interrupt_machine(machine_t* machine, interrupt_t interrupt) {
  switch(interrupt) {
    case interrupt_irq:
      irq65c02(machine->cpu);
      break;;
    case interrupt_nmi:
      nmi65c02(machine->cpu);
      break;;
  }
}

int handle_cmd(machine_t *machine) {
  int cmd_handled = 0;
  switch (machine->cmd) {
  case CMD_HOOK:
    machine->debug_steps = read_memory(machine, machine->cmd_data_address);
    cmd_handled = 1;
    break;
  case CMD_HOOK_CALL:
    machine->hooked_call = 1;
    cmd_handled = 1;
    break;
  case CMD_HOOK_FUNC:
    machine->hooked_call = 1;
    machine->call_level = 1;
    cmd_handled = 1;
    break;
  case CMD_HALT:
    machine->state.cpu_halted = 1;
    machine->exit_code = read_memory(machine, machine->cmd_data_address);
    cmd_handled = 1;
    break;
  case CMD_IRQ_REQ:
    machine->irq_request = 1;
    machine->irq_delay = read_memory(machine, machine->cmd_data_address);
    cmd_handled = 1;
    break;
  case CMD_CHAR_REQ:
    machine->char_request = 1;
    machine->char_sync = read_memory(machine, machine->cmd_data_address);
    cmd_handled = 1;
    break;
  default:
    printf("Unknown command: %04x\n", machine->cmd);
  }
  machine->cmd = 0;
  return cmd_handled;
}

int step_machine(machine_t *machine) {
  if (machine->state.cpu_halted) {
    return 0;
  }
  if (machine->state.pending_interrupt) {
    interrupt_machine(machine, interrupt_nmi);
  } else {
    if (machine->cpu != NULL) {
      step65c02(machine->cpu);
      if (machine->cpu->stopped) {
        machine->state.cpu_halted = 1;
      }
    }
    if (machine->ppu != NULL)
      step_ppu(machine->ppu);
  }
  if (machine->state.cmd_enabled && machine->state.cmd_pending) {
    handle_cmd(machine);
  }
  return 1;
}

void hook_machine(machine_t *machine) {
  machine->cpu->hook = hook;
}

void unhook_machine(machine_t *machine) {
  machine->cpu->hook = NULL;
}

void reset_machine(machine_t* machine) {
  if (machine->cpu != NULL)
    reset65c02(machine->cpu);
  if (machine->ppu != NULL)
    reset_ppu(machine->ppu);
  // Clear state
  machine->cmd = 0;
  machine->exit_code = 0;
  machine->call_level = 0;
  machine->hooked_call = 0;
  machine->char_request = 0;
  machine->irq_request = 0;
  machine->irq_delay = 0;
  machine->irq_wait = 0;
  machine->serial_data = 0;
  machine->debug_steps = 0;

  // Clear bitpacked state
  machine->state.cpu_halted = 0;
  machine->state.pending_interrupt = 0;
  machine->state.serial_written = 0;

  // Reset feature flags
  machine->state.serial_enabled = 0;
  machine->state.cmd_enabled = 0;
  machine->state.writable_rom = 0;
  machine->state.writable_vectors = 0;
}