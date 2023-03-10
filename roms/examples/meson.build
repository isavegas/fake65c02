roms_examples_source = files([
    'hello_world.s',
])

# Meson's support for this kind of stuff is nonexistant, at least
# in documentation.
foreach r: roms_examples_source
    rom = custom_target(
        '@0@'.format(r).replace('/', '_'),
        input: r,
        output: [
            '@BASENAME@.bin',
            '@BASENAME@.lst',
        ],
        command: [
            vasm,
            '-Fbin',
            '-esc',
            '-c02',
            '-wdc02',
            '-quiet',
            '-chklabels',
            '-o',
            '@OUTPUT0@',
            '-L',
            '@OUTPUT1@',
            '@INPUT@',
        ],
    )
endforeach
