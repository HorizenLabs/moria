
message(STATUS "Building tests ... ${ZEN_MAIN_SRC_DIR}/zen/core/*_test.cpp")
find_package(Catch2 CONFIG REQUIRED)

# Zen Core Tests
file(GLOB_RECURSE ZEN_CORE_TESTS CONFIGURE_DEPENDS "${ZEN_MAIN_SRC_DIR}/core/*_test.cpp")
add_executable(core_test unit_test.cpp ${ZEN_CORE_TESTS})
target_link_libraries(core_test zen_core Catch2::Catch2)

if(MSVC)
    target_compile_options(core_test PRIVATE /EHa- /EHsc)
else()
    target_compile_options(core_test PRIVATE -fno-exceptions)
endif()

if(NOT ZEN_CORE_ONLY)

    # Zen Node Tests
    file(GLOB_RECURSE ZEN_NODE_TESTS CONFIGURE_DEPENDS "${ZEN_MAIN_SRC_DIR}/node/*_test.cpp")
    list(LENGTH ZEN_NODE_TESTS ZEN_NODE_SOURCE_ITEMS)
    if(NOT ZEN_NODE_SOURCE_ITEMS EQUAL 0)
        add_executable(node_test unit_test.cpp ${ZEN_NODE_TESTS})
        target_link_libraries(node_test zen_node Catch2::Catch2)
    endif()

endif()
