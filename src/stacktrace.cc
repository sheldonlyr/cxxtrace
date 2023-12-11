#include "cxxtrace/cxxtrace.h"

#include <fcntl.h>
#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <ctime>

#include <backtrace-supported.h>
#include <backtrace.h>

#include "demangle.h"
#include "snprintf_ss.h"

typedef struct stacktrace_context
{
    struct backtrace_state* backtrace_st;
    int max_depth;
    int output_fd;
    int backtrace_idx;
} stacktrace_context_t;

//
static stacktrace_context_t* stacktrace_ctx_ = nullptr;

// clang-format off
#define SIGNAMEANDNUM(s)  { #s, s }
// clang-format on

static struct
{
    const char* name;
    int value;
} known_failure_signals[] = {
    SIGNAMEANDNUM(SIGILL),
    SIGNAMEANDNUM(SIGTRAP),
    SIGNAMEANDNUM(SIGABRT),
    SIGNAMEANDNUM(SIGFPE),
    SIGNAMEANDNUM(SIGSEGV),
    SIGNAMEANDNUM(SIGTERM),
};

const char* get_failure_signal_name(int sig)
{
    const char* name = "unknown signal";

    for (int i = 0;
         i < sizeof(known_failure_signals) / sizeof(*known_failure_signals);
         i++) {
        if (sig == known_failure_signals[i].value) {
            name = known_failure_signals[i].name;
            break;
        }
    }

    return name;
}

static ssize_t safe_write(int fd, const void* buf, size_t count)
{
    return write(fd, buf, count);
}

static size_t safe_print_to(int fd, const char* fmt, ...)
{
    char to[4096];
    size_t result;
    va_list args;
    va_start(args, fmt);
    result = safe_vsnprintf(to, sizeof(to), fmt, args);
    va_end(args);
    safe_write(fd, to, result);
    return result;
}

static void backtrace_err_cb(void* data, const char* msg, int errnum)
{
    (void) data;
    safe_print_to(STDERR_FILENO,
                  "error %d occurred while getting the stacktrace: %s\n",
                  errnum,
                  msg);
}

static void backtrace_err_cb_create(void* data, const char* msg, int errnum)
{
    (void) data;
    safe_print_to(STDERR_FILENO,
                  "error %d occurred while initializing the stacktrace: %s\n",
                  errnum,
                  msg);
}

static void stacktrace_print_with_context(stacktrace_context_t* ctx,
                                          const char* filename,
                                          int lineno,
                                          const char* function)
{
    if (!ctx || ctx->output_fd < 0) {
        return;
    }

    if (!filename || !function || (lineno == 0)) {
        return;
    }

    const char* funcname = function;

    char demangled_funcname[1024] = {0};
    if (absl::debugging_internal::Demangle(
            function, demangled_funcname, sizeof(demangled_funcname))) {
        funcname = demangled_funcname;
    }

    safe_print_to(ctx->output_fd,
                  "#%d:  %s  at  %s:%d\n",
                  ctx->backtrace_idx,
                  funcname,
                  filename,
                  lineno);
}

static int backtrace_full_cb(void* data,
                             uintptr_t pc,
                             const char* filename,
                             int lineno,
                             const char* function)
{
    (void) pc;

    if (!data) {
        return -1;
    }

    stacktrace_context_t* ctx = (stacktrace_context_t*) data;
    stacktrace_print_with_context(ctx, filename, lineno, function);
    ctx->backtrace_idx++;

    if (ctx->backtrace_idx > ctx->max_depth) {
        return -1;
    }

    return 0;
}

static void stacktrace_print_to(int sig, int fd)
{
    if (fd < 0) {
        return;
    }

    if (!stacktrace_ctx_->backtrace_st) {
        safe_print_to(STDERR_FILENO,
                      "safe_stacktrace_init() must be called before print or "
                      "dump stacktrace\n");
        abort();
    }

    struct timespec tp;
    if (clock_gettime(CLOCK_REALTIME, &tp) < 0) {
        // Failed to get system clock time
        return;
    }

    // `readlink()` does not append a terminating null byte
    char execpath[1024] = {0};
    int pathlen = readlink("/proc/self/exe", execpath, sizeof(execpath) - 1);
    if (pathlen < 0) {
        // Failed to get executable file path
        return;
    }

    safe_print_to(fd, "\n%s\n", "*************** BACKTRACES: ***************");
    safe_print_to(fd, "EXECUTABLE: %s\n", execpath);
    safe_print_to(fd, "TIMESTAMP:  %ld.%ld\n", tp.tv_sec, tp.tv_nsec);
    safe_print_to(fd, "SIGNAL:     %s\n\n", get_failure_signal_name(sig));

    stacktrace_ctx_->output_fd = fd;
    backtrace_full(stacktrace_ctx_->backtrace_st,
                   0, // Skip `safe_stacktrace_xxx` and `stacktrace_print_to`??
                   backtrace_full_cb,
                   backtrace_err_cb,
                   stacktrace_ctx_);

    stacktrace_ctx_->output_fd     = -1;
    stacktrace_ctx_->backtrace_idx = 0;
}

namespace cxxtrace {
int stacktrace_init(int max_depth)
{
    stacktrace_ctx_ =
        (stacktrace_context_t*) malloc(sizeof(stacktrace_context_t));
    if (!stacktrace_ctx_) {
        return -1;
    }

    stacktrace_ctx_->backtrace_st  = nullptr;
    stacktrace_ctx_->max_depth     = max_depth;
    stacktrace_ctx_->output_fd     = -1;
    stacktrace_ctx_->backtrace_idx = 0;

    stacktrace_ctx_->backtrace_st = backtrace_create_state(
        nullptr, 0, backtrace_err_cb_create, (void*) stacktrace_ctx_);
    if (!stacktrace_ctx_->backtrace_st) {
        free(stacktrace_ctx_);
        return -1;
    }

    return 0;
}

void stacktrace_deinit()
{
    if (stacktrace_ctx_) {
        free(stacktrace_ctx_);
        stacktrace_ctx_ = nullptr;
    }
}

void stacktrace_print(int sig)
{
    stacktrace_print_to(sig, STDERR_FILENO);
}

void stacktrace_dump(int sig, const char* filename)
{
    if (!filename) {
        return;
    }

    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        // Create new file for writing stacktrace failed
        return;
    }

    stacktrace_print_to(sig, fd);

    fsync(fd);
    close(fd);
}
} // namespace cxxtrace
