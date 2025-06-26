#ifndef CEXCEPT_LIBRARY_H
#define CEXCEPT_LIBRARY_H

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <string.h>
#include <execinfo.h>   // backtrace, backtrace_symbols
#include <time.h>       // time, localtime, strftime
#include <unistd.h>     // getpid
#include <pthread.h>    // pthread_self
#include <signal.h>

typedef enum {
    EXC_INVALID_ARGUMENT,
    EXC_OUT_OF_RANGE,
    EXC_LENGTH_ERROR,
    EXC_DOMAIN_ERROR,
    EXC_LOGIC_ERROR,
    EXC_RUNTIME_ERROR,
    EXC_OVERFLOW_ERROR,
    EXC_UNDERFLOW_ERROR,
    EXC_RANGE_ERROR,
    EXC_UNKNOWN_ERROR,

    SYS_SEGFAULT
} exception_t;

typedef struct {
    jmp_buf env;
    exception_t current;
    int active;
} exception_context_t;

// Thread-local, żeby mieć kontekst per wątek i uniknąć link errors
#ifdef __GNUC__
static __thread exception_context_t* current_context = NULL;
#else
static _Thread_local exception_context_t* current_context = NULL;
#endif

static inline const char* exception_to_string(exception_t e) {
    switch (e) {
        case EXC_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case EXC_OUT_OF_RANGE: return "OUT_OF_RANGE";
        case EXC_LENGTH_ERROR: return "LENGTH_ERROR";
        case EXC_DOMAIN_ERROR: return "DOMAIN_ERROR";
        case EXC_LOGIC_ERROR: return "LOGIC_ERROR";
        case EXC_RUNTIME_ERROR: return "RUNTIME_ERROR";
        case EXC_OVERFLOW_ERROR: return "OVERFLOW_ERROR";
        case EXC_UNDERFLOW_ERROR: return "UNDERFLOW_ERROR";
        case EXC_RANGE_ERROR: return "RANGE_ERROR";
        case EXC_UNKNOWN_ERROR: return "UNKNOWN_ERROR";

        case SYS_SEGFAULT: return "SYS_SEGFAULT";

        default: return "UNDEFINED_EXCEPTION";
    }
}

#define TRY \
    for (exception_context_t ctx = {0}, *prev_context = current_context; \
    (current_context = &ctx, ctx.active = 1, setjmp(ctx.env) == 0); \
    current_context = prev_context)

#define CATCH(e) \
    if (current_context->current == (e))

#define CATCH_ALL \
    else

#define _GET_MACRO(_1, _2, NAME, ...) NAME

#define THROW(...) _GET_MACRO(__VA_ARGS__, THROW2, THROW1)(__VA_ARGS__)

#define THROW1(e) do { \
    if (current_context && current_context->active) { \
        current_context->current = (e); \
        longjmp(current_context->env, 1); \
    } else { \
        time_t now = time(NULL); \
        char timebuf[64]; \
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
        pid_t pid = getpid(); \
        pthread_t tid = pthread_self(); \
        void* array[64]; \
        int size = backtrace(array, 64); \
        char** symbols = backtrace_symbols(array, size); \
        fprintf(stderr, "\033[1;31m"); \
        fprintf(stderr, "==================== EXCEPTION THROW ====================\n"); \
        fprintf(stderr, "Time: %s\n", timebuf); \
        fprintf(stderr, "Process ID: %d, Thread ID: %lu\n", pid, (unsigned long)tid); \
        fprintf(stderr, "Exception: %s\n", exception_to_string(e)); \
        fprintf(stderr, "Build informations: %s %s\n", __DATE__, __TIME__); \
        fprintf(stderr, "Compiler data: %s %d.%d\n", __VERSION__, __GNUC__, __GNUC_MINOR__); \
        fprintf(stderr, "Stack trace (most recent call first):\n"); \
        for (int i = 0; i < size; ++i) { \
            fprintf(stderr, "  %s\n", symbols[i]); \
        } \
        fprintf(stderr, "==================== END EXCEPTION ======================\n"); \
        fprintf(stderr, "\033[0m"); \
        free(symbols); \
        abort(); \
    } \
} while (0)

#define THROW2(e, msg) do { \
    if (current_context && current_context->active) { \
        current_context->current = (e); \
        longjmp(current_context->env, 1); \
    } else { \
        time_t now = time(NULL); \
        char timebuf[64]; \
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
        pid_t pid = getpid(); \
        pthread_t tid = pthread_self(); \
        void* array[64]; \
        int size = backtrace(array, 64); \
        char** symbols = backtrace_symbols(array, size); \
        fprintf(stderr, "\033[1;31m"); \
        fprintf(stderr, "==================== EXCEPTION THROW ====================\n"); \
        fprintf(stderr, "Time: %s\n", timebuf); \
        fprintf(stderr, "Process ID: %d, Thread ID: %lu\n", pid, (unsigned long)tid); \
        fprintf(stderr, "Exception: %s\n", exception_to_string(e)); \
        fprintf(stderr, "Description: %s\n", msg); \
        fprintf(stderr, "Build informations: %s %s\n", __DATE__, __TIME__); \
        fprintf(stderr, "Compiler data: %s %d.%d\n", __VERSION__, __GNUC__, __GNUC_MINOR__); \
        fprintf(stderr, "Stack trace (most recent call first):\n"); \
        for (int i = 0; i < size; ++i) { \
            fprintf(stderr, "  %s\n", symbols[i]); \
        } \
        fprintf(stderr, "==================== END EXCEPTION ======================\n"); \
        fprintf(stderr, "\033[0m"); \
        free(symbols); \
        abort(); \
    } \
} while (0)

static inline void segfault_handler(int signum, siginfo_t *info, void *context) {
    (void)context;
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
            "Fatal error: signal %d (SIGSEGV) received. The program attempted to access memory it shouldn't. Check for invalid pointers or buffer overflows. Consider running under a debugger for stack trace.",
            signum, info->si_addr);
    THROW(SYS_SEGFAULT, buffer);
}

static inline void init_handlers() {
    struct sigaction sa;
    sa.sa_sigaction = segfault_handler;
    sa.sa_flags = SA_SIGINFO;

    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        return;
    }
}

#endif // CEXCEPT_LIBRARY_H
