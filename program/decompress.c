#include "huffman.h"
#include <time.h>  // 添加头文件

// 从文件中加载编码表
DecodeEntry *load_code_table(const char *filename, long *original_size) {
    FILE *file = fopen(filename, "r");  // 以读取模式打开文件
    if (file == NULL) {
        perror("Failed to open code table file");
        return NULL;
    }
    fscanf(file, "%ld\n", original_size);  // 读取原始文件大小
    DecodeEntry *table = (DecodeEntry *)malloc(256 * sizeof(DecodeEntry));  // 分配内存用于存储解码表
    if (table == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }
    int index = 0;  // 解码表的索引
    char line[256];  // 读取行的缓冲区
    while (fgets(line, sizeof(line), file)) {
        uint8_t byte;  // 字节值
        int code_length;  // 编码长度
        uint8_t bits[16];  // 编码字节数组
        sscanf(line, "0x%02hhx %d ", &byte, &code_length);  // 修改格式说明符
        char *ptr = strstr(line, "0x");  // 查找编码字节的起始位置
        int count = 0;  // 编码字节的数量
        while (ptr) {
            uint8_t b;
            sscanf(ptr, "0x%02x", &b);  // 读取编码字节
            bits[count++] = b;
            ptr = strstr(ptr + 3, "0x");  // 查找下一个编码字节的起始位置
        }
        table[index].byte = byte;  // 设置字节值
        table[index].code_length = code_length;  // 设置编码长度
        memcpy(table[index].bits, bits, count);  // 复制编码字节数组
        index++;  // 索引加 1
    }
    fclose(file);  // 关闭文件
    return table;
}

// 解压缩文件
void decompress_file(const char *input_file, const char *output_file, const char *code_file,
                     bool decrypt) {
    clock_t start_time = clock();  // 记录解码开始时间

    long original_size;  // 原始文件大小
    DecodeEntry *table = load_code_table(code_file, &original_size);  // 加载编码表
    if (table == NULL) return;

    FILE *in = fopen(input_file, "rb");  // 以二进制读取模式打开输入文件
    if (in == NULL) {
        perror("Failed to open input file");
        free(table);
        return;
    }
    fseek(in, 0, SEEK_END);  // 将文件指针移动到文件末尾
    long file_size = ftell(in);  // 获取文件大小
    fseek(in, 0, SEEK_SET);  // 将文件指针移动到文件开头
    uint8_t *data = (uint8_t *)malloc(file_size);  // 分配内存用于存储压缩数据
    if (data == NULL) {
        perror("Memory allocation failed");
        fclose(in);
        free(table);
        return;
    }
    fread(data, 1, file_size, in);  // 读取压缩数据到缓冲区
    fclose(in);  // 关闭输入文件

    if (decrypt)
        decrypt_bytes(data, file_size, 0x55);  // 对数据进行解密

    FILE *out = fopen(output_file, "wb");  // 以二进制写入模式打开输出文件
    if (out == NULL) {
        perror("Failed to open output file");
        free(data);
        free(table);
        return;
    }

    HuffmanNode *decode_root = build_decode_tree(table, 256);  // 构建解码树

    int bit_pos = 0;  // 位位置
    HuffmanNode *current = decode_root;
    while (bit_pos < file_size * 8) {
        int bit = (data[bit_pos / 8] >> (7 - (bit_pos % 8))) & 1;
        current = bit ? current->right : current->left;
        if (!current->left && !current->right) {
            fputc(current->byte, out);
            current = decode_root;
        }
        bit_pos++;
    }

    // 显示解码时间
    clock_t end_time = clock();
    double decode_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("解码时间: %.3f秒\n", decode_time);

    fclose(out);  // 关闭输出文件
    free(data);  // 释放数据缓冲区内存
    free(table);  // 释放解码表内存
}