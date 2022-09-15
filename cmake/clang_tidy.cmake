if ((CMAKE_GENERATOR MATCHES "Makefiles") OR
    (CMAKE_GENERATOR MATCHES "Watcom WMake") OR
    (CMAKE_GENERATOR MATCHES "Ninja"))
    set(CLANG_TIDY_CONFIG_SUPPORT_FLAG ON CACHE INTERNAL)
endif()

if (NOT CLANG_TIDY_CONFIG_SUPPORT_FLAG)
    message(WARNING
        "clang-tidy might not supported in configuration process.\n"
        "This configuration use <LANG>_CLANG_TIDY to include clang-tidy.\n"
        "Unfortunately, your generator \"${CMAKE_GENERATOR}\" might not support the feature to add it in configure process.\n"
        "If your conpiler support clang-tidy on build, you may have to exclusively add it after configuration process.\n"
        "This warning is triggered because the generator doesn't satisfy the requested generator CMake provide.\n"
        "We still try to proceed the operation you requested.\n"
        "For more infomation, check out: https://cmake.org/cmake/help/git-master/prop_tgt/LANG_CLANG_TIDY.html \n"
    )
endif()

find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)

if (CLANG_TIDY_EXECUTABLE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
else()
    message(WARNING "${PROJECT_NAME}_CLANG_TIDY is ON but we cannot found clang-tidy.")
endif()
