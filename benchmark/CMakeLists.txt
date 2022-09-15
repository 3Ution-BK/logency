function(add_benchmark FILENAME)
    add_executable(${FILENAME} ${ARGN})

    target_include_directories(${FILENAME}
        PRIVATE
        ${${PROJECT_NAME}_BENCHMARK_DIR}
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
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark/lib/$<CONFIG>
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark/lib/$<CONFIG>
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark/bin/$<CONFIG>
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark/bin/$<CONFIG>
    )

    if((${PROJECT_NAME}_CLANG_TIDY) AND(CLANG_TIDY_EXECUTABLE))
        set_target_properties(${FILENAME}
            PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}"
        )
    endif()
endfunction()

add_benchmark(stream_bench ${${PROJECT_NAME}_BENCHMARK_DIR}/stream_bench.cpp)

if(${PROJECT_NAME}_LIBRARY_FMT)
    add_benchmark(fmt_bench ${${PROJECT_NAME}_BENCHMARK_DIR}/fmt_bench.cpp)
    target_link_libraries(fmt_bench PRIVATE fmt)
endif()
