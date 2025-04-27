#include "huffman.h"

// 将编码表保存到文件中
void save_code_table(CodeEntry *code_table, int size, long original_size, const char *filename) {
    FILE *file = fopen(filename, "w");  // 以写入模式打开文件
    if (file == NULL) {
        perror("Failed to open code table file");
        return;
    }
    fprintf(file, "%ld\n", original_size);  // 写入原始文件大小
    for (int i = 0; i < size; i++) {
        fprintf(file, "0x%02x %d ", code_table[i].byte, code_table[i].code_length);  // 写入字节值和编码长度
        int padding = 8 - (code_table[i].code_length % 8);  // 计算填充位数
        if (padding == 8) padding = 0;
        uint8_t byte = 0;
        for (int j = 0; j < code_table[i].code_length; j++) {
            byte <<= 1;  // 左移一位
            byte |= (code_table[i].code[j] == '1');  // 将编码位添加到字节中
        }
        byte <<= padding;  // 填充零
        fprintf(file, "0x%02x\n", byte);  // 写入编码字节
    }
    fclose(file);  // 关闭文件
}

// 压缩文件
void compress_file(const char *input_file, const char *output_file, const char *code_file,
                   const char *sender, const char *receiver, bool encrypt) {
    FILE *in = fopen(input_file, "rb");  // 以二进制读取模式打开输入文件
    if (in == NULL) {
        perror("Failed to open input file");
        return;
    }
    fseek(in, 0, SEEK_END);  // 将文件指针移动到文件末尾
    long original_size = ftell(in);  // 获取文件大小
    fseek(in, 0, SEEK_SET);  // 将文件指针移动到文件开头

    char header[256];  // 定义头部信息缓冲区
    snprintf(header, sizeof(header), "发件人：%s\n收件人：%s\n", sender, receiver);  // 格式化头部信息
    size_t header_size = strlen(header);  // 获取头部信息长度

    uint8_t *data = (uint8_t *)malloc(header_size + original_size);  // 分配内存用于存储数据和头部信息
    if (data == NULL) {
        perror("Memory allocation failed");
        fclose(in);
        return;
    }
    fread(data + header_size, 1, original_size, in);  // 读取文件内容到数据缓冲区
    fclose(in);  // 关闭输入文件
    memcpy(data, header, header_size);  // 将头部信息复制到数据缓冲区

    CodeEntry original_code_table[256] = {0};
    if (encrypt) {
        printf("启用0x55偏移加密，新编码表生成中...\n");
        // 先统计原始频率和生成原始编码表
        Frequency freq[256] = {0};
        for (size_t i = 0; i < header_size + original_size; i++)
            freq[data[i]].frequency++;
        int n = 0;
        Frequency unique_freq[256];
        for (int i = 0; i < 256; i++)
            if (freq[i].frequency > 0) {
                unique_freq[n].byte = (uint8_t)i;
                unique_freq[n].frequency = freq[i].frequency;
                n++;
            }
        heap_sort(unique_freq, n);
        HuffmanNode *original_root = build_huffman_tree(unique_freq, n);
        char **original_codes = (char **)malloc(256 * sizeof(char *));
        generate_codes(original_root, original_codes, 0, original_code_table);
        // 加密
        encrypt_bytes(data, header_size + original_size, 0x55);
    }

    Frequency freq[256] = {0};  // 初始化频率数组
    for (size_t i = 0; i < header_size + original_size; i++)
        freq[data[i]].frequency++;  // 统计每个字节的频率

    int n = 0;  // 唯一字节的数量
    Frequency unique_freq[256];  // 存储唯一字节的频率数组
    for (int i = 0; i < 256; i++)
        if (freq[i].frequency > 0) {
            unique_freq[n].byte = (uint8_t)i;  // 存储唯一字节值
            unique_freq[n].frequency = freq[i].frequency;  // 存储该字节的频率
            n++;  // 唯一字节数量加 1
        }
    heap_sort(unique_freq, n);  // 对唯一字节的频率数组进行排序
    HuffmanNode *root = build_huffman_tree(unique_freq, n);  // 构建哈夫曼树
    if (root == NULL) {
        free(data);
        return;
    }

    // 计算并显示哈夫曼树WPL
    uint64_t wpl = calculate_wpl(root, 0);
    printf("霍夫曼树WPL: %lu\n", wpl);

    char **codes = (char **)malloc(256 * sizeof(char *));  // 分配内存用于存储编码
    if (codes == NULL) {
        perror("Memory allocation failed for codes");
        free(data);
        return;
    }
    CodeEntry code_table[256];  // 编码表
    generate_codes(root, codes, 0, code_table);  // 生成编码表

    if (encrypt) {
        show_code_table_diff(original_code_table, code_table);
    }

    save_code_table(code_table, n, original_size, code_file);  // 保存编码表到文件

    FILE *out = fopen(output_file, "wb");  // 以二进制写入模式打开输出文件
    if (out == NULL) {
        perror("Failed to open output file");
        free(data);
        return;
    }
    uint8_t *compressed_data = NULL;
    int compressed_bit_count = 0;
    for (size_t i = 0; i < header_size + original_size; i++) {
        for (int j = 0; j < n; j++)
            if (code_table[j].byte == data[i]) {
                for (int k = 0; k < code_table[j].code_length; k++)
                    append_bit(&compressed_data, &compressed_bit_count, (code_table[j].code[k] == '1'));  // 将编码位添加到缓冲区
                break;
            }
    }
    flush_buffer(&compressed_data, &compressed_bit_count, out);

    // 显示压缩后字节数和最后16字节
    long compressed_size = ftell(out);
    printf("压缩后字节数: %ld\n", compressed_size);
    printf("最后16字节HEX值: ");
    for (int i = 0; i < (compressed_size < 16 ? compressed_size : 16); i++) {
        printf("0x%02x ", compressed_data[compressed_size - (compressed_size < 16 ? compressed_size : 16) + i]);
    }
    printf("\n");

    // 计算并显示压缩文本HASH值
    uint64_t hash = fnv1a_64(compressed_data, compressed_size);
    printf("压缩文本HASH值: 0x%016lx\n", hash);

    fclose(out);  // 关闭输出文件
    free(data);  // 释放数据缓冲区内存
    // 释放编码表内存
    for (int i = 0; i < n; i++) {
        free(code_table[i].code);
    }
    free(codes);
    // 释放哈夫曼树内存
    free(root);
}    