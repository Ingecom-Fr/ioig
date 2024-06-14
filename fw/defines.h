#pragma once

//--------------------------------------------------------------
// Macros
//--------------------------------------------------------------

#define IOIG_DEBUG_MODE 1
#define IOIG_DBG_FN 0

#define CHECK_STATE()                   \
    {                                   \
        Task::State st = getState();    \
        if (st != Task::State::RUNNING) \
        {                               \
            return;                     \
        }                               \
    }

#if IOIG_DEBUG_MODE
#define DBG_MSG(...)         \
    do                       \
    {                        \
        printf(__VA_ARGS__); \
    } while (0)

#define DBG_MSG_T(interval_ms, ...)                     \
    {                                                   \
        static uint32_t start_ms = 0;                   \
                                                        \
        if ((board_millis() - start_ms) >= interval_ms) \
        {                                               \
            printf(__VA_ARGS__);                        \
            fflush(stdout);                             \
            start_ms += interval_ms;                    \
        }                                               \
    }

#define DBG_CALLBACK_T(interval_ms, cbk)                \
    {                                                   \
        static uint32_t start_ms = 0;                   \
                                                        \
        if ((board_millis() - start_ms) >= interval_ms) \
        {                                               \
            cbk;                                        \
            start_ms += interval_ms;                    \
        }                                               \
    }

#else
#define DBG_MSG(...) do { } while (0)
#define DBG_MSG_T(interval_ms, ...) do { } while (0)
#define DBG_CALLBACK_T(interval_ms, cbk) do { } while (0)
#endif

