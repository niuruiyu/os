#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5   // 缓冲区大小
#define PRO_NUM 2       // 生产者线程数
#define CON_NUM 2       // 消费者线程数
#define PRODUCT_COUNT 10// 总生产数量

// 共享资源
int buffer[BUFFER_SIZE];
int in = 0, out = 0;    // 生产/消费指针

// 同步机制
sem_t empty;           // 空缓冲区数量
sem_t full;            // 满缓冲区数量
pthread_mutex_t mutex;  // 缓冲区互斥锁

// 生产者线程函数
void* producer(void* arg) {
    int id = *(int*)arg;
    free(arg);
    for (int i = 0; i < PRODUCT_COUNT / PRO_NUM; i++) {
        int item = rand() % 100;  // 生产随机数据

        sem_wait(&empty);         // 等待空缓冲区
        pthread_mutex_lock(&mutex);// 互斥访问缓冲区

        buffer[in] = item;
        printf("生产者%d：生产数据 %d，位置 %d\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);           // 增加满缓冲区数量

        sleep(1);  // 模拟生产耗时
    }
    printf("生产者%d 退出\n", id);
    return NULL;
}

// 消费者线程函数
void* consumer(void* arg) {
    int id = *(int*)arg;
    free(arg);
    while (1) {
        sem_wait(&full);          // 等待满缓冲区
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        printf("消费者%d：消费数据 %d，位置 %d\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);         // 增加空缓冲区数量

        sleep(2);  // 模拟消费耗时
    }
    return NULL;
}

int main() {
    pthread_t pro_tid[PRO_NUM], con_tid[CON_NUM];

    // 初始化同步工具
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // 创建生产者线程
    for (int i = 0; i < PRO_NUM; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&pro_tid[i], NULL, producer, id);
    }
    // 创建消费者线程
    for (int i = 0; i < CON_NUM; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&con_tid[i], NULL, consumer, id);
    }

    // 等待生产者完成
    for (int i = 0; i < PRO_NUM; i++) {
        pthread_join(pro_tid[i], NULL);
    }

    // 销毁资源（简化版，消费者为死循环）
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    return 0;
}
