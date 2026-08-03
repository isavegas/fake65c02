/* Fake65c02 CPU emulator core ***********************
 * (c)2011 Mike Chambers (miker00lz@gmail.com)       *
 * (c)2021 Kenzi Jeanis (kenzi.jeanis@isavegas.dev)  *
 *****************************************************
 * v2.0 - Added 65c02 instructions                   *
 *        Switched from globals to a cpu based   *
 *        API. (Nov. 20, 2021)                       *
 * v1.1 - Small bugfix in BIT opcode, but it was the *
 *        difference between a few games in my NES   *
 *        emulator working and being broken!         *
 *        I went through the rest carefully again    *
 *        after fixing it just to make sure I didn't *
 *        have any other typos! (Dec. 17, 2011)      *
 *                                                   *
 * v1.0 - First release (Nov. 24, 2011)              *
 *****************************************************
 * LICENSE: This source code is released into the    *
 * public domain, but if you use it please do give   *
 * credit. I put a lot of effort into writing this!  *
 *                                                   *
 *****************************************************
 * LICENSE for modifications by Kenzi Jeanis:        *
 * All modifications I've made to this file are also *
 * released into the public domain.                  *
 *                                                   *
 *****************************************************
 * Notes by Mike Chambers                            *
 *                                                   *
 * Fake6502 is a MOS Technology 6502 CPU emulation   *
 * engine in C. It was written as part of a Nintendo *
 * Entertainment System emulator I've been writing.  *
 *                                                   *
 * A couple important things to know about are two   *
 * defines in the code. One is "UNDOCUMENTED" which, *
 * when defined, allows Fake6502 to compile with     *
 * full support for the more predictable             *
 * undocumented instructions of the 6502. If it is   *
 * undefined, undocumented opcodes just act as NOPs. *
 *                                                   *
 * The other define is "NES_CPU", which causes the   *
 * code to compile without support for binary-coded  *
 * decimal (BCD) support for the ADC and SBC         *
 * opcodes. The Ricoh 2A03 CPU in the NES does not   *
 * support BCD, but is otherwise identical to the    *
 * standard MOS 6502. (Note that this define is      *
 * enabled in this file if you haven't changed it    *
 * yourself. If you're not emulating a NES, you      *
 * should comment it out.)                           *
 *                                                   *
 * If you do discover an error in timing accuracy,   *
 * or operation in general please e-mail me at the   *
 * address above so that I can fix it. Thank you!    *
 *                                                   *
 *****************************************************
 * Notes by Kenzi Jeanis                             *
 *                                                   *
 * Ben Eater's YouTube series on building a          *
 * breadboard computer using a 65c02 left me wanting *
 * to work on a 65c02 project of my own. As I don't  *
 * currently have the components necessary to follow *
 * along with his videos, I decided to set up an     *
 * emulated 65c02 with which I could run 65c02       *
 * roms built with VASM. Unfortunately, Fake6502     *
 * doesn't support any 65c02 instructions, so I      *
 * opted to fork it and extend the supported         *
 * instruction set, along with making it a bit more  *
 * pleasant to use for my purposes.                  *
 *                                                   *
 *****************************************************/

#include <stdlib.h>

#include "fake65c02.h"

fake65c02_t *new_fake65c02(void *context) {
  fake65c02_t *cpu = calloc(1, sizeof(fake65c02_t));
  cpu->context = context;
  return cpu;
}

void free_fake65c02(fake65c02_t *cpu) { free(cpu); }

#define FLAG_CARRY 0x01
#define FLAG_ZERO 0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL 0x08
#define FLAG_BREAK 0x10
#define FLAG_CONSTANT 0x20
#define FLAG_OVERFLOW 0x40
#define FLAG_SIGN 0x80

// NOLINTNEXTLINE(modernize-macro-to-enum)
#define BASE_STACK 0x100

#define saveaccum(cpu, n) cpu->a = (uint8_t)((n)&0x00FF)

// flag modifier macros
#define setcarry(cpu) cpu->status |= FLAG_CARRY
#define clearcarry(cpu) cpu->status &= (~FLAG_CARRY)
#define setzero(cpu) cpu->status |= FLAG_ZERO
#define clearzero(cpu) cpu->status &= (~FLAG_ZERO)
#define setinterrupt(cpu) cpu->status |= FLAG_INTERRUPT
#define clearinterrupt(cpu) cpu->status &= (~FLAG_INTERRUPT)
#define setdecimal(cpu) cpu->status |= FLAG_DECIMAL
#define cleardecimal(cpu) cpu->status &= (~FLAG_DECIMAL)
#define setoverflow(cpu) cpu->status |= FLAG_OVERFLOW
#define clearoverflow(cpu) cpu->status &= (~FLAG_OVERFLOW)
#define setsign(cpu) cpu->status |= FLAG_SIGN
#define clearsign(cpu) cpu->status &= (~FLAG_SIGN)

