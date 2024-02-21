# Meson's support for this kind of stuff is nonexistant, at least
# in documentation. I would prefer to simply have a build template
# defined in roms/meson.build.

roms_test_source = files([
    'test_hook.s',
    'test_cmos.s',
    'test_wdc.s',
    'test_wai.s',
    'test_stp.s',
    'test_str.s',
    'test_fail.s', # _fail.s indicates expected failure
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

foreach r: roms_test_source
    rom = custom_target(
        'build_@0@'.format(r).replace('/', '_'),
        input: r,
        output: [
            '@BASENAME@.bin',
            '@BASENAME@.lst',
        ],
        command: vasm_command,
    )
    bin_path = rom[0].full_path()
    test(
        # I cannot for the life of me figure out how to get the relative path to the
        # output of a custom_target output. bin.path() doesn't work.
        '@0@'.format(r).replace('.s', '.bin'),
        fake65c02_exe,
        depends: rom,
        should_fail: bin_path.endswith('_fail.bin'),
        args: [
            bin_path,
        ],
    )
endforeach
