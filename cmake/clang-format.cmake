if (NOT CLANGFORMAT_CONFIGURED)
    set(CLANGFORMAT_CONFIGURED true)

    set(CLANGFORMAT_CONFIG ${PROJECT_SOURCE_DIR}/.clang-format)
    if(EXISTS ${CLANGFORMAT_CONFIG})
        set(CLANGFORMAT_STYLE -style=file)
    else()
        message("Unable to find .clang-format file, using google style")
        set(CLANGFORMAT_STYLE -style=google)
    endif()

    find_program(CLANGFORMAT clang-format)
    if(CLANGFORMAT)
        add_custom_target(format)

        function(clang_format TARGET)
            set(SOURCES ${ARGN})

            add_custom_target(format-${TARGET}
                COMMAND
                    ${CLANGFORMAT} ${CLANGFORMAT_STYLE} -i
                    ${SOURCES}
                COMMENT
                    "Formatting sources..."
            )

            add_dependencies(format format-${TARGET})
        endfunction(clang_format)
    else()
        message(WARNING "Unable to find clang-format binary")
        function(clang_format TARGET)
        endfunction(clang_format)
    endif()
endif()

