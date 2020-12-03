cmake_minimum_required(VERSION 3.10)
project(rtti CXX)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Build options
option(ENABLE_DOCS             "Enable documentation target" OFF)
option(ENABLE_CODE_COVERAGE    "Enable code coverage generation" OFF)
option(BUILD_TESTS             "Build unittest binaries" ON)

# CXX compilation options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS} -O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")

# CMake module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Vendor installation path
set(EXTERNAL_INSTALL_DIR ${PROJECT_BINARY_DIR}/external)
include_directories(${EXTERNAL_INSTALL_DIR}/include)
link_directories(${EXTERNAL_INSTALL_DIR}/lib)

# Code formattingh
include(clang-format)

# Code coverage generation
if(ENABLE_CODE_COVERAGE)
    include(code-coverage)
    append_coverage_compiler_flags()
endif()

# The library
add_library(rtti STATIC ${CMAKE_CURRENT_SOURCE_DIR}/include/rtti.hh)
target_include_directories(rtti PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(rtti PROPERTIES LINKER_LANGUAGE CXX)
clang_format(rtti ${CMAKE_CURRENT_SOURCE_DIR}/include/rtti.hh)

# Documentation
if (ENABLE_DOCS)
    find_package(Doxygen REQUIRED dot mscgen dia)

    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)

    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Generate documentation") 
endif()

# Testing
if (BUILD_TESTS)
    enable_testing()
    include(googletest)
    add_subdirectory(tests)
endif()