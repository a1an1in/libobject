#ifndef __TRY_H__
#define __TRY_H__

#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <setjmp.h>

typedef struct exception_frame_s {
    jmp_buf env;
    int line;
    const char *func;
    const char *file;
    int error_code;
    struct exception_frame_s *prev;
#define EXCEPTION_MESSAGE_LENGTH  512
    char message[EXCEPTION_MESSAGE_LENGTH + 1];
#undef EXCEPTION_MESSAGE_LENGTH
} exception_frame_t;

enum {
    EXCEPTION_ENTERED = 0,
    EXCEPTION_THROWN,
    EXCEPTION_HANDLED,
    EXCEPTION_FINALIZED
};

extern void exception_throw(int error_code, const char *func, const char *file, int line, const char *cause, ...);
extern pthread_key_t try_key;

#define ERROR_MESSAGE() frame.message
#define ERROR_LINE() frame.line
#define ERROR_FILE() extract_filename_in_macro(frame.file)
#define ERROR_FUNC() frame.func
#define ERROR_CODE() frame.error_code

#define THROW(e, cause, ...)  exception_throw(e, __func__, __FILE__, __LINE__, cause, ##__VA_ARGS__, NULL)

#define EXEC(expression)                                                                         \
    do {                                                                                         \
        int ret = 0;                                                                             \
        if ((ret = (expression)) < 0) {                                                          \
            exception_throw(ret, __func__, __FILE__, __LINE__, "");                              \
        }                                                                                        \
    } while (0);

#define EXEC_IF(condition, expression)                                                           \
    do {                                                                                         \
        if (((condition)) == 1) {                                                                \
            int ret = 0;                                                                         \
            if ((ret = (expression)) < 0) {                                                      \
                exception_throw(ret, __func__, __FILE__, __LINE__, "");                          \
            }                                                                                    \
        }                                                                                        \
    } while (0);

#define THROW_IF(condition, error_code)                                                          \
    do {                                                                                         \
        if (((condition)) == 1) {                                                                \
            exception_throw(error_code, __func__, __FILE__, __LINE__, #condition);               \
        }                                                                                        \
    } while (0);

#define CONTINUE_IF(expression)                                                                  \
    if ((expression) == 1) {                                                                     \
        continue;                                                                                \
    }

#define RETURN_IF(expression, return_value)                                                      \
    if ((expression) == 1) {                                                                     \
        return return_value;                                                                     \
    }

#define TRY                                                                                      \
    do {                                                                                         \
        volatile int exception_flag;                                                             \
        exception_frame_t frame;                                                                 \
        frame.message[0] = 0;                                                                    \
        frame.prev = (exception_frame_t*)pthread_getspecific(try_key);                           \
        pthread_setspecific(try_key, &frame);                                                    \
        exception_flag = setjmp(frame.env);                                                      \
        if (exception_flag == EXCEPTION_ENTERED) {

#define CATCH                                                                                    \
        } else if (frame.error_code < 0) {                                                       \
            exception_flag = EXCEPTION_HANDLED;                                                  \
            dbg_str(DBG_ERROR, "error_func:%s, error_file: %s, error_line:%d, error_code:%d",    \
                    ERROR_FUNC(), ERROR_FILE(), ERROR_LINE(), ERROR_CODE());

#define FINALLY                                                                                  \
        }                                                                                        \
        {                                                                                        \

#define ENDTRY                                                                                   \
        }                                                                                        \
        if (exception_flag == EXCEPTION_ENTERED) {                                               \
            pthread_setspecific(try_key,                                                         \
                    ((exception_frame_t*)pthread_getspecific(try_key))->prev);                   \
            exception_flag = EXCEPTION_HANDLED;                                                  \
        }                                                                                        \
        if (exception_flag == EXCEPTION_THROWN)  {                                               \
            exception_throw(frame.error_code, frame.func, frame.file, frame.line, frame.message);\
        }                                                                                        \
    } while (0)

#endif
