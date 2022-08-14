#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    pid_t id =  fork();
    if (id > 0)
    {
        int stat;
        printf("这里是 %d \n" , getpid());   
        int ret_val = waitpid(id , &stat , WUNTRACED );
        if (ret_val > 0)
        {
            if (WIFEXITED(stat))
            {
                printf("子进程的退出值：%d\n" , WEXITSTATUS(stat) );
            }
            
        }
        
    }
    else if (id == 0 )
    {
        printf("__%d__%s\n" , __LINE__ , __FUNCTION__ );
        // 使用exec 函数簇来覆盖子进程的整个进程空间 （不会从父进程中继承）
        execl("./output", "output" , "Helo " , "ERven",NULL);
        printf("__%d__%s\n" , __LINE__ , __FUNCTION__ );

        // execl("/bin/ls", "ls" , "-l" , "-a",NULL); // 使用exec函数簇来加载一个命令到子进程中
    }
    
    return 0;
}


