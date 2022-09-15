function(add_executable_test FILENAME)
    add_executable(${FILENAME} ${ARGN})

    target_include_directories(${FILENAME}
        PRIVATE
        ${${PROJECT_NAME}_TEST_DIR}
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
        doctest
    )

    set_target_properties(${FILENAME}
        PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION}
        SOVERSION ${${PROJECT_NAME}_SOVERSION}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/lib/$<CONFIG>
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/bin/$<CONFIG>
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/bin/$<CONFIG>
    )

    if((${PROJECT_NAME}_CLANG_TIDY) AND(CLANG_TIDY_EXECUTABLE))
        set_target_properties(${FILENAME}
            PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}"
        )
    endif()
endfunction()

cmake_minimum_required(VERSION 3.15.0)

include(${${PROJECT_NAME}_TEST_DIR}/sources.cmake)

enable_testing()

add_executable_test(${PROJECT_NAME}_unit_test_basic
    ${${PROJECT_NAME}_UNIT_TEST_BASIC_HEADER}
    ${${PROJECT_NAME}_UNIT_TEST_BASIC_SOURCE}
)

add_executable_test(${PROJECT_NAME}_unit_test_file
    ${${PROJECT_NAME}_UNIT_TEST_FILE_HEADER}
    ${${PROJECT_NAME}_UNIT_TEST_FILE_SOURCE}
)

doctest_discover_tests(
    ${PROJECT_NAME}_unit_test_basic
    ${PROJECT_NAME}_unit_test_file
)
