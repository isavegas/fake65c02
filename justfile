set shell := ["sh", "-cu"]
set dotenv-load
set positional-arguments

name := "fake65c02"

alias make := build
alias gmake := build

alias b := build
alias t := test
alias w := watch
alias l := list
alias c := clean

# Use gmake on BSD family. Might need if/else tree if this doesn't work for all of them.
# Note that FreeBSD's pkg repo version of justfile doesn't have regex support built-in.
# You'll need to install it via cargo, unfortunately.
build_cmd := if os() =~ ".*bsd" { "gmake" } else { "make" }

# -> build
@default: build

# List all just targets
@list:
    just --list --unsorted --list-heading "$(printf 'Targets for {{name}}::\n\r')"


# Show platform info
@info:
    echo {{name}} :: {{os()}} {{arch()}}

# Build project
@build:
    {{build_cmd}} && {{build_cmd}} -C roms

# Clean project
@clean:
    {{build_cmd}} clean

# Run tests for project
@test:
    {{build_cmd}} test

# Watch our project, building and running cmd on updates
@watch cmd="./fake65c02 roms/tests/test_65c02.bin":
    watchexec -c -r -w main.c -w main.h -w fake65c02.c -w fake65c02.h -w roms "just build && {{cmd}}"
