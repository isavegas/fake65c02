SHELL= /bin/sh

ASM ?= vasm6502_oldstyle
ASMFLAGS ?= -Fbin -esc -dotdir

ROMS= rom.rom hello_world.rom

$(ROMS): %.rom: %.s
	${ASM} ${ASMFLAGS} -o $@ $<

%.s:
	touch $@

test: ${ROMS}
	$(foreach var,$(ROMS),echo running $(var):;../fake6502 $(var);)

clean:
	-rm -f *.rom
