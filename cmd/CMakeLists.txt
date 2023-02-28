#[[
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs

   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
]]

if(MSVC)
    add_link_options(/STACK:0x1000000)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_link_options(-Wl,-stack_size -Wl,0x1000000)
else()
    add_link_options(-Wl,-z,stack-size=0x1000000)
endif()

if(ZEN_TESTS)
    add_subdirectory(test) # The testing suites
endif()
