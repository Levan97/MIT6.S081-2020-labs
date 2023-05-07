#include "kernel/types.h"     // 包含了Unix系统调用所需的数据类型
#include "kernel/stat.h"      // 包含了Unix系统调用所需的常量
#include "user/user.h"        // 包含了Unix系统调用所需的库函数

int main(int argc, char *argv[]) {
    int fds[2];               // 创建一个长度为2的整数数组，存放管道的读写描述符
    char buf[2];              // 创建一个长度为2的字符数组，存放读写缓冲区
    pipe(fds);                // 创建一个管道，将读写描述符存入fds数组中
    if (fork() == 0) {        // 在子进程中
        read(fds[0], buf, sizeof(buf));    // 从管道的读描述符中读取数据到buf中
        printf("%d: received ping\n", getpid());  // 输出子进程的PID，表示接收到了ping
        write(fds[1], "0", 1);  // 将字符'0'写入到管道的写描述符中，表示子进程已经完成任务
    } else {                  // 在父进程中
        write(fds[1], "0", 1);  // 将字符'0'写入到管道的写描述符中，表示父进程已经完成任务
        read(fds[0], buf, sizeof(buf));  // 从管道的读描述符中读取数据到buf中
        printf("%d: received pong\n", getpid());  // 输出父进程的PID，表示接收到了pong
        write(fds[1], "0", 1);  // 将字符'0'写入到管道的写描述符中，表示父进程已经完成任务
        wait(0);               // 等待子进程结束
    }
    exit(0);                  // 退出进程
}