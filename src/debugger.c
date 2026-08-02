#include "debugger.h"


debugger_t* create_debugger(machine_t *machine) {
  return calloc(1, sizeof(debugger_t));
}