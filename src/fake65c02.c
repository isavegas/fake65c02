/* Fake65c02 CPU emulator core ***********************
 * (c)2011 Mike Chambers (miker00lz@gmail.com)       *
 * (c)2021 Kenzi Jeanis (kenzi.jeanis@isavegas.dev)  *
 *****************************************************
 * v2.0 - Added 65c02 instructions                   *
 *        Switched from globals to a context based   *
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

#include "fake65c02.h"

// Disabled defines here. We're defining these via compiler flags.
// 6502 defines
// #define UNDOCUMENTED //when this is defined, undocumented opcodes are
// handled.
// otherwise, they're simply treated as NOPs.

// #define NES_CPU      //when this is defined, the binary-coded decimal (BCD)
//  status flag is not honored by ADC and SBC. the 2A03
//  CPU in the Nintendo Entertainment System does not
//  support BCD operation.

fake65c02_t *new_fake65c02(void *m) {
  fake65c02_t *c = calloc(1, sizeof(fake65c02_t));
  c->m = m;
  return c;
}

void free_fake65c02(fake65c02_t *context) { free(context); }

#define FLAG_CARRY 0x01
#define FLAG_ZERO 0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL 0x08
#define FLAG_BREAK 0x10
#define FLAG_CONSTANT 0x20
#define FLAG_OVERFLOW 0x40
#define FLAG_SIGN 0x80

#define BASE_STACK 0x100

#define saveaccum(context, n) context->a = (uint8_t)((n)&0x00FF)

// flag modifier macros
#define setcarry(context) context->status |= FLAG_CARRY
#define clearcarry(context) context->status &= (~FLAG_CARRY)
#define setzero(context) context->status |= FLAG_ZERO
#define clearzero(context) context->status &= (~FLAG_ZERO)
#define setinterrupt(context) context->status |= FLAG_INTERRUPT
#define clearinterrupt(context) context->status &= (~FLAG_INTERRUPT)
#define setdecimal(context) context->status |= FLAG_DECIMAL
#define cleardecimal(context) context->status &= (~FLAG_DECIMAL)
#define setoverflow(context) context->status |= FLAG_OVERFLOW
#define clearoverflow(context) context->status &= (~FLAG_OVERFLOW)
#define setsign(context) context->status |= FLAG_SIGN
#define clearsign(context) context->status &= (~FLAG_SIGN)

// flag calculation macros
#define zerocalc(context, n)                                                   \
  {                                                                            \
    if ((n)&0x00FF)                                                            \
      clearzero(context);                                                      \
    else                                                                       \
      setzero(context);                                                        \
  }

#define signcalc(context, n)                                                   \
  {                                                                            \
    if ((n)&0x0080)                                                            \
      setsign(context);                                                        \
    else                                                                       \
      clearsign(context);                                                      \
  }

#define carrycalc(context, n)                                                  \
  {                                                                            \
    if ((n)&0xFF00)                                                            \
      setcarry(context);                                                       \
    else                                                                       \
      clearcarry(context);                                                     \
  }

#define overflowcalc(context, n, m, o)                                         \
  { /* n = result, m = accumulator, o = memory */                              \
    if (((n) ^ (uint16_t)(m)) & ((n) ^ (o)) & 0x0080)                          \
      setoverflow(context);                                                    \
    else                                                                       \
      clearoverflow(context);                                                  \
  }

// a few general functions used by various other functions
void push16(fake65c02_t *context, uint16_t pushval) {
  context->write(context, BASE_STACK + ((uint16_t)context->sp), pushval >> 8);
  context->write(context, BASE_STACK + (((uint16_t)context->sp) - 1),
                 pushval & 0xff);
  context->sp -= 2;
}

void push8(fake65c02_t *context, uint8_t pushval) {
  context->write(context, BASE_STACK + context->sp, pushval);
  context->sp--;
}

uint16_t pull16(fake65c02_t *context) {
  uint16_t temp16 = context->read(context, BASE_STACK + context->sp + 1);
  temp16 |= (context->read(context, BASE_STACK + context->sp + 2) << 8);

  context->sp += 2;

  return temp16;
}

uint8_t pull8(fake65c02_t *context) {
  context->sp++;
  return (context->read(context, BASE_STACK + context->sp));
}

