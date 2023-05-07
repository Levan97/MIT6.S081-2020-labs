#include "kernel/types.h"  // 引入类型定义头文件
#include "user/user.h"     // 引入用户定义头文件

#define RD 0   // 用于读取管道符的下标
#define WR 1   // 用于写入管道符的下标

const uint INT_LEN = sizeof(int); // 整型数据长度

/**
 * @brief 读取左邻居的第一个数据
 * @param lpipe 左邻居的管道符
 * @param pfirst 用于存储第一个数据的地址
 * @return 如果没有数据返回-1,有数据返回0
 */
int lpipe_first_data(int lpipe[2], int *dst)
{
  if (read(lpipe[RD], dst, sizeof(int)) == sizeof(int)) {
    printf("prime %d\n", *dst); // 输出第一个数据
    return 0;
  }
  return -1;
}

/**
 * @brief 读取左邻居的数据，将不能被first整除的写入右邻居
 * @param lpipe 左邻居的管道符
 * @param rpipe 右邻居的管道符
 * @param first 左邻居的第一个数据
 */
void transmit_data(int lpipe[2], int rpipe[2], int first)
{
  int data;
  // 从左管道读取数据
  while (read(lpipe[RD], &data, sizeof(int)) == sizeof(int)) {
    // 将无法整除的数据传递入右管道
    if (data % first)
      write(rpipe[WR], &data, sizeof(int));
  }
  close(lpipe[RD]);  // 关闭左管道的读取端
  close(rpipe[WR]);  // 关闭右管道的写入端
}

/**
 * @brief 寻找素数
 * @param lpipe 左邻居管道
 */
void primes(int lpipe[2])
{
  close(lpipe[WR]);  // 关闭左管道的写入端
  int first;
  if (lpipe_first_data(lpipe, &first) == 0) {  // 获取第一个数据
    int p[2];
    pipe(p);  // 当前的管道
    transmit_data(lpipe, p, first);

    if (fork() == 0) {
      primes(p);  // 递归执行
    } else {
      close(p[RD]);  // 关闭当前管道的读取端
      wait(0);       // 等待子进程结束
    }
  }
  exit(0);  // 结束进程
}

int main(int argc, char const *argv[])
{
  int p[2];
  pipe(p);  // 创建管道

  for (int i = 2; i <= 35; ++i)  // 写入初始数据
    write(p[WR], &i, INT_LEN);

  if (fork() == 0) {
    primes(p);  // 开始执行
  } else {
    close(p[WR]);  // 关闭管道的写入端
    close(p[RD]);  // 关闭管道的读取端
    wait(0);       // 等待子进程结束
  }

  exit(0);
}