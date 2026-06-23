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

# On macOS, _Static_assert is used in system headers, which GCC does not support
# in C++ mode
cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'gcc' and host_machine.system() == 'darwin'
  add_global_arguments('-D_Static_assert=static_assert', language: 'cpp')
endif

use_iokit = get_option('use_iokit')

fmt_sub = subproject('fmt', default_options: ['default_library=static'])
fmt_dep = fmt_sub.get_variable('fmt_dep')

cpp_args = [f'-DTHES_USE_IOKIT=@use_iokit@']
if host_machine.system() == 'windows'
  cpp_args += ['-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00']
endif

dependencies = [
  fmt_dep,
  dependency('boost-preprocessor'),
  dependency('openmp'),
  dependency('pcg-cpp'),
  dependency('unordered_dense'),
]

if host_machine.system() == 'darwin' and use_iokit
  applefw_dep = dependency(
    'appleframeworks',
    modules: ['CoreFoundation', 'IOKit'],
    required: host_machine.system() == 'darwin',
  )
  dependencies += [applefw_dep]

  thesauros_lib = library(
    'thesauros',
    ['src/resources/cpu-info-macos.cpp'],
    include_directories: include_directories('include'),
    dependencies: dependencies,
    cpp_args: cpp_args,
  )
else
  thesauros_lib = []
endif

thesauros_dep = declare_dependency(
  include_directories: include_directories('include'),
  dependencies: dependencies,
  link_with: thesauros_lib,
  compile_args: cpp_args,
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
