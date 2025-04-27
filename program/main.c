#include "huffman.h"

// 打印程序使用说明
void usage() {
    printf("Usage: program [compress|decompress] input output code sender receiver [encrypt]\n");
}

// 主函数
int main(int argc, char *argv[]) {
    if (argc < 6) {
        usage();  // 如果参数数量不足，打印使用说明
        return 1;
    }

    bool encrypt = (argc > 7 && strcmp(argv[7], "encrypt") == 0);  // 判断是否需要加密
    char *mode = argv[1];  // 操作模式（compress 或 decompress）
    char *input = argv[2];  // 输入文件路径
    char *output = argv[3];  // 输出文件路径
    char *code_file = argv[4];  // 编码表文件路径
    char *sender = argv[5];  // 发送者信息
    char *receiver = argv[6];  // 接收者信息

    // 检查附加信息格式（学号,姓名）
    if (strlen(sender) < 12 || strchr(sender, ',') == NULL ||
        strlen(receiver) < 12 || strchr(receiver, ',') == NULL) {
        fprintf(stderr, "错误：发送人/接收人信息格式应为'学号,姓名'\n");
        return 1;
    }

    if (strcmp(mode, "compress") == 0) {
        compress_file(input, output, code_file, sender, receiver, encrypt);  // 调用压缩函数
        printf("Compression successful!\n");  // 打印压缩成功信息
    } else if (strcmp(mode, "decompress") == 0) {
        // 核对接收人信息
        FILE *temp = fopen(input, "rb");
        char header[256];
        fread(header, 1, 256, temp);
        if (strstr(header, receiver) == NULL) {
            fprintf(stderr, "错误：接收人信息不匹配，解压终止\n");
            fclose(temp);
            return 1;
        }
        fclose(temp);
        printf("发送人信息：%s\n", sender);
        printf("接收人信息：%s\n", receiver);
        decompress_file(input, output, code_file, encrypt);  // 调用解压缩函数
        printf("Decompression successful!\n");  // 打印解压缩成功信息
    } else {
        usage();  // 如果操作模式无效，打印使用说明
        return 1;
    }

    return 0;
}    