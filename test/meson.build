options_sub = subproject('options')
opts = []
args = options_sub.get_variable('all_args')

foreach name, deps : {
  'arrays': [],
  'blocked-dynamic-array': [],
  'concepts': [],
  'concurrency': [],
  'divisor': [],
  'dynamic-bitset': [],
  'empty': [],
  'execution': [],
  'fancy-visit': [],
  'fixed-bitset': [],
  'flat': [],
  'format': [],
  'index-position-wrapper': [],
  'index-segmenter': [],
  'json': [],
  'macropolis': [],
  'math': [],
  'multi-byte-integers': [],
  'primitives': [],
  'quantity': [],
  'ranges': [],
  'static-bitset': [],
  'static-map': [],
  'static-ranges': [],
  'static-string': [],
  'std-extensions': [],
  'swap-or-equal': [],
  'tiling': [],
  'type-seq': [],
  'value-tag': [],
}
  space_name = name.replace('-', ' ')
  test(
    name,
    executable(
      f'test @space_name@',
      [f'@name@.cpp'],
      override_options: opts,
      cpp_args: args,
      dependencies: [thesauros_dep] + deps,
    ),
  )
endforeach
