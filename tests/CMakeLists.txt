add_executable(tests ${CMAKE_CURRENT_SOURCE_DIR}/rtti_test.cc)
target_link_libraries(tests PUBLIC rtti gmock gtest gtest_main pthread)
clang_format(tests ${CMAKE_CURRENT_SOURCE_DIR}/rtti_test.cc)

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