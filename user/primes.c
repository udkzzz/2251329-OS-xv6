#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  //创建中间变量
  int first_prime, each;
  //创建第一个管道和之后的管道
  int first_fd[2];
  int second_fd[2];
  //为每个管道建立文件描述符（实际就是数组指针）
  int *fd1;
  int *fd2;
  fd1 = first_fd;
  fd2 = second_fd;
  //创建第一个管道
  pipe(fd1);

  //在此处创建子进程，父子进程分别执行不同的逻辑
  if (fork() > 0)//这里是父进程，初次把所有数送入管道1
  {
    for (int i = 2; i <= 35; i++)
      write(fd1[1], &i, sizeof(i));
    // 输入后关闭管道1的写端
    close(fd1[1]);
    // 等待子进程结束
    int status;
    wait(&status);
  }
  else//这里是子进程，循环处理，为素数创建新进程
  {
    while (1)
    {
      // 创建第二个管道，为第一次筛选做准备。
      pipe(fd2);
      //从管道fd1中读取第一个值(肯定是素数)并打印
      close(fd1[1]);
      if (read(fd1[0], &first_prime, sizeof(first_prime))){
        printf("prime %d\n", first_prime);
      }
      else{
        //无法读则说明管道已空，算法已经执行完毕，可以跳出循环并结束了
        break;
      }

      //在这里再创建一个子进程用于筛选一轮素数
      if (fork() > 0)//这是原先的子进程逻辑
      {
        int i = 0;
        while (read(fd1[0], &each, sizeof(each)))
        {
          // 如果可以整除，则说明该数必是素数，不必加入fd2管道中
          if (each % first_prime == 0)
            continue;
          i++;
          // 如果不可以整除，则将该值写入fd2管道中
          write(fd2[1], &each, sizeof(each));
        }
        // 关闭fd1的读端和fd2的写端，并等待子进程结束后跳出循环终止。
        close(fd1[0]);
        close(fd2[1]);
        int status;
    	wait(&status);
        break;
      }
      else//这是新的子进程
      {
        close(fd1[0]);
        //交换两个指针，以达到管道重复利用的效果
        int *tmp = fd1;
        fd1 = fd2;
        fd2 = tmp;
        //这个子进程会执行完交换后会重新回到while(1)的循环处，成为一个新的父进程，继续进程素数的输出和筛选
      }
    }
  }
  //父进程在子进程结束后退出
  exit(0);
}