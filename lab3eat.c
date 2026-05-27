#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5  // 5个哲学家，5根筷子

sem_t chopsticks[N];  // 信号量表示筷子
sem_t room;           // 限制同时就餐人数（避免死锁）

// 哲学家线程
void* philosopher(void* num) {
    int id = *(int*)num;
    free(num);
    while (1) {
        printf("哲学家%d 正在思考\n", id);
        sleep(1);

        // 进入房间（限制4人，避免死锁）
        sem_wait(&room);
        // 拿左右筷子
        sem_wait(&chopsticks[id]);
        sem_wait(&chopsticks[(id + 1) % N]);

        // 吃饭
        printf("哲学家%d 开始吃饭\n", id);
        sleep(2);
        printf("哲学家%d 吃完放下筷子\n", id);

        // 放回筷子
        sem_post(&chopsticks[id]);
        sem_post(&chopsticks[(id + 1) % N]);
        // 离开房间
        sem_post(&room);
    }
    return NULL;
}

int main() {
    pthread_t tid[N];
    int i;

    // 初始化：每根筷子初始值1，房间最多4人
    sem_init(&room, 0, 4);
    for (i = 0; i < N; i++) {
        sem_init(&chopsticks[i], 0, 1);
    }

    // 创建哲学家线程
    for (i = 0; i < N; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&tid[i], NULL, philosopher, id);
    }

    // 等待线程
    for (i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }

    // 销毁资源
    sem_destroy(&room);
    for (i = 0; i < N; i++) {
        sem_destroy(&chopsticks[i]);
    }
    return 0;
}
