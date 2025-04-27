#include "huffman.h"

// 创建一个指定容量的最小堆
MinHeap *create_min_heap(int capacity) {
    MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));  // 为最小堆结构体分配内存
    if (heap == NULL) {
        perror("Memory allocation failed for min heap");
        return NULL;
    }
    heap->size = 0;  // 初始化堆的大小为 0
    heap->capacity = capacity;  // 设置堆的最大容量
    heap->array = (HuffmanNode **)malloc(capacity * sizeof(HuffmanNode *));  // 为存储哈夫曼节点指针的数组分配内存
    if (heap->array == NULL) {
        perror("Memory allocation failed for min heap array");
        free(heap);
        return NULL;
    }
    return heap;
}

// 交换两个哈夫曼节点指针
void swap_nodes(HuffmanNode **a, HuffmanNode **b) {
    HuffmanNode *temp = *a;  // 临时存储节点指针
    *a = *b;  // 交换节点指针
    *b = temp;
}

// 维护最小堆的性质
void heapify(MinHeap *heap, int index) {
    int smallest = index;  // 假设当前节点是最小节点
    int left = 2 * index + 1;  // 左子节点的索引
    int right = 2 * index + 2;  // 右子节点的索引

    // 如果左子节点存在且频率小于当前节点的频率
    if (left < heap->size && heap->array[left]->frequency < heap->array[smallest]->frequency)
        smallest = left;
    // 如果右子节点存在且频率小于当前节点的频率
    if (right < heap->size && heap->array[right]->frequency < heap->array[smallest]->frequency)
        smallest = right;
    // 如果最小节点不是当前节点，则交换它们并继续维护堆的性质
    if (smallest != index) {
        swap_nodes(&heap->array[index], &heap->array[smallest]);
        heapify(heap, smallest);
    }
}

// 从最小堆中提取最小元素
HuffmanNode *extract_min(MinHeap *heap) {
    if (heap->size <= 0) return NULL;  // 如果堆为空，返回 NULL
    HuffmanNode *temp = heap->array[0];  // 存储堆顶元素
    heap->array[0] = heap->array[heap->size - 1];  // 将最后一个元素移到堆顶
    heap->size--;  // 堆的大小减 1
    heapify(heap, 0);  // 维护堆的性质
    return temp;
}

// 向最小堆中插入一个节点
void insert_min_heap(MinHeap *heap, HuffmanNode *node) {
    if (heap->size == heap->capacity) {
        fprintf(stderr, "Heap is full\n");
        return;
    }
    heap->array[heap->size] = node;  // 将节点插入到堆的末尾
    int i = heap->size;  // 当前节点的索引
    // 向上调整堆，确保堆的性质
    while (i > 0 && heap->array[(i - 1) / 2]->frequency > node->frequency) {
        heap->array[i] = heap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    heap->array[i] = node;  // 将节点插入到正确的位置
    heap->size++;  // 堆的大小加 1
}

// 根据频率数组构建最小堆
MinHeap *build_min_heap(Frequency freq[], int n) {
    MinHeap *heap = create_min_heap(n);  // 创建一个容量为 n 的最小堆
    if (heap == NULL) return NULL;
    for (int i = 0; i < n; i++) {
        HuffmanNode *node = create_huffman_node(freq[i].byte, freq[i].frequency);  // 创建一个哈夫曼节点
        if (node == NULL) {
            // 释放已分配的堆内存
            for (int j = 0; j < i; j++) {
                free(heap->array[j]);
            }
            free(heap->array);
            free(heap);
            return NULL;
        }
        insert_min_heap(heap, node);  // 将节点插入到最小堆中
    }
    for (int i = (n - 1) / 2; i >= 0; i--)
        heapify(heap, i);  // 维护堆的性质
    return heap;
}

// 创建一个哈夫曼树节点
HuffmanNode *create_huffman_node(uint8_t byte, uint64_t frequency) {
    HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));  // 为哈夫曼节点分配内存
    if (node == NULL) {
        perror("Memory allocation failed for Huffman node");
        return NULL;
    }
    node->byte = byte;  // 设置字节值
    node->frequency = frequency;  // 设置频率
    node->left = node->right = NULL;  // 初始化左右子节点指针为 NULL
    return node;
}

// 根据频率数组构建哈夫曼树
HuffmanNode *build_huffman_tree(Frequency freq[], int n) {
    MinHeap *heap = build_min_heap(freq, n);  // 根据频率数组构建最小堆
    if (heap == NULL) return NULL;
    while (heap->size > 1) {
        HuffmanNode *left = extract_min(heap);  // 从最小堆中提取最小元素作为左子节点
        HuffmanNode *right = extract_min(heap);  // 从最小堆中提取次小元素作为右子节点
        uint64_t sum = left->frequency + right->frequency;  // 计算左右子节点频率之和
        HuffmanNode *parent = create_huffman_node(
            (left->byte > right->byte) ? left->byte : right->byte, sum);  // 创建一个父节点
        if (parent == NULL) {
            free(left);
            free(right);
            // 释放堆内存
            for (int i = 0; i < heap->size; i++) {
                free(heap->array[i]);
            }
            free(heap->array);
            free(heap);
            return NULL;
        }
        parent->left = left;  // 设置父节点的左子节点
        parent->right = right;  // 设置父节点的右子节点
        insert_min_heap(heap, parent);  // 将父节点插入到最小堆中
    }
    HuffmanNode *root = extract_min(heap);  // 提取堆顶元素作为哈夫曼树的根节点
    free(heap->array);
    free(heap);
    return root;
}

