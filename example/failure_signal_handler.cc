#include "failure_signal_handler.h"

#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>

static bool setup_alternate_stack_once()
{
    const size_t page_mask = static_cast<size_t>(sysconf(_SC_PAGESIZE)) - 1;
    size_t stack_size =
        (std::max(static_cast<size_t>(SIGSTKSZ), size_t{65536}) + page_mask) &
        ~page_mask;

    stack_t sigstk;
    memset(&sigstk, 0, sizeof(sigstk));
    sigstk.ss_size = stack_size;

    sigstk.ss_sp = malloc(sigstk.ss_size);
    if (sigstk.ss_sp == nullptr) {
        // Log `malloc` failed...
        return false;
    }

    if (sigaltstack(&sigstk, nullptr) != 0) {
        // Log `sigaltstack` failed...
        return false;
    }

    return true;
}

int install_failure_signal_handler(int sig,
                                   void (*handler)(int, siginfo_t*, void*))
{
    // Only set up alternate stack once!
    static const bool ok __attribute__((unused)) = setup_alternate_stack_once();

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    sigemptyset(&act.sa_mask);
    act.sa_flags |= SA_SIGINFO;
    act.sa_flags |= SA_NODEFER; // SA_NODEFER is required to handle SIGABRT
    if (ok) {
        act.sa_flags |= SA_ONSTACK;
    }
    act.sa_sigaction = handler;
    if (sigaction(sig, &act, nullptr) < 0) {
        int saved_errno = errno;
        return saved_errno;
    }
    return 0;
}

static const int common_failure_signals[] = {
    SIGILL,
    SIGTRAP,
    SIGABRT,
    SIGFPE,
    SIGSEGV,
    SIGTERM,
};

int install_all_failure_signals_handler(void (*handler)(int, siginfo_t*, void*))
{
    for (int sig: common_failure_signals) {
        int err = install_failure_signal_handler(sig, handler);
        if (err != 0) {
            return err;
        }
    }
    return 0;
}
