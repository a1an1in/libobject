#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/data_structure/array_stack.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/try.h>

pthread_key_t try_key;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void init_once(void) {
    pthread_key_create(&try_key, NULL);
}

void exception_init(void) {
    pthread_once(&once_control, init_once);
}

void 
exception_throw(int error_code, const char *func,
                const char *file, int line, 
                const char *cause, ...)
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
