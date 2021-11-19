SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir
CC = clang
CFLAGS ?= -march=native
INCLUDES += -Iinclude
RELEASE_CFLAGS ?= -O2
DEBUG_CFLAGS ?= -g -O0
LDFLAGS ?= -fuse-ld=lld
CLANG_TIDY ?= clang-tidy
CLANG_FORMAT ?= clang-format
LIBS ?=

OBJS= main.o fake6502.o
SUBPROJS= roms

.PHONY: all

all: fake6502

fake6502: ${OBJS} ${ROMS}
	${CC} ${CFLAGS} ${INCLUDES} ${RELEASE_CFLAGS} ${LDFLAGS} -o $@ ${OBJS} ${LIBS}

fake6502_debug: ${OBJS} ${ROMS}
	${CC} ${CFLAGS} ${INCLUDES} ${DEBUG_CFLAGS} ${LDFLAGS} -o $@ ${OBJS} ${LIBS}

$(OBJS): %.o: %.c

%.c:
	touch $@

subprojects: $(SUBPROJS)
	$(foreach var,$(SUBPROJS),echo building $(var):;$(MAKE) -C $(var);)

format:
	${CLANG_FORMAT}

test: fake6502 ${ROMS}
	./fake6502 rom
	./fake6502 hello_world

tidy:
	${CLANG_TIDY} -checks='*' main.c

tidy_all:
	${CLANG_TIDY} -checks='*' main.c fake6502.c

clean:
	-rm -f fake6502 fake6502_debug *.o
	$(MAKE) -C roms clean
