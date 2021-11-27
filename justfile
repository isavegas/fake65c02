set shell := ["sh", "-cu"]
set dotenv-load
set positional-arguments

name := "fake65c02"

alias make := build
alias gmake := build

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
    command -v uname > /dev/null 2>&1 && uname -a || echo {{name}} :: {{os()}} {{arch()}}

# Build project
@build:
    {{build_cmd}}

# Clean project
@clean:
    {{build_cmd}} clean

# Run tests for project
@test:
    {{build_cmd}} test

# Watch our project, building and running cmd on updates
@watch cmd:
    watchexec -c -r -w . -w roms "just build && {{cmd}}"
