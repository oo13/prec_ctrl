# Build settings for Unit Test of prec_ctrl.
# Copyright © 2023 OOTA, Masato
# This is published under CC0 1.0.
# For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.

project('test_prec_ctrl', 'cpp',
    default_options: [
        'cpp_std=c++17',
        'buildtype=debugoptimized',
        'warning_level=3',
    ],
    license: 'GPL-3.0-or-later',
    # license_files: ['../LICENSE'],
)

srcs = [
    'main.cpp',
    'double_limit.cpp',
    'equivalence.cpp',
    'FixedPoint.cpp',
    'FixedPointComplex.cpp',
    'fixed_numeric.cpp'
]
incdirs = ['../include']
deps = [dependency('catch2')]

add_project_arguments('-DCATCH_CONFIG_ENABLE_BENCHMARKING', language: 'cpp')
opt_sanitize = []
if get_option('sanitize-undefined')
   opt_sanitize += ['-fsanitize=undefined']
endif
if get_option('sanitize-float-cast-overflow')
   opt_sanitize += ['-fsanitize=float-cast-overflow']
endif
compiler = meson.get_compiler('cpp')
opt_sanitize = compiler.get_supported_arguments(opt_sanitize)

exe_sanitized = executable(
    'test_prec_ctrl_sanitized',
    srcs,
    include_directories: incdirs,
    dependencies: deps,
    cpp_args: opt_sanitize,
    link_args: opt_sanitize,
)
exe = executable(
    'test_prec_ctrl',
    srcs,
    include_directories: incdirs,
    dependencies: deps,
)
test('Unit Test', exe_sanitized)
benchmark('Bench', exe, args: ['[!benchmark]'])
