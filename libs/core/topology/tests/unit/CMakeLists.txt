# Copyright (c) 2025 Shreyas Atre
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Threading tests
set(topo_tests
    topo_mapping
)

foreach(test ${topo_tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  add_hpx_executable(
    ${test}_test INTERNAL_FLAGS
    SOURCES ${sources}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Modules/Core/Topology"
  )

  add_hpx_unit_test("modules.topology" ${test})

endforeach()


target_link_libraries(topo_mapping_test PRIVATE
    numa
    papi
)

target_compile_options(topo_mapping_test PRIVATE -g -O0)