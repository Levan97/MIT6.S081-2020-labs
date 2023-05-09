#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 根据给定路径，返回该路径中最后一个斜杠后的字符
// 返回值会被空格填充到DIRSIZ大小
char* fmtname(char *path)
{
    static char buf[DIRSIZ+1];  // 静态字符数组，用于保存填充后的文件名
    char *p;                   // 定义指针 p

    // 找到最后一个斜杠后的字符
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    //没有斜杠，p位于path前一位，++指向文件名第一个字符
    //有斜杠，p位于斜杠处，++指向斜杠下一个字符，即文件名第一个字符
    p++;  // 跳过斜杠，p 指向文件名第一个字符

    // 返回空格填充后的字符
    if(strlen(p) >= DIRSIZ)
        return p;

    // 如果文件名长度小于 DIRSIZ，则将其复制到 buf 中，并用空格填充 buf 的剩余部分
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

void find(char *path, char *expression) {
    char buf[512], *p; //定义一个缓冲区和一个指针p
    int fd; //文件描述符    
    struct dirent de; //目录项结构体
    struct stat st; //文件信息结构体

    // 尝试打开文件
    if((fd = open(path, 0)) < 0){ //打开文件，如果失败
        fprintf(2, "find: cannot open %s\n", path); //在标准错误输出流上输出错误信息
        return; //返回
    }

    // 尝试获取文件信息
    if(fstat(fd, &st) < 0){ //获取文件信息，如果失败
        fprintf(2, "find: cannot stat %s\n", path); //在标准错误输出流上输出错误信息
        close(fd); //关闭文件描述符
        return; //返回
    }

    // 判断文件类型
    switch(st.type){ //根据文件类型分别处理
        case T_FILE: //如果是文件
            // 如果是文件，则判断文件名是否匹配目标表达式
            if (strcmp(fmtname(path), expression) == 0) { //如果文件名与目标表达式相同
                printf("%s\n", fmtname(path)); //输出文件名
            }
            break;

        case T_DIR: //如果是目录
            // 如果是目录，则依次处理目录下的文件和子目录
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){ //如果文件名太长
                printf("find: path too long\n"); //在标准输出流上输出错误信息
                break; //退出循环
            }
            // 构造下一级文件或目录的路径
            strcpy(buf, path); //将当前路径复制到缓冲区中
            p = buf+strlen(buf); //指针p指向路径字符串的结尾
            *p++ = '/'; //在路径字符串结尾处添加斜杠
            while(read(fd, &de, sizeof(de)) == sizeof(de)){ //读取目录项
                if(de.inum == 0) //如果目录项的inode号为0，则说明该目录项无效
                    continue; //跳过该目录项
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) //如果是当前目录或上级目录
                    continue; //跳过该目录项
                // 将当前目录项的名称拼接到路径中
                memmove(p, de.name, DIRSIZ); //将目录项的名称拼接到路径中
                p[DIRSIZ] = 0; //路径结尾添加结束符
                // 尝试获取当前目录项的信息
                if(stat(buf, &st) < 0){ //获取当前目录项的信息，如果失败
                    printf("find: cannot stat %s\n", buf); //在标准输出流上输出错误信息
                    continue; //跳过该目录项
                }
                // 根据文件类型分别处理
                switch(st.type){ //根据文件类型分别处理
                    case T_FILE: //
                        // 如果是文件，则判断文件名是否匹配目标表达式
                        if (strcmp(de.name, expression) == 0) {
                            printf("%s\n", buf);
                        }
                        break;
                    case T_DIR:
                        // 如果是目录，则递归处理子目录
                        find(buf, expression);
                        break;
        }
    }
    break;
    }
close(fd);
}

int main(int argc, char *argv[]) {
    // 检查参数个数是否正确
    if (argc != 3) {
        printf("Usage: find [path] [expression]");
        exit(0);
    }

    // 遍历指定的目录树
    // 1 to argc-2， all is dir， argc-1 is filename
    for(int i = 1; i < argc - 1; i++)
        find(argv[i], argv[argc - 1]);

    // 退出程序
    exit(0);
}
