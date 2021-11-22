local ffi = require("ffi")

local fake6502 = ffi.load('./fake65c02.so')

ffi.cdef[[

int printf(const char *fmt, ...);

typedef struct fake6502 fake6502_t;

typedef uint8_t (*read_memory)(fake6502_t *ctx, uint16_t address);
typedef void (*write_memory)(fake6502_t *ctx, uint16_t address, uint8_t value);

struct fake6502 {
    void *m;

    read_memory read;
    write_memory write;
    void (*hook)(fake6502_t *ctx);

    uint16_t pc;
    uint16_t ea;

    uint8_t sp;

    uint8_t a;
    uint8_t x;
    uint8_t y;

    uint32_t instructions;
    uint32_t clockticks;

// internal
    uint8_t status;
    uint8_t clockgoal;
    uint8_t reladdr;
    uint8_t value;
    uint8_t opcode;
    uint8_t oldstatus;
    uint8_t result;

    uint16_t oldpc;

    uint8_t penaltyop;
    uint8_t penaltyaddr;
};

fake6502_t *new_fake6502(void* m);
void free_fake6502(fake6502_t *context);

int reset6502(fake6502_t *context);
int step6502(fake6502_t *context);
int irq6502(fake6502_t *context);
int nmi6502(fake6502_t *context);
int exec6502(fake6502_t *context, uint32_t tickcount);

]]

local IO_IN = 0x7fff
local IO_CMD = 0x8000
local IO_OUT = 0x8001
local SERIAL = 0x8002

local IO_HALT = 0x01
local IO_HOOK_CALL = 0xfd
local IO_HOOK_FUNC = 0xfe
local IO_HOOK = 0xff

local io_out = 0x00
local serial = 0x00
local last_serial = 0x00
local io_cmd = 0x00

ffi.C.printf("Running from %s on %s %s\n\n", jit.version, jit.os, jit.arch)

local ctx = fake6502.new_fake6502(nil);

ffi.cdef[[
  typedef struct bank32k* bank32k_t;
  struct bank32k {
    uint16_t location;
    uint16_t memory[0x8000];
  };
]]

local ram = {}
ram.location = 0x0000
ram.memory = {}
local rom = {}
rom.location = 0x8000
rom.memory = {}

function set(addr, value)
    if addr >= ram.location and addr < ram.location + 0x8000 then
        ram[addr - ram.location] = value
    end
    if addr >= rom.location and addr < rom.location + 0x8000 then
        rom[addr - rom.location] = value
    end
end

function get(addr)
    if addr >= ram.location and addr < ram.location + 0x8000 then
        return ram[addr - ram.location] or 0x00
    end
    if addr >= rom.location and addr < rom.location + 0x8000 then
        return rom[addr - rom.location] or 0x00
    end
end

function set16(addr, value)
    set(addr, bit.band(value, 0x00ff))
    set(addr+1, bit.rshift(value, 8))
    return 2
end

function set8(addr, value)
    set(addr, bit.band(value, 0x00ff))
    return 1
end

function get16(addr)
    return bit.band(get(addr), 0x00ff) + bit.lshift(get(addr+1), 8)
end

function get8(addr)
    return bit.band(get(addr), 0x00ff)
end


--[[ Simple test machine code ]]

-- Jump to 0xff when CPU is reset
-- We're using 0xff to ensure that the
-- RAM bank is set correctly along with
-- the ROM bank. Skip past 0x00ff to ensure
-- we don't have our code overwritten by
-- writes to the stack
local entry = 0x00ff
set16(0xfffc, entry)

local i = entry
local LDA_imm = 0xa9
local STA_abs = 0x8d
local JSR = 0x20
local RTS = 0x60
local JMP = 0x4c

-- Simple way to build a ROM
local s = "Hello, world"
for c = 1, string.len(s) do
    i = i + set8(i, LDA_imm)
    i = i + set8(i, string.byte(string.sub(s, c, c))) -- ASCII char
    i = i + set8(i, STA_abs)
    i = i + set16(i, SERIAL) -- Address of serial port
end

-- Now let's use JSR/RTS to test out the stack.
i = i + set8(i, JSR)
i = i + set16(i, i+5)
i = i + set8(i, JMP) -- RTS should jump back here
i = i + set16(i, 7) -- Jump down past the RTS instruction
i = i + set8(i, LDA_imm)
i = i + set8(i, string.byte('\n'))
i = i + set8(i, STA_abs)
i = i + set16(i, SERIAL)
i = i + set8(i, RTS)

-- Halt the emulator loop by pushing the halt command on the
-- IO command port
i = i + set8(i, LDA_imm)
i = i + set8(i, 0x00) -- exit code
i = i + set8(i, STA_abs)
i = i + set16(i, IO_OUT)
i = i + set8(i, LDA_imm)
i = i + set8(i, IO_HALT)
i = i + set8(i, STA_abs)
i = i + set16(i, IO_CMD)

local function read_memory(c, addr)
    return get8(addr)
end
local function write_memory(c, addr, value)
  -- Detect that our program above has worked correctly
  if addr == IO_OUT then
    io_out = value
    return
  end
  if addr == IO_CMD then
    io_cmd = value
    return
  end
  if addr == SERIAL then
    io.write(string.char(value))
  end
  set8(addr, value)
end


ctx.read = ffi.cast("read_memory", read_memory)
ctx.write = ffi.cast("write_memory", write_memory)


fake6502.reset6502(ctx);

local halted = false
local exit_code = 1
local ins = 0
while not halted do
    fake6502.step6502(ctx);
    io.write(string.format("$%02x", ctx.sp))
    for f = 0xfd, 0xfd-255, -1 do
      io.write(string.format("$%02x", get8(f)))
    end
    io.write("\n")

    if io_cmd == IO_HALT then
        exit_code = io_out
        halted = true
    end
    ins = ins + 1
end

if exit_code == 0 then
  print("Successfully ran test rom")
else
  error("Unable to run test rom")
  os.exit(1)
end
