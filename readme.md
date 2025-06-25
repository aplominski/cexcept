# cexcept

Minimalistic header-only exception handling library for C using `setjmp`/`longjmp`.  
Provides macros for structured error handling: `TRY`, `CATCH`, `CATCH_ALL`, and `THROW`.

Supports thread-local exception contexts for safe multithreaded usage.  
Automatically logs uncaught exceptions with timestamps, process and thread IDs, and stack traces.

---

## Features

- Lightweight, header-only — easy integration
- Thread-local storage for exception context
- Simple and clear API macros for exception handling
- Detailed logging with stack trace on uncaught exceptions
- No dependencies outside standard C and GNU/Linux backtrace facilities

---