#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 根据给定路径，返回该路径中最后一个斜杠后的字符
// 返回值会被空格填充到DIRSIZ大小
char* fmtname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;

    // 找到最后一个斜杠后的字符
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
    p++;

    // 返回空格填充后的字符
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

void find(char *path, char *expression) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // 尝试打开文件
    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 尝试获取文件信息
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 判断文件类型
    switch(st.type){
        case T_FILE:
            // 如果是文件，则判断文件名是否匹配目标表达式
            if (strcmp(fmtname(path), expression) == 0) {
                printf("%s\n", fmtname(path));
            }
            break;

        case T_DIR:
            // 如果是目录，则依次处理目录下的文件和子目录
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("find: path too long\n");
                break;
            }
            // 构造下一级文件或目录的路径
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;
                // 将当前目录项的名称拼接到路径中
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                // 尝试获取当前目录项的信息
                if(stat(buf, &st) < 0){
                    printf("find: cannot stat %s\n", buf);
                    continue;
                }
                // 根据文件类型分别处理
                switch(st.type){
                    case T_FILE:
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
    for(int i = 1; i < argc - 1; i++)
        find(argv[i], argv[argc - 1]);

    // 退出程序
    exit(0);
}
