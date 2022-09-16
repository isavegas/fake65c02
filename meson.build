project(
    'fake65c02',
    'c',
    version: '0.1.0',
    default_options: [
        'b_lto=true',
        'b_lto_mode=thin',
        'b_lundef=false',
        #'b_sanitize=leak',
        'b_pie=true',
        'b_staticpie=true',
        'c_std=c2x',
        'default_library=both',
        'strip=true',
    ],
)

add_project_arguments(
    [
        '-march=native',
        '-mtune=native',
    ],
    language: 'c'
)

# Defines for fake65c02.{c,h}
fake65c02_defines = [
    '-DUNDOCUMENTED',
]

fake65c02_include = include_directories(
    'include',
)

fake65c02_shared = shared_library(
    'fake65c02',
    c_args: fake65c02_defines,
    include_directories: fake65c02_include,
    sources: [ 'src/fake65c02.c' ],
)

fake65c02_static = static_library(
    'fake65c02',
    c_args: fake65c02_defines,
    include_directories: fake65c02_include,
    sources: [ 'src/fake65c02.c' ],
)

fake65c02_static_dep = declare_dependency(
    link_with: fake65c02_static,
    include_directories: fake65c02_include,
)

fake65c02_exe = executable(
    'fake65c02',
    c_args: [
        '-DWRITABLE_VECTORS',
    ],
    dependencies: [
        fake65c02_static_dep
    ],
    sources: [ 'src/main.c' ],
)

subdir('roms')
