#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/regisTRY/regisTRY.h>
#include <libobject/core/utils/data_structure/array_stack.h>
#include <libobject/core/utils/data_structure/vector.h>

pthread_key_t try_key;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void init_once(void) {
    pthread_key_create(&try_key, NULL);
}

void exception_init(void) {
    pthread_once(&once_control, init_once);
}

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

#define THROW(e, cause, ...)  exception_throw(e, __func__, __FILE__, __LINE__, cause, ##__VA_ARGS__, NULL)

#define TRY                                                                                           \
            do {                                                                                      \
                volatile int exception_flag;                                                          \
                exception_frame_t frame;                                                              \
                frame.message[0] = 0;                                                                 \
                frame.prev = (exception_frame_t*)pthread_getspecific(try_key);                        \
                pthread_setspecific(try_key, &frame);                                                 \
                exception_flag = setjmp(frame.env);                                                   \
                if (exception_flag == EXCEPTION_ENTERED) {


#define CATCH_EQ(e)                                                                                   \
                    if (exception_flag == EXCEPTION_ENTERED) {                                        \
                        pthread_setspecific(try_key,                                                  \
                                            ((exception_frame_t*)pthread_getspecific(try_key))->prev);\
                     }                                                                                \
                } else if (frame.error_code == e) {                                                   \
                    exception_flag = EXCEPTION_HANDLED;


#define CATCH(e)                                                                                \
                    if (exception_flag == EXCEPTION_ENTERED) {                                        \
                        pthread_setspecific(try_key,                                                  \
                                            ((exception_frame_t*)pthread_getspecific(try_key))->prev);\
                     }                                                                                \
                } else if (frame.error_code < e){                                                     \
                    exception_flag = EXCEPTION_HANDLED;                                               \
                    e = frame.error_code;

#define FINALLY                                                                                       \
                    if (exception_flag == EXCEPTION_ENTERED) {                                        \
                        pthread_setspecific(try_key,                                                  \
                                            ((exception_frame_t*)pthread_getspecific(try_key))->prev);\
                     }                                                                                \
                }                                                                                     \
                {                                                                                     \
                    if (exception_flag == EXCEPTION_ENTERED)                                          \
                        exception_flag = EXCEPTION_FINALIZED;

#define ENDTRY                                                                                        \
                    if (exception_flag == EXCEPTION_ENTERED) {                                        \
                        pthread_setspecific(try_key,                                                  \
                                            ((exception_frame_t*)pthread_getspecific(try_key))->prev);\
                     }                                                                                \
                }                                                                                     \
                if (exception_flag == EXCEPTION_THROWN)  {                                            \
                    exception_throw(frame.error_code, frame.func, frame.file, frame.line, NULL);      \
                }                                                                                     \
            } while (0)


void exception_throw(int error_code, const char *func, const char *file, int line, const char *cause, ...)
{
    va_list ap;
    exception_frame_t *frame = (exception_frame_t *) pthread_getspecific(try_key);

    if (frame) {
        frame->error_code = error_code;
        frame->func = func;
        frame->file = file;
        frame->line = line;

        if (cause) {
            va_start(ap, cause);
            vsnprintf(frame->message, sizeof(frame->message), cause, ap);
            va_end(ap);
        }

        pthread_setspecific(try_key,
                            ((exception_frame_t *) pthread_getspecific(try_key))->prev);

        longjmp(frame->env, EXCEPTION_THROWN);

    }
}

static int test_try_catch3(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    exception_init();
    int ret;

#if 1
    TRY {
        TRY {
            THROW(-1, "recall B");
        } CATCH (ret) {
            printf("CATCH %d at level2\n", ret);
        }
        ENDTRY;
        printf("throw -2\n");
        THROW(-2, NULL);
    } CATCH_EQ (-1) {
        printf("CATCH -1 at level1\n");
    } CATCH (ret) {
        printf("CATCH %d at level1\n", ret);
    }
    ENDTRY;
#endif
}
REGISTER_TEST_FUNC(test_try_catch3);


void *thread(void *args) {

    pthread_t selfid = pthread_self();

    TRY {
        THROW (-1, "throw -1");
    } CATCH_EQ (-1) {
        printf("CATCH -1 : %ld\n", selfid);
    }
    ENDTRY;

    TRY {
        THROW(-2, "throw -2");
    } CATCH_EQ (-2){
        printf("CATCH -2 : %ld\n", selfid);
    }
    ENDTRY;

    TRY {
        THROW(-3, "throw -3");
    } CATCH_EQ (-3){
        printf("CATCH -3 : %ld\n", selfid);
    }
    ENDTRY;

    TRY {
        THROW(-4, "throw -4");
    } CATCH_EQ (-4){
        printf("CATCH -4 : %ld\n", selfid);
    }
    ENDTRY;

    TRY {
        THROW(-1, "-1 Again");
        THROW(-2, "-2 Again");
        THROW(-3, "-3 Again");
        THROW(-4, "-4 Again");
    } CATCH_EQ (-1){
        printf("CATCH -1 again : %ld\n", selfid);
    } CATCH_EQ (-2){
        printf("CATCH -2 again : %ld\n", selfid);
    } CATCH_EQ (-3){
        printf("CATCH -3 again : %ld\n", selfid);
    } CATCH_EQ (-4){
        printf("CATCH -4 again : %ld\n", selfid);
    } FINALLY {
        printf("finaly: %ld\n", selfid);
    };
    ENDTRY;

}

#define THREADS  50
static int test_try_catch4(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    exception_init();

    int i = 0;
    pthread_t threads[THREADS];

    for (i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, thread, NULL);
    }

    for (i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

}
REGISTER_TEST_FUNC(test_try_catch4);

static try_catch_test_fuc()
{
    TRY {
        THROW(-5, "recall B");
    }
    ENDTRY;
}
static int test_try_catch5(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    int ret = -2;

    exception_init();

#if 1
    TRY {
        try_catch_test_fuc();
    } CATCH (ret) {
        printf("CATCH error code, ret =%d\n", ret);
    } FINALLY {
        printf("run at finally\n");
    };
    ENDTRY;
#endif
}
REGISTER_TEST_FUNC(test_try_catch5);
