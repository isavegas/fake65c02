roms_test_source = files([
    'test_stack.s',
    'test_cmos.s',
    'test_wai_nointerrupt.s',
    'test_wai_interrupt.s',
    'test_stp.s',
])

# Meson's support for this kind of stuff is nonexistant, at least
# in documentation.
foreach r: roms_test_source
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
    bin = rom[0]
    test(
        'test_rom_@0@'.format(bin.full_path()),
        fake65c02_exe,
        depends: rom,
        args: [
            bin.full_path(),
        ],
    )
endforeach
