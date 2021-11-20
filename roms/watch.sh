#!/usr/bin/env sh

if [ -z "$1" ]; then
    echo Please specify a make .bin target
    exit 1
fi

watchexec -r -c -w ../main.c -w ../fake6502.c -w ../fake65c02.c -w lib.s -w tests "make -C .. && make $1 && echo fake65c02 && ../fake65c02 $1 | sed 's/^/  /' && echo && echo fake6502 && ../fake6502 $1 | sed 's/^/  /'"
