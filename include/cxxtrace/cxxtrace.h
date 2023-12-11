#ifndef _CXX_TRACE_H_INC_
#define _CXX_TRACE_H_INC_

namespace cxxtrace {
/**
 * Initializes some background context for executing stacktrace routines
 *
 * @note You should call this routine in your `main` function as soone as
 * possible, and call only once!
 *
 * @return int On success, 0 is returned. On error, -1 is returned.
 */
int stacktrace_init(int max_depth);

/**
 * Deinitializes background context
 *
 * @note You should call this routine before you are ready to exit running
 * program
 */
void stacktrace_deinit();

/**
 * Print stacktrace to STDERR in async-signal-safety mode
 *
 * @note Most of the time, call this in your signal handler for catching common
 * failure signals like `SIGSEGV`, `SIGILL`, `SIGFPE`, `SIGABRT`, `SIGTERM`,
 * `SIGBUG`, and `SIGTRAP`
 */
void stacktrace_print(int signo);

/**
 * Dump stacktrace to FILE in async-signal-safety mode
 *
 * @note Most of the time, call this in your signal handler for catching common
 * failure signals like `SIGSEGV`, `SIGILL`, `SIGFPE`, `SIGABRT`, `SIGTERM`,
 * `SIGBUG`, and `SIGTRAP`
 */
void stacktrace_dump(int signo, const char* filename);
} // namespace cxxtrace

#endif
