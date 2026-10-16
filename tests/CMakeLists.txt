set(SOURCES
    declarations_test.cc
    hash_test.cc
    hierarchy_test.cc
    traversal_test.cc
    typeinfo_test.cc
    typeinfo_linkage_test.cc
)

find_package(Threads REQUIRED)

foreach(TARGET rtti-tests rtti-tests-no-rtti)
    add_executable(${TARGET} ${SOURCES})
    target_link_libraries(${TARGET} PRIVATE rtti gtest gtest_main Threads::Threads)
    target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(${TARGET} googletest-external)
endforeach()

if(MSVC)
    target_compile_options(rtti-tests-no-rtti PRIVATE /GR-)
else()
    target_compile_options(rtti-tests-no-rtti PRIVATE -fno-rtti)
endif()

foreach(SOURCE IN LISTS SOURCES)
    list(APPEND FORMAT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE})
endforeach()
clang_format(rtti-tests ${FORMAT_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/typeinfo_linkage.hh)
gtest_discover_tests(rtti-tests TEST_PREFIX "native.")
gtest_discover_tests(rtti-tests-no-rtti TEST_PREFIX "no-rtti.")

# Require the intended diagnostic so missing headers or compiler errors cannot
# accidentally satisfy a test that is supposed to reject an invalid hierarchy.
foreach(CASE unrelated_parent unsupported_parent)
    if(CASE STREQUAL "unrelated_parent")
        set(EXPECTED_DIAGNOSTIC "One or more parents are not a base of this type.")
    else()
        set(EXPECTED_DIAGNOSTIC "One or more parent hierarchies is not based on top of RTTI::Enable.")
    endif()
    try_compile(COMPILED
        ${CMAKE_CURRENT_BINARY_DIR}/compile-fail/${CASE}
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_fail
        invalid_hierarchy invalid
        CMAKE_FLAGS
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
            -DCMAKE_CXX_STANDARD_REQUIRED=ON
            -DCMAKE_CXX_EXTENSIONS=OFF
            -DRTTI_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/include
            -DRTTI_TEST_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/compile_fail/${CASE}.cc
            -DRTTI_USE_POINTER_TYPE_IDS=${RTTI_USE_POINTER_TYPE_IDS}
        OUTPUT_VARIABLE DIAGNOSTIC
    )
    string(FIND "${DIAGNOSTIC}" "${EXPECTED_DIAGNOSTIC}" DIAGNOSTIC_POSITION)
    if(COMPILED OR DIAGNOSTIC_POSITION EQUAL -1)
        message(FATAL_ERROR "Invalid hierarchy ${CASE} did not produce the expected diagnostic:\n${DIAGNOSTIC}")
    endif()
endforeach()

if(ENABLE_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        DEPENDENCIES rtti-tests rtti-tests-no-rtti
        EXCLUDE "build/*" 
    )
endif()