// flag calculation macros
#define zerocalc(cpu, n)                                                   \
  {                                                                            \
    if ((n)&0x00FF)                                                            \
      clearzero(cpu);                                                      \
    else                                                                       \
      setzero(cpu);                                                        \
  }

#define signcalc(cpu, n)                                                   \
  {                                                                            \
    if ((n)&0x0080)                                                            \
      setsign(cpu);                                                        \
    else                                                                       \
      clearsign(cpu);                                                      \
  }

#define carrycalc(cpu, n)                                                  \
  {                                                                            \
    if ((n)&0xFF00)                                                            \
      setcarry(cpu);                                                       \
    else                                                                       \
      clearcarry(cpu);                                                     \
  }

#define overflowcalc(cpu, n, m, o)                                         \
  { /* n = result, m = accumulator, o = memory */                              \
    if (((n) ^ (uint16_t)(m)) & ((n) ^ (o)) & 0x0080)                          \
      setoverflow(cpu);                                                    \
    else                                                                       \
      clearoverflow(cpu);                                                  \
  }

// a few general functions used by various other functions
void push16(fake65c02_t *cpu, uint16_t pushval) {
  cpu->write(cpu, BASE_STACK + ((uint16_t)cpu->sp), pushval >> 8);
  cpu->write(cpu, BASE_STACK + (((uint16_t)cpu->sp) - 1),
                 pushval & 0xff);
  cpu->sp -= 2;
}

void push8(fake65c02_t *cpu, uint8_t pushval) {
  cpu->write(cpu, BASE_STACK + cpu->sp, pushval);
  cpu->sp--;
}

uint16_t pull16(fake65c02_t *cpu) {
  uint16_t temp16 = cpu->read(cpu, BASE_STACK + cpu->sp + 1);
  temp16 |= (cpu->read(cpu, BASE_STACK + cpu->sp + 2) << 8);

  cpu->sp += 2;

  return temp16;
}

uint8_t pull8(fake65c02_t *cpu) {
  cpu->sp++;
  return (cpu->read(cpu, BASE_STACK + cpu->sp));
}

int reset65c02(fake65c02_t *cpu) {
  if (cpu->read == NULL || cpu->write == NULL) {
    return 0;
  }
  cpu->pc = (uint16_t)cpu->read(cpu, 0xFFFC) |
                ((uint16_t)cpu->read(cpu, 0xFFFD) << 8);
  cpu->a = 0;
  cpu->x = 0;
  cpu->y = 0;
  cpu->sp = 0xFD;
  cpu->status |= FLAG_CONSTANT;
  return 1;
}

static void (*addrtable[256])(fake65c02_t *cpu);
static void (*optable[256])(fake65c02_t *cpu);
uint8_t penaltyop, penaltyaddr;

// addressing mode functions, calculates effective addresses
static void imp(fake65c02_t *cpu) { (void)cpu; } // implied

static void acc(fake65c02_t *cpu) { (void)cpu; } // accumulator

static void imm(fake65c02_t *cpu) { // immediate
  cpu->ea = cpu->pc++;
}

static void zp(fake65c02_t *cpu) { // zero-page
  cpu->ea = (uint16_t)cpu->read(cpu, (uint16_t)cpu->pc++);
}

static void zpx(fake65c02_t *cpu) { // zero-page,X
  cpu->ea = ((uint16_t)cpu->read(cpu, (uint16_t)cpu->pc++) +
                 (uint16_t)cpu->x) &
                0xFF; // zero-page wraparound
}

static void zpy(fake65c02_t *cpu) { // zero-page,Y
  cpu->ea = ((uint16_t)cpu->read(cpu, (uint16_t)cpu->pc++) +
                 (uint16_t)cpu->y) &
                0xFF; // zero-page wraparound
}

static void rel(fake65c02_t *cpu) { // relative for branch ops (8-bit
                                        // immediate value, sign-extended)
  cpu->reladdr = (uint16_t)cpu->read(cpu, cpu->pc++);
  if (cpu->reladdr & 0x80)
    cpu->reladdr |= 0xFF00;
}

// specific to bbr/bbs/rmb/smb. Uses both zp and rel
static void zpr(fake65c02_t *cpu) {
  cpu->ea = (uint16_t)cpu->read(cpu, (uint16_t)cpu->pc++);
  cpu->reladdr = (uint16_t)cpu->read(cpu, cpu->pc++);
  if (cpu->reladdr & 0x80)
    cpu->reladdr |= 0xFF00;
}

static void abso(fake65c02_t *cpu) { // absolute
  cpu->ea = (uint16_t)(cpu->read(cpu, cpu->pc)) |
                ((uint16_t)cpu->read(cpu, cpu->pc + 1) << 8);
  cpu->pc += 2;
}

static void absx(fake65c02_t *cpu) { // absolute,X
  uint16_t startpage;
  cpu->ea = ((uint16_t)cpu->read(cpu, cpu->pc) |
                 ((uint16_t)cpu->read(cpu, cpu->pc + 1) << 8));
  startpage = cpu->ea & 0xFF00;
  cpu->ea += (uint16_t)cpu->x;

  if (startpage !=
      (cpu->ea &
       0xFF00)) { // one cycle penlty for page-crossing on some opcodes
    penaltyaddr = 1;
  }

  cpu->pc += 2;
}

static void absy(fake65c02_t *cpu) { // absolute,Y
  uint16_t startpage;
  cpu->ea = ((uint16_t)cpu->read(cpu, cpu->pc) |
                 ((uint16_t)cpu->read(cpu, cpu->pc += 1) << 8));
  startpage = cpu->ea & 0xFF00;
  cpu->ea += (uint16_t)cpu->y;

  if (startpage !=
      (cpu->ea &
       0xFF00)) { // one cycle penlty for page-crossing on some opcodes
    penaltyaddr = 1;
  }

  cpu->pc += 2;
}

static void ind(fake65c02_t *cpu) { // indirect
  uint16_t eahelp, eahelp2;
  eahelp = (uint16_t)cpu->read(cpu, cpu->pc) |
           (uint16_t)((uint16_t)cpu->read(cpu, cpu->pc + 1) << 8);
  eahelp2 =
      (eahelp & 0xFF00) |
      ((eahelp + 1) & 0x00FF); // replicate 65c02 page-boundary wraparound bug
  cpu->ea = (uint16_t)cpu->read(cpu, eahelp) |
                ((uint16_t)cpu->read(cpu, eahelp2) << 8);
  cpu->pc += 2;
}

static void indx(fake65c02_t *cpu) { // (indirect,X)
  uint16_t eahelp;
  eahelp = (uint16_t)(((uint16_t)cpu->read(cpu, cpu->pc++) +
                       (uint16_t)cpu->x) &
                      0xFF); // zero-page wraparound for table pointer
  cpu->ea = (uint16_t)cpu->read(cpu, eahelp & 0x00FF) |
                ((uint16_t)cpu->read(cpu, (eahelp + 1) & 0x00FF) << 8);
}

static void indy(fake65c02_t *cpu) { // (indirect),Y
  uint16_t eahelp, eahelp2, startpage;
  eahelp = (uint16_t)cpu->read(cpu, cpu->pc++);
  eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); // zero-page wraparound
  cpu->ea = (uint16_t)cpu->read(cpu, eahelp) |
                ((uint16_t)cpu->read(cpu, eahelp2) << 8);
  startpage = cpu->ea & 0xFF00;
  cpu->ea += (uint16_t)cpu->y;

  if (startpage !=
      (cpu->ea &
       0xFF00)) { // one cycle penlty for page-crossing on some opcodes
    penaltyaddr = 1;
  }
}

static uint16_t getvalue(fake65c02_t *cpu) {
  if (addrtable[cpu->opcode] == acc)
    return ((uint16_t)cpu->a);
  else
    return ((uint16_t)cpu->read(cpu, cpu->ea));
}

static void putvalue(fake65c02_t *cpu, uint16_t saveval) {
  if (addrtable[cpu->opcode] == acc)
    cpu->a = (uint8_t)(saveval & 0x00FF);
  else
    cpu->write(cpu, cpu->ea, (saveval & 0x00FF));
}

// instruction handler functions
static void adc(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->a + cpu->value +
                    (uint16_t)(cpu->status & FLAG_CARRY);

  carrycalc(cpu, cpu->result);
  zerocalc(cpu, cpu->result);
  overflowcalc(cpu, cpu->result, cpu->a, cpu->value);
  signcalc(cpu, cpu->result);

  if (cpu->nes_mode) {
    if (cpu->status & FLAG_DECIMAL) {
      clearcarry(cpu);

      if ((cpu->a & 0x000F) > 0x09) {
        cpu->a += 0x0006;
      }
      if ((cpu->a & 0x00F0) > 0x90) {
        cpu->a += 0x0060;
        setcarry(cpu);
      }

      cpu->clockticks++;
    }
  }

  saveaccum(cpu, cpu->result);
}

