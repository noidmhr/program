#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// FNV-1a 64位哈希算法的初始值和素数
#define FNV1A_64_INIT 0xcbf29ce484222325ULL
#define FNV1A_64_PRIME 0x100000001b3ULL

// 用于统计每个字节出现频率的结构体
typedef struct {
    uint8_t byte;  // 字节值
    uint64_t frequency;  // 该字节出现的频率
} Frequency;

// 哈夫曼树节点的结构体
typedef struct HuffmanNode {
    uint8_t byte;  // 字节值
    uint64_t frequency;  // 该字节出现的频率
    struct HuffmanNode *left, *right;  // 左右子节点指针
} HuffmanNode;

// 最小堆的结构体
typedef struct {
    HuffmanNode **array;  // 存储哈夫曼节点指针的数组
    int size;  // 当前堆中元素的数量
    int capacity;  // 堆的最大容量
} MinHeap;

// 编码表条目的结构体
typedef struct {
    uint8_t byte;  // 字节值
    char *code;  // 该字节对应的哈夫曼编码
    int code_length;  // 编码的长度
} CodeEntry;

// 解码表条目的结构体
typedef struct {
    uint8_t byte;  // 字节值
    int code_length;  // 编码长度
    uint8_t bits[16];  // 编码字节数组，最多支持 128 位编码
} DecodeEntry;

// 堆操作函数声明
MinHeap *create_min_heap(int capacity);  // 创建一个指定容量的最小堆
void swap_nodes(HuffmanNode **a, HuffmanNode **b);  // 交换两个哈夫曼节点指针
void heapify(MinHeap *heap, int index);  // 维护最小堆的性质
HuffmanNode *extract_min(MinHeap *heap);  // 从最小堆中提取最小元素
void insert_min_heap(MinHeap *heap, HuffmanNode *node);  // 向最小堆中插入一个节点
MinHeap *build_min_heap(Frequency freq[], int n);  // 根据频率数组构建最小堆

// 哈夫曼树操作函数声明
HuffmanNode *create_huffman_node(uint8_t byte, uint64_t frequency);  // 创建一个哈夫曼树节点
HuffmanNode *build_huffman_tree(Frequency freq[], int n);  // 根据频率数组构建哈夫曼树
void generate_codes(HuffmanNode *root, char **codes, int top, CodeEntry *code_table);  // 生成哈夫曼编码表
HuffmanNode *build_decode_tree(DecodeEntry *table, int n);  // 构建解码树

// 堆排序函数声明
void heap_sort(Frequency arr[], int n);  // 对频率数组进行堆排序

// 哈希计算函数声明
uint64_t fnv1a_64(const void *data, size_t length);  // 计算 FNV-1a 64 位哈希值

// 位操作函数声明
void append_bit(uint8_t **buffer, int *bit_count, int bit);  // 向缓冲区追加一个位
void flush_buffer(uint8_t **buffer, int *bit_count, FILE *file);  // 将缓冲区的内容刷新到文件

// 扩展功能函数声明
void encrypt_bytes(uint8_t *data, size_t length, uint8_t offset);  // 对字节数据进行加密
void decrypt_bytes(uint8_t *data, size_t length, uint8_t offset);  // 对字节数据进行解密

// 新增：计算哈夫曼树加权路径长度
uint64_t calculate_wpl(HuffmanNode *root, int depth);

// 新增：显示编码表差异
void show_code_table_diff(CodeEntry *original, CodeEntry *new_table);

#endif    