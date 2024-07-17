#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  //为子进程和父进程分别创建管道，以达到双向通信。同时对管道是否建立成功检查。
  int parent[2];//父->子
  int child[2]; //子->父
  if (pipe(parent) == -1 || pipe(child) == -1) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  //用fork创建当前父进程下的子进程，并获取子进程的pid
  int pid=fork();

  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0)//表示进入子进程
  {
    //读入的字节的存储结构(大小为1)
    char buf[10];
    //子进程读入字节（读的是父传子的那个管道的读端parent[0]）并打印
    read(parent[0], buf, sizeof buf);
    int id = getpid();
    printf("%d: received %s\n", id, buf);
    //向父进程写一个“pong”
    write(child[1], "pong", 4);
    //使用完毕后把管道端口关闭
    close(parent[0]);
    close(child[1]);
  }
  else//表示父进程
  {
    //读入的字节的存储结构(大小为1)
    char buf[10];
    //向子进程写一个“ping”
    write(parent[1], "ping", 4);	
    //确保父进程读之前子进程已经结束，保证进程间的同步！！！
    int status;
    wait(&status); 
    //父进程读入字节（读的是子传父的那个管道的读端child[0]）并打印
    read(child[0], buf, sizeof buf);
    int id = getpid();
    printf("%d: received %s\n", id, buf);
    //使用完毕后把管道端口关闭
    close(parent[1]);
    close(child[0]);
  }

  exit(0);
}