int reset65c02(fake65c02_t *context) {
  if (context->read == NULL || context->write == NULL) {
    return 0;
  }
  context->pc = (uint16_t)context->read(context, 0xFFFC) |
                ((uint16_t)context->read(context, 0xFFFD) << 8);
  context->a = 0;
  context->x = 0;
  context->y = 0;
  context->sp = 0xFD;
  context->status |= FLAG_CONSTANT;
  return 1;
}

static void (*addrtable[256])(fake65c02_t *context);
static void (*optable[256])(fake65c02_t *context);
uint8_t penaltyop, penaltyaddr;

// addressing mode functions, calculates effective addresses
static void imp(fake65c02_t *context) {} // implied

static void acc(fake65c02_t *context) {} // accumulator

static void imm(fake65c02_t *context) { // immediate
  context->ea = context->pc++;
}

static void zp(fake65c02_t *context) { // zero-page
  context->ea = (uint16_t)context->read(context, (uint16_t)context->pc++);
}

static void zpx(fake65c02_t *context) { // zero-page,X
  context->ea = ((uint16_t)context->read(context, (uint16_t)context->pc++) +
                 (uint16_t)context->x) &
                0xFF; // zero-page wraparound
}

static void zpy(fake65c02_t *context) { // zero-page,Y
  context->ea = ((uint16_t)context->read(context, (uint16_t)context->pc++) +
                 (uint16_t)context->y) &
                0xFF; // zero-page wraparound
}

static void rel(fake65c02_t *context) { // relative for branch ops (8-bit
                                        // immediate value, sign-extended)
  context->reladdr = (uint16_t)context->read(context, context->pc++);
  if (context->reladdr & 0x80)
    context->reladdr |= 0xFF00;
}

// specific to bbr/bbs/rmb/smb. Uses both zp and rel
static void zpr(fake65c02_t *context) {
  context->ea = (uint16_t)context->read(context, (uint16_t)context->pc++);
  context->reladdr = (uint16_t)context->read(context, context->pc++);
  if (context->reladdr & 0x80)
    context->reladdr |= 0xFF00;
}

static void abso(fake65c02_t *context) { // absolute
  context->ea = (uint16_t)(context->read(context, context->pc)) |
                ((uint16_t)context->read(context, context->pc + 1) << 8);
  context->pc += 2;
}

static void absx(fake65c02_t *context) { // absolute,X
  uint16_t startpage;
  context->ea = ((uint16_t)context->read(context, context->pc) |
                 ((uint16_t)context->read(context, context->pc + 1) << 8));
  startpage = context->ea & 0xFF00;
  context->ea += (uint16_t)context->x;

  if (startpage !=
      (context->ea &
       0xFF00)) { // one cycle penlty for page-crossing on some opcodes
    penaltyaddr = 1;
  }

  context->pc += 2;
}

static void absy(fake65c02_t *context) { // absolute,Y
  uint16_t startpage;
  context->ea = ((uint16_t)context->read(context, context->pc) |
                 ((uint16_t)context->read(context, context->pc += 1) << 8));
  startpage = context->ea & 0xFF00;
  context->ea += (uint16_t)context->y;

  if (startpage !=
      (context->ea &
       0xFF00)) { // one cycle penlty for page-crossing on some opcodes
    penaltyaddr = 1;
  }

  context->pc += 2;
}

static void ind(fake65c02_t *context) { // indirect
  uint16_t eahelp, eahelp2;
  eahelp = (uint16_t)context->read(context, context->pc) |
           (uint16_t)((uint16_t)context->read(context, context->pc + 1) << 8);
  eahelp2 =
      (eahelp & 0xFF00) |
      ((eahelp + 1) & 0x00FF); // replicate 65c02 page-boundary wraparound bug
  context->ea = (uint16_t)context->read(context, eahelp) |
                ((uint16_t)context->read(context, eahelp2) << 8);
  context->pc += 2;
}

static void indx(fake65c02_t *context) { // (indirect,X)
  uint16_t eahelp;
  eahelp = (uint16_t)(((uint16_t)context->read(context, context->pc++) +
                       (uint16_t)context->x) &
                      0xFF); // zero-page wraparound for table pointer
  context->ea = (uint16_t)context->read(context, eahelp & 0x00FF) |
                ((uint16_t)context->read(context, (eahelp + 1) & 0x00FF) << 8);
}

