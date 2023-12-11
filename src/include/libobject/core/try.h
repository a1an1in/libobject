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

/*
 *#define USE_JMP_TRY_CATCH
 */
#ifdef USE_JMP_TRY_CATCH

#define ERROR_MESSAGE() __exception_frame.message
#define ERROR_LINE() __exception_frame.line
#define ERROR_FILE() extract_filename_from_path(__exception_frame.file)
#define ERROR_FUNC() __exception_frame.func
#define ERROR_CODE() __exception_frame.error_code

#define THROW(e)  exception_throw(e, __func__, __FILE__, __LINE__, NULL)

#define EXEC(expression)                                                                         \
    do {                                                                                         \
        int __ret__ = 0;                                                                         \
        if ((__ret__ = (int)(expression)) < 0) {                                                 \
            exception_throw(__ret__, __func__, __FILE__, __LINE__, "");                          \
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


#define TRY                                                                                      \
    volatile int __exception_flag;                                                               \
    exception_frame_t __exception_frame;                                                         \
    __exception_frame.message[0] = 0;                                                            \
    __exception_frame.prev = (exception_frame_t*)pthread_getspecific(try_key);                   \
    pthread_setspecific(try_key, &__exception_frame);                                            \
    __exception_flag = setjmp(__exception_frame.env);                                            \
    if (__exception_flag == EXCEPTION_ENTERED) {

#define CATCH(ret)                                                                               \
        pthread_setspecific(try_key,                                                             \
                            ((exception_frame_t*)pthread_getspecific(try_key))->prev);           \
        __exception_flag = EXCEPTION_HANDLED;                                                    \
        ret = __exception_frame.error_code;                                                      \
    }                                                                                            \
    if (__exception_flag == EXCEPTION_THROWN)                                                    \

#define FINALLY                                                                                  \

#else

#define ERROR_LINE()     __error_line
#define ERROR_FUNC()     __error_func
#define ERROR_CODE()     __error_code
#define ERROR_FILE()     __error_file
#define ERROR_INT_PAR1() __error_int_par1
#define ERROR_INT_PAR2() __error_int_par2
#define ERROR_PTR_PAR1() __error_ptr_par1
#define ERROR_PTR_PAR2() __error_ptr_par2

#define TRY_SHOW_STR_PARS(level)                                                                 \
    dbg_str(level, "ERROR_FUNC:%s, ERROR_PTR_PAR1=%s, ERROR_PTR_PAR2=%s",                        \
            ERROR_FUNC(), ERROR_PTR_PAR1(), ERROR_PTR_PAR2() == NULL ? "" : ERROR_PTR_PAR2());

#define TRY_SHOW_INT_PARS(level)                                                                 \
    dbg_str(level, "ERROR_FUNC:%s, ERROR_INT_PAR1=%d, ERROR_INT_PAR2=%d",                        \
            __func__, ERROR_INT_PAR1(), ERROR_INT_PAR2());

#define TRY_SHOW_ERROR_PARS()                                                                    \
        dbg_str(DBG_ERROR, "ERROR_FUNC:%s, ERROR_LINE:%d, ERROR_CODE:%d",                        \
                __func__, ERROR_LINE(), ERROR_CODE());                                           \

#define THROW(error_code)                                                                        \
    do {                                                                                         \
        __error_line = __LINE__;                                                                 \
        __error_func = (char *)__func__;                                                         \
        __error_file = (char *)extract_filename_from_path(__FILE__);                              \
        __error_code = error_code;                                                               \
        goto __error_tab;                                                                        \
    } while (0);

#define EXEC(expression)                                                                         \
    do {                                                                                         \
        int __ret__ = 0;                                                                         \
        if ((__ret__ = (int)(expression)) < 0) {                                                 \
            THROW(-1);                                                                           \
        }                                                                                        \
    } while (0);

#define EXEC_IF(condition, expression)                                                           \
    do {                                                                                         \
        if (((condition)) == 1) {                                                                \
            int __ret = 0;                                                                       \
            if ((__ret = (expression)) < 0) {                                                    \
                THROW(__ret);                                                                    \
            }                                                                                    \
        }                                                                                        \
    } while (0);

#define THROW_IF(condition, error_code)                                                          \
    do {                                                                                         \
        if (((int)((condition))) == 1) {                                                         \
            THROW(error_code)                                                                    \
        }                                                                                        \
    } while (0);

#define SET_CATCH_INT_PARS(par1, par2)                                                           \
    do {                                                                                         \
        __error_int_par1 = (par1);                                                               \
        __error_int_par2 = (par2);                                                               \
    } while (0);

#define SET_CATCH_STR_PARS(par1, par2)                                                           \
    do {                                                                                         \
        __error_ptr_par1 = (par1);                                                               \
        __error_ptr_par2 = (par2);                                                               \
    } while (0);

#define TRY                                                                                      \
    int __error_line;                                                                            \
    int __error_code = 1;                                                                        \
    char *__error_func;                                                                          \
    char *__error_file __attribute__((unused));                                                  \
    int __error_int_par1 = 0;                                                                    \
    int __error_int_par2 = 0;                                                                    \
    void *__error_ptr_par1 = NULL;                                                               \
    void *__error_ptr_par2 = NULL;                                                               \

#define CATCH(ret)                                                                               \
    __error_tab:                                                                                 \
    ret = __error_code;                                                                          \
    if (__error_code < 0) {                                                                      \
        TRY_SHOW_ERROR_PARS();                                                                   \
    }                                                                                            \
    if (__error_code < 0)                                                                        \

#define FINALLY                                                                                  \


#define CONTINUE_IF(expression)                                                                  \
    if ((int)(expression) == 1) {                                                                \
        continue;                                                                                \
    }

#define RETURN_IF(condition, return_value)                                                       \
    if ((condition) == 1) {                                                                      \
        return return_value;                                                                     \
    }

#define CONTINUE_IF2(func, outpar_condition) {                                                   \
    (func);                                                                                      \
    if (outpar_condition) {                                                                      \
        continue;                                                                                \
    }                                                                                            \
}

#define TRY_EXEC(expression) ({                                                                  \
    int __ret__ = 0;                                                                             \
    if ((__ret__ = (int)(expression)) < 0) {                                                     \
        dbg_str(DBG_ERROR, "ERROR_FUNC:%s, ERROR_LINE=%d, ERROR_CODE=%d",                        \
                __func__, __LINE__, __ret__);                                                    \
        return __ret__;                                                                          \
    }                                                                                            \
    (__ret__);                                                                                   \
});

#endif //end of USE_JMP_TRY_CATCH

#endif
