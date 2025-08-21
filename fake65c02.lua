#!/usr/bin/env lua

local VM_VERSION = nil
if jit then
  VM_VERSION = jit.version
else
  VM_VERSION = _VERSION
end
local FAKE65C02_VERSION = '0.1.0'
local machine = require('machine')

-- [[ Handle command line arguments ]]
-- Keeping it simple, as this is just
-- an example and alternate testing
-- tool.

local args = { ... }
local rom_files = {}

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
    local state = machine()
    ffi.copy(state:add_bank(0x8000, 0x8000, nil, nil, not writable_rom), f:read(0x8000), 0x8000)
    f:close()
    state:add_bank(0x8000, 0x0000, nil, nil, false)
    log("Resetting VM state")
    state:reset()
    log("Executing ROM")
    state:run()
  else
    print(string.format("Error opening %s", err))
    os.exit(1)
  end
end
