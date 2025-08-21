local ffi = require('ffi')
local bit = require('bit')


--[[ Load fake65c02 ]]

local lib_paths = {
  './builddir/',
  './',
}

-- Try loading it from LD_LIBRARY_PATH
local success, fake65c02 = pcall(function()
  return ffi.load('fake65c02')
end)
if not success then
  local ext = ""
  if jit.os == "OSX" then
    ext = ".dylib"
  elseif jit.os == "Linux" then
    ext = ".so"
  elseif jit.os == "Windows" then
    ext = ".dll"
  end
  for _, p in ipairs(lib_paths) do
    -- Try in working directory if LD_LIBRARY_PATH doesn't have it
    success, fake65c02 = pcall(function()
      --print(p .. 'libfake65c02' .. ext)
      return ffi.load(p .. 'libfake65c02' .. ext)
    end)
    if success then
      break
    end
  end
end

if not success then
  print("Could not find fake65c02!")
  -- TODO: Show error screen?
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

typedef struct bank bank_t;
typedef uint8_t (*read_bank)(bank_t *bank, uint16_t address);
typedef void (*write_bank)(bank_t *bank, uint16_t address, uint8_t value);

struct bank {
  uint16_t size;
  uint16_t location;
  read_bank read;
  write_bank write;
  uint8_t read_only;
  uint8_t memory[?];
};
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

--[[ Memory utility functions ]]

local read = function(bank, address)
  address = address - bank.location
  if address < bank.size then
    return bank.memory[address], true
  else
    return 0x00, false
  end
end

local write = function(bank, address, value)
  address = address - bank.location
  if address < bank.size then
    bank.memory[address] = value
    return true
  else
    return false
  end
end

-- [[ Functions to read and write to our memory banks ]]

local machine = {}

function machine:add_bank(size, location, read_function, write_function, read_only)
  if not read_function then read_function = read end
  if not write_function then write_function = write end
  local bank = ffi.new("bank_t", size, { size, location, read_function, write_function, read_only })
  table.insert(self.banks, bank)
  return bank
end

function machine:set(addr, value)
  for _,bank in ipairs(self.banks) do
    if addr >= bank.location and addr < bank.location + bank.size then
      if not bank.read_only then
        bank:write(addr, value)
      end
      break
    end
  end
end

function machine:get(addr)
  local value = 0x00
  for _,bank in ipairs(self.banks) do
    if addr >= bank.location and addr < bank.location + bank.size then
      value = bank:read(addr)
      break
    end
  end
  return value
end

-- Unused PoC
function machine:set16(addr, value)
  self:set(addr, bit.band(value, 0x00ff))
  self:set(addr + 1, bit.rshift(value, 8))
end

function machine:set8(addr, value)
  self:set(addr, bit.band(value, 0x00ff))
end

-- Unused PoC
function machine:get16(addr)
  return bit.band(self:get(addr), 0x00ff) + bit.lshift(self:get(addr + 1), 8)
end

function machine:get8(addr)
  return bit.band(self:get(addr), 0x00ff)
end

function machine:read_memory(addr)
  if addr == IO_IN then
    return self.io_in
  end
  return self:get8(addr)
end

-- [[ Handle VM IO ]]
-- Note that we currently only support serial out, as Lua impl is mostly PoC
-- TODO: Protocol for establishing arbitrary location/size shared memory bus?
function machine:write_memory(addr, value)
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

function machine:reset()
  fake65c02.reset65c02(self.context)
end

function machine:step()
  fake65c02.step65c02(self.context)
end

function machine:run()
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
end

function machine.new_state()
  local state = {
    io_out = 0,
    io_cmd = 0,
    io_in = 0,
    banks = {},
  }
  setmetatable(state, {
    __index = machine
  })
  state.context = fake65c02.new_fake65c02(nil)
  state.context.read = function(_context, address)
    return state:read_memory(address) or 0x00
  end
  state.context.write = function(_context, address, value)
    state:write_memory(address, value)
  end
  return state
end

setmetatable(machine, {
  __call = machine.new_state,
})

return machine