static void and (fake65c02_t * cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->a & cpu->value;

  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  saveaccum(cpu, cpu->result);
}

static void trb(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->result = (~(uint16_t)cpu->a) & cpu->value;

  zerocalc(cpu, (uint16_t)cpu->a & cpu->value);

  putvalue(cpu, cpu->result);
}

static void tsb(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->a | cpu->value;

  zerocalc(cpu, (uint16_t)cpu->a & cpu->value);

  putvalue(cpu, cpu->result);
}

static void asl(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = cpu->value << 1;

  carrycalc(cpu, cpu->result);
  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  putvalue(cpu, cpu->result);
}

static void bcc(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_CARRY) == 0) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void bcs(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_CARRY) == FLAG_CARRY) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void beq(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_ZERO) == FLAG_ZERO) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void bra(fake65c02_t *cpu) {
  cpu->oldpc = cpu->pc;
  cpu->pc += cpu->reladdr;
  if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
    cpu->clockticks += 2; // check if jump crossed a page boundary
  else
    cpu->clockticks++;
}

static void bit(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->a & cpu->value;

  zerocalc(cpu, cpu->result);
  cpu->status = (cpu->status & 0x3F) | (uint8_t)(cpu->value & 0xC0);
}

static void bmi(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_SIGN) == FLAG_SIGN) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void bne(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_ZERO) == 0) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void bpl(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_SIGN) == 0) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void brk(fake65c02_t *cpu) {
  cpu->pc++;
  push16(cpu, cpu->pc); // push next instruction address onto stack
  push8(cpu, cpu->status | FLAG_BREAK); // push CPU status to stack
  setinterrupt(cpu);                        // set interrupt flag
  cpu->pc = (uint16_t)cpu->read(cpu, 0xFFFE) |
                ((uint16_t)cpu->read(cpu, 0xFFFF) << 8);
}

static void bvc(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_OVERFLOW) == 0) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void bvs(fake65c02_t *cpu) {
  if ((cpu->status & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
    cpu->oldpc = cpu->pc;
    cpu->pc += cpu->reladdr;
    if ((cpu->oldpc & 0xFF00) != (cpu->pc & 0xFF00))
      cpu->clockticks += 2; // check if jump crossed a page boundary
    else
      cpu->clockticks++;
  }
}

static void clc(fake65c02_t *cpu) { clearcarry(cpu); }

static void cld(fake65c02_t *cpu) { cleardecimal(cpu); }

static void cli(fake65c02_t *cpu) { clearinterrupt(cpu); }

static void clv(fake65c02_t *cpu) { clearoverflow(cpu); }

static void cmp(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->a - cpu->value;

  if (cpu->a >= (uint8_t)(cpu->value & 0x00FF))
    setcarry(cpu);
  else
    clearcarry(cpu);
  if (cpu->a == (uint8_t)(cpu->value & 0x00FF))
    setzero(cpu);
  else
    clearzero(cpu);
  signcalc(cpu, cpu->result);
}

static void cpx(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->x - cpu->value;

  if (cpu->x >= (uint8_t)(cpu->value & 0x00FF))
    setcarry(cpu);
  else
    clearcarry(cpu);
  if (cpu->x == (uint8_t)(cpu->value & 0x00FF))
    setzero(cpu);
  else
    clearzero(cpu);
  signcalc(cpu, cpu->result);
}

static void cpy(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->y - cpu->value;

  if (cpu->y >= (uint8_t)(cpu->value & 0x00FF))
    setcarry(cpu);
  else
    clearcarry(cpu);
  if (cpu->y == (uint8_t)(cpu->value & 0x00FF))
    setzero(cpu);
  else
    clearzero(cpu);
  signcalc(cpu, cpu->result);
}

static void dec(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = cpu->value - 1;

  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  putvalue(cpu, cpu->result);
}

static void dea(fake65c02_t *cpu) {
  cpu->a--;

  zerocalc(cpu, cpu->a);
  signcalc(cpu, cpu->a);
}

static void dex(fake65c02_t *cpu) {
  cpu->x--;

  zerocalc(cpu, cpu->x);
  signcalc(cpu, cpu->x);
}

static void dey(fake65c02_t *cpu) {
  cpu->y--;

  zerocalc(cpu, cpu->y);
  signcalc(cpu, cpu->y);
}

static void eor(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->a ^ cpu->value;

  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  saveaccum(cpu, cpu->result);
}

static void inc(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = cpu->value + 1;

  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  putvalue(cpu, cpu->result);
}

static void ina(fake65c02_t *cpu) {
  cpu->a++;

  zerocalc(cpu, cpu->a);
  signcalc(cpu, cpu->a);
}

static void inx(fake65c02_t *cpu) {
  cpu->x++;

  zerocalc(cpu, cpu->x);
  signcalc(cpu, cpu->x);
}

static void iny(fake65c02_t *cpu) {
  cpu->y++;

  zerocalc(cpu, cpu->y);
  signcalc(cpu, cpu->y);
}

static void jmp(fake65c02_t *cpu) { cpu->pc = cpu->ea; }

static void jsr(fake65c02_t *cpu) {
  push16(cpu, cpu->pc - 1);
  cpu->pc = cpu->ea;
}

static void lda(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->a = (uint8_t)(cpu->value & 0x00FF);

  zerocalc(cpu, cpu->a);
  signcalc(cpu, cpu->a);
}

static void ldx(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->x = (uint8_t)(cpu->value & 0x00FF);

  zerocalc(cpu, cpu->x);
  signcalc(cpu, cpu->x);
}

static void ldy(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->y = (uint8_t)(cpu->value & 0x00FF);

  zerocalc(cpu, cpu->y);
  signcalc(cpu, cpu->y);
}

static void lsr(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = cpu->value >> 1;

  if (cpu->value & 1)
    setcarry(cpu);
  else
    clearcarry(cpu);
  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  putvalue(cpu, cpu->result);
}

static void nop(fake65c02_t *cpu) {
  // NOLINTNEXTLINE(bugprone-switch-missing-default-case)
  switch (cpu->opcode) {
  case 0x1C:
  case 0x3C:
  case 0x5C:
  case 0x7C:
  case 0xDC:
  case 0xFC:
    penaltyop = 1;
    break;
  }
}

static void ora(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu);
  cpu->result = (uint16_t)cpu->a | cpu->value;

  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  saveaccum(cpu, cpu->result);
}

