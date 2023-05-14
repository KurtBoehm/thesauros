options_sub = subproject('options')
opts = options_sub.get_variable('all_opts')
args = options_sub.get_variable('all_args')

foreach name, info : {
  'Arrays': [['arrays.cpp'], []],
  'BlockedDynamicArray': [['blocked-dynamic-array.cpp'], []],
  'DynamicBitset': [['dynamic-bitset.cpp'], []],
  'Empty': [['empty.cpp'], []],
  'Execution': [['execution.cpp'], []],
  'FancyVisit': [['fancy-visit.cpp'], []],
  'IndexSegmenter': [['index-segmenter.cpp'], []],
  'Macropolis': [['macropolis.cpp'], []],
  'Primitives': [['primitives.cpp'], []],
  'Ranges': [['ranges.cpp'], []],
  'StaticMap': [['static-map.cpp'], []],
  'StaticRanges': [['static-ranges.cpp'], []],
  'StaticString': [['static-string.cpp'], []],
  'Tiling': [['tiling.cpp'], []],
  'Quantity': [['quantity.cpp'], []],
}
  sources = info[0]
  deps = info[1]
  test(
    name,
    executable('Test' + name, sources,
      override_options: opts,
      cpp_args: args,
      dependencies: [thesauros_dep] + deps,
    )
  )
endforeach
