#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ===================== 系统常量定义 =====================
#define BLOCK_NUM    32      // 磁盘总块数量
#define BLOCK_SIZE   64      // 每个磁盘块字节大小
#define MAX_FILE     16      // 目录最大文件数
#define FILE_NAME_LEN 16     // 文件名最大长度

// 文件类型标记
#define FILE_EMPTY   0       // 目录项空闲
#define FILE_NORMAL  1       // 普通文件

// ===================== 数据结构定义 =====================
// 磁盘块结构
typedef struct {
    char data[BLOCK_SIZE];
} DiskBlock;

// 文件控制块 FCB
typedef struct {
    char name[FILE_NAME_LEN];  // 文件名
    int type;                  // 文件类型
    int start_block;           // 文件起始磁盘块号
    int file_size;             // 文件实际大小(字节)
    int block_count;           // 文件占用块数
} FCB;

// 全局模拟磁盘、位示图、目录
DiskBlock disk[BLOCK_NUM];    // 模拟整个磁盘
unsigned char bitmap[BLOCK_NUM]; // 位示图：0空闲 1已占用
FCB dir[MAX_FILE];            // 一级目录（FCB数组）

// ===================== 空闲空间管理：位示图操作 =====================
// 初始化位示图：所有块初始空闲
void InitBitmap()
{
    for (int i = 0; i < BLOCK_NUM; i++)
        bitmap[i] = 0;
}

// 分配一个空闲磁盘块，返回块号，-1表示无空闲块
int AllocBlock()
{
    for (int i = 0; i < BLOCK_NUM; i++)
    {
        if (bitmap[i] == 0)
        {
            bitmap[i] = 1;
            return i;
        }
    }
    return -1;
}

// 回收指定磁盘块
void FreeBlock(int block_id)
{
    if (block_id >= 0 && block_id < BLOCK_NUM)
        bitmap[block_id] = 0;
}

// ===================== 目录与FCB操作 =====================
// 初始化目录：所有目录项置空
void InitDir()
{
    for (int i = 0; i < MAX_FILE; i++)
    {
        dir[i].type = FILE_EMPTY;
        memset(dir[i].name, 0, FILE_NAME_LEN);
        dir[i].start_block = -1;
        dir[i].file_size = 0;
        dir[i].block_count = 0;
    }
}

// 根据文件名查找目录项，返回下标，-1表示不存在
int FindFile(const char *filename)
{
    for (int i = 0; i < MAX_FILE; i++)
    {
        if (dir[i].type == FILE_NORMAL && strcmp(dir[i].name, filename) == 0)
            return i;
    }
    return -1;
}

// 查找空闲目录项，返回下标，-1表示目录已满
int FindEmptyDirItem()
{
    for (int i = 0; i < MAX_FILE; i++)
    {
        if (dir[i].type == FILE_EMPTY)
            return i;
    }
    return -1;
}

// ===================== 文件基础操作 =====================
/**
 * @brief 创建空文件
 * @param filename 文件名
 * @return 0成功，-1失败
 */
int FileCreate(const char *filename)
{
    // 检查文件是否已存在
    if (FindFile(filename) != -1)
    {
        printf("错误：文件 %s 已存在！\n", filename);
        return -1;
    }
    // 检查目录是否已满
    int idx = FindEmptyDirItem();
    if (idx == -1)
    {
        printf("错误：目录已满，无法创建新文件！\n");
        return -1;
    }
    // 分配第一个磁盘块
    int block = AllocBlock();
    if (block == -1)
    {
        printf("错误：磁盘空间不足！\n");
        return -1;
    }
    // 填充FCB
    strcpy(dir[idx].name, filename);
    dir[idx].type = FILE_NORMAL;
    dir[idx].start_block = block;
    dir[idx].file_size = 0;
    dir[idx].block_count = 1;

    printf("文件 %s 创建成功，起始块：%d\n", filename, block);
    return 0;
}

/**
 * @brief 向文件写入数据（覆盖写入）
 * @param filename 文件名
 * @param buf 待写入数据
 * @param len 数据长度
 * @return 0成功，-1失败
 */
int FileWrite(const char *filename, const char *buf, int len)
{
    int idx = FindFile(filename);
    if (idx == -1)
    {
        printf("错误：文件 %s 不存在！\n", filename);
        return -1;
    }

    FCB *fcb = &dir[idx];
    int total_need = len;
    int cur_block = fcb->start_block;
    int used_block = fcb->block_count;

    // 1. 先清空原有所有块（模拟覆盖）
    for (int i = 0; i < used_block; i++)
    {
        memset(disk[cur_block].data, 0, BLOCK_SIZE);
        cur_block = (cur_block + 1) % BLOCK_NUM;
    }

    // 2. 重新计算需要多少块
    int new_block_cnt = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    cur_block = fcb->start_block;

    // 3. 若需要更多块，追加分配
    while (used_block < new_block_cnt)
    {
        int new_b = AllocBlock();
        if (new_b == -1)
        {
            printf("错误：磁盘空间不足，写入失败！\n");
            return -1;
        }
        used_block++;
        cur_block = new_b;
    }

    // 4. 逐块写入数据
    int offset = 0;
    cur_block = fcb->start_block;
    for (int i = 0; i < new_block_cnt; i++)
    {
        int copy_len = (total_need > BLOCK_SIZE) ? BLOCK_SIZE : total_need;
        memcpy(disk[cur_block].data, buf + offset, copy_len);
        offset += copy_len;
        total_need -= copy_len;
        cur_block = (cur_block + 1) % BLOCK_NUM;
    }

    // 更新FCB信息
    fcb->file_size = len;
    fcb->block_count = new_block_cnt;
    printf("文件 %s 写入成功，写入长度：%d 字节\n", filename, len);
    return 0;
}

