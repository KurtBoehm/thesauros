options_sub = subproject('options')
opts = []
args = options_sub.get_variable('all_args')

foreach name, deps : {
  'arrays': [],
  'blocked-dynamic-array': [],
  'concepts': [],
  'divisor': [],
  'dynamic-bitset': [],
  'empty': [],
  'execution': [],
  'factorization': [],
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
  'randomize-range': [],
  'static-bitset': [],
  'static-map': [],
  'static-ranges': [],
  'static-string': [],
  'std-extensions': [],
  'swap-or-equal': [],
  'tiling': [],
  'tuples': [],
  'type-seq': [],
  'value-tag': [],
}
  space_name = name.replace('-', ' ')
  test(
    f'test @space_name@',
    executable(
      f'test-@name@',
      [f'@name@.cpp'],
      override_options: opts,
      cpp_args: args,
      dependencies: [thesauros_dep] + deps,
    ),
  )
endforeach
