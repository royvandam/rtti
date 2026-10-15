include(ExternalProject)
ExternalProject_Add(googlebenchmark-external
    GIT_REPOSITORY    https://github.com/google/benchmark.git 
    GIT_TAG           main
    PREFIX            ${VENDOR_PREFIX}
    INSTALL_DIR       ${EXTERNAL_INSTALL_DIR}
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
                      -DCMAKE_INSTALL_LIBDIR=lib
                      -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                      -DBENCHMARK_ENABLE_GTEST_TESTS=OFF
)
