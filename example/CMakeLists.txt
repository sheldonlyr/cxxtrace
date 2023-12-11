project(cxxtrace_example VERSION 1.0.0 LANGUAGES CXX)

add_executable(cxxtrace_example
    main.cc
    failure_signal_handler.cc
)

set_target_properties(cxxtrace_example
    PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS ON
        CXX_STANDARD_REQUIRED ON

        ENABLE_EXPORTS ON
)
target_compile_options(cxxtrace_example
    PRIVATE
        -Wall -Wextra -Wpedantic
)
target_compile_definitions(cxxtrace_example
    PRIVATE
)
target_link_options(cxxtrace_example
    PRIVATE
)
target_include_directories(cxxtrace_example
    PRIVATE
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

target_link_libraries(cxxtrace_example
    PUBLIC
        cxxtrace
        dl
)
