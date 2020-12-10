file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cc)

add_executable(rtti-tests ${SOURCES})
target_link_libraries(rtti-tests PRIVATE rtti gmock gtest gtest_main pthread)
target_include_directories(rtti-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
clang_format(rtti-tests ${SOURCES})

add_dependencies(rtti-tests googletest-external)
gtest_discover_tests(rtti-tests)

if(ENABLE_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        DEPENDENCIES rtti-tests
        EXCLUDE "build/*" 
    )
endif()