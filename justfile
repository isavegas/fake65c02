set shell := ["sh", "-cu"]
set dotenv-load
set positional-arguments

name := "fake65c02"

alias make := build
alias gmake := build


@default:
    just --list --unsorted --list-heading "$(printf 'Targets for {{name}}::\n\r')"

# Platform info.
info:
    #!/usr/bin/env sh
    command -v uname > /dev/null 2>&1 && uname -a && exit 0
    echo user justfile :: linux x86_64

# Build using GNU Make, regardless of platform
@build:
    [ "$(uname)" = *"BSD" ] && gmake || make

# Watch our project, building on updates
@watch cmd:
    watchexec -c -r -w . -w roms "just build && {{cmd}}"
