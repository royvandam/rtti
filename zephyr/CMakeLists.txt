if(CONFIG_RTTI_OPEN_HIERARCHY)
    zephyr_interface_library_named(rtti)
    target_include_directories(rtti INTERFACE ${ZEPHYR_RTTI_MODULE_DIR}/include)

    zephyr_link_libraries(rtti)
endif()
