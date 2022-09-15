if(MSVC)
    set(${PROJECT_NAME}_CXX_FLAGS_DEBUG
        "/W4"
        "/WX"
        "/wd4774" # format string expected in argument is not a string literal
        "/wd4514" # unreferenced inline function
        "/wd4820" # byte padding
        "/wd5045" # Spectre mitigation (desable bacause it makes too much false positive for now.)
    )

    if(NOT(MSVC_VERSION LESS 1910)) # MSVC v14.1 (Visual Studio 2017 15.0)
        list(APPEND ${PROJECT_NAME}_CXX_FLAGS_DEBUG "/permissive-")
    endif()

    set(${PROJECT_NAME}_CXX_FLAGS_RELEASE
        "/W1"
        "/O2"
    )

    set(${PROJECT_NAME}_CXX_FLAGS
        "/EHsc" # This is default. But clang-tidy want it explicitly.
        "/utf-8"
    )
else()
    set(${PROJECT_NAME}_CXX_FLAGS_DEBUG
        "-Og"
        "-fno-common"
        "-pedantic"
        "-pedantic-errors"
        "-W"
        "-Wall"
        "-Wconversion"
        "-Wdouble-promotion"
        "-Werror"
        "-Wextra"
        "-Wfloat-equal"
        "-Wformat=2"
        "-Winline"
        "-Wno-long-long"
        "-Wshadow"
        "-Wsign-promo"
        "-Wundef"
    )

    set(${PROJECT_NAME}_CXX_FLAGS_RELEASE "-O3")

    set(${PROJECT_NAME}_CXX_FLAGS
    )
endif()
