cc = find_program('cc65', required: true)
ld = find_program('ld65', required: true)
ca = find_program('ca65', required: true)

# Might need this for custom_target.

cc65_arguments = [ '--cpu', '65c02', '-t', 'none', '-o', '@OUTPUT@', '@INPUT@']

if get_option('debug') or get_option('buildtype') == 'debug'
    cc65_arguments += ['-DDEBUG']
    message('using debug define for cc65')
endif

compile = generator(
    cc,
    arguments: cc65_arguments,
    output: [ '@BASENAME@.s' ]
)


assemble = generator(
    ca,
    arguments: cc65_arguments,
    output: [ '@BASENAME@.o' ]
)

ca_objects = assemble.process(['crt0.s', 'test.s'])
cc_objects = assemble.process(['crt0.s', compile.process('rom.c')])

ca_rom = custom_target(
    'build_ca_test',
    input: [ 'fake65c02.cfg', ca_objects ],
    output: [ 'test.bin' ],
    build_by_default: true,
    command: [
        ld,
        '-o',
        '@OUTPUT0@',
        '--config',
        '@INPUT@',
    ],
)

cc_rom = custom_target(
    'build_cc_rom',
    input: ['fake65c02.cfg', cc_objects],
    output: [ 'rom.bin' ],
    build_by_default: true,
    command: [
        ld,
        '-o',
        '@OUTPUT0@',
        '--config',
        '@INPUT@',
    ],
)

run_target(
    'run_ca_test',
    command: [fake65c02_exe, '@BUILD_ROOT@/roms/c/test.bin'],
    depends: ca_rom,
)

run_target(
    'run_cc_test',
    command: [fake65c02_exe, '@BUILD_ROOT@/roms/c/rom.bin'],
    depends: cc_rom,
)
