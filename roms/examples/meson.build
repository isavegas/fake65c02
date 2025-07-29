roms_examples_source = files([
    'hello_world.s',
    'char_request.s',
])

foreach r: roms_examples_source
    custom_target(
        'build_@0@'.format(r).replace('/', '_'),
        input: r,
        output: [
            '@BASENAME@.bin',
            '@BASENAME@.lst',
        ],
        build_by_default: true,
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
