## Fake65c02, an enhanced fork of [fake6502](http://rubbermallet.org/fake6502.c)

Ben Eater's videos on using the 6502 to build a breadboard computer
left me aching to play with a 6502 of my own. This project is being
developed so I can work on 6502 software without building my own
breadboard computer, although I'm still planning on doing that down
the line.

This project includes an application to run 6502 code with a simple IO
framework for writing to serial, halting the CPU, and (todo) handle
interrupts. See the `roms` subdirectory for various small assembly
files. Of particular note is `roms/tests/test65c02.s`, which tests
each added CMOS instruction and prints pass/fail messages to the designated
serial port memory address in `roms/lib.s`. Note that you *must* have an up-to-date
version of vasm6502_oldstyle, as older versions do not support `include ../`
directives. I had to compile it from source for my roms to build on FreeBSD.

## Licensing

`fake65c02.c` and `fake65c02.h` are released and maintained in the public
domain to respect Mike Chambers' work and contribution to the public domain.

Everything else is licensed under the [MIT license](./LICENSE).


## Notes on usage

In order to provide a method of pairing a `fake65c02_t` instance with your
own data structure, a `void* m` field is included in the struct. It is unused
by `fake65c02.c` and is reserved solely for use as a pointer to your data
structure. An example of its usage can be found in `main.c` as a method of
determining how to access each `fake65c02_t`'s ram/rom banks from a static and
shared memory access functions. Feel free to give it a `NULL` value if you
will not be using it in your `read` and `write` functions.

## Differences from [fake6502.c](http://rubbermallet.org/fake6502.c)

* Added CMOS instructions [(see below)](#instructions_implemented)
* Refactored API to use a `fake65c02_t` struct as context, ala Lua
* Added a header file for ease of use, although the API is *not* very stable at the moment.

## Attribution

Mike Chamber's [fake6502](http://rubbermallet.org/fake6502.c) provides
everything I needed to get up and running with 6502 emulation. His
work allowed me to get this project (and dependent projects) up and running
far faster than if I had had to write my own emulator from scratch.

[Differences between NMOS 6502 and CMOS 65c02](http://wilsonminesco.com/NMOS-CMOSdif/)
provided a nice list of changes for me to implement. The instructions are listed in
a table below for ease of access when working on my fake65c02.c


### Instructions added in the CMOS 65c02

Instruction   | OP        | Description
--------------|-----------|-----------------------------------------------------
`PHX`         | `$DA`     | push X onto the hardware stack, without disturbing A.
`PLX`         | `$FA`     | pull X  off the hardware stack, without disturbing A.
`PHY`         | `$5A`     | push Y onto the hardware stack, without disturbing A.
`PLY`         | `$7A`     | pull Y  off the hardware stack, without disturbing A.
`STZ abs`     | `$9C`     | At the 16 bit addr indicated by the operand, store zero.
`STZ abs,X`   | `$9E`     | At the 16 bit addr indicated by the operand plus X, store zero.
`STZ ZP`      | `$64`     | At the ZP addr indicated by the operand, store zero.
`STZ ZP,X`    | `$74`     | At the ZP addr indicated by the operand plus X, store zero.
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
`BBR ZP`      | `$0F-$7F` | Branch if specified Bit is Reset.
`BBS ZP`      | `$8F-$FF` | Branch if specified Bit is Set.
`RMB ZP`      | `$07-$77` | Reset specified Memory Bit.
`SMB ZP`      | `$87-$F7` | Set specified Memory Bit.


### Instructions added in the WDC variant of the 65c02

Instruction   | OP        | Description
--------------|-----------|-----------------------------------------------------
`STP`         | $DB       | SToP the processor until the next RST.
`WAI`         | $CB       | WAIt.  It's like STP, but any interrupt will resume execution.

### Instructions Implemented

#### CMOS
- [X] `PHX`
- [X] `PLX`
- [X] `PHY`
- [X] `PLY`
- [X] `STZ abs`
- [X] `STZ ZP`
- [X] `STZ abs,X`
- [X] `STZ ZP,X`
- [X] `BIT ZP,X`
- [X] `BIT abs,X`
- [X] `JMP (abs,X)`
- [X] `BRA`
- [X] `TRB addr`
- [X] `TRB ZP`
- [X] `TSB addr`
- [X] `TSB ZP`

#### WDC and Rockwell
- [X] `BBR ZP`
- [X] `BBS ZP`
- [X] `RMB ZP`
- [X] `SMB ZP`

#### WDC
- [X] `STP`
- [X] `WAI`
