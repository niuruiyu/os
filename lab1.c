#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESS 100  // 最大进程数

// 进程控制块 PCB
typedef struct {
    int pid;         // 进程ID
    int arrive;      // 到达时间
    int burst;       // 服务时间
    int priority;    // 优先级（越小越高）

    int start;       // 开始时间
    int finish;      // 完成时间
    int turn;        // 周转时间
    float w_turn;     // 带权周转时间

    int remain;      // 剩余时间（RR专用）
    int flag;        // 是否完成（0未完成 1完成）
} Process;

// 工具函数：按到达时间排序
void sortByArrive(Process p[], int n) {
    int i, j;
    Process temp;
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            if (p[j].arrive < p[i].arrive) {
                temp = p[i];
                p[i] = p[j];
                p[j] = temp;
            }
        }
    }
}

// ===================== 1. FCFS 先来先服务 =====================
void FCFS(Process p[], int n) {
    Process proc[MAX_PROCESS];
    memcpy(proc, p, n * sizeof(Process));
    sortByArrive(proc, n);

    int current = 0;
    printf("\n===== FCFS 调度结果 =====\n");
    printf("运行顺序：");

    for (int i = 0; i < n; i++) {
        if (current < proc[i].arrive)
            current = proc[i].arrive;

        proc[i].start = current;
        proc[i].finish = current + proc[i].burst;
        proc[i].turn = proc[i].finish - proc[i].arrive;
        proc[i].w_turn = (float)proc[i].turn / proc[i].burst;

        printf("P%d -> ", proc[i].pid);
        current = proc[i].finish;
    }
    printf("结束\n");

    float avg_t = 0, avg_wt = 0;
    printf("PID\t到达\t服务\t优先级\t完成\t周转\t带权周转\n");
    for (int i = 0; i < n; i++) {
        avg_t += proc[i].turn;
        avg_wt += proc[i].w_turn;
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%.2f\n",
               proc[i].pid, proc[i].arrive, proc[i].burst, proc[i].priority,
               proc[i].finish, proc[i].turn, proc[i].w_turn);
    }
    printf("平均周转时间：%.2f\n", avg_t / n);
    printf("平均带权周转时间：%.2f\n", avg_wt / n);
}

// ===================== 2. SJF 短作业优先（非抢占） =====================
void SJF(Process p[], int n) {
    Process proc[MAX_PROCESS];
    memcpy(proc, p, n * sizeof(Process));

    int current = 0, completed = 0;
    int min_idx;

    printf("\n===== SJF 调度结果 =====\n");
    printf("运行顺序：");

    while (completed < n) {
        min_idx = -1;
        int min_burst = 9999;

        for (int i = 0; i < n; i++) {
            if (proc[i].flag == 0 && proc[i].arrive <= current && proc[i].burst < min_burst) {
                min_burst = proc[i].burst;
                min_idx = i;
            }
        }

        if (min_idx == -1) {
            current++;
            continue;
        }

        proc[min_idx].start = current;
        proc[min_idx].finish = current + proc[min_idx].burst;
        proc[min_idx].turn = proc[min_idx].finish - proc[min_idx].arrive;
        proc[min_idx].w_turn = (float)proc[min_idx].turn / proc[min_idx].burst;
        proc[min_idx].flag = 1;

        printf("P%d -> ", proc[min_idx].pid);
        current = proc[min_idx].finish;
        completed++;
    }
    printf("结束\n");

    float avg_t = 0, avg_wt = 0;
    printf("PID\t到达\t服务\t优先级\t完成\t周转\t带权周转\n");
    for (int i = 0; i < n; i++) {
        avg_t += proc[i].turn;
        avg_wt += proc[i].w_turn;
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%.2f\n",
               proc[i].pid, proc[i].arrive, proc[i].burst, proc[i].priority,
               proc[i].finish, proc[i].turn, proc[i].w_turn);
    }
    printf("平均周转时间：%.2f\n", avg_t / n);
    printf("平均带权周转时间：%.2f\n", avg_wt / n);
}