static void indy(fake65c02_t *context) { // (indirect),Y
  uint16_t eahelp, eahelp2, startpage;
  eahelp = (uint16_t)context->read(context, context->pc++);
  eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); // zero-page wraparound
  context->ea = (uint16_t)context->read(context, eahelp) |
                ((uint16_t)context->read(context, eahelp2) << 8);
  startpage = context->ea & 0xFF00;
  context->ea += (uint16_t)context->y;

  if (startpage !=
      (context->ea &
       0xFF00)) { // one cycle penlty for page-crossing on some opcodes
    penaltyaddr = 1;
  }
}

static uint16_t getvalue(fake65c02_t *context) {
  if (addrtable[context->opcode] == acc)
    return ((uint16_t)context->a);
  else
    return ((uint16_t)context->read(context, context->ea));
}

static void putvalue(fake65c02_t *context, uint16_t saveval) {
  if (addrtable[context->opcode] == acc)
    context->a = (uint8_t)(saveval & 0x00FF);
  else
    context->write(context, context->ea, (saveval & 0x00FF));
}

// instruction handler functions
static void adc(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->result = (uint16_t)context->a + context->value +
                    (uint16_t)(context->status & FLAG_CARRY);

  carrycalc(context, context->result);
  zerocalc(context, context->result);
  overflowcalc(context, context->result, context->a, context->value);
  signcalc(context, context->result);

#ifndef NES_CPU
  if (context->status & FLAG_DECIMAL) {
    clearcarry(context);

    if ((context->a & 0x000F) > 0x09) {
      context->a += 0x0006;
    }
    if ((context->a & 0x00F0) > 0x90) {
      context->a += 0x0060;
      setcarry(context);
    }

    context->clockticks++;
  }
#endif

  saveaccum(context, context->result);
}

static void and (fake65c02_t * context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->result = (uint16_t)context->a & context->value;

  zerocalc(context, context->result);
  signcalc(context, context->result);

  saveaccum(context, context->result);
}

static void trb(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->result = (~(uint16_t)context->a) & context->value;

  zerocalc(context, (uint16_t)context->a & context->value);

  putvalue(context, context->result);
}

static void tsb(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->result = (uint16_t)context->a | context->value;

  zerocalc(context, (uint16_t)context->a & context->value);

  putvalue(context, context->result);
}

static void asl(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = context->value << 1;

  carrycalc(context, context->result);
  zerocalc(context, context->result);
  signcalc(context, context->result);

  putvalue(context, context->result);
}

