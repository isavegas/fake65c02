project(
    'fake65c02',
    'c',
    version: '0.1.0',
    default_options: [
        'b_lto=true',
        'b_lto_mode=thin',
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
    '-DWRITABLE_VECTORS',
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

fake65c02_binary = executable(
    'fake65c02',
    dependencies: [
        fake65c02_static_dep
    ],
    sources: [ 'src/main.c' ],
)
