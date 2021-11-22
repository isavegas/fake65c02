SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir
CC = clang -Werror=implicit-function-declaration
CFLAGS ?= -march=native
INCLUDES +=
RELEASE_CFLAGS ?= -O2
export DEBUG ?=
DEBUG_CFLAGS ?= -g -O0 -D DEBUG
LDFLAGS ?= -fuse-ld=lld
CLANG_TIDY ?= clang-tidy
TIDY_FLAGS ?= -checks='*'
CLANG_FORMAT ?= clang-format
FORMAT_FLAGS ?=

ifdef FIX
	TIDY_FLAGS += -fix-errors
endif

ifdef DEBUG
	CFLAGS += ${DEBUG_CFLAGS}
else
  CFLAGS += ${RELEASE_CFLAGS}
endif

CFLAGS += -D WRITABLE_VECTORS
CFLAGS += -D UNDOCUMENTED
CFLAGS += -fPIC

OBJS = main.o fake65c02.o
SUBPROJS = roms

OUT_BINS = fake65c02
OUT_LIBS = fake65c02.so

.PHONY: all ${SUBPROJS}

all: ${OUT_BINS} ${OUT_LIBS}

fake65c02: main.o fake65c02.o
	${CC} ${CFLAGS} ${INCLUDES} ${LDFLAGS} -o $@ $^

fake65c02.so: fake65c02.o
	${CC} ${CFLAGS} -shared ${INCLUDES} ${LDFLAGS} -o $@ $^

# We need to prevent this from catching other targets
${filter %.o,${OBJS}}: %.o: %.c

${SUBPROJS}: %
	${MAKE} -C $@

test: all roms
	${MAKE} -C roms test

format:
	${CLANG_FORMAT} --style=llvm -i main.c

tidy:
	${CLANG_TIDY} ${TIDY_FLAGS} main.c -- ${CFLAGS}

tidy_all:
	${CLANG_TIDY} ${TIDY_FLAGS} main.c fake6502.c -- ${CFLAGS}

clean:
	-rm -f ${OUT_BINS} ${OUT_LIBS} *.o
