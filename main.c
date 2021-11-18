#include <stddef.h>
#include <stdint.h>
// NOLINTNEXTLINE(llvmlibc-restrict-system-libc-headers)
#include <stdio.h>

#include "include/fake6502.h"

#define IO_OUT 0x0300

#define ROM_SIZE 32768
uint8_t ROM[ROM_SIZE]; // NOLINT

uint8_t read6502(uint16_t address) {
    if (address > ROM_SIZE) {
        address -= ROM_SIZE;
    }
    uint8_t value = ROM[(unsigned int)address];
    return value;
}

void write6502(uint16_t address, uint8_t value) {
    if (address == IO_OUT) {
        printf("%c", value);
    } else {
        ROM[(int)address] = value;
    }
}

// Fill ROM with noop
void initialize_rom() {
    for (int i = 0; i < ROM_SIZE; i++) {
        ROM[i] = 0xea; // NOLINT
    }
}

int load_rom(char* path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return 0;
    } else {
        unsigned char buffer[4096];
        unsigned int p = 0;
        size_t size;
        while ((size = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            for (int i = 0; i < size; i++) {
                ROM[p+1] = buffer[i];
                /*if ( i % 2 == 0 ) {
                    ROM[p+1] = buffer[i];
                } else {
                    ROM[p-1] = buffer[i];
                }*/
                p++;
            }
        }
    }
    return 1;
}

void l(){
    //printf("Clock: %i, Instructions: %i\n", clockticks6502, instructions);
}

#undef NES_CPU
#define UNDOCUMENTED

int main(int argc, char* argv[]) {
    initialize_rom();
    if (argc < 2) {
        printf("Please supply a rom\n");
        return 1;
    } else {
        if (load_rom(argv[1])) {
            l();
            reset6502();
            l();
            for (int i = 0; i < 100; i++) {
                step6502();
                l();
            }
            return 0;
        } else {
            printf("Unable to read rom\n");
        return 1;
        }
    }
}
