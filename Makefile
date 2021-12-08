SHELL= /bin/sh

# Kept specifically for use with compilers not supported by meson

CC = clang
CFLAGS ?= -march=native -Werror=implicit-function-declaration -DWRITABLE_VECTORS -DUNDOCUMENTED -fPIC -Iinclude
export DEBUG ?=
RELEASE_CFLAGS ?= -O2
DEBUG_CFLAGS ?= -g -O0 -DDEBUG
LDFLAGS ?= -fuse-ld=lld

# If CC contains clang
ifneq (,$(findstring clang,$(CC)))
	RELEASE_CFLAGS += -flto=thin
	LDFLAGS += -flto=thin
endif

ifdef DEBUG
	CFLAGS += ${DEBUG_CFLAGS}
else
  CFLAGS += ${RELEASE_CFLAGS}
endif

OBJS = src/main.o src/fake65c02.o
SUBPROJS = roms

OUT_BINS = build/fake65c02
OUT_LIBS = build/libfake65c02.so build/libfake65c02.a

.PHONY: all ${SUBPROJS}

all: ${OUT_BINS} ${OUT_LIBS} ${SUBPROJS}

build/fake65c02: src/main.o src/fake65c02.o
	@mkdir -p $(@D)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^

build/libfake65c02.so: src/fake65c02.o
	@mkdir -p $(@D)
	${CC} ${CFLAGS} -shared ${LDFLAGS} -o $@ $^

build/libfake65c02.a: src/fake65c02.o
	@mkdir -p $(@D)
	#${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^
	ar -rcs $@ $^

# We need to prevent this from catching other targets
${filter %.o,${OBJS}}: %.o: %.c
	${CC} ${CFLAGS} ${LDFLAGS} -c -o $@ $^

${SUBPROJS}:
	${MAKE} -C $@

test: all roms
	${MAKE} -C roms test

clean:
	-rm -f ${OUT_BINS} ${OUT_LIBS} src/*.o
