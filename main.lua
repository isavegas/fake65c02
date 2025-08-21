local ffi = require('ffi')
local jit = require('jit')
local bit = require('bit')

local char_rows = 24
local char_columns = 80
local screen_width = 800
local screen_height = 400
local font_size

local cursor_y = 1
local cursor_x = 1

local lib_paths = {
  './builddir/',
  './',
}

--[[ Load fake65c02 ]]

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
  for _, prefix in ipairs(lib_paths) do
    -- Try in working directory if LD_LIBRARY_PATH doesn't have it
    success, fake65c02 = pcall(function()
      print(prefix .. 'libfake65c02' .. ext)
      return ffi.load(prefix .. 'libfake65c02' .. ext)
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
local CHARACTER_MEMORY_LOCATION = 0xB800
local CHARACTER_MEMORY_SIZE = 1920 -- 80 * 24

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

local new_bank = function(size, location)
  return ffi.new("bank_t", size, { location, size })
end

-- [[ Functions to read and write to our memory banks ]]

local state_mt = {}

function state_mt:set(addr, value)
  local ram = self.ram
  if addr >= ram.location and addr < ram.location + ram.size then
    ram.memory[addr - ram.location] = value
    return
  end
  local char_ram = self.char_ram
  if addr >= char_ram.location and addr < char_ram.location + char_ram.size then
    char_ram.memory[addr - char_ram.location] = value
    return
  end
  local rom = self.rom
  if addr >= rom.location and addr < rom.location + rom.size then
    -- rom.memory[addr - rom.location] = value
    return
  end
end

function state_mt:get(addr)
  local ram = self.ram
  if addr >= ram.location and addr < ram.location + ram.size then
    return ram.memory[addr - ram.location] or 0x00
  end
  local char_ram = self.char_ram
  if addr >= char_ram.location and addr < char_ram.location + char_ram.size then
    return char_ram.memory[addr - char_ram.location] or 0x00
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
    fake65c02.step65c02(c)
  end
  -- print("halted")
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
  state.char_ram = new_bank(CHARACTER_MEMORY_LOCATION, CHARACTER_MEMORY_SIZE)
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

local machine

local function char_address(x, y)
  return CHARACTER_MEMORY_LOCATION + (x - 1) + (y - 1) * char_columns
end

local function set_char(c, x, y)
  if not x and not y then
    x = cursor_x
    y = cursor_y
  end
  machine:set8(char_address(x, y), string.byte(c))
end

local function get_char(x, y)
  if not x and not y then
    x = cursor_x
    y = cursor_y
  end
  -- print(char_address(x,y))
  local cbyte = machine:get8(char_address(x, y))
  if cbyte == 0 then
    return nil
  else
    return string.char(cbyte)
  end
end

local function cursor_move(x, y, wrap)
  cursor_x = cursor_x + x
  if cursor_x > char_columns then
    if wrap then
      cursor_x = cursor_x - char_columns
    else
      cursor_x = char_columns
    end
  end
  if cursor_x < 1 then
    if wrap then
      cursor_y = cursor_y - 1
      cursor_x = char_columns
    else
      cursor_x = 1
    end
  end
  cursor_y = cursor_y + y
  if cursor_y > char_rows then
    cursor_y = char_rows
  end
  if cursor_y < 1 then
    cursor_y = 1
  end
end

local function carriage_return()
  cursor_move(0, 1)
  cursor_x = 1
end

local function write_string(str, wrap)
  for i = 1, #str do
    set_char(string.sub(str, i, i))
    cursor_move(1, 0, wrap)
  end
end

function love.load()
  love.window.setMode(screen_width, screen_height, {
    resizable = false
  })
  love.window.setTitle("Character Grid")
  font_size = screen_height / char_rows
  cursor_x = 1
  cursor_y = 1
  machine = new_state()
  local rom_file = './builddir/roms/examples/hello_world.bin'
  local f, err = io.open(rom_file, 'rb')
  if f then
    ffi.copy(machine.rom.memory, f:read(0x8000), 0x8000)
    f:close()
  else
    cursor_x = 1
    cursor_y = 1
    write_string('Unable to load ' .. rom_file, true)
    machine.io_cmd = IO_HALT
    carriage_return()
    write_string(err)
  end
  -- machine:run()
  -- write_string('test')
end

local timer = 0
local rate = .001
function love.update(dt)
  if timer > rate then
    if machine.io_cmd ~= IO_HALT then
      machine:step()
      if machine.io_cmd == IO_HALT then
        write_string('halted')
        print('halted')
      end
    else
      love.window.setTitle("Halted")
    end
    timer = timer - rate
  else
    timer = timer + dt
  end
end

function love.draw()
  love.graphics.clear(0, 0, 0)
  for x = 1, char_columns do
    for y = 1, char_rows do
      local char = get_char(x, y)
      if char then
        love.graphics.print(char, (x - 1) * (screen_width / char_columns),
          (y * (screen_height / char_rows)) - font_size)
      end
    end
  end
end

function love.keypressed(key)
  if key == "escape" then
    love.event.quit()
    --[[
    elseif key == "up" then
        cursor_y = cursor_y - 1
    elseif key == "down" then
        cursor_y = cursor_y + 1
    elseif key == "left" then
        cursor_x = cursor_x - 1
    elseif key == "right" then
        cursor_x = cursor_x + 1
    else
        charGrid[cursor_y][cursor_x] = key
    ]]
  end
end

--[[function love.keyreleased(key)
    -- Handle key releases here (if needed)
end]]
