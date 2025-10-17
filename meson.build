project(
  'thesauros',
  'cpp',
  default_options: ['cpp_std=c++23', 'warning_level=3'],
)

fmt_sub = subproject('fmt', default_options: ['default_library=static'])
fmt_dep = fmt_sub.get_variable('fmt_dep')
cpp_args = []
if host_machine.system() == 'windows'
  cpp_args += ['-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00']
endif

thesauros_dep = declare_dependency(
  include_directories: include_directories('include'),
  compile_args: cpp_args,
  dependencies: [
    fmt_dep,
    dependency('boost-preprocessor'),
    dependency('openmp'),
    dependency('unordered_dense'),
  ],
)

# install_subdir('include/thesauros', install_dir: 'include')

# pc = import('pkgconfig')
# pc.generate(
#   name: 'thesauros',
#   version: meson.project_version(),
#   description: 'thesauros',
# )

if get_option('test')
  subdir('test')
endif
