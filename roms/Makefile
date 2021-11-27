SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASM_FLAGS ?= -Fbin -esc -c02 -quiet -chklabels
DEBUG_FLAGS = -DDEBUG

ifdef DEBUG
	ASM_FLAGS += -DDEBUG
	override DEBUG_FLAGS=
endif

TESTS = tests/test_stack.bin tests/test_65c02.bin
TESTS_DEBUG = tests/test_hook.bin
BENCHES = benches/fibonacci.bin
EXAMPLES = examples/hello_world.bin

.PHONY: all test clean examples

all: examples ${BENCHES} ${TESTS} ${TESTS_DEBUG}

${TESTS}: %.bin: %.s %.lst lib.s
	${ASM} ${ASM_FLAGS} -o $@ -L ${word 2, $^} $<

# Always build TESTS_DEBUG with DEBUG defined
${TESTS_DEBUG}: %.bin: %.s %.lst lib.s
	${ASM} ${ASM_FLAGS} ${DEBUG_FLAGS} -o $@ -L ${word 2, $^} $<

${EXAMPLES}: %.bin: %.s %.lst lib.s
	${ASM} ${ASM_FLAGS} -o $@ -L ${word 2, $^} $<

${BENCHES}: %.bin: %.s %.lst lib.s
	${ASM} ${ASM_FLAGS} -o $@ -L ${word 2, $^} $<

%.lst:
	touch $@

%.s:
	touch $@

examples: ${EXAMPLES} ;

test: ${TESTS} ${TESTS_DEBUG}
	${foreach var,${TESTS},../fake65c02 ${var};}
	${foreach var,${TESTS_DEBUG},../fake65c02 ${var};}

clean:
	-rm -f *.bin tests/*.bin examples/*.bin
