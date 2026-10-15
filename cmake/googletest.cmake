include(ExternalProject)

ExternalProject_Add(googletest-external
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           main
    PREFIX            ${VENDOR_PREFIX}
    INSTALL_DIR       ${EXTERNAL_INSTALL_DIR}
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
                      -DCMAKE_INSTALL_LIBDIR=lib
                      -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
)

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
include(GoogleTest)