static void bcc(fake65c02_t *context) {
  if ((context->status & FLAG_CARRY) == 0) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void bcs(fake65c02_t *context) {
  if ((context->status & FLAG_CARRY) == FLAG_CARRY) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void beq(fake65c02_t *context) {
  if ((context->status & FLAG_ZERO) == FLAG_ZERO) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void bra(fake65c02_t *context) {
  context->oldpc = context->pc;
  context->pc += context->reladdr;
  if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
    context->clockticks += 2; // check if jump crossed a page boundary
  else
    context->clockticks++;
}

static void bit(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = (uint16_t)context->a & context->value;

  zerocalc(context, context->result);
  context->status = (context->status & 0x3F) | (uint8_t)(context->value & 0xC0);
}

static void bmi(fake65c02_t *context) {
  if ((context->status & FLAG_SIGN) == FLAG_SIGN) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void bne(fake65c02_t *context) {
  if ((context->status & FLAG_ZERO) == 0) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void bpl(fake65c02_t *context) {
  if ((context->status & FLAG_SIGN) == 0) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void brk(fake65c02_t *context) {
  context->pc++;
  push16(context, context->pc); // push next instruction address onto stack
  push8(context, context->status | FLAG_BREAK); // push CPU status to stack
  setinterrupt(context);                        // set interrupt flag
  context->pc = (uint16_t)context->read(context, 0xFFFE) |
                ((uint16_t)context->read(context, 0xFFFF) << 8);
}

static void bvc(fake65c02_t *context) {
  if ((context->status & FLAG_OVERFLOW) == 0) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void bvs(fake65c02_t *context) {
  if ((context->status & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
    context->oldpc = context->pc;
    context->pc += context->reladdr;
    if ((context->oldpc & 0xFF00) != (context->pc & 0xFF00))
      context->clockticks += 2; // check if jump crossed a page boundary
    else
      context->clockticks++;
  }
}

static void clc(fake65c02_t *context) { clearcarry(context); }

static void cld(fake65c02_t *context) { cleardecimal(context); }

static void cli(fake65c02_t *context) { clearinterrupt(context); }

static void clv(fake65c02_t *context) { clearoverflow(context); }

static void cmp(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->result = (uint16_t)context->a - context->value;

  if (context->a >= (uint8_t)(context->value & 0x00FF))
    setcarry(context);
  else
    clearcarry(context);
  if (context->a == (uint8_t)(context->value & 0x00FF))
    setzero(context);
  else
    clearzero(context);
  signcalc(context, context->result);
}

static void cpx(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = (uint16_t)context->x - context->value;

  if (context->x >= (uint8_t)(context->value & 0x00FF))
    setcarry(context);
  else
    clearcarry(context);
  if (context->x == (uint8_t)(context->value & 0x00FF))
    setzero(context);
  else
    clearzero(context);
  signcalc(context, context->result);
}

static void cpy(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = (uint16_t)context->y - context->value;

  if (context->y >= (uint8_t)(context->value & 0x00FF))
    setcarry(context);
  else
    clearcarry(context);
  if (context->y == (uint8_t)(context->value & 0x00FF))
    setzero(context);
  else
    clearzero(context);
  signcalc(context, context->result);
}

static void dec(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = context->value - 1;

  zerocalc(context, context->result);
  signcalc(context, context->result);

  putvalue(context, context->result);
}

static void dea(fake65c02_t *context) {
  context->a--;

  zerocalc(context, context->a);
  signcalc(context, context->a);
}

static void dex(fake65c02_t *context) {
  context->x--;

  zerocalc(context, context->x);
  signcalc(context, context->x);
}

static void dey(fake65c02_t *context) {
  context->y--;

  zerocalc(context, context->y);
  signcalc(context, context->y);
}

static void eor(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->result = (uint16_t)context->a ^ context->value;

  zerocalc(context, context->result);
  signcalc(context, context->result);

  saveaccum(context, context->result);
}

static void inc(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = context->value + 1;

  zerocalc(context, context->result);
  signcalc(context, context->result);

  putvalue(context, context->result);
}

static void ina(fake65c02_t *context) {
  context->a++;

  zerocalc(context, context->a);
  signcalc(context, context->a);
}

static void inx(fake65c02_t *context) {
  context->x++;

  zerocalc(context, context->x);
  signcalc(context, context->x);
}

static void iny(fake65c02_t *context) {
  context->y++;

  zerocalc(context, context->y);
  signcalc(context, context->y);
}

static void jmp(fake65c02_t *context) { context->pc = context->ea; }

static void jsr(fake65c02_t *context) {
  push16(context, context->pc - 1);
  context->pc = context->ea;
}

static void lda(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->a = (uint8_t)(context->value & 0x00FF);

  zerocalc(context, context->a);
  signcalc(context, context->a);
}

static void ldx(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->x = (uint8_t)(context->value & 0x00FF);

  zerocalc(context, context->x);
  signcalc(context, context->x);
}

static void ldy(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->y = (uint8_t)(context->value & 0x00FF);

  zerocalc(context, context->y);
  signcalc(context, context->y);
}

static void lsr(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = context->value >> 1;

  if (context->value & 1)
    setcarry(context);
  else
    clearcarry(context);
  zerocalc(context, context->result);
  signcalc(context, context->result);

  putvalue(context, context->result);
}

static void nop(fake65c02_t *context) {
  switch (context->opcode) {
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

static void ora(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context);
  context->result = (uint16_t)context->a | context->value;

  zerocalc(context, context->result);
  signcalc(context, context->result);

  saveaccum(context, context->result);
}

static void pha(fake65c02_t *context) { push8(context, context->a); }
static void phx(fake65c02_t *context) { push8(context, context->x); }
static void phy(fake65c02_t *context) { push8(context, context->y); }

static void php(fake65c02_t *context) {
  push8(context, context->status | FLAG_BREAK);
}

static void pla(fake65c02_t *context) {
  context->a = pull8(context);

  zerocalc(context, context->a);
  signcalc(context, context->a);
}

static void plx(fake65c02_t *context) {
  context->x = pull8(context);

  zerocalc(context, context->x);
  signcalc(context, context->x);
}

static void ply(fake65c02_t *context) {
  context->y = pull8(context);

  zerocalc(context, context->y);
  signcalc(context, context->y);
}

static void plp(fake65c02_t *context) {
  context->status = pull8(context) | FLAG_CONSTANT;
}

static void rol(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result = (context->value << 1) | (context->status & FLAG_CARRY);

  carrycalc(context, context->result);
  zerocalc(context, context->result);
  signcalc(context, context->result);

  putvalue(context, context->result);
}

static void ror(fake65c02_t *context) {
  context->value = getvalue(context);
  context->result =
      (context->value >> 1) | ((context->status & FLAG_CARRY) << 7);

  if (context->value & 1)
    setcarry(context);
  else
    clearcarry(context);
  zerocalc(context, context->result);
  signcalc(context, context->result);

  putvalue(context, context->result);
}

static void rti(fake65c02_t *context) {
  context->status = pull8(context);
  context->value = pull16(context);
  context->pc = context->value;
}

static void rts(fake65c02_t *context) {
  context->value = pull16(context);
  context->pc = context->value + 1;
}

static void sbc(fake65c02_t *context) {
  penaltyop = 1;
  context->value = getvalue(context) ^ 0x00FF;
  context->result = (uint16_t)context->a + context->value +
                    (uint16_t)(context->status & FLAG_CARRY);

  carrycalc(context, context->result);
  zerocalc(context, context->result);
  overflowcalc(context, context->result, context->a, context->value);
  signcalc(context, context->result);

#ifndef NES_CPU
  if (context->status & FLAG_DECIMAL) {
    clearcarry(context);

    context->a -= 0x66;
    if ((context->a & 0x0F) > 0x09) {
      context->a += 0x06;
    }
    if ((context->a & 0xF0) > 0x90) {
      context->a += 0x60;
      setcarry(context);
    }

    context->clockticks++;
  }
#endif

  saveaccum(context, context->result);
}

static void sec(fake65c02_t *context) { setcarry(context); }

static void sed(fake65c02_t *context) { setdecimal(context); }

static void sei(fake65c02_t *context) { setinterrupt(context); }

static void sta(fake65c02_t *context) { putvalue(context, context->a); }

static void stx(fake65c02_t *context) { putvalue(context, context->x); }

static void sty(fake65c02_t *context) { putvalue(context, context->y); }

static void stz(fake65c02_t *context) { putvalue(context, 0); }

static void tax(fake65c02_t *context) {
  context->x = context->a;

  zerocalc(context, context->x);
  signcalc(context, context->x);
}

static void tay(fake65c02_t *context) {
  context->y = context->a;

  zerocalc(context, context->y);
  signcalc(context, context->y);
}

static void tsx(fake65c02_t *context) {
  context->x = context->sp;

  zerocalc(context, context->x);
  signcalc(context, context->x);
}

static void txa(fake65c02_t *context) {
  context->a = context->x;

  zerocalc(context, context->a);
  signcalc(context, context->a);
}

static void txs(fake65c02_t *context) { context->sp = context->x; }

static void tya(fake65c02_t *context) {
  context->a = context->y;

  zerocalc(context, context->a);
  signcalc(context, context->a);
}

static void bbr(fake65c02_t *context) {
  context->value = getvalue(context);
  uint8_t bit = context->opcode >> 0x04;
  if (((context->value >> bit) & 0x01) == 0) {
    context->pc += context->reladdr;
  }
}

static void bbs(fake65c02_t *context) {
  context->value = getvalue(context);
  uint8_t bit = (context->opcode >> 0x04) - 8;
  if (((context->value >> bit) & 0x01) == 1) {
    context->pc += context->reladdr;
  }
}

static void rmb(fake65c02_t *context) {
  context->value = getvalue(context);
  uint8_t bit = context->opcode >> 0x04;
  putvalue(context, context->value &= ~(0x01 << bit));
}

static void smb(fake65c02_t *context) {
  context->value = getvalue(context);
  uint8_t bit = (context->opcode >> 0x04) - 8;
  putvalue(context, context->value |= 0x01 << bit);
}

static void stp(fake65c02_t *context) { context->stopped = 1; }

static void wai(fake65c02_t *context) { context->waiting = 1; }

// undocumented instructions
#ifdef UNDOCUMENTED
static void lax(fake65c02_t *context) {
  lda(context);
  ldx(context);
}

static void sax(fake65c02_t *context) {
  sta(context);
  stx(context);
  putvalue(context, context->a & context->x);
  if (penaltyop && penaltyaddr)
    context->clockticks--;
}

static void dcp(fake65c02_t *context) {
  dec(context);
  cmp(context);
  if (penaltyop && penaltyaddr)
    context->clockticks--;
}

static void isb(fake65c02_t *context) {
  inc(context);
  sbc(context);
  if (penaltyop && penaltyaddr)
    context->clockticks--;
}

static void slo(fake65c02_t *context) {
  asl(context);
  ora(context);
  if (penaltyop && penaltyaddr)
    context->clockticks--;
}

static void rla(fake65c02_t *context) {
  rol(context);
  and(context);
  if (penaltyop && penaltyaddr)
    context->clockticks--;
}

static void sre(fake65c02_t *context) {
  lsr(context);
  eor(context);
  if (penaltyop && penaltyaddr)
    context->clockticks--;
}

static void rra(fake65c02_t *context) {
  ror(context);
  adc(context);
  if (penaltyop && penaltyaddr)
    context->clockticks--;
}
#else
#define lax nop
#define sax nop
#define dcp nop
#define isb nop
#define slo nop
#define rla nop
#define sre nop
#define rra nop
#endif

static void (*addrtable[256])(fake65c02_t *context) = {
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

static void (*optable[256])(fake65c02_t *context) = {
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

int nmi65c02(fake65c02_t *context) {
  if (!context->stopped) {
    if (context->waiting) {
      context->waiting = 0;
    }
    // Only perform nmi handling if FLAG_INTERRUPT
    // is clear. This should **NOT** do anything besides
    // resume execution for a running `wai` instruction
    // otherwise.
    if ((context->status & FLAG_INTERRUPT) == 0) {
      push16(context, context->pc);
      push8(context, context->status);
      context->status |= FLAG_INTERRUPT;
      context->pc = (uint16_t)context->read(context, 0xFFFA) |
                    ((uint16_t)context->read(context, 0xFFFB) << 8);
    }
    return 1;
  }
  return 0;
}

int irq65c02(fake65c02_t *context) {
  if (!context->stopped) {
    if (context->waiting) {
      context->waiting = 0;
    }
    // Only perform irq handling if FLAG_INTERRUPT
    // is clear. This should **NOT** do anything besides
    // resume execution for a running `wai` instruction
    // otherwise.
    if ((context->status & FLAG_INTERRUPT) == 0) {
      push16(context, context->pc);
      push8(context, context->status);
      context->status |= FLAG_INTERRUPT;
      context->pc = (uint16_t)context->read(context, 0xFFFE) |
                    ((uint16_t)context->read(context, 0xFFFF) << 8);
    }
    return 1;
  }
  return 0;
}

int exec(fake65c02_t *context, uint32_t tickcount) {
  if (!context->stopped) {
    context->clockgoal += tickcount;
    while (context->clockticks < context->clockgoal) {
      if (!context->stopped && !context->waiting) {
        context->opcode = context->read(context, context->pc++);
        context->status |= FLAG_CONSTANT;

        context->penaltyop = 0;
        context->penaltyaddr = 0;

        (*addrtable[context->opcode])(context);
        (*optable[context->opcode])(context);
        context->clockticks += ticktable[context->opcode];
        if (penaltyop && penaltyaddr)
          context->clockticks++;

        context->instructions++;

        if (context->hook != NULL)
          context->hook(context);
      }
    }
    return 1;
  }
  return 0;
}

int step65c02(fake65c02_t *context) {
  if (!context->stopped && !context->waiting) {
    context->opcode = context->read(context, context->pc++);
    context->status |= FLAG_CONSTANT;

    context->penaltyop = 0;
    context->penaltyaddr = 0;

    (*addrtable[context->opcode])(context);
    (*optable[context->opcode])(context);
    context->clockticks += ticktable[context->opcode];
    if (penaltyop && penaltyaddr)
      context->clockticks++;
    context->clockgoal = context->clockticks;

    context->instructions++;

    if (context->hook != NULL)
      context->hook(context);

    return 1;
  }
  return 0;
}
