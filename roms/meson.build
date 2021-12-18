vasm = find_program('vasm6502_oldstyle', required: true)

# Might need this for custom_target.
#if get_option('buildtype') == 'debug'
#    add_project_arguments('-DDEBUG')
#endif


#vasm_build = generator(
#    vasm,
#    output: [
#        '@BASENAME@.bin',
#        '@BASENAME@.lst',
#    ],
#    arguments: [
#        'vasm6502_oldstyle',
#        '-Fbin',
#        '-esc',
#        '-c02',
#        '-quiet',
#        '-chklabels',
#        '-o',
#        '@OUTPUT0@',
#        '-L',
#        '@OUTPUT1@',
#        '@INPUT@',
#    ],
#)

subdir('tests')

#test_roms = vasm_build.process(roms_test_source)