// ===================== 3. RR 时间片轮转 =====================
void RR(Process p[], int n, int slice) {
    Process proc[MAX_PROCESS];
    memcpy(proc, p, n * sizeof(Process));

    int queue[MAX_PROCESS], front = 0, rear = 0;
    int current = 0, completed = 0;
    int in_queue[MAX_PROCESS] = {0};

    printf("\n===== RR 调度结果（时间片=%d）=====\n", slice);
    printf("运行顺序：");

    // 初始化剩余时间
    for (int i = 0; i < n; i++)
        proc[i].remain = proc[i].burst;

    while (completed < n) {
        // 把已到达的进程加入队列
        for (int i = 0; i < n; i++) {
            if (proc[i].arrive <= current && proc[i].flag == 0 && in_queue[i] == 0) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (front == rear) {
            current++;
            continue;
        }

        int idx = queue[front++];
        printf("P%d -> ", proc[idx].pid);

        if (proc[idx].remain > slice) {
            current += slice;
            proc[idx].remain -= slice;
        } else {
            current += proc[idx].remain;
            proc[idx].finish = current;
            proc[idx].turn = proc[idx].finish - proc[idx].arrive;
            proc[idx].w_turn = (float)proc[idx].turn / proc[idx].burst;
            proc[idx].flag = 1;
            completed++;
        }

        // 再次加入已到达的进程
        for (int i = 0; i < n; i++) {
            if (proc[i].arrive <= current && proc[i].flag == 0 && in_queue[i] == 0) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        // 未完成则重新入队
        if (proc[idx].flag == 0)
            queue[rear++] = idx;
    }

    printf("结束\n");
    float avg_t = 0, avg_wt = 0;
    printf("PID\t到达\t服务\t优先级\t完成\t周转\t带权周转\n");
    for (int i = 0; i < n; i++) {
        avg_t += proc[i].turn;
        avg_wt += proc[i].w_turn;
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%.2f\n",
               proc[i].pid, proc[i].arrive, proc[i].burst, proc[i].priority,
               proc[i].finish, proc[i].turn, proc[i].w_turn);
    }
    printf("平均周转时间：%.2f\n", avg_t / n);
    printf("平均带权周转时间：%.2f\n", avg_wt / n);
}

// ===================== 4. 优先级调度（非抢占） =====================
void Priority(Process p[], int n) {
    Process proc[MAX_PROCESS];
    memcpy(proc, p, n * sizeof(Process));

    int current = 0, completed = 0;
    int min_idx;

    printf("\n===== 优先级调度 结果 =====\n");
    printf("运行顺序：");

    while (completed < n) {
        min_idx = -1;
        int min_prio = 9999;

        for (int i = 0; i < n; i++) {
            if (proc[i].flag == 0 && proc[i].arrive <= current && proc[i].priority < min_prio) {
                min_prio = proc[i].priority;
                min_idx = i;
            }
        }

        if (min_idx == -1) {
            current++;
            continue;
        }

        proc[min_idx].start = current;
        proc[min_idx].finish = current + proc[min_idx].burst;
        proc[min_idx].turn = proc[min_idx].finish - proc[min_idx].arrive;
        proc[min_idx].w_turn = (float)proc[min_idx].turn / proc[min_idx].burst;
        proc[min_idx].flag = 1;

        printf("P%d -> ", proc[min_idx].pid);
        current = proc[min_idx].finish;
        completed++;
    }
    printf("结束\n");

    float avg_t = 0, avg_wt = 0;
    printf("PID\t到达\t服务\t优先级\t完成\t周转\t带权周转\n");
    for (int i = 0; i < n; i++) {
        avg_t += proc[i].turn;
        avg_wt += proc[i].w_turn;
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%.2f\n",
               proc[i].pid, proc[i].arrive, proc[i].burst, proc[i].priority,
               proc[i].finish, proc[i].turn, proc[i].w_turn);
    }
    printf("平均周转时间：%.2f\n", avg_t / n);
    printf("平均带权周转时间：%.2f\n", avg_wt / n);
}

// ===================== 主函数 =====================
int main() {
    Process p[MAX_PROCESS];
    int n, slice;

    printf("===== 处理机调度算法模拟 =====\n");
    printf("请输入进程数量：");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        printf("\n进程 P%d\n", i + 1);
        p[i].pid = i + 1;
        printf("到达时间：");
        scanf("%d", &p[i].arrive);
        printf("服务时间：");
        scanf("%d", &p[i].burst);
        printf("优先级（数值越小优先级越高）：");
        scanf("%d", &p[i].priority);
        p[i].flag = 0;
    }

    printf("\n请输入 RR 时间片大小：");
    scanf("%d", &slice);

    // 运行所有算法
    FCFS(p, n);
    SJF(p, n);
    RR(p, n, slice);
    Priority(p, n);
    return 0;
}
