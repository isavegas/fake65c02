// Vectors are in ROM. If ROM is writable, vectors should be
// as well. The inverse isn't necessarily true.
#ifdef WRITABLE_ROM
#define WRITABLE_VECTORS
#endif

#include <stdio.h>

#include "machine.h"
#include "debugger.h"

//TODO: Figure out how to use argus
/*ARGUS_OPTIONS(
  options,
  HELP_OPTION(),
  OPTION_FLAG('v', "verbose", HELP("Enable verbose output")),
  OPTION_FLAG('-d', "debug", HELP("Enable debugger")),
  OPTION_STRING('-b', "breakpoint", HELP("Set breakpoint (requires debug enabled)"))
)*/

// TODO: Implement help
// TODO: Implement verbose. Use instead of DEBUG define for output?
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please supply a rom\n");
    return 1;
  }
  int return_code = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      // TODO: Debugger
    }
    char *file_name = argv[i];
#ifdef DEBUG
    printf("Running %s\n", file_name);
#endif
    machine_t* machine = new_machine();
    if (load_rom(machine, file_name)) {
      reset_machine(machine);
      machine->state.serial_enabled = 1;
      machine->state.cmd_enabled = 1;
/*
#ifdef DEBUG
      hook_machine(machine);
#endif
*/
      uint8_t requested_character = 0;
      uint8_t unfinished_output = 0;
      while (!machine->state.cpu_halted) {
        if (machine->irq_request != 0) {
          machine->irq_delay--;
          if (machine->irq_delay == 0) {
            machine->irq_request = 0;
            interrupt_machine(machine, interrupt_irq);
          }
        }
        if (machine->char_request != 0) {
          if (read(STDIN_FILENO, &requested_character, 1)) {
            write_memory(machine, machine->cmd_data_address, requested_character);
            if (machine->char_sync == 0) {
              interrupt_machine(machine, interrupt_irq);
            }
          } else {
            machine->char_sync = 0;
          }
          machine->char_request = 0;
        }
        step_machine(machine);
        if (machine->state.serial_written) {
          machine->state.serial_written = 0;
          if (machine->serial_data != '\0') {
            printf("%c", machine->serial_data);
            unfinished_output = 1;
            if (machine->serial_data == '\n') {
              fflush(stdout);
              unfinished_output = 0;
            }
          }
        }
      }
      if (unfinished_output) {
        printf("%%\n");
      }
      if (machine->exit_code != 0) {
        printf("Exited with code: %i\n", machine->exit_code);
      }
      return_code += machine->exit_code;
#ifdef DEBUG
      printf("Finished running %s\n", argv[i]);
#endif
    } else {
      printf("Unable to read %s\n", argv[i]);
      return_code += 1;
    }
  }

  return return_code;
}
