set shell := ["sh", "-cu"]
set dotenv-load
set positional-arguments

name := "fake65c02"

build_dir := justfile_directory()+"/build"
dll_suffix := if os() == "windows" { ".dll" } else if os() == "macos" { ".dylib" } else { ".so" }

alias b := build
alias t := test
alias l := list
alias c := clean
alias dc := deep_clean

@default: build

@list:
    just --list --unsorted --list-heading "$(printf 'Targets for {{name}}::\n\r')"

@info:
    echo {{name}} :: {{os()}} {{arch()}}

@setup buildtype='debug':
    if [ ! -f "{{build_dir}}/build.ninja" ]; then meson setup --buildtype "{{buildtype}}" "{{build_dir}}"; fi

@build buildtype='debug': (setup buildtype)
    ninja -C "{{build_dir}}"

@tidy:
    ninja -C "{{build_dir}}" clang-tidy

#@format:
#    ninja -C "{{build_dir}}" clang-format

@clean:
    ninja -C "{{build_dir}}" clean

# Deep clean project, forcing fresh `meson setup`
[confirm]
deep_clean:
    rm -rf "{{build_dir}}"

@test:
    ninja -C "{{build_dir}}" test

@run: (build 'debug')
    exec "{{build_dir}}/fake65c02"

@gui: (build 'debug')
    exec "{{build_dir}}/fake65c02_gui"
