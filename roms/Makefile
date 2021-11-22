SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASM_FLAGS ?= -Fbin -esc -c02 -quiet -chklabels
DEBUG ?=

ifdef DEBUG
	ASM_FLAGS += -DDEBUG
endif

TESTS = tests/test_stack.bin tests/test_65c02.bin
EXAMPLES = examples/hello_world.bin

.PHONY: all test clean examples test_6502 test_65c02

all: examples $(TESTS)

$(TESTS): %.bin: %.s %.lst lib.s
	${ASM} ${ASM_FLAGS} -o $@ -L $(word 2, $^) $<

$(EXAMPLES): %.bin: %.s %.lst lib.s
	${ASM} ${ASM_FLAGS} -o $@ -L $(word 2, $^) $<

%.lst:
	touch $@

%.s:
	touch $@

examples: $(EXAMPLES) ;

test: ${TESTS}
	$(foreach var,$(TESTS),echo Running $(var):;../fake65c02 $(var);)

clean:
	-rm -f *.bin tests/*.bin examples/*.bin
