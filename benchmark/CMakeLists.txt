add_executable(rtti-benchmark ${CMAKE_CURRENT_SOURCE_DIR}/rtti_benchmark.cc)
target_link_libraries(rtti-benchmark PUBLIC rtti benchmark pthread)
add_dependencies(rtti-benchmark googlebenchmark-external)

clang_format(rtti-benchtest ${CMAKE_CURRENT_SOURCE_DIR}/rtti_benchmark.cc)