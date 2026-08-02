project(
    'fake65c02',
    'c',
    version: '0.1.0',
    license: 'MIT',
    default_options: [
        'c_std=c23',
        'default_library=both',
        'strip=true',
        'native_tuning=true',
        'gui=enabled',
    ],
)

if get_option('debug') or get_option('buildtype') == 'debug'
    add_project_arguments([
        '-DDEBUG',
        '-O0',
    ], language: 'c')
else
    add_project_arguments('-flto=true', language: 'c')
    if get_option('native_tuning')
        add_project_arguments(
            [
                '-march=native',
                '-mtune=native',
            ],
            language: 'c',
        )
    endif
endif

# Temporary fix to vcpkg's raylib installation having SUPPORT_CUSTOM_FRAME_CONTROL enabled
if get_option('gui').enabled() and build_machine.system() == 'windows' and target_machine.system() == 'windows'
    add_project_arguments('-DVCPKG_FIX', language: 'c')
endif

includes = include_directories(
    'include',
)

shared_library(
    'fake65c02',
    include_directories: includes,
    sources: [ 'src/fake65c02.c' ],
)

fake65c02_static = static_library(
    'fake65c02',
    include_directories: includes,
    sources: [ 'src/fake65c02.c' ],
)

fake65c02_static_dep = declare_dependency(
    link_with: fake65c02_static,
    include_directories: includes,
)

fake65c02_exe = executable(
    'fake65c02',
    dependencies: [
        fake65c02_static_dep,
    ],
    sources: [ 'src/main.c', 'src/machine.c', 'src/debugger.c', 'src/ppu.c' ],
)

if get_option('gui').enabled()
    raylib_dep = dependency('raylib', version: '>=0.6.0')
    win_subsystem = 'windows'
    if get_option('debug') or get_option('buildtype') == 'debug'
        win_subsystem = 'console'
    endif
    executable(
        'fake65c02_gui',
        c_args: [ '-DWRITABLE_VECTORS' ],
        win_subsystem: win_subsystem,
        dependencies: [
            fake65c02_static_dep,
            raylib_dep,
        ],
        sources: [ 'src/gui.c', 'src/machine.c', 'src/ppu.c' ],
    )
endif

subdir('tests')
subdir('roms')
