#!/usr/bin/env lua

FAKE65C02_VERSION='0.1.0'

-- [[ Handle command line arguments ]]
-- Keeping it simple, as this is just
-- an example and alternate testing
-- tool.

local args = {...}
local rom_files = {}

local table_banks = false

for i, a in pairs(args) do
    if a == [[--help]] or a == [[-h]] then
        print("TODO: --help")
        os.exit(0)
    elseif a == [[--version]] or a == [[-v]] then
        print(string.format("fake65c02.lua v%s :: %s", FAKE65C02_VERSION, jit.version))
        os.exit(0)
    elseif a == [[--table_banks]] then
        table_banks = true
    else
        rom_files[#rom_files+1] = a
    end
end

--[[ Load fake65c02 ]]

local ffi = require("ffi")

local fake6502 = ffi.load('./libfake65c02.so')

-- [[ Definitions for our fake65c02.so ]]

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

--[[ Define CDATA objects for storing our memory banks]]

ffi.cdef([[

  typedef struct bank32 bank32_t;
  struct bank32 {
    uint16_t location;
    uint8_t memory[0x8000];
  };

]])

--[[ Initialize memory banks ]]

local new_bank32

if table_banks then
--    print('Using table memory banks')
    new_bank32 = function(location)
        return { memory = {}, location = location }
    end
else
--    print('Using CDATA memory banks')
    new_bank32 = function(location)
        local bank = ffi.new("bank32_t")
        bank.location = location
        return bank
    end
end

-- [[ Functions to read and write to our memory banks ]]

local state_mt = {}

function state_mt:set(addr, value)
    local ram = self.ram
    if addr >= ram.location and addr < ram.location + 0x8000 then
        ram.memory[addr - ram.location] = value
    end
    local rom = self.rom
    if addr >= rom.location and addr < rom.location + 0x8000 then
        --rom.memory[addr - rom.location] = value
    end
end

function state_mt:get(addr)
    local ram = self.ram
    if addr >= ram.location and addr < ram.location + 0x8000 then
        return ram.memory[addr - ram.location] or 0x00
    end
    local rom = self.rom
    if addr >= rom.location and addr < rom.location + 0x8000 then
        return rom.memory[addr - rom.location] or 0x00
    end
end

function state_mt:set16(addr, value)
    self:set(addr, bit.band(value, 0x00ff))
    self:set(addr+1, bit.rshift(value, 8))
    return 2
end

function state_mt:set8(addr, value)
    self:set(addr, bit.band(value, 0x00ff))
    return 1
end

function state_mt:get16(addr)
    return bit.band(self:get(addr), 0x00ff) + bit.lshift(self:get(addr+1), 8)
end

function state_mt:get8(addr)
    return bit.band(self:get(addr), 0x00ff)
end

function state_mt:read_memory(_context, addr)
  return self:get8(addr)
end

function state_mt:write_memory(_context, addr, value)
  if addr == IO_OUT then
    self.io_out = value
    return
  end
  if addr == IO_CMD then
    self.io_cmd = value
    return
  end
  if addr == SERIAL then
    io.write(string.char(value))
    return
  end

  self:set8(addr, value)
end

function state_mt:reset()
    fake6502.reset6502(self.context)
end

function state_mt:step()
    fake6502.step6502(self.context)
end

function state_mt:run(n)
    local c = self.context
    while self.io_cmd ~= IO_HALT do
      fake6502.step6502(c)
    end
    print("halted")
end

function new_state()
  local state = {
    io_out = 0,
    io_cmd = 0,
    io_in = 0,
  }
  setmetatable(state, {__index=state_mt})
  state.context = fake6502.new_fake6502(nil)
  state.ram = new_bank32(0x0000)
  state.rom = new_bank32(0x8000)
  state.context.read = function(context, address)
    return state:read_memory(context, address) or 0x00
  end
  state.context.write = function(context, address, value)
    state:write_memory(context, address, value)
  end
  return state
end

--[[ Utility function for debugging purposes ]]
local function p8(i) print(string.format('%02x',i)) end
local function p16(i) print(string.format('%04x',i)) end

for _, path in pairs(rom_files) do
  local f, err = io.open(path, 'rb')
  if not err then
    local s = new_state()
    local i = 0x00
    if table_banks then
      local m = s.rom.memory
      local n = 0x00
      local chunk_size = 0x0400
      while n - chunk_size < 0x8000 do
        local d = f:read(chunk_size)
        if not d then break end
        for i = 1,#d do
            m[n + (i-1)] = string.byte(string.sub(d, i, i))
        end
        n = n + #d
      end
    else
      -- TODO: Handle mismatched sizes correctly. What if a rom is too small?
      ffi.copy(s.rom.memory, f:read(0x8000), 0x8000)
    end
    f:close()
    s:reset()
    s:run()
  else
    print(string.format("Error opening %s", err))
    os.exit(1)
  end
end
