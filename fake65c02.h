#ifndef FAKE6502_H
#define FAKE6502_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct fake6502 fake6502_t;

struct fake6502 {
    void *m;

    uint8_t (*read)(fake6502_t *ctx, uint16_t address);
    void (*write)(fake6502_t *ctx, uint16_t address, uint8_t value);
    void (*hook)(fake6502_t *ctx);

    uint16_t pc;

    uint8_t sp;

    uint8_t a;
    uint8_t x;
    uint8_t y;

    uint32_t instructions;
    uint32_t clockticks;

    uint16_t ea;

// internal
    uint8_t status;
    uint8_t opcode;
    uint8_t oldstatus;

    uint8_t penaltyop;
    uint8_t penaltyaddr;

    uint16_t reladdr;
    uint16_t result;
    uint16_t value;
    uint16_t oldpc;

    uint32_t clockgoal;
};

fake6502_t *new_fake6502(void* m);
void free_fake6502(fake6502_t *context);

int reset6502(fake6502_t *context);
int step6502(fake6502_t *context);
int irq6502(fake6502_t *context);
int nmi6502(fake6502_t *context);
int exec6502(fake6502_t *context, uint32_t tickcount);

/**
* These functions need to be provided
*/
// void read6502(uint16_t address);
// void write6502(uint16_t address, uint8_t value);

#endif
