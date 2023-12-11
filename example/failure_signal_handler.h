#ifndef _FAILURE_SIGNAL_HANDLER_H_INC_
#define _FAILURE_SIGNAL_HANDLER_H_INC_

#include <signal.h>

// install_failure_signal_handler()
//
// Installs a signal handler for the common failure signals `SIGSEGV`, `SIGILL`,
// `SIGFPE`, `SIGABRT`, `SIGTERM`, `SIGBUG`, and `SIGTRAP` (provided they exist
// on the given platform). The failure signal handler SHOULD dumps program
// failure data useful for debugging to stderr or to FILE.
int install_failure_signal_handler(int sig,
                                   void (*handler)(int, siginfo_t*, void*));

// install_all_failure_signal_handler()
//
// @note See `install_failure_signal_handler` and `common_failure_signals`
int install_all_failure_signals_handler(void (*handler)(int, siginfo_t*, void*));

#endif
