project(
  'thesauros',
  'cpp',
  default_options: ['cpp_std=c++20', 'warning_level=3'],
)

boost_preprocessor_dep = dependency('boost-preprocessor')
expected_dep = dependency('expected')

thesauros_dep = declare_dependency(
  include_directories: include_directories('include'),
  dependencies: [boost_preprocessor_dep, expected_dep],
)

install_subdir('include', install_dir: 'include')

pc = import('pkgconfig')
pc.generate(
  name: 'thesauros',
  version: meson.project_version(),
  description: 'Thesauros',
)

if get_option('test')
  subdir('test')
endif
