set shell := ["sh", "-cu"]
set dotenv-load
set positional-arguments

name := "fake65c02"

build_dir := justfile_directory()+"/builddir"
love_dir := justfile_directory()+"/love2d_example"
shared_ext := if os() == "windows" { "dll" } else if os() == "macos" { "dylib" } else { "so" }

alias b := build
alias t := test
#alias w := watch
alias l := list
alias c := clean
alias dc := deep_clean

# -> build
@default: build

@list:
    just --list --unsorted --list-heading "$(printf 'Targets for {{name}}::\n\r')"

@info:
    echo {{name}} :: {{os()}} {{arch()}}

@setup buildtype:
    if [ ! -f "{{build_dir}}/build.ninja" ]; then meson setup --buildtype "{{buildtype}}" "{{build_dir}}"; fi

@build buildtype='release': (setup buildtype)
    ninja -C "{{build_dir}}"

@tidy:
    ninja -C "{{build_dir}}" clang-tidy

@format:
    ninja -C "{{build_dir}}" clang-format

@clean:
    ninja -C "{{build_dir}}" clean

# Deep clean project, forcing fresh `meson setup`
deep_clean:
    rm -rf "{{build_dir}}"
    rm -f "{{justfile_directory()}}/src/fake65c02"
    rm -f "{{justfile_directory()}}/src/**/.so"
    rm -f "{{justfile_directory()}}/src/**/.o"
    rm -f "{{justfile_directory()}}/src/**/*.exe"
    rm -f "{{justfile_directory()}}/roms/**/*.bin"
    rm -f "{{justfile_directory()}}/roms/**/*.lst"

@test:
    ninja -C "{{build_dir}}" test

@setup_love:
    if [ ! -f "{{love_dir}}/hello_world.bin" ]; then ln -sf "{{build_dir}}/roms/examples/hello_world.bin" "{{love_dir}}/"; fi
    if [ ! -f "{{love_dir}}/libfake65c02.{{shared_ext}}" ]; then ln -sf "{{build_dir}}/libfake65c02.{{shared_ext}}" "{{love_dir}}/"; fi

@love:
    pushd "{{love_dir}}" && love "{{love_dir}}"
