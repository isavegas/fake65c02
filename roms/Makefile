SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -c02 -quiet -chklabels

TESTS = tests/test_stack.bin tests/test_65c02.bin
EXAMPLES = examples/hello_world.bin

.PHONY: all test clean examples test_6502 test_65c02

all: examples $(TESTS)

$(TESTS): %.bin: %.s %.lst lib.s
	${ASM} ${ASMFLAGS} -o $@ -L $(word 2, $^) $<

$(EXAMPLES): %.bin: %.s %.lst lib.s
	${ASM} ${ASMFLAGS} -o $@ -L $(word 2, $^) $<

%.lst:
	touch $@

%.s:
	touch $@

examples: $(EXAMPLES) ;

test_6502: ${TESTS}
	$(foreach var,$(TESTS),echo Running $(var):;../fake6502 $(var);)

test_65c02: ${TESTS}
	$(foreach var,$(TESTS),echo Running $(var):;../fake65c02 $(var);)

test: test_65c02 test_6502 ;

clean:
	-rm -f *.bin tests/*.bin examples/*.bin
