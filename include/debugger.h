#ifndef DEBUGGER_H
#define DEBUGGER_H

//#include <argus.h>

#include "machine.h"

/* Goals: Implement an LLDB-esque debugger that can:
          - View & edit memory
          - Create & manage breakpoints
          - Watch memory addresses
          - Trigger interrupts                      */

typedef struct debugger debugger_t;
struct debugger {
    uint8_t dummy;
};

debugger_t* create_debugger(machine_t *machine);

#endif