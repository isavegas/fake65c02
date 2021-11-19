## Fake6502 project

Ben Eater's videos on using the 6502 to build a breadboard computer
left me aching to play with a 6502 of my own. This project is being
developed so I can work on 6502 software without building my own
breadboard computer, although I'm still planning on doing that down
the line.

## Attribution

Mike Chamber's [fake6502](http://rubbermallet.org/fake6502.c) provides
everything I needed to get up and running with 6502 emulation.
The version in my repository has had `clang-format` run on it and has had a
few small changes made to it, and I have started working on a modified
version (`fake65c02`) to add 65c02 operators to it.

[Differences between NMOS 6502 and CMOS 65c02](http://wilsonminesco.com/NMOS-CMOSdif/)
provided a nice list of changes for me to implement. The instructions are listed in
a table below for ease of access when working on my fake65c02.c


### Changes to fake6502.c

* Clang-format used on the source file
* Line defining `NES_CPU` commented out
* Line defining `UNDOCUMENTED` has been commented out

### Instructions added in the CMOS 65c02

Instruction   | OP        | Description
--------------|-----------|-----------------------------------------------------
`PHX`         | `$DA`     | push X onto the hardware stack, without disturbing A
`PLX`         | `$FA`     | pull X  off the hardware stack, without disturbing A
`PHY`         | `$5A`     | push Y onto the hardware stack, without disturbing A
`PLY`         | `$7A`     | pull Y  off the hardware stack, without disturbing A
`STZ ZP,X`    | `$74`     | At the ZP addr indicated by the operand plus X, store 00.
`STZ abs,X`   | `$9E`     | At the 16|bit addr indicated by the operand plus X, store 00.
`BIT ZP,X`    | `$34`     | (new addressing mode for the BIT instruction)
`BIT abs,X`   | `$3C`     | (new addressing mode for the BIT instruction)
`JMP (abs,X)` | `$7C`     | (new addressing mode for the JMP instruction)
`BRA rel`     | `$80`     | Branch Relative Always (unconditionally), range -128 to +127
`TRB addr`    | `$1C`     | Test & Reset memory Bits with A.
`TRB ZP`      | `$14`     | Test & Reset memory Bits with A.
`TSB addr`    | `$0C`     | Test & Set memory Bits with A.
`TSB ZP`      | `$04`     | Test & Set memory Bits with A.

### Instructions added in the WDC and Rockwell variants of the 65c02

Instruction   | OP        | Description
--------------|-----------|-----------------------------------------------------
`BBR ZP`      | `$0F-$7F` | Branch if specified Bit is Reset. ‾⌉ These are most useful
`BBS ZP`      | `$8F-$FF` | Branch if specified Bit is Set.    | when I/O is in ZP.  They
`RMB ZP`      | `$07-$77` | Reset specified Memory Bit.        | are on WDC & Rockwell but
`SMB ZP`      | `$87-$F7` | Set specified Memory Bit.         _⌋ not GTE/CMD or Synertek.


### Instructions added in the WDC variant of the 65c02

Instruction   | OP        | Description
--------------|-----------|-----------------------------------------------------
`STP`         | $DB       | SToP the processor until the next RST.
`WAI`         | $CB       | WAIt.  It's like STP, but any interrupt will resume execution.

### TODO
- [ ] `PHX`
- [ ] `PLX`
- [ ] `PHY`
- [ ] `PLY`
- [ ] `STZ ZP,X`
- [ ] `STZ abs,X`
- [ ] `BIT ZP,X`
- [ ] `BIT abs,X`
- [ ] `JMP (abs,X)`
- [ ] `BRA`
- [ ] `TRB addr`
- [ ] `TRB ZP`
- [ ] `TSB addr`
- [ ] `TSB ZP`
- [ ] `BBR ZP`
- [ ] `BBS ZP`
- [ ] `RMB ZP`
- [ ] `SMB ZP`
- [ ] `STP`
- [ ] `WAI`
