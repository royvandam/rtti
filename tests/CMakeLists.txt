file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cc)

add_executable(tests ${SOURCES})
target_link_libraries(tests PRIVATE rtti gmock gtest gtest_main pthread)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
clang_format(tests ${SOURCES})

add_dependencies(tests googletest-external)
gtest_discover_tests(tests)

if(ENABLE_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        DEPENDENCIES tests
        EXCLUDE "build/*" 
    )
endif()