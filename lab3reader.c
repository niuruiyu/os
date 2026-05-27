#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define READER_NUM 3  // 读者数量
#define WRITER_NUM 2  // 写者数量

int read_count = 0;               // 读者计数
pthread_mutex_t r_mutex;          // 保护读者计数
pthread_mutex_t w_mutex;          // 写者互斥锁

// 读者线程
void* reader(void* arg) {
    int id = *(int*)arg;
    free(arg);
    while (1) {
        // 申请读者计数锁
        pthread_mutex_lock(&r_mutex);
        read_count++;
        // 第一个读者，阻塞写者
        if (read_count == 1) {
            pthread_mutex_lock(&w_mutex);
        }
        pthread_mutex_unlock(&r_mutex);

        // 读取操作
        printf("读者%d 正在读取数据\n", id);
        sleep(1);

        // 释放读者计数锁
        pthread_mutex_lock(&r_mutex);
        read_count--;
        // 最后一个读者，释放写者
        if (read_count == 0) {
            pthread_mutex_unlock(&w_mutex);
        }
        pthread_mutex_unlock(&r_mutex);

        sleep(2);
    }
    return NULL;
}

// 写者线程
void* writer(void* arg) {
    int id = *(int*)arg;
    free(arg);
    while (1) {
        // 互斥写操作
        pthread_mutex_lock(&w_mutex);
        printf("写者%d 正在写入数据\n", id);
        sleep(2);
        pthread_mutex_unlock(&w_mutex);

        sleep(3);
    }
    return NULL;
}

int main() {
    pthread_t r_tid[READER_NUM], w_tid[WRITER_NUM];
    pthread_mutex_init(&r_mutex, NULL);
    pthread_mutex_init(&w_mutex, NULL);

    // 创建读者
    for (int i = 0; i < READER_NUM; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&r_tid[i], NULL, reader, id);
    }
    // 创建写者
    for (int i = 0; i < WRITER_NUM; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&w_tid[i], NULL, writer, id);
    }

    // 等待线程（无限循环）
    for (int i = 0; i < READER_NUM; i++) {
        pthread_join(r_tid[i], NULL);
    }
    for (int i = 0; i < WRITER_NUM; i++) {
        pthread_join(w_tid[i], NULL);
    }

    pthread_mutex_destroy(&r_mutex);
    pthread_mutex_destroy(&w_mutex);
    return 0;
}
