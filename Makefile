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
TIDY_FLAGS ?= -checks='*'
CLANG_FORMAT ?= clang-format
FORMAT_FLAGS ?=
LIBS ?=

ifdef FIX
	TIDY_FLAGS += -fix-errors
endif

CFLAGS += -D FAKE${CPU}
CFLAGS += -D WRITABLE_VECTORS
CFLAGS += -D UNDOCUMENTED

OBJS= main.o fake6502.o fake65c02.o
SUBPROJS= roms

.PHONY: all $(SUBPROJS)

all: fake6502 fake65c02

fake6502: main.o fake6502.o
	${CC} ${CFLAGS} ${INCLUDES} ${RELEASE_CFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}

fake65c02: main.o fake65c02.o
	${CC} ${CFLAGS} ${INCLUDES} ${RELEASE_CFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}

$(OBJS): %.o: %.c

%.c:
	touch $@

subprojects: $(SUBPROJS)
	$(foreach var,$(SUBPROJS),echo building $(var):;$(MAKE) -C $(var);)

test: fake6502 ${ROMS}
	$(MAKE) -C roms test

format:
	${CLANG_FORMAT} --style=llvm -i *.c

tidy:
	${CLANG_TIDY} ${TIDY_FLAGS} main.c -- ${CFLAGS}

tidy_all:
	${CLANG_TIDY} ${TIDY_FLAGS} main.c fake6502.c -- ${CFLAGS}

clean:
	-rm -f fake6502 fake6502_debug *.o
	$(MAKE) -C roms clean
