#!/usr/bin/env sh

if [ -z "$1" ]; then
    echo Please specify a make .bin target
    exit 1
fi
defines=
if [ "$2" = "debug" ]; then
    defines="DEBUG=1"
fi

watchexec -r -c -w ../Makefile -w ../main.c -w ../fake6502.c -w ../fake65c02.c -w lib.s -w tests "make -C .. $defines && make $1 $defines && echo fake65c02 && ../fake65c02 $1 | sed 's/^/  /' && echo && echo fake6502 && ../fake6502 $1 | sed 's/^/  /'"
