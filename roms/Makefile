SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir
CPU ?= 65c02

ROMS= rom.rom hello_world.rom

ifeq "${CPU}" "65c02"
  ASMFLAGS += -c02
	ROMS += test_65c02.rom
endif

$(ROMS): %.rom: %.s
	${ASM} ${ASMFLAGS} -o $@ $<

%.s:
	touch $@

test: ${ROMS}
	$(foreach var,$(ROMS),echo running $(var):;../fake6502 $(var);)

clean:
	-rm -f *.rom
