cmake_minimum_required(VERSION 3.15.0)

project("logency"
    VERSION 1.0.0
    DESCRIPTION "Logger agency.")

set(${PROJECT_NAME}_VERSION ${PROJECT_VERSION})
set(${PROJECT_NAME}_SOVERSION ${PROJECT_VERSION_MAJOR})

set(${PROJECT_NAME}_BENCHMARK_DIR "${CMAKE_SOURCE_DIR}/benchmark")
set(${PROJECT_NAME}_BENCHMARK_DIR "${CMAKE_SOURCE_DIR}/benchmark")
set(${PROJECT_NAME}_MODULE_DIR "${CMAKE_SOURCE_DIR}/cmake")
set(${PROJECT_NAME}_EXAMPLE_DIR "${CMAKE_SOURCE_DIR}/example")
set(${PROJECT_NAME}_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(${PROJECT_NAME}_THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/thirdparty")
set(${PROJECT_NAME}_TEST_DIR "${CMAKE_SOURCE_DIR}/test")

include(${${PROJECT_NAME}_MODULE_DIR}/cmake_policy.cmake)
include(${${PROJECT_NAME}_MODULE_DIR}/compiler_options.cmake)

option(${PROJECT_NAME}_BUILD_BENCHMARK "Enable to build benchmark executable." ON)
option(${PROJECT_NAME}_BUILD_EXAMPLE "Enable to build example executable." ON)
option(${PROJECT_NAME}_BUILD_TEST "Enable to build unit test executable." ON)

option(${PROJECT_NAME}_LIBRARY_FMT "Enable to include library {fmt}" ON)

option(${PROJECT_NAME}_CLANG_TIDY "Enable clang-tidy check for this library. Useful for developing this library." OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable to export compile commands." FORCE)

if(${PROJECT_NAME}_CLANG_TIDY)
    include(${${PROJECT_NAME}_MODULE_DIR}/clang_tidy.cmake)
endif()

set(${PROJECT_NAME}_LIBRARY_NAME ${PROJECT_NAME})

add_subdirectory(${${PROJECT_NAME}_THIRDPARTY_DIR})

add_library(${${PROJECT_NAME}_LIBRARY_NAME} INTERFACE)

target_include_directories(${${PROJECT_NAME}_LIBRARY_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${${PROJECT_NAME}_INCLUDE_DIR}>
)

target_compile_features(${${PROJECT_NAME}_LIBRARY_NAME}
    INTERFACE
    cxx_std_17
)

target_compile_options(${${PROJECT_NAME}_LIBRARY_NAME}
    INTERFACE
    "${${PROJECT_NAME}_CXX_FLAGS}"
    "$<$<CONFIG:DEBUG>:${${PROJECT_NAME}_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:RELEASE>:${${PROJECT_NAME}_CXX_FLAGS_RELEASE}>"
)

set_target_properties(${${PROJECT_NAME}_LIBRARY_NAME}
    PROPERTIES
    VERSION ${${PROJECT_NAME}_VERSION}
    SOVERSION ${${PROJECT_NAME}_SOVERSION}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
)

if((${PROJECT_NAME}_CLANG_TIDY) AND(CLANG_TIDY_EXECUTABLE))
    set_target_properties(${${PROJECT_NAME}_LIBRARY_NAME}
        PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}"
    )
endif()

if(${PROJECT_NAME}_BUILD_TEST)
    add_subdirectory(${${PROJECT_NAME}_TEST_DIR})
endif()

if(${PROJECT_NAME}_BUILD_BENCHMARK)
    add_subdirectory(${${PROJECT_NAME}_BENCHMARK_DIR})
endif()

if(${PROJECT_NAME}_BUILD_EXAMPLE)
    add_subdirectory(${${PROJECT_NAME}_EXAMPLE_DIR})
endif()
