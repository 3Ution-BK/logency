if(${PROJECT_NAME}_BUILD_TEST)
    set(${PROJECT_NAME}_DOCTEST_INCLUDE_DIR doctest CACHE INTERNAL "Path to include folder for doctest")
    add_subdirectory(${${PROJECT_NAME}_DOCTEST_INCLUDE_DIR} EXCLUDE_FROM_ALL)
    include(${${PROJECT_NAME}_DOCTEST_INCLUDE_DIR}/scripts/cmake/doctest.cmake)
endif()

if (${${PROJECT_NAME}_LIBRARY_FMT})
    add_subdirectory(fmt EXCLUDE_FROM_ALL)
endif()
