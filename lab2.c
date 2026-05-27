#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MEM_SIZE 1024    // 总内存大小
#define MAX_PAGES 20         // 最大页面数
#define MAX_FRAMES 10        // 最大物理块数

// ==================== 动态分区管理（首次适应/最佳适应）====================
typedef struct Partition {
    int id;             // 分区号
    int size;           // 分区大小
    int start;          // 起始地址
    int used;           // 0=空闲 1=占用
    struct Partition *next;
} Partition;

Partition *mem_head = NULL;
int part_id = 1;

// 初始化内存
void init_memory() {
    mem_head = (Partition *)malloc(sizeof(Partition));
    mem_head->id = 0;
    mem_head->size = MAX_MEM_SIZE;
    mem_head->start = 0;
    mem_head->used = 0;
    mem_head->next = NULL;
    printf("内存初始化完成！总大小：%d\n", MAX_MEM_SIZE);
}

// 显示内存分区
void show_memory() {
    printf("\n===== 当前内存分区状态 =====\n");
    Partition *p = mem_head;
    while (p) {
        printf("分区%d：起始=%d，大小=%d，状态=%s\n",
               p->id, p->start, p->size, p->used ? "占用" : "空闲");
        p = p->next;
    }
}

// 首次适应分配 FF
int alloc_ff(int size) {
    Partition *p = mem_head;
    while (p) {
        if (!p->used && p->size >= size) {
            // 分割分区
            if (p->size > size) {
                Partition *new_part = (Partition *)malloc(sizeof(Partition));
                new_part->id = part_id++;
                new_part->size = p->size - size;
                new_part->start = p->start + size;
                new_part->used = 0;
                new_part->next = p->next;
                p->next = new_part;
            }
            p->size = size;
            p->used = 1;
            printf("【首次适应】分配成功！分区%d\n", p->id);
            return 1;
        }
        p = p->next;
    }
    printf("分配失败：无足够空闲分区\n");
    return 0;
}

// 最佳适应分配 BF
int alloc_bf(int size) {
    Partition *p = mem_head;
    Partition *best = NULL;

    while (p) {
        if (!p->used && p->size >= size) {
            if (!best || p->size < best->size) best = p;
        }
        p = p->next;
    }

    if (best) {
        if (best->size > size) {
            Partition *new_part = (Partition *)malloc(sizeof(Partition));
            new_part->id = part_id++;
            new_part->size = best->size - size;
            new_part->start = best->start + size;
            new_part->used = 0;
            new_part->next = best->next;
            best->next = new_part;
        }
        best->size = size;
        best->used = 1;
        printf("【最佳适应】分配成功！分区%d\n", best->id);
        return 1;
    }
    printf("分配失败：无足够空闲分区\n");
    return 0;
}

// 回收内存
void free_mem(int id) {
    Partition *p = mem_head;
    while (p) {
        if (p->id == id && p->used) {
            p->used = 0;
            printf("回收分区%d 成功\n", id);

            // 向后合并
            if (p->next && !p->next->used) {
                Partition *temp = p->next;
                p->size += temp->size;
                p->next = temp->next;
                free(temp);
            }
            return;
        }
        p = p->next;
    }
    printf("回收失败：分区不存在或已空闲\n");
}

// ==================== 页面置换算法（FIFO / LRU）====================
// FIFO页面置换
void fifo(int pages[], int n, int frames) {
    int frame[MAX_FRAMES], front = 0, count = 0;
    memset(frame, -1, sizeof(frame));

    printf("\n===== FIFO页面置换过程 =====\n");
    for (int i = 0; i < n; i++) {
        int find = 0;
        for (int j = 0; j < frames; j++) {
            if (frame[j] == pages[i]) {
                find = 1;
                break;
            }
        }

        if (!find) {
            frame[front] = pages[i];
            front = (front + 1) % frames;
            count++;
        }

        printf("访问页面%d：", pages[i]);
        for (int j = 0; j < frames; j++) {
            if (frame[j] == -1) printf("空 ");
            else printf("%d ", frame[j]);
        }
        printf("\n");
    }

    printf("缺页次数：%d，缺页率：%.2f%%\n", count, (count * 100.0) / n);
}

// LRU页面置换
void lru(int pages[], int n, int frames) {
    int frame[MAX_FRAMES], cnt[MAX_FRAMES] = {0}, count = 0;
    memset(frame, -1, sizeof(frame));

    printf("\n===== LRU页面置换过程 =====\n");
    for (int i = 0; i < n; i++) {
        int find = -1;
        for (int j = 0; j < frames; j++) {
            if (frame[j] == pages[i]) {
                find = j;
                cnt[j] = i + 1;
                break;
            }
        }

        if (find == -1) {
            int min = 0;
            for (int j = 1; j < frames; j++) {
                if (cnt[j] < cnt[min]) min = j;
            }
            frame[min] = pages[i];
            cnt[min] = i + 1;
            count++;
        }

        printf("访问页面%d：", pages[i]);
        for (int j = 0; j < frames; j++) {
            if (frame[j] == -1) printf("空 ");
            else printf("%d ", frame[j]);
        }
        printf("\n");
    }

    printf("缺页次数：%d，缺页率：%.2f%%\n", count, (count * 100.0) / n);
}

// ==================== 菜单 ====================
void menu() {
    printf("\n========== 内存管理模拟 ==========\n");
    printf("1. 动态分区管理（首次适应/最佳适应）\n");
    printf("2. 页面置换算法（FIFO/LRU）\n");
    printf("0. 退出\n");
    printf("请输入选择：");
}

void part_menu() {
    printf("\n===== 动态分区管理 =====\n");
    printf("1. 首次适应分配\n");
    printf("2. 最佳适应分配\n");
    printf("3. 回收分区\n");
    printf("4. 查看内存\n");
    printf("0. 返回上一级\n");
    printf("请输入选择：");
}

void page_menu() {
    printf("\n===== 页面置换算法 =====\n");
    printf("1. FIFO置换\n");
    printf("2. LRU置换\n");
    printf("0. 返回上一级\n");
    printf("请输入选择：");
}

int main() {
    int choice, c2, size, id;
    int pages[MAX_PAGES], n, frames;

    init_memory();
    while (1) {
        menu();
        scanf("%d", &choice);
        if (choice == 0) break;

        if (choice == 1) {
            while (1) {
                part_menu();
                scanf("%d", &c2);
                if (c2 == 0) break;
                if (c2 == 1) {
                    printf("输入分配大小：");
                    scanf("%d", &size);
                    alloc_ff(size);
                } else if (c2 == 2) {
                    printf("输入分配大小：");
                    scanf("%d", &size);
                    alloc_bf(size);
                } else if (c2 == 3) {
                    printf("输入回收分区号：");
                    scanf("%d", &id);
                    free_mem(id);
                } else if (c2 == 4) {
                    show_memory();
                }
            }
        } else if (choice == 2) {
            printf("输入页面访问序列长度：");
            scanf("%d", &n);
            printf("输入页面序列（空格分隔）：");
            for (int i = 0; i < n; i++) scanf("%d", &pages[i]);
            printf("输入物理块数：");
            scanf("%d", &frames);

            while (1) {
                page_menu();
                scanf("%d", &c2);
                if (c2 == 0) break;
                if (c2 == 1) fifo(pages, n, frames);
                else if (c2 == 2) lru(pages, n, frames);
            }
        }
    }
    return 0;
}
