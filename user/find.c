#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int match(char *filepath, char *filename)
{
    char *p;

    // 查找最后一个'/'后的第一个字符
    for (p = filepath + strlen(filepath); p >= filepath && *p != '/'; p--)
        ;
    p++;

    if (strcmp(p, filename) == 0)
        return 1;
    else
        return 0;
}

void find(char *filepath, char *filename)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    //打开指定路径
    if ((fd = open(filepath, 0)) < 0)
    {
        fprintf(2, "ls: cannot open %s\n", filepath);
        return;
    }

    //获取状态（检查是文件还是路径）
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "ls: cannot stat %s\n", filepath);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        //如果给出的路径就是一个文件，则用match函数进行对比
        if (match(filepath, filename))
        {
            printf("%s\n", filepath);
        }
        break;

    case T_DIR:
        //如果给出的路径不是文件，则先检查路径长度是否超出限制
        if (strlen(filepath) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, filepath);
        p = buf + strlen(buf);
        // 给path后面添加 /
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            // 如果读取成功, 一直都会在while loop中
            if (de.inum == 0)
                continue;
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            // 把 de.name 拷贝到p中
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            find(buf, filename);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    //检查参数个数是否符合要求
    if (argc < 3)
    {
        printf("your argc number is %d and it cannot be less than 2\n", argc);
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}