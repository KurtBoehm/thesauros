# This file is part of https://github.com/KurtBoehm/thesauros.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

options_sub = subproject('options')
opts = []
args = options_sub.get_variable('all_args')

# Per-test timeout overrides (in seconds), for tests whose workload exceeds Meson’s default.
test_timeouts = {
  'randomize-range': 600,
}

# One directory per sub-library, mirroring `include/thesauros`.
foreach module, names : {
  'algorithms': ['sort-indices', 'swap-or-equal', 'tiling'],
  'charconv': ['charconv', 'concat'],
  'concepts': ['concepts'],
  'containers': [
    'array-policies',
    'arrays',
    'chunked-dynamic-array',
    'dynamic-bitset',
    'dynamic-buffer',
    'fixed-bitset',
    'flat',
    'flat-map',
    'flat-set',
    'multi-bit-integers',
    'multi-byte-integers',
    'nested-dynamic-array',
    'set-algorithms',
    'static-bitset',
  ],
  'execution': ['execution'],
  'filesystem': ['tempfile'],
  'format': ['format', 'formatters'],
  'functional': ['functional'],
  'io': ['file', 'json', 'serialization'],
  'iterator': ['iterator-facades'],
  'math': [
    'arithmetic',
    'divisor',
    'factorization',
    'integer-cast',
    'math',
    'overflow',
    'tessellation',
  ],
  'memory': ['byte-read'],
  'quantity': ['quantity'],
  'random': ['lcg', 'randomize-range'],
  'ranges': ['ranges'],
  'reflection': ['reflection'],
  'static-ranges': ['static-ranges', 'views-and-sinks'],
  'string': ['static-capacity-string', 'static-string'],
  'types': [
    'empty',
    'integer-traits',
    'primitives',
    'tuples',
    'type-seq',
    'type-transformations',
    'value-tag',
    'void-storage',
  ],
  'utility': [
    'fancy-visit',
    'index-position-wrapper',
    'index-segmenter',
    'check-variant',
    'multi-bit-reference',
    'multi-size',
    'optional',
    'small-utilities',
    'static-map',
    'std-extensions',
  ],
}
  foreach name : names
    space_name = name.replace('-', ' ')
    test(
      space_name,
      executable(
        f'test-@module@-@name@',
        [f'@module@/@name@.cpp'],
        dependencies: [thesauros_dep],
        override_options: opts,
        cpp_args: args,
      ),
      suite: module,
      timeout: test_timeouts.get(name, 30),
    )
  endforeach
endforeach

#---------------------------------------------------------------------------------------------------
# Structural checks, which belong to no single sub-library
#---------------------------------------------------------------------------------------------------

# Built against `thesauros_core_dep` alone, so it fails if a dependency-free sub-library
# starts pulling in an external one.
test(
  'core dependency',
  executable(
    'test-core-dep',
    ['core-dep.cpp'],
    dependencies: [thesauros_core_dep],
    override_options: opts,
    cpp_args: args,
  ),
  suite: 'structure',
)

python = import('python').find_installation('python3', required: false)
if python.found()
  test(
    'check includes',
    python,
    args: [files('check-includes.py')],
    suite: 'structure',
  )
endif
