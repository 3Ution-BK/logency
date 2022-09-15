# CMP0092: MSVC warning flags.
# Do not added default warning flags.
if (POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
else()
    if (MSVC)
        # CMake enforce /W3 by default. This line tend to delete the regulation.
        string (REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
    endif()
endif()
