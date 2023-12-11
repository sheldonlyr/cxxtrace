include(ExternalProject)

set(BACKTRACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libbacktrace)
set(BACKTRACE_BIN ${CMAKE_CURRENT_BINARY_DIR}/libbacktrace)
set(BACKTRACE_STATIC_LIB ${BACKTRACE_BIN}/lib/libbacktrace.a)
set(BACKTRACE_INCLUDES ${BACKTRACE_BIN}/include)

file(MAKE_DIRECTORY ${BACKTRACE_INCLUDES})

ExternalProject_Add(libbacktrace
    PREFIX ${BACKTRACE_BIN}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libbacktrace
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libbacktrace/configure --prefix=${BACKTRACE_BIN}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS ${BACKTRACE_STATIC_LIB}
)

add_library(backtrace-static STATIC IMPORTED GLOBAL)
add_dependencies(backtrace-static libbacktrace)

set_target_properties(backtrace-static PROPERTIES IMPORTED_LOCATION ${BACKTRACE_STATIC_LIB})
set_target_properties(backtrace-static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${BACKTRACE_INCLUDES})