static void pha(fake65c02_t *cpu) { push8(cpu, cpu->a); }
static void phx(fake65c02_t *cpu) { push8(cpu, cpu->x); }
static void phy(fake65c02_t *cpu) { push8(cpu, cpu->y); }

static void php(fake65c02_t *cpu) {
  push8(cpu, cpu->status | FLAG_BREAK);
}

static void pla(fake65c02_t *cpu) {
  cpu->a = pull8(cpu);

  zerocalc(cpu, cpu->a);
  signcalc(cpu, cpu->a);
}

static void plx(fake65c02_t *cpu) {
  cpu->x = pull8(cpu);

  zerocalc(cpu, cpu->x);
  signcalc(cpu, cpu->x);
}

static void ply(fake65c02_t *cpu) {
  cpu->y = pull8(cpu);

  zerocalc(cpu, cpu->y);
  signcalc(cpu, cpu->y);
}

static void plp(fake65c02_t *cpu) {
  cpu->status = pull8(cpu) | FLAG_CONSTANT;
}

static void rol(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result = (cpu->value << 1) | (cpu->status & FLAG_CARRY);

  carrycalc(cpu, cpu->result);
  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  putvalue(cpu, cpu->result);
}

static void ror(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  cpu->result =
      (cpu->value >> 1) | ((cpu->status & FLAG_CARRY) << 7);

  if (cpu->value & 1)
    setcarry(cpu);
  else
    clearcarry(cpu);
  zerocalc(cpu, cpu->result);
  signcalc(cpu, cpu->result);

  putvalue(cpu, cpu->result);
}

static void rti(fake65c02_t *cpu) {
  cpu->status = pull8(cpu);
  cpu->value = pull16(cpu);
  cpu->pc = cpu->value;
}

static void rts(fake65c02_t *cpu) {
  cpu->value = pull16(cpu);
  cpu->pc = cpu->value + 1;
}

static void sbc(fake65c02_t *cpu) {
  penaltyop = 1;
  cpu->value = getvalue(cpu) ^ 0x00FF;
  cpu->result = (uint16_t)cpu->a + cpu->value +
                    (uint16_t)(cpu->status & FLAG_CARRY);

  carrycalc(cpu, cpu->result);
  zerocalc(cpu, cpu->result);
  overflowcalc(cpu, cpu->result, cpu->a, cpu->value);
  signcalc(cpu, cpu->result);

#ifndef NES_CPU
  if (cpu->status & FLAG_DECIMAL) {
    clearcarry(cpu);

    cpu->a -= 0x66;
    if ((cpu->a & 0x0F) > 0x09) {
      cpu->a += 0x06;
    }
    if ((cpu->a & 0xF0) > 0x90) {
      cpu->a += 0x60;
      setcarry(cpu);
    }

    cpu->clockticks++;
  }
#endif

  saveaccum(cpu, cpu->result);
}

