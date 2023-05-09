#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

void do_command(int argc, char *argv[]) {
  // 定义变量
  int find = 0, pid = 0;

  // 查找管道符位置
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "|") == 0) {
      find = i + 1;
      break;
    }
  }

  int fds[2];
  if (find > 0) { // 如果存在管道符，创建管道并在子进程中递归执行后半段命令
    pipe(fds);    // 创建管道
    pid = fork(); // 创建子进程
    if (pid == 0) {  // 子进程中递归执行后半段命令
      close(0);      //关闭键盘输入
      dup(fds[0]);   //子进程复制读文件描述符
      close(fds[0]); //关闭父进程读，防止干扰
      close(fds[1]); //关闭父进程写，防止干扰
      do_command(argc - find, argv + find);
      exit(0);
    } else {         // 父进程中将输出重定向到管道中
      close(1);      //关闭显示器输出
      dup(fds[1]);   //复制写管道符，用于写道管道
      close(fds[0]); //关闭子进程读，防止干扰
      close(fds[1]); //关闭子进程写，防止干扰
    }
  } else {       // 如果不存在管道符，直接执行当前命令
    find = argc; //没有管道符即执行xargs，将find直接复制为argc
  }

  // 如果是 xargs 命令，逐行读取标准输入并执行指定的命令
  if (strcmp(argv[0], "xargs") == 0) {
    char *params[MAXARG] = {0};                            // 定义参数列表
    memcpy(params, argv + 1, sizeof(char *) * (find - 1)); // 复制命令参数
    char buf[256];
    params[find - 1] =
        buf; // 将标准输入作为最后一个参数，存入buf的地址。因此往buf里面写入等价于往params里面写入
    int i = 0;
    while (read(0, &buf[i], sizeof(char)) > 0) { // 逐行读取标准输入
      if (buf[i++] == '\n') {                    // 如果读到一行末尾
        buf[i - 1] = '\0'; // 将末尾换行符替换为字符串结束符
        if (fork() == 0) { // 子进程中执行指定的命令
          exec(params[0], params);
        }
        wait(0); // 等待子进程结束
        i = 0;   // 重置缓冲区指针
      }
    }
  }

  // 等待子进程结束
  if (pid > 0)
    wait(0);
}

int main(int argc, char *argv[]) {
  // 如果命令参数为空，直接退出
  if (argc < 1) {
    exit(0);
  }
  // 执行命令
  do_command(argc, argv);
  // 退出程序
  exit(0);
}
