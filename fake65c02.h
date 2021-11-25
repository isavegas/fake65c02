#ifndef FAKE65C02_H
#define FAKE65C02_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct fake65c02 fake65c02_t;

struct fake65c02 {
    void *m;

    uint8_t (*read)(fake65c02_t *ctx, uint16_t address);
    void (*write)(fake65c02_t *ctx, uint16_t address, uint8_t value);
    void (*hook)(fake65c02_t *ctx);

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

fake65c02_t *new_fake65c02(void* m);
void free_fake65c02(fake65c02_t *context);

int reset65c02(fake65c02_t *context);
int step65c02(fake65c02_t *context);
int irq65c02(fake65c02_t *context);
int nmi65c02(fake65c02_t *context);
int exec65c02(fake65c02_t *context, uint32_t tickcount);

#endif
