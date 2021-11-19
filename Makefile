SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir
CC = clang -Werror=implicit-function-declaration
CFLAGS ?= -march=native
INCLUDES +=
RELEASE_CFLAGS ?= -O2
DEBUG_CFLAGS ?= -g -O0
LDFLAGS ?= -fuse-ld=lld
CLANG_TIDY ?= clang-tidy
CLANG_FORMAT ?= clang-format
LIBS ?=
CPU ?= 6502

ifeq "${CPU}" "65C02"
	override CPU = 65c02
endif

CFLAGS += -D FAKE${CPU}

OBJS= main.o fake${CPU}.o
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
	$(MAKE) -C roms test

tidy:
	${CLANG_TIDY} -checks='*' main.c

tidy_all:
	${CLANG_TIDY} -checks='*' main.c fake6502.c

clean:
	-rm -f fake6502 fake6502_debug *.o
	$(MAKE) -C roms clean
