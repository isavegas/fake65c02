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

[65c02's (and 65816's) added instructions, relevant to stacks](http://wilsonminesco.com/stacks/65c02added_stack_inst.html)
provided a nice list of added instructions for me to implement, along with notes
on what they do. The list has been copied and reformatted below, with checkmarks on
their status.


### Changes to fake6502.c

* Clang-format used on the source file
* Line defining `NES_CPU` commented out
* Line defining `UNDOCUMENTED` has been commented out

### Instructions added in fake65c02.c

- [ ] `PHX` - `$DA` - push X onto the hardware stack, without disturbing A
- [ ] `PLX` - `$FA` - pull X  off the hardware stack, without disturbing A
- [ ] `PHY` - `$5A` - push Y onto the hardware stack, without disturbing A
- [ ] `PLY` - `$7A` - pull Y  off the hardware stack, without disturbing A
- [ ] `STZ ZP,X` - `$74` - At the ZP addr indicated by the operand plus X, store 00.
- [ ] `STZ abs,X` - `$9E` - At the 16-bit addr indicated by the operand plus X, store 00.
- [ ] `BIT ZP,X` - `$34` - (new addressing mode for the BIT instruction)
- [ ] `BIT abs,X` - `$3C` - (new addressing mode for the BIT instruction)
- [ ] `JMP (abs,X)` - `$7C` - (new addressing mode for the JMP instruction)
