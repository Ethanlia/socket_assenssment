/*
 * @Descripttion   : 
 * @Author         : Ethan.liang
 * @Date           : 2022-08-05 10:21:23
 * @LastEditors    : Do not Edit
 * @LastEditTime   : 2022-08-05 14:55:59
 */


#include "func.h"
#include "log.h"
#include "data.h"

int func_flag = 0;

int main(int argc, char const *argv[])
{
     while (1)
    {
        /* 初始化log队列 */
        P_Queue log_que = calloc(1, 512);
        QueueInit(log_que);

        /* 获取功能 */
        func_window();
        scanf("%d", &func_flag);
        getchar();
        printf("puts:%d\n", func_flag);
      

        switch ( func_flag )
        {
            case 0:
                    QueueDestory(log_que);
                    return 0;
                // break;

            case 1:
                    log_function(log_que);
                    
                continue;

            case 2:
                    data_collection();
                continue;
            
            default:
                    printf("请重新输入!\n");
                break;
        }
        
    }

    return 0;
}
