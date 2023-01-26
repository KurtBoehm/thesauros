project(
  'treasury',
  'cpp',
  default_options: ['cpp_std=c++20', 'warning_level=3']
)

treasury_dep = declare_dependency(
  include_directories: include_directories('include')
)

install_subdir(
  'include',
  install_dir: 'include'
)

pc = import('pkgconfig')
pc.generate(
  name: 'treasury',
  version: meson.project_version(),
  description: 'The treasury'
)

if get_option('test')
  subdir('test')
endif
