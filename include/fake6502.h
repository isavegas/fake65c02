#ifndef FAKE6502_H
#define FAKE6502_H

#include <stddef.h>
#include <stdint.h>

void reset6502();
void step6502();
void irq6502();
void exec6502(uint32_t tickcount);
void hookexternal(void *funcptr);

extern uint32_t clockticks6502; // NOLINT
extern uint32_t instructions; // NOLINT

#endif
