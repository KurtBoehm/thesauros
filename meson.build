project(
  'thesauros',
  'cpp',
  default_options: ['cpp_std=c++23', 'warning_level=3'],
)

thesauros_dep = declare_dependency(
  include_directories: include_directories('include'),
  dependencies: [
    dependency('boost-preprocessor'),
    dependency('fmt'),
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
