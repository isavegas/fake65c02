SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir
CPU ?= 65c02

ROMS= rom.bin hello_world.bin

ifeq "${CPU}" "65c02"
  ASMFLAGS += -c02
	ROMS += 6502_functional_test.bin
	ROMS += 6502_decimal_test.bin
endif

.PHONY: all functional_tests

all: $(ROMS)

$(ROMS): %.bin: %.s
	${ASM} ${ASMFLAGS} -o $@ $<

functional_tests: 6502_functional_test.bin 6502_decimal_test.bin

6502_functional_test.bin:
	sh -c "cd 6502_65C02_functional_tests/ca65 && ./asm_cmp.sh 6502_functional_test && mv 6502_functional_test.bin ../../"

6502_decimal_test.bin:
	sh -c "cd 6502_65C02_functional_tests/ca65 && ./asm_cmp.sh 6502_decimal_test && mv 6502_decimal_test.bin ../../"

%.s:
	touch $@

test: ${ROMS}
	$(foreach var,$(ROMS),echo running $(var):;../fake6502 $(var);)

clean:
	-rm -f *.bin