static void sec(fake65c02_t *cpu) { setcarry(cpu); }

static void sed(fake65c02_t *cpu) { setdecimal(cpu); }

static void sei(fake65c02_t *cpu) { setinterrupt(cpu); }

static void sta(fake65c02_t *cpu) { putvalue(cpu, cpu->a); }

static void stx(fake65c02_t *cpu) { putvalue(cpu, cpu->x); }

static void sty(fake65c02_t *cpu) { putvalue(cpu, cpu->y); }

static void stz(fake65c02_t *cpu) { putvalue(cpu, 0); }

static void tax(fake65c02_t *cpu) {
  cpu->x = cpu->a;

  zerocalc(cpu, cpu->x);
  signcalc(cpu, cpu->x);
}

static void tay(fake65c02_t *cpu) {
  cpu->y = cpu->a;

  zerocalc(cpu, cpu->y);
  signcalc(cpu, cpu->y);
}

static void tsx(fake65c02_t *cpu) {
  cpu->x = cpu->sp;

  zerocalc(cpu, cpu->x);
  signcalc(cpu, cpu->x);
}

static void txa(fake65c02_t *cpu) {
  cpu->a = cpu->x;

  zerocalc(cpu, cpu->a);
  signcalc(cpu, cpu->a);
}

static void txs(fake65c02_t *cpu) { cpu->sp = cpu->x; }

static void tya(fake65c02_t *cpu) {
  cpu->a = cpu->y;

  zerocalc(cpu, cpu->a);
  signcalc(cpu, cpu->a);
}

static void bbr(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  uint8_t bit = cpu->opcode >> 0x04;
  if (((cpu->value >> bit) & 0x01) == 0) {
    cpu->pc += cpu->reladdr;
  }
}

static void bbs(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  uint8_t bit = (cpu->opcode >> 0x04) - 8;
  if (((cpu->value >> bit) & 0x01) == 1) {
    cpu->pc += cpu->reladdr;
  }
}

static void rmb(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  uint8_t bit = cpu->opcode >> 0x04;
  putvalue(cpu, cpu->value &= ~(0x01 << bit));
}

static void smb(fake65c02_t *cpu) {
  cpu->value = getvalue(cpu);
  uint8_t bit = (cpu->opcode >> 0x04) - 8;
  putvalue(cpu, cpu->value |= 0x01 << bit);
}

static void stp(fake65c02_t *cpu) { cpu->stopped = 1; }

static void wai(fake65c02_t *cpu) { cpu->waiting = 1; }

// undocumented instructions
static void lax(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    lda(cpu);
    ldx(cpu);
  }
}

static void sax(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    sta(cpu);
    stx(cpu);
    putvalue(cpu, cpu->a & cpu->x);
    if (penaltyop && penaltyaddr)
      cpu->clockticks--;
  }
}

static void dcp(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    dec(cpu);
    cmp(cpu);
    if (penaltyop && penaltyaddr)
      cpu->clockticks--;
  }
}

static void isb(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    inc(cpu);
    sbc(cpu);
    if (penaltyop && penaltyaddr)
      cpu->clockticks--;
  }
}

static void slo(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    asl(cpu);
    ora(cpu);
    if (penaltyop && penaltyaddr)
      cpu->clockticks--;
  }
}

static void rla(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    rol(cpu);
    and(cpu);
    if (penaltyop && penaltyaddr)
      cpu->clockticks--;
  }
}

static void sre(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    lsr(cpu);
    eor(cpu);
    if (penaltyop && penaltyaddr)
      cpu->clockticks--;
  }
}

static void rra(fake65c02_t *cpu) {
  if (cpu->enable_undocumented) {
    ror(cpu);
    adc(cpu);
    if (penaltyop && penaltyaddr)
      cpu->clockticks--;
  }
}

