#!/usr/bin/env lua

local VM_VERSION = nil
if jit then
  VM_VERSION = jit.version
else
  VM_VERSION = _VERSION
end
local FAKE65C02_VERSION = '0.1.0'

-- [[ Handle command line arguments ]]
-- Keeping it simple, as this is just
-- an example and alternate testing
-- tool.

local args = { ... }
local rom_files = {}

local table_banks = false
local verbose = false
local writable_vectors = false
local writable_rom = false
local parse_args = true

local io = require('io')

-- TODO: Support tar-style combined single character flags (example: -tv)
for i, a in pairs(args) do
    if parse_args and string.sub(a, 1, 1) == "-" then
        if a == [[--]] then
            parse_args = false
        elseif a == [[--help]] or a == [[-h]] then
            print("Usage: fake65c02.lua [options] <roms...>")
            print("LuaJIT is required for FFI")
            print("Available options:")
            print("\t--help\t\t\tPrint this help dialog")
            print("\t--table_banks(-t)\tUse Lua tables for 65c02 memory banks")
            print("\t--version\t\tShow version information")
            print("\t--writable_rom\tWritable ROM")
            print("\t--writable_vectors\tWritable Vectors")
            print("\t--verbose (-v)\t\tShow debug output for VM lifecycle")
            os.exit(0)
        elseif a == [[--verbose]] or a == [[-v]] then
            verbose = true
        elseif a == [[--version]] then
            print(string.format("fake65c02.lua v%s :: %s", FAKE65C02_VERSION, VM_VERSION))
            os.exit(0)
        elseif a == [[--table_banks]] or a == [[-t]] then
            table_banks = true
        elseif a == [[--writable_rom]] then
            writable_rom = true
        elseif a == [[--writable_vectors]] then
            writable_vectors = true
        else
            print(string.format("Unknown argument: %s", a))
            os.exit(1)
        end
    else
        rom_files[#rom_files+1] = a
    end
end

local success, ffi = pcall(function()
  return require("ffi")
end)
if not success then
  print("FFI not available")
  os.exit(1)
end

if #rom_files == 0 then
  print("Please supply rom file(s)")
  os.exit(0)
end

--[[ Load fake65c02 ]]

-- Try loading it from LD_LIBRARY_PATH
local success, fake65c02 = pcall(function()
  return ffi.load('fake65c02')
end)
if not success then
    -- Try in working directory if LD_LIBRARY_PATH doesn't have it
    local ext = jit.os == "OSX" and ".dylib" or "" -- LuaJIT on Darwin doesn't automatically append .dylib
    success, fake65c02 = pcall(function() return ffi.load('./build/libfake65c02'..ext) end)
end
if not success then
  print("Could not find fake65c02!")
  os.exit(1)
end

-- [[ Definitions for our fake65c02.so ]]

ffi.cdef([[

int printf(const char *fmt, ...);

typedef struct fake65c02 fake65c02_t;

typedef uint8_t (*read_memory)(fake65c02_t *ctx, uint16_t address);
typedef void (*write_memory)(fake65c02_t *ctx, uint16_t address, uint8_t value);

struct fake65c02 {
    void *m;

    read_memory read;
    write_memory write;
    void (*hook)(fake65c02_t *ctx);

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

fake65c02_t *new_fake65c02(void* m);
void free_fake65c02(fake65c02_t *context);

int reset65c02(fake65c02_t *context);
int step65c02(fake65c02_t *context);
int irq65c02(fake65c02_t *context);
int nmi65c02(fake65c02_t *context);
int exec65c02(fake65c02_t *context, uint32_t tickcount);

]])

local IO_IN = 0x7fff
local IO_CMD = 0x8000
local IO_OUT = 0x8001
local SERIAL = 0x8002

local IO_HALT = 0x01
local IO_HOOK = 0xff
local IO_HOOK_FUNC = 0xfe
local IO_HOOK_CALL = 0xfd
local IO_IRQ_REQ = 0xfc
local IO_CHAR_REQ = 0xfb
local IO_BANK_SWITCH = 0xfa

--[[ Define CDATA objects for storing our memory banks ]]

ffi.cdef([[
  typedef struct bank bank_t;
  struct bank {
    uint16_t location;
    uint16_t size;
    uint8_t memory[?];
  };
]])

--[[ Initialize memory banks ]]

local new_bank

if table_banks then
  new_bank = function(size, location)
    return {
      memory = {},
      location = location
    }
  end
else
  new_bank = function(size, location)
    return ffi.new("bank_t", size, { location, size })
  end
end

-- [[ Functions to read and write to our memory banks ]]

local state_mt = {}

function state_mt:set(addr, value)
  local ram = self.ram
  if addr >= ram.location and addr < ram.location + ram.size then
    ram.memory[addr - ram.location] = value
    return
  end
  local rom = self.rom
  if addr >= rom.location and addr < rom.location + rom.size then
    if writable_rom or (addr >= 0xfffc and writable_vectors) then
             rom.memory[addr - rom.location] = value
        end
    return
  end
end

function state_mt:get(addr)
  local ram = self.ram
  if addr >= ram.location and addr < ram.location + ram.size then
    return ram.memory[addr - ram.location] or 0x00
  end
  local rom = self.rom
  if addr >= rom.location and addr < rom.location + rom.size then
    return rom.memory[addr - rom.location] or 0x00
  end
end

-- Unused PoC
--[[function state_mt:set16(addr, value)
    self:set(addr, bit.band(value, 0x00ff))
    self:set(addr+1, bit.rshift(value, 8))
end]]

function state_mt:set8(addr, value)
  self:set(addr, bit.band(value, 0x00ff))
end

-- Unused PoC
--[[function state_mt:get16(addr)
    return bit.band(self:get(addr), 0x00ff) + bit.lshift(self:get(addr+1), 8)
end]]

function state_mt:get8(addr)
  return bit.band(self:get(addr), 0x00ff)
end

function state_mt:read_memory(_context, addr)
  if addr == IO_IN then
    return self.io_in
  end
  return self:get8(addr)
end

-- [[ Handle VM IO ]]
-- Note that we currently only support serial out, as Lua impl is mostly PoC
-- TODO: Protocol for establishing arbitrary location/size shared memory bus?
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
  fake65c02.reset65c02(self.context)
end

function state_mt:step()
  fake65c02.step65c02(self.context)
end

function state_mt:run(n)
    local c = self.context
    while self.io_cmd ~= IO_HALT do
      if self.io_cmd == IO_CHAR_REQ then
        local char = io.read(1)
        if char then
          self.io_in = string.byte(char)
        else
          self.io_in = 0
        end
        self.io_cmd = 0
      end
      fake65c02.step65c02(c)
    end
    --print("halted")
end

local function new_state()
  local state = {
    io_out = 0,
    io_cmd = 0,
    io_in = 0
  }
  setmetatable(state, {
    __index = state_mt
  })
  state.context = fake65c02.new_fake65c02(nil)
  state.ram = new_bank(0x8000, 0x0000)
  state.rom = new_bank(0x8000, 0x8000)
  state.context.read = function(context, address)
    return state:read_memory(context, address) or 0x00
  end
  state.context.write = function(context, address, value)
    state:write_memory(context, address, value)
  end
  return state
end

--[[ Utility function for debugging purposes ]]
local function p8(i)
  print(string.format('%02x', i))
end
local function p16(i)
  print(string.format('%04x', i))
end

local log = nil
if verbose then
  log = print
else
  -- NOP
  log = function()
  end
end

for _, path in pairs(rom_files) do
  log(string.format("Running %s", path))
  local f, err = io.open(path, 'rb')
  if f and not err then
    log("Creating state")
    local s = new_state()
    if table_banks then
      log("Table memory copy")
      local m = s.rom.memory
      local n = 0x00
      local chunk_size = 0x0400
      while n - chunk_size < 0x8000 do
        local d = f:read(chunk_size)
        if not d then
          break
        end
        for i = 1, #d do
          m[n + (i - 1)] = string.byte(string.sub(d, i, i))
        end
        n = n + #d
      end
    else
      log("FFI memory copy")
      ffi.copy(s.rom.memory, f:read(0x8000), 0x8000)
    end
    f:close()
    log("Resetting VM state")
    s:reset()
    log("Executing ROM")
    s:run()
  else
    print(string.format("Error opening %s", err))
    os.exit(1)
  end
end
