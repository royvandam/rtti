cmake_minimum_required(VERSION 3.10)
project(invalid_hierarchy CXX)

add_library(invalid OBJECT ${RTTI_TEST_SOURCE})
target_include_directories(invalid PRIVATE ${RTTI_INCLUDE_DIR})
target_compile_definitions(invalid PRIVATE RTTI_USE_POINTER_TYPE_IDS=$<BOOL:${RTTI_USE_POINTER_TYPE_IDS}>)