static void (*addrtable[256])(fake65c02_t *cpu) = {
    // clang-format off
    /*    |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B |  C  |  D  |  E  |  F  |     */
    /* 0 */ imp,  indx, imp, indx, zp,   zp,   zp,   zp,   imp,  imm,  acc, imm,  abso, abso, abso,  zpr, /* 0 */
    /* 1 */ rel,  indy, imp, indy, zp,   zpx,  zpx,  zp,   imp,  absy, imp, absy, abso, absx, absx,  zpr, /* 1 */
    /* 2 */ abso, indx, imp, indx, zp,   zp,   zp,   zp,   imp,  imm,  acc, imm,  abso, abso, abso,  zpr, /* 2 */
    /* 3 */ rel,  indy, imp, indy, zpx,  zpx,  zpx,  zp,   imp,  absy, imp, absy, absx, absx, absx,  zpr, /* 3 */
    /* 4 */ imp,  indx, imp, indx, zp,   zp,   zp,   zp,   imp,  imm,  acc, imm,  abso, abso, abso,  zpr, /* 4 */
    /* 5 */ rel,  indy, imp, indy, zpx,  zpx,  zpx,  zp,   imp,  absy, imp, absy, absx, absx, absx,  zpr, /* 5 */
    /* 6 */ imp,  indx, imp, indx, zp,   zp,   zp,   zp,   imp,  imm,  acc, imm,  ind,  abso, abso,  zpr, /* 6 */
    /* 7 */ rel,  indy, imp, indy, zpx,  zpx,  zpx,  zp,   imp,  absy, imp, absy, indx, absx, absx,  zpr, /* 7 */
    /* 8 */ rel,  indx, imm, indx, zp,   zp,   zp,   zp,   imp,  imm,  imp, imm,  abso, abso, abso,  zpr, /* 8 */
    /* 9 */ rel,  indy, imp, indy, zpx,  zpx,  zpy,  zp,   imp,  absy, imp, absy, abso, absx, absx,  zpr, /* 9 */
    /* A */ imm,  indx, imm, indx, zp,   zp,   zp,   zp,   imp,  imm,  imp, imm,  abso, abso, abso,  zpr, /* A */
    /* B */ rel,  indy, imp, indy, zpx,  zpx,  zpy,  zp,   imp,  absy, imp, absy, absx, absx, absy,  zpr, /* B */
    /* C */ imm,  indx, imm, indx, zp,   zp,   zp,   zp,   imp,  imm,  imp, imm,  abso, abso, abso,  zpr, /* C */
    /* D */ rel,  indy, imp, indy, zpx,  zpx,  zpx,  zp,   imp,  absy, imp, absy, absx, absx, absx,  zpr, /* D */
    /* E */ imm,  indx, imm, indx, zp,   zp,   zp,   zp,   imp,  imm,  imp, imm,  abso, abso, abso,  zpr, /* E */
    /* F */ rel,  indy, imp, indy, zpx,  zpx,  zpx,  zp,   imp,  absy, imp, absy, absx, absx, absx,  zpr  /* F */
    // clang-format on
};

static void (*optable[256])(fake65c02_t *cpu) = {
    // clang-format off
    /*    |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 |  A |  B |  C |  D |  E |  F   |    */
    /* 0 */ brk, ora, nop, slo, tsb, ora, asl, rmb, php, ora, asl, nop, tsb, ora, asl, bbr, /* 0 */
    /* 1 */ bpl, ora, nop, slo, trb, ora, asl, rmb, clc, ora, ina, slo, trb, ora, asl, bbr, /* 1 */
    /* 2 */ jsr, and, nop, rla, bit, and, rol, rmb, plp, and, rol, nop, bit, and, rol, bbr, /* 2 */
    /* 3 */ bmi, and, nop, rla, bit, and, rol, rmb, sec, and, dea, rla, bit, and, rol, bbr, /* 3 */
    /* 4 */ rti, eor, nop, sre, nop, eor, lsr, rmb, pha, eor, lsr, nop, jmp, eor, lsr, bbr, /* 4 */
    /* 5 */ bvc, eor, nop, sre, nop, eor, lsr, rmb, cli, eor, phy, sre, nop, eor, lsr, bbr, /* 5 */
    /* 6 */ rts, adc, nop, rra, stz, adc, ror, rmb, pla, adc, ror, nop, jmp, adc, ror, bbr, /* 6 */
    /* 7 */ bvs, adc, nop, rra, stz, adc, ror, rmb, sei, adc, ply, rra, jmp, adc, ror, bbr, /* 7 */
    /* 8 */ bra, sta, nop, sax, sty, sta, stx, smb, dey, nop, txa, nop, sty, sta, stx, bbs, /* 8 */
    /* 9 */ bcc, sta, nop, nop, sty, sta, stx, smb, tya, sta, txs, nop, stz, sta, stz, bbs, /* 9 */
    /* A */ ldy, lda, ldx, lax, ldy, lda, ldx, smb, tay, lda, tax, nop, ldy, lda, ldx, bbs, /* A */
    /* B */ bcs, lda, nop, lax, ldy, lda, ldx, smb, clv, lda, tsx, lax, ldy, lda, ldx, bbs, /* B */
    /* C */ cpy, cmp, nop, dcp, cpy, cmp, dec, smb, iny, cmp, dex, wai, cpy, cmp, dec, bbs, /* C */
    /* D */ bne, cmp, nop, dcp, nop, cmp, dec, smb, cld, cmp, phx, stp, nop, cmp, dec, bbs, /* D */
    /* E */ cpx, sbc, nop, isb, cpx, sbc, inc, smb, inx, sbc, nop, sbc, cpx, sbc, inc, bbs, /* E */
    /* F */ beq, sbc, nop, isb, nop, sbc, inc, smb, sed, sbc, plx, isb, nop, sbc, inc, bbs  /* F */
    // clang-format on
};

static const uint32_t ticktable[256] = {
    // clang-format off
    /*      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
    /* 0 */ 7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6, /* 0 */
    /* 1 */ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /* 1 */
    /* 2 */ 6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6, /* 2 */
    /* 3 */ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /* 3 */
    /* 4 */ 6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6, /* 4 */
    /* 5 */ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /* 5 */
    /* 6 */ 6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6, /* 6 */
    /* 7 */ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /* 7 */
    /* 8 */ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4, /* 8 */
    /* 9 */ 2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5, /* 9 */
    /* A */ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4, /* A */
    /* B */ 2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4, /* B */
    /* C */ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6, /* C */
    /* D */ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 3, 7, 4, 4, 7, 7, /* D */
    /* E */ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6, /* E */
    /* F */ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7  /* F */
    // clang-format on
};

int nmi65c02(fake65c02_t *cpu) {
  if (!cpu->stopped) {
    if (cpu->waiting) {
      cpu->waiting = 0;
    }
    // Only perform nmi handling if FLAG_INTERRUPT
    // is clear. This should **NOT** do anything besides
    // resume execution for a running `wai` instruction
    // otherwise.
    if ((cpu->status & FLAG_INTERRUPT) == 0) {
      push16(cpu, cpu->pc);
      push8(cpu, cpu->status);
      cpu->status |= FLAG_INTERRUPT;
      cpu->pc = (uint16_t)cpu->read(cpu, 0xFFFA) |
                    ((uint16_t)cpu->read(cpu, 0xFFFB) << 8);
    }
    return 1;
  }
  return 0;
}

int irq65c02(fake65c02_t *cpu) {
  if (!cpu->stopped) {
    if (cpu->waiting) {
      cpu->waiting = 0;
    }
    // Only perform irq handling if FLAG_INTERRUPT
    // is clear. This should **NOT** do anything besides
    // resume execution for a running `wai` instruction
    // otherwise.
    if ((cpu->status & FLAG_INTERRUPT) == 0) {
      push16(cpu, cpu->pc);
      push8(cpu, cpu->status);
      cpu->status |= FLAG_INTERRUPT;
      cpu->pc = (uint16_t)cpu->read(cpu, 0xFFFE) |
                    ((uint16_t)cpu->read(cpu, 0xFFFF) << 8);
    }
    return 1;
  }
  return 0;
}

int exec(fake65c02_t *cpu, uint32_t tickcount) {
  if (!cpu->stopped) {
    cpu->clockgoal += tickcount;
    while (cpu->clockticks < cpu->clockgoal) {
      if (!cpu->stopped && !cpu->waiting) {
        cpu->opcode = cpu->read(cpu, cpu->pc++);
        cpu->status |= FLAG_CONSTANT;

        cpu->penaltyop = 0;
        cpu->penaltyaddr = 0;

        (*addrtable[cpu->opcode])(cpu);
        (*optable[cpu->opcode])(cpu);
        cpu->clockticks += ticktable[cpu->opcode];
        if (penaltyop && penaltyaddr)
          cpu->clockticks++;

        cpu->instructions++;

        if (cpu->hook != NULL)
          cpu->hook(cpu);
      }
    }
    return 1;
  }
  return 0;
}

int step65c02(fake65c02_t *cpu) {
  if (!cpu->stopped && !cpu->waiting) {
    cpu->opcode = cpu->read(cpu, cpu->pc++);
    cpu->status |= FLAG_CONSTANT;

    cpu->penaltyop = 0;
    cpu->penaltyaddr = 0;

    (*addrtable[cpu->opcode])(cpu);
    (*optable[cpu->opcode])(cpu);
    cpu->clockticks += ticktable[cpu->opcode];
    if (penaltyop && penaltyaddr)
      cpu->clockticks++;
    cpu->clockgoal = cpu->clockticks;

    cpu->instructions++;

    if (cpu->hook != NULL)
      cpu->hook(cpu);

    return 1;
  }
  return 0;
}
