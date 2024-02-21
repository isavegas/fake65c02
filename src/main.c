#include "main.h"

// Courtesy of Chris. Thanks!
#define MIN(a, b) ((a) < (b)) ? (a) : (b)

// Opcodes
#define OP_NOOP 0xea
#define OP_JSR 0x20
#define OP_RTS 0x60

#define ADDRESS_SPACE 65536

//#define BANK_SIZE 0x2000
//#define BANK_COUNT 8

#define RAM_SIZE 0x8000
const uint16_t RAM_LOCATION = 0x0000;

#define ROM_SIZE 0x8000
const uint16_t ROM_LOCATION = 0x8000;

#define VECTORS_LOCATION 0xfffc

#define ALIGNMENT 128

// Used for implementing halt functionality
// when stp and wai aren't available
const uint8_t HALTED = 0b00000001;

// TODO: Implement banks. Standard bank size of 8KiB?
typedef struct machine *machine_t;
struct machine {
  fake65c02_t *context;
  uint8_t state;

  uint8_t io_in;
  uint8_t io_out;
  uint8_t io_cmd;
  uint8_t call_level;
  uint8_t serial_last;
  uint8_t serial;
  uint8_t serial_written;
  uint8_t exit_code;
  uint8_t irq_request;
  uint8_t irq_delay;
  uint8_t irq_wait;
  uint8_t char_request;
  uint8_t char_sync;

#ifdef DEBUG
  uint8_t debug_steps;
  uint8_t hooked_call;
#endif

//  uint8_t bank_map[BANK_COUNT];

  uint8_t ram[RAM_SIZE];
  uint8_t rom[ROM_SIZE];
} __attribute__((aligned(ALIGNMENT))) __attribute__((packed));

uint8_t read_memory(fake65c02_t *context, uint16_t address) {
  machine_t machine = (machine_t)context->m;
  uint8_t *ram = machine->ram;
  uint8_t *rom = machine->rom;
  if (address == IO_IN) {
    return machine->io_in;
  }
  if (address >= RAM_LOCATION && address < RAM_LOCATION + RAM_SIZE) {
    return ram[(unsigned int)(address - RAM_LOCATION)];
  }
  if (address >= ROM_LOCATION && address < ROM_LOCATION + ROM_SIZE) {
    return rom[(unsigned int)(address - ROM_LOCATION)];
  }
  return OP_NOOP;
}

void write_memory(fake65c02_t *context, uint16_t address, uint8_t value) {
  machine_t machine = (machine_t)context->m;
  uint8_t *ram = machine->ram;
#if defined(WRITABLE_ROM) || defined(WRITABLE_VECTORS)
  uint8_t *rom = machine->rom;
#endif

  switch (address) {
  case SERIAL_OUT:
    if (!machine->serial_written) {
      machine->serial_written = 1;
    }
    printf("%c", value);
#ifdef DEBUG
    fflush(stdout); // Flush every byte in debug build
#else
    if (value == '\n' || value == '\0') { // Only flush on \n in release build
      fflush(stdout);
    }
#endif
    machine->serial_last = machine->serial;
    machine->serial = value;
    break;
  case IO_OUT:
    machine->io_out = value;
    break;
  case IO_CMD:
    machine->io_cmd = value;
    switch (machine->io_cmd) {
#ifdef DEBUG
    case IO_HOOK:
      machine->debug_steps = machine->io_out;
      machine->io_out = 0;
      machine->io_cmd = 0;
      break;
    case IO_HOOK_CALL:
      machine->hooked_call = 1;
      machine->io_out = 0;
      machine->io_cmd = 0;
      break;
    case IO_HOOK_FUNC:
      machine->hooked_call = 1;
      machine->call_level = 1;
      machine->io_out = 0;
      machine->io_cmd = 0;
      break;
#endif
    case IO_HALT:
      machine->state |= HALTED;
      machine->exit_code = machine->io_out;
      machine->io_out = 0;
      machine->io_cmd = 0;
      break;
    case IO_IRQ_REQ:
      machine->irq_request = 1;
      machine->irq_delay = machine->io_out;
      machine->io_out = 0;
      machine->io_cmd = 0;
      break;
    case IO_CHAR_REQ:
      machine->char_request = 1;
      machine->char_sync = machine->io_out;
      machine->io_out = 0;
      machine->io_cmd = 0;
      break;
    default:
#ifdef DEBUG
      printf("Unknown io command: %04x\n", machine->io_cmd);
#endif
      machine->io_out = 0;
      machine->io_cmd = 0;
      break;
    }
  default:
    if (address >= RAM_LOCATION && address < RAM_LOCATION + RAM_SIZE) {
      ram[(unsigned int)(address - RAM_LOCATION)] = value;
    }
#ifdef WRITABLE_ROM
    if (address >= ROM_LOCATION && address < ROM_LOCATION + ROM_SIZE) {
      unsigned int addr = (unsigned int)(address - ROM_LOCATION);
      rom[addr] = value;
    }
#endif
#if defined(WRITABLE_VECTORS) && !defined(WRITABLE_ROM)
    if (address >= ROM_LOCATION && address < ROM_LOCATION + ROM_SIZE) {
      unsigned int addr = (unsigned int)(address - ROM_LOCATION);
      // Most of ROM is not writable, but we allow vectors to be written
      // to allow direct redefinition of interrupt handler pointers.
      if (addr >= VECTORS_LOCATION) {
        rom[addr] = value;
      }
    }
#endif
  }
}