// 生成哈夫曼编码表
void generate_codes(HuffmanNode *root, char **codes, int top, CodeEntry *code_table) {
    static int index = 0;  // 编码表的索引
    if (root->left) {
        codes[top] = (char *)malloc(2);  // 分配内存
        codes[top][0] = '0';
        codes[top][1] = '\0';
        generate_codes(root->left, codes, top + 1, code_table);  // 递归生成左子树的编码
    }
    if (root->right) {
        codes[top] = (char *)malloc(2);  // 分配内存
        codes[top][0] = '1';
        codes[top][1] = '\0';
        generate_codes(root->right, codes, top + 1, code_table);  // 递归生成右子树的编码
    }
    if (!root->left && !root->right) {  // 如果是叶子节点
        code_table[index].byte = root->byte;  // 设置字节值
        code_table[index].code = (char *)malloc((top + 1) * sizeof(char));  // 为编码分配内存
        if (code_table[index].code == NULL) {
            perror("Memory allocation failed for code");
            return;
        }
        strncpy(code_table[index].code, codes[top], top);  // 复制编码
        code_table[index].code[top] = '\0';  // 字符串结束符
        code_table[index].code_length = top;  // 设置编码长度
        index++;  // 索引加 1
    }
}

// 对频率数组进行堆排序
void heap_sort(Frequency arr[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--)
        for (int j = i; j < n; j++)
            if (arr[j].frequency > arr[(j - 1) / 2].frequency)
                swap_nodes((HuffmanNode **)&arr[j], (HuffmanNode **)&arr[(j - 1) / 2]);
    for (int i = n - 1; i > 0; i--) {
        swap_nodes((HuffmanNode **)&arr[0], (HuffmanNode **)&arr[i]);
        for (int j = 0; j < i; j++)
            if (arr[j].frequency > arr[(j * 2 + 1) < i ? (j * 2 + 1) : j].frequency)
                swap_nodes((HuffmanNode **)&arr[j], (HuffmanNode **)&arr[(j * 2 + 1) < i ? (j * 2 + 1) : j]);
    }
}

// 计算 FNV - 1a 64 位哈希值
uint64_t fnv1a_64(const void *data, size_t length) {
    uint64_t hash = FNV1A_64_INIT;  // 初始化哈希值
    const uint8_t *byte_data = (const uint8_t *)data;  // 将数据转换为字节数组
    for (size_t i = 0; i < length; i++) {
        hash ^= byte_data[i];  // 异或当前字节
        hash *= FNV1A_64_PRIME;  // 乘以素数
    }
    return hash;
}

// 向缓冲区追加一个位
void append_bit(uint8_t **buffer, int *bit_count, int bit) {
    if (*bit_count == 8) {  // 如果缓冲区已满
        *buffer = (uint8_t *)realloc(*buffer, (*buffer ? 1 : 0) + 1);  // 重新分配内存
        if (*buffer == NULL) {
            perror("Memory allocation failed for buffer");
            return;
        }
        **buffer = 0;  // 清空缓冲区
        *bit_count = 0;  // 重置位计数器
    }
    if (bit)
        (*buffer)[0] |= (1 << (7 - *bit_count));  // 将位添加到缓冲区
    (*bit_count)++;  // 位计数器加 1
}

// 将缓冲区的内容刷新到文件
void flush_buffer(uint8_t **buffer, int *bit_count, FILE *file) {
    if (*bit_count > 0) {  // 如果缓冲区有数据
        fwrite(buffer, 1, 1, file);  // 将缓冲区的内容写入文件
        *buffer = NULL;  // 清空缓冲区指针
        *bit_count = 0;  // 重置位计数器
    }
}

// 对字节数据进行加密
void encrypt_bytes(uint8_t *data, size_t length, uint8_t offset) {
    for (size_t i = 0; i < length; i++)
        data[i] += offset;  // 对每个字节加上偏移量
}

// 对字节数据进行解密
void decrypt_bytes(uint8_t *data, size_t length, uint8_t offset) {
    for (size_t i = 0; i < length; i++)
        data[i] -= offset;  // 对每个字节减去偏移量
}

// 计算哈夫曼树加权路径长度
uint64_t calculate_wpl(HuffmanNode *root, int depth) {
    if (!root) return 0;
    if (!root->left && !root->right)  // 叶子节点
        return root->frequency * depth;
    return calculate_wpl(root->left, depth + 1) + calculate_wpl(root->right, depth + 1);
}

// 显示编码表差异
void show_code_table_diff(CodeEntry *original, CodeEntry *new_table) {
    // 这里简单示例，可根据实际情况完善
    printf("编码表差异显示：\n");
    for (int i = 0; i < 256; i++) {
        if (original[i].code && new_table[i].code) {
            if (strcmp(original[i].code, new_table[i].code) != 0) {
                printf("Byte 0x%02x: 原编码 %s, 新编码 %s\n", original[i].byte, original[i].code, new_table[i].code);
            }
        }
    }
}

// 构建解码树
HuffmanNode *build_decode_tree(DecodeEntry *table, int n) {
    HuffmanNode *root = create_huffman_node(0, 0);
    for (int i = 0; i < n; i++) {
        HuffmanNode *current = root;
        for (int j = 0; j < table[i].code_length; j++) {
            int bit = (table[i].bits[j / 8] >> (7 - (j % 8))) & 1;
            if (bit) {
                if (!current->right) {
                    current->right = create_huffman_node(0, 0);
                }
                current = current->right;
            } else {
                if (!current->left) {
                    current->left = create_huffman_node(0, 0);
                }
                current = current->left;
            }
        }
        current->byte = table[i].byte;
    }
    return root;
}