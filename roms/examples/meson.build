roms_examples_source = files([
    'hello_world.s',
    'char_request.s',
])

vasm_command = [
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
]

if get_option('debug') or get_option('buildtype') == 'debug'
    vasm_command += ['-DDEBUG']
endif

foreach r: roms_examples_source
    custom_target(
        'build_@0@'.format(r).replace('/', '_'),
        input: r,
        output: [
            '@BASENAME@.bin',
            '@BASENAME@.lst',
        ],
        build_by_default: true,
        command: vasm_command,
    )
endforeach