#define FLAG_CARRY 0x01U
#define FLAG_ZERO 0x02U
#define FLAG_INTERRUPT 0x04U
#define FLAG_DECIMAL 0x08U
#define FLAG_BREAK 0x10U
#define FLAG_CONSTANT 0x20U
#define FLAG_OVERFLOW 0x40U
#define FLAG_SIGN 0x80U

// TODO: Implement a better memory + register view. NCurses?
#ifdef DEBUG
void debug_hook(fake65c02_t *context) {
  fprintf(stderr, " [debug] A: $%02x, X: $%02x, Y: $%02x, Z: %i, C: %i\n", context->a,
         context->x, context->y, (context->status & FLAG_ZERO) > 0U,
         (context->status & FLAG_CARRY) > 0);
  fprintf(stderr, "         PC: $%04x, EA: $%04x ::: $%02x $%02x $%02x $%02x\n",
         context->pc, context->ea, read_memory(context, context->pc),
         read_memory(context, context->pc + 1),
         read_memory(context, context->pc + 2),
         read_memory(context, context->pc + 3));
  fflush(stdout);
}

void hook(fake65c02_t *context) {
  machine_t machine = (machine_t)context->m;
  if (machine->debug_steps > 0) {
    machine->debug_steps--;
    debug_hook(context);
  }
  if (machine->hooked_call) {
    if (context->opcode == OP_JSR) {
      machine->call_level++;
    } else if (context->opcode == OP_RTS) {
      machine->call_level--;
      if (machine->call_level == 0) {
        machine->hooked_call = 0;
      }
    }
    if (machine->call_level > 0) {
      debug_hook(context);
    }
  }
}
#endif

#define BUFFER_SIZE 4096
size_t load_bank(uint8_t *bank, char *path, unsigned int bank_size) {
  FILE *fp = fopen(path, "rbe");
  if (fp == NULL) {
    return 0;
  }

  size_t i = 0;
  for (size_t chunk = 1; chunk > 0; i += chunk) {
    chunk = fread(&bank[i], sizeof(char), MIN(bank_size - i, BUFFER_SIZE), fp);
    if (chunk < 0) { // error
      return 0;
    }
  }

  // return how many bytes we read so caller can double-check
  return i;
}

// TODO: Implement help
// TODO: Implement verbose. Use instead of DEBUG define for output?
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please supply a rom\n");
    return 1;
  }
  int return_code = 0;
  for (int i = 1; i < argc; i++) {
    char *file_name = argv[i];
#ifdef DEBUG
    printf("Running %s\n", file_name);
#endif
    machine_t m = calloc(1, sizeof(struct machine));
    if (load_bank(m->rom, file_name, ROM_SIZE)) {
      m->ram[0x0fff] = 'z';
      m->context = new_fake65c02(m);
      m->context->read = read_memory;
      m->context->write = write_memory;
#ifdef DEBUG
      m->context->hook = hook;
#endif
      reset65c02(m->context);
      uint8_t c = 0;
      while ((m->state & HALTED) == 0 && !m->context->stopped) {
        if (m->irq_request != 0) {
          m->irq_delay--;
          if (m->irq_delay == 0) {
            m->irq_request = 0;
            irq65c02(m->context);
          }
        }
        // TODO: Implement asynchronous character requests.
        if (m->char_request != 0) {
          if (read(STDIN_FILENO, &c, 1)) {
            m->io_in = c;
            if (m->char_sync == 0) {
              irq65c02(m->context);
            }
          } else {
            m->io_in = 0;
            m->char_sync = 0;
          }
          m->char_request = 0;
        }
        step65c02(m->context);
      }
      if (m->serial_written == 1 && m->serial != '\n' &&
          (m->serial == 0 && m->serial_last != '\n')) {
        printf("\n");
        fflush(stdout);
      }
      if (m->exit_code > 0) {
        printf("Exited with code: %i\n", m->exit_code);
      }
      return_code += m->exit_code;
#ifdef DEBUG
      printf("Finished running %s\n", argv[i]);
#endif
    } else {
      printf("Unable to read %s\n", argv[i]);
      return_code += 1;
    }
    free_fake65c02(m->context);
    free(m);
  }

  return return_code;
}
