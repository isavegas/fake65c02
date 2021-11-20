SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir -c02
CPU ?= 65c02

TESTS = tests/test_stack.bin tests/test_65c02.bin
EXAMPLES = examples/hello_world.bin

.PHONY: all test clean examples

all: examples $(TESTS)

$(TESTS): %.bin: %.s lib.s
	${ASM} ${ASMFLAGS} -o $@ $<

$(EXAMPLES): %.bin: %.s lib.s
	${ASM} ${ASMFLAGS} -o $@ $<

%.s:
	touch $@

examples: $(EXAMPLES) ;

test_6502: ${TESTS}
	$(foreach var,$(TESTS),echo running $(var):;../fake6502 $(var);)

test_65c02: ${TESTS}
	$(foreach var,$(TESTS),echo running $(var):;../fake65c02 $(var);)

test: test_6502 test_65c02 ;

clean:
	-rm -f *.bin tests/*.bin examples/*.bin
