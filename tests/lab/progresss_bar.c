#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/try.h>

#define PROGRESS_BAR_ROW 1  // 进度条固定在第1行
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// 进度条线程
void* progress_bar(void* arg) {
    int width = 50;
    for (int i = 0; i <= width; ++i) {
        pthread_mutex_lock(&print_mutex);
        
        // 定位到进度条行并刷新
        printf("\033[%d;1H\033[K[", PROGRESS_BAR_ROW);
        for (int j = 0; j < i; ++j) printf("=");
        for (int j = i; j < width; ++j) printf(" ");
        printf("] %d%%\r", i*100/width);
        fflush(stdout);
        
        pthread_mutex_unlock(&print_mutex);
        usleep(100000);  // 100ms刷新
    }
    return NULL;
}

// 日志打印函数（线程安全）
void safe_print(const char* msg) {
    pthread_mutex_lock(&print_mutex);
    printf("\033[%d;1H\033[K%s\n", PROGRESS_BAR_ROW + 1, msg);  // 在进度条下方打印
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);
}

int test_process_bar() {
    printf("\n\n");  // 为进度条预留空间
    
    pthread_t tid;
    pthread_create(&tid, NULL, progress_bar, NULL);

    // 模拟日志打印
    for (int i = 1; i <= 10; ++i) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Log message %d at %ld", i, time(NULL));
        safe_print(buf);
        sleep(1);
    }

    pthread_join(tid, NULL);
    return 0;
}
REGISTER_TEST_CMD(test_process_bar);