SHELL= /bin/sh

ASM= vasm6502_oldstyle
ASMFLAGS= -Fbin -esc -dotdir
CC= clang
CFLAGS= -Iinclude
RELEASE_CFLAGS= -O2
DEBUG_CFLAGS= -g -O0
LDFLAGS= -fuse-ld=lld
CLANG_TIDY= clang-tidy
CLANG_FORMAT= clang-format
LIBS=

OBJS= main.o fake6502.o
ASM_BINS= rom hello_world

fake6502: ${OBJS} ${ASM_BINS}
	${CC} ${CFLAGS} ${RELEASE_CFLAGS} ${LDFLAGS} -o $@ ${OBJS} ${LIBS}

fake6502_debug: ${OBJS} ${ASM_BINS}
	${CC} ${CFLAGS} ${DEBUG_CFLAGS} ${LDFLAGS} -o $@ ${OBJS} ${LIBS}

$(OBJS): %.o: %.c

%.c:
	touch $@
rom: rom.s
	${ASM} ${ASMFLAGS} -o rom rom.s
hello_world: hello_world.s
	${ASM} ${ASMFLAGS} -o hello_world hello_world.s

%.s:
	touch $@

format:
	${CLANG_FORMAT}

tidy:
	${CLANG_TIDY} -checks='*' main.c

tidy_all:
	${CLANG_TIDY} -checks='*' main.c fake6502.c

clean:
	-rm -f fake6502 fake6502_debug rom *.o
