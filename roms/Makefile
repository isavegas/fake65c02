SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir
CPU ?= 65c02
#FUNC_TESTS ?=

ROMS = tests/rom.bin tests/hello_world.bin tests/test_stack.bin

ifeq "${CPU}" "65c02"
  ASMFLAGS += -c02
	ROMS += tests/test_65c02.bin
	ifdef FUNC_TESTS
		ROMS += 6502_functional_test.bin
		ROMS += 6502_decimal_test.bin
	endif
endif

.PHONY: all functional_tests

all: $(ROMS) functional_tests

$(ROMS): tests/%.bin: tests/%.s
	${ASM} ${ASMFLAGS} -o $@ $<

functional_tests:
	sh -c "cd 6502_65C02_functional_tests/ca65 && ./asm_cmp.sh 6502_functional_test && mv 6502_functional_test.bin ../../"
	sh -c "cd 6502_65C02_functional_tests/ca65 && ./asm_cmp.sh 6502_decimal_test && mv 6502_decimal_test.bin ../../"

%.s:
	touch $@

#as65:
#	unzip 6502_65C02_functional_tests/as65_142.zip as65

test: ${ROMS}
	$(foreach var,$(ROMS),echo running $(var):;../fake6502 $(var);)

clean:
	-rm -f *.bin tests/*.bin