/**
 * @brief 读取文件全部内容
 * @param filename 文件名
 * @param buf 接收缓冲区
 * @param buf_len 缓冲区大小
 * @return 读取字节数，-1失败
 */
int FileRead(const char *filename, char *buf, int buf_len)
{
    int idx = FindFile(filename);
    if (idx == -1)
    {
        printf("错误：文件 %s 不存在！\n", filename);
        return -1;
    }

    FCB *fcb = &dir[idx];
    if (fcb->file_size == 0)
    {
        printf("文件 %s 为空文件\n", filename);
        buf[0] = '\0';
        return 0;
    }
    if (buf_len < fcb->file_size)
    {
        printf("错误：缓冲区空间不足！\n");
        return -1;
    }

    memset(buf, 0, buf_len);
    int cur_block = fcb->start_block;
    int read_len = fcb->file_size;
    int offset = 0;

    // 逐块读取
    for (int i = 0; i < fcb->block_count; i++)
    {
        int copy = (read_len > BLOCK_SIZE) ? BLOCK_SIZE : read_len;
        memcpy(buf + offset, disk[cur_block].data, copy);
        offset += copy;
        read_len -= copy;
        cur_block = (cur_block + 1) % BLOCK_NUM;
    }
    printf("文件 %s 读取成功\n", filename);
    return fcb->file_size;
}

/**
 * @brief 删除文件：回收磁盘块 + 清空目录项
 * @param filename 文件名
 * @return 0成功，-1失败
 */
int FileDelete(const char *filename)
{
    int idx = FindFile(filename);
    if (idx == -1)
    {
        printf("错误：文件 %s 不存在！\n", filename);
        return -1;
    }

    FCB *fcb = &dir[idx];
    int cur_block = fcb->start_block;
    // 回收所有占用的磁盘块
    for (int i = 0; i < fcb->block_count; i++)
    {
        FreeBlock(cur_block);
        cur_block = (cur_block + 1) % BLOCK_NUM;
    }
    // 清空目录项
    fcb->type = FILE_EMPTY;
    memset(fcb->name, 0, FILE_NAME_LEN);
    fcb->start_block = -1;
    fcb->file_size = 0;
    fcb->block_count = 0;

    printf("文件 %s 删除成功，磁盘块已回收\n", filename);
    return 0;
}

// 列出当前目录所有文件
void ListDir()
{
    printf("\n========== 文件目录列表 ==========\n");
    int has_file = 0;
    for (int i = 0; i < MAX_FILE; i++)
    {
        if (dir[i].type == FILE_NORMAL)
        {
            has_file = 1;
            printf("文件名：%s | 大小：%d 字节 | 起始块：%d | 占用块数：%d\n",
                   dir[i].name, dir[i].file_size, dir[i].start_block, dir[i].block_count);
        }
    }
    if (!has_file)
        printf("目录为空\n");
    printf("==================================\n");
}

// 查看磁盘位示图（空闲空间状态）
void ShowBitmap()
{
    printf("\n========== 磁盘位示图(0空闲 1已占用) ==========\n");
    for (int i = 0; i < BLOCK_NUM; i++)
    {
        printf("%d ", bitmap[i]);
        if ((i + 1) % 8 == 0)
            printf("\n");
    }
    printf("==============================================\n");
}

// ===================== 主函数：交互测试 =====================
int main()
{
    // 系统初始化
    InitBitmap();
    InitDir();
    char filename[FILE_NAME_LEN];
    char buffer[256];
    int choice, ret;

    while (1)
    {
        printf("\n===== 简易文件系统菜单 =====\n");
        printf("1. 创建文件\n");
        printf("2. 写入文件\n");
        printf("3. 读取文件\n");
        printf("4. 删除文件\n");
        printf("5. 列出目录\n");
        printf("6. 查看磁盘位示图\n");
        printf("0. 退出系统\n");
        printf("请输入操作选择：");
        scanf("%d", &choice);
        getchar(); // 吸收换行符

        switch (choice)
        {
            case 1:
                printf("请输入文件名：");
                scanf("%s", filename);
                FileCreate(filename);
                break;
            case 2:
                printf("请输入文件名：");
                scanf("%s", filename);
                getchar();
                printf("请输入写入内容：");
                fgets(buffer, sizeof(buffer), stdin);
                // 去掉fgets读取的换行符
                buffer[strcspn(buffer, "\n")] = '\0';
                FileWrite(filename, buffer, strlen(buffer));
                break;
            case 3:
                printf("请输入文件名：");
                scanf("%s", filename);
                memset(buffer, 0, sizeof(buffer));
                ret = FileRead(filename, buffer, sizeof(buffer));
                if (ret >= 0)
                    printf("文件内容：%s\n", buffer);
                break;
            case 4:
                printf("请输入文件名：");
                scanf("%s", filename);
                FileDelete(filename);
                break;
            case 5:
                ListDir();
                break;
            case 6:
                ShowBitmap();
                break;
            case 0:
                printf("退出文件系统\n");
                return 0;
            default:
                printf("输入错误，请重新选择！\n");
                break;
        }
    }
    return 0;
}
