cmake_minimum_required(VERSION 3.16)

# Determine if fmt is built as a subproject (using add_subdirectory)
# or if it is the master project.
if(NOT DEFINED CXXTRACE_MASTER_PROJECT)
    set(CXXTRACE_MASTER_PROJECT OFF)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(CXXTRACE_MASTER_PROJECT ON)
        message(STATUS "CMake version: ${CMAKE_VERSION}")
    endif()
endif()

# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (CXXTRACE_MASTER_PROJECT AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or "
        "CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
endif()

project(cxxtrace VERSION 1.0.0 LANGUAGES CXX)

# Options that control generation of various targets.
option(CXXTRACE_EXAMPLE "Generate the example target." ${CXXTRACE_MASTER_PROJECT})

# libbacktrace
include(cmake/backtrace.cmake)

add_library(cxxtrace STATIC
    src/snprintf_ss.cc
    src/stacktrace.cc
    src/demangle.cc
)

set_target_properties(cxxtrace
    PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS ON
        CXX_STANDARD_REQUIRED ON
)

# sets the search paths for the include files after installation
# as well as during when building the library (as these may differ)
# this allows the library itself and users to #include the library headers
target_include_directories(cxxtrace
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(cxxtrace
    PRIVATE
        backtrace-static
)

if(CXXTRACE_EXAMPLE)
    add_subdirectory(example)
endif()
