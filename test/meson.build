options_sub = subproject('options')
opts = options_sub.get_variable('all_opts')
args = options_sub.get_variable('all_args')

foreach name, info : {
  'Arrays': [['arrays.cpp'], []],
  'BlockedDynamicArray': [['blocked-dynamic-array.cpp'], []],
  'Concurrency': [['concurrency.cpp'], []],
  'Divisor': [['divisor.cpp'], []],
  'DynamicBitset': [['dynamic-bitset.cpp'], []],
  'Empty': [['empty.cpp'], []],
  'Execution': [['execution.cpp'], []],
  'FancyVisit': [['fancy-visit.cpp'], []],
  'FixedBitset': [['fixed-bitset.cpp'], []],
  'Format': [['format.cpp'], []],
  'IndexPositionWrapper': [['index-position-wrapper.cpp'], []],
  'IndexSegmenter': [['index-segmenter.cpp'], []],
  'Json': [['json.cpp'], []],
  'Macropolis': [['macropolis.cpp'], []],
  'Math': [['math.cpp'], []],
  'MultiByteIntegers': [['multi-byte-integers.cpp'], []],
  'Primitives': [['primitives.cpp'], []],
  'Quantity': [['quantity.cpp'], []],
  'Ranges': [['ranges.cpp'], []],
  'StaticBitset': [['static-bitset.cpp'], []],
  'StaticMap': [['static-map.cpp'], []],
  'StaticRanges': [['static-ranges.cpp'], []],
  'StaticString': [['static-string.cpp'], []],
  'StdExtensions': [['std-extensions.cpp'], []],
  'StronglyOrdered': [['strongly-ordered.cpp'], []],
  'SwapOrEqual': [['swap-or-equal.cpp'], []],
  'Tiling': [['tiling.cpp'], []],
  'TypeSeq': [['type-seq.cpp'], []],
  'TypeTraits': [['type-traits.cpp'], []],
  'ValueTag': [['value-tag.cpp'], []],
}
  sources = info[0]
  deps = info[1]
  test(
    name,
    executable(
      'Test' + name,
      sources,
      override_options: opts,
      cpp_args: args,
      dependencies: [thesauros_dep] + deps,
    ),
  )
endforeach
