# This file is part of https://github.com/KurtBoehm/thesauros.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

options_sub = subproject('options')
opts = []
args = options_sub.get_variable('all_args')

foreach name, deps : {
  'arrays': [],
  'chunked-dynamic-array': [],
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
  'tempfile': [],
  'tessellation': [],
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
