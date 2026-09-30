# This file is part of https://github.com/KurtBoehm/thesauros.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

args = subproject('options').get_variable('all_args')

# One directory per sub-library, mirroring `include/thesauros`.
foreach name : ['thread-pools']
  executable(
    f'bm-@name@',
    [f'@name@.cpp'],
    dependencies: [thesauros_dep],
    cpp_args: args,
  )
endforeach
