# This file is part of https://github.com/KurtBoehm/thesauros.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

project(
  'thesauros',
  'cpp',
  default_options: ['cpp_std=c++23', 'warning_level=3'],
)

if not meson.is_subproject()
  # On macOS, _Static_assert is used in system headers, which GCC does not
  # support in C++ mode
  cpp = meson.get_compiler('cpp')
  if cpp.get_id() == 'gcc' and host_machine.system() == 'darwin'
    add_global_arguments('-D_Static_assert=static_assert', language: 'cpp')
  endif
endif

fmt_sub = subproject('fmt', default_options: ['default_library=static'])
fmt_dep = fmt_sub.get_variable('fmt_dep')

cpp_args = []
if host_machine.system() == 'windows'
  cpp_args += ['-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00']
endif

boost_pp_dep = dependency('boost-preprocessor')

inc_dir = include_directories('include')

#-------------------------------------------------------------------------------
# Dependencies per group of sub-libraries
#-------------------------------------------------------------------------------

# Everything that needs nothing beyond the standard library: `algorithms`,
# `charconv`, `concepts`, `containers`, `execution`, `filesystem`, `functional`,
# `iterator`, `literals`, `math`, `memory`, `quantity`, `random`, `ranges`,
# `string`, and `types`.
thesauros_core_dep = declare_dependency(
  include_directories: inc_dir,
  compile_args: cpp_args,
)

# `macropolis`, `reflection` and everything using their macros: `io`,
# `static-ranges`, `utility`.
thesauros_macros_dep = declare_dependency(
  dependencies: [thesauros_core_dep, boost_pp_dep],
)

# `argparse` writes its help text and error messages with {fmt}, but needs
# nothing else.
thesauros_argparse_dep = declare_dependency(
  dependencies: [thesauros_core_dep, fmt_dep],
)

# `format` and `test` are the only sub-libraries reaching {fmt}; `format` also
# formats the reflection and resource types, so it inherits both of those.
thesauros_format_dep = declare_dependency(
  dependencies: [thesauros_macros_dep, fmt_dep],
)

# Everything, for consumers that would rather not track which sub-libraries they
# use.
thesauros_dep = declare_dependency(
  dependencies: [
    thesauros_core_dep,
    thesauros_argparse_dep,
    thesauros_macros_dep,
    thesauros_format_dep,
  ],
)

if not meson.is_subproject()
  install_subdir('include/thesauros', install_dir: 'include')
  pc = import('pkgconfig')
  pc.generate(
    name: 'thesauros',
    version: meson.project_version(),
    description: 'thesauros',
  )
endif

if get_option('build_tests')
  subdir('test')
endif

if get_option('build_bench')
  subdir('bench')
endif
