#include <stdio.h>
#include <unistd.h>

#include <cxxtrace/cxxtrace.h>

#include "failure_signal_handler.h"

void failure_signal_handler(int sig, siginfo_t* info, void* context)
{
    (void) info;
    (void) context;

    cxxtrace::stacktrace_print(sig);
    // cxxtrace::stacktrace_dump(sig, "backtrace.dump");
    _exit(1);
}

void function_that_crashes()
{
    printf("demo programe crash here...\n");
    *((void**) 0) = 0; // program crashes here
}

void function_that_normal()
{
    printf("call function_that_normal()\n");
    function_that_crashes();
}

// unsigned infinite_recursion(unsigned x)
// {
//     return infinite_recursion(x) + 1;
// }

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    // Setup common failure signal handling
    install_all_failure_signals_handler(failure_signal_handler);

    // Initialize stacktrace lib
    cxxtrace::stacktrace_init(255);

    // infinite_recursion(0);
    function_that_normal();

    // Deinitialize stacktrace lib
    cxxtrace::stacktrace_deinit();
    return 0;
}
