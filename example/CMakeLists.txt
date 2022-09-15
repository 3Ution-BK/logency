function(add_example FILENAME)
    add_executable(${FILENAME} ${ARGN})

    target_include_directories(${FILENAME}
        PRIVATE
        ${${PROJECT_NAME}_EXAMPLE_DIR}
        ${${PROJECT_NAME}_INCLUDE_DIR}
    )

    target_compile_features(${FILENAME} PRIVATE cxx_std_17)

    target_compile_options(${FILENAME}
        PRIVATE
        "${${PROJECT_NAME}_CXX_FLAGS}"
        "$<$<CONFIG:DEBUG>:${${PROJECT_NAME}_CXX_FLAGS_DEBUG}>"
        "$<$<CONFIG:RELEASE>:${${PROJECT_NAME}_CXX_FLAGS_RELEASE}>"
    )

    target_link_libraries(${FILENAME}
        PRIVATE
        ${${PROJECT_NAME}_LIBRARY_NAME}
    )

    set_target_properties(${FILENAME}
        PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION}
        SOVERSION ${${PROJECT_NAME}_SOVERSION}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/example/lib/$<CONFIG>
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/example/lib/$<CONFIG>
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/example/bin/$<CONFIG>
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/example/bin/$<CONFIG>
    )

    if((${PROJECT_NAME}_CLANG_TIDY) AND(CLANG_TIDY_EXECUTABLE))
        set_target_properties(${FILENAME}
            PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}"
        )
    endif()
endfunction()

add_example(basic_file
    ${${PROJECT_NAME}_EXAMPLE_DIR}/basic_file.cpp
)

add_example(console
    ${${PROJECT_NAME}_EXAMPLE_DIR}/console.cpp
)

add_example(color_console
    ${${PROJECT_NAME}_EXAMPLE_DIR}/color_console.cpp
)

add_example(rotation_file
    ${${PROJECT_NAME}_EXAMPLE_DIR}/rotation_file.cpp
)

add_example(custom_message
    ${${PROJECT_NAME}_EXAMPLE_DIR}/custom_message.cpp
)

add_example(filter
    ${${PROJECT_NAME}_EXAMPLE_DIR}/filter.cpp
)

add_example(flusher
    ${${PROJECT_NAME}_EXAMPLE_DIR}/flusher.cpp
)

add_example(error_handler
    ${${PROJECT_NAME}_EXAMPLE_DIR}/error_handler.cpp
)
