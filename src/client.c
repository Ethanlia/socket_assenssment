/*
 * @Descripttion   : 
 * @Author         : Ethan.liang
 * @Date           : 2022-08-14 20:42:18
 * @LastEditors    : Do not Edit
 * @LastEditTime   : 2022-08-15 14:24:22
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "log.h"
#include "data.h"
#include "func.h"

#define QUECTEL_TIME_MAX_LEN      128
#define QUECTEL_MSG_MAX_LEN       128

#define INFO "请连续采集10个数据 每个数据以, 分隔 如输入值为110, 10, 1, 5, 123, 48, 1, 2, 3, 99\n"
#define CLIENT_RESPOND_ALIVE      "alive"

void Client_recv_func(int ret_val , int connectfd, char * msg ,char * Addr)
{
    
    int read_ret = -1 ,send_ret =-1 ,log_ret =-1;

    if (0 ==  ret_val)
    {
        printf("📢  接收到关闭指令,关闭连接...\n");
        write(connectfd, "Thank you", 10);  //向服务器端发送表示感谢的消息，若服务器端未关闭输入流，则可接收到此消息ret_val
        close(connectfd);

        sleep(1);
        exit(1);
        // break;             
    }      

    printf("msg:%s \n" , msg);   
    if (0 == strncmp(msg, "log_ok",strlen("log_ok"))) {
        /* code */
        log_window();  
        fgets(msg , QUECTEL_MSG_MAX_LEN , stdin);
        send(connectfd , msg, strlen(msg) , 0 );  
        sscanf(msg, "%d", &send_ret);
    
        switch (send_ret)
        {
            case 1:
                printf("请输入要保存的数:\n");
                fgets(msg , QUECTEL_MSG_MAX_LEN , stdin);
                ret_val = send(connectfd , msg, sizeof(msg) , 0 );
                if ( ret_val > 0 )
                {
                    printf("📢  成功发送%d字节\n" , ret_val );                
                }
                break;
            default: break;
        }     
    }  
    else if (0 == strncmp(msg, "data_ok",strlen("data_ok"))) {
        /* code */
        data_collection_window();   
        fgets(msg , QUECTEL_MSG_MAX_LEN , stdin);
        send(connectfd , msg, strlen(msg) , 0 );  
        sscanf(msg, "%d", &send_ret);
        switch (send_ret)
        {
            case 1 :
                printf(INFO);
                memset(msg, 0 , sizeof(msg));
                fgets(msg , QUECTEL_MSG_MAX_LEN , stdin);
                printf("msg:%s", msg);
                send(connectfd , msg, strlen(msg) , 0 );
                
            default:
                break;
        }   
        // continue;
    }
    else if (0 == strncmp(msg, "ok",strlen("ok"))) {
        /* code */
        func_window();
        snprintf( msg, QUECTEL_TIME_MAX_LEN, "hello ,This is %s\n", Addr );
        send(connectfd,msg,QUECTEL_TIME_MAX_LEN,0);
        memset(msg, 0 , sizeof(msg));
        // continue;
    }
    else if ( 0 == strncmp(msg, "quit",strlen("quit")) ) {
        shutdown(connectfd, SHUT_RD); //关闭套接字的输入流,并发送返回值
        printf("📢  接收到关闭指令,关闭连接...\n");
        write(connectfd, "Thank you", 10);  //向服务器端发送表示感谢的消息，若服务器端未关闭输入流，则可接收到此消息ret_val
        // system("killall client");
        exit(0);

    }   
}


int main(int argc, char *argv[])
{
    int connectfd = -1;

    char buff[QUECTEL_TIME_MAX_LEN];
    char buff_msg[QUECTEL_MSG_MAX_LEN];
    char Addr[QUECTEL_MSG_MAX_LEN];

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    // 创建套接字
    connectfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connectfd < 0)
    {
        printf("create socket error :%s %d",strerror(errno), errno);
        exit(0);
    }
    
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(13910);
    // server_addr.sin_port=inet_addr("10.55.36.137"); 
    
    client_addr.sin_family=AF_INET;
    // client_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    client_addr.sin_addr.s_addr=inet_addr("10.55.36.138");
    client_addr.sin_port=htons(13911);
    
    // 绑定  
    socklen_t  addrlen  = sizeof(struct sockaddr_in);
    inet_ntop (AF_INET, &client_addr.sin_addr.s_addr, Addr, sizeof(Addr));

    if(connect(connectfd, (struct sockaddr *)(&server_addr),addrlen) == -1)
    {
        printf("conect error:%s %d",strerror(errno), errno);
        exit(0);
    }else
    {
        // 链接成功那么 connectfd  变为已连接套接字
        printf("📢  链接成功!!!\n");
        printf("hello ,This is %d\n",server_addr.sin_port);       
    }

    char * msg =  calloc(1, QUECTEL_MSG_MAX_LEN);

    pid_t id =  fork();
    if ( id > 0 )  {
        sleep(1);
        int stat;
        
        struct timeval timeout = {
            .tv_sec = 30 ,
            .tv_usec = 0 
        }; // 设置时间结构体为30秒

        while ( 1 )
        {
            /* 超时检测 */
            int ret_val = select( 0 , NULL, NULL , NULL , &timeout);
            if ( 0 == ret_val ) {
                printf("⚠️  超时警告！！！\n");
                /* code */
                memset(msg, 0, sizeof(msg));
                strcpy(msg, CLIENT_RESPOND_ALIVE);
                send(connectfd, msg, sizeof(msg), 0);
                sleep(1);
            }

            ret_val = waitpid(id , &stat , WUNTRACED );
            if (ret_val > 0) {
                if (WIFEXITED(stat)) {
                    printf("子进程的退出值：%d\n" , WEXITSTATUS(stat) );
                    return 0;
                }        
            }
                
        }
       
    }
    else if ( 0 == id  )
    { 
        /* code */
        while (1)
        {
            // 把已连接套接字 + 标准输入描述符添加到 集合中
            fd_set set ; 
            FD_ZERO(&set);  // 清空集合
            FD_SET(connectfd , &set); // 添加 套记字到集合中
            FD_SET(STDIN_FILENO , &set); // 添加标准输入到集合中

            memset(msg, 0 , sizeof(msg));
            // 找到描述符最大值
            int max_fd = connectfd > STDIN_FILENO ? connectfd : STDIN_FILENO ;

            struct timeval timeout = {
                .tv_sec = 60 ,
                .tv_usec = 0 
            }; // 设置时间结构体为20秒

            // 设置多路复用 监听标准输入以及已链接套接字   ， 超时设置为20秒
            int ret_val = select( max_fd + 1 , &set, NULL , NULL , &timeout);

            // 超时
            if (0 == ret_val) {  
                printf("⚠️  超时警告！！！\n输入任意值唤醒\n");
                continue ;
            }
            // 出错
            else if (-1 == ret_val)  {
                perror("select error>>>>");
                continue ;
            }

            // 检查哪个文件的数据到达
            if( FD_ISSET(connectfd , &set)) {
            int ret_val = recv(connectfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                Client_recv_func(ret_val, connectfd, msg, Addr);          
            }

            // 为所欲为 发送数据
            if(FD_ISSET(STDIN_FILENO, &set)) {
                memset(msg , 0, QUECTEL_MSG_MAX_LEN );
                fgets(msg , QUECTEL_MSG_MAX_LEN , stdin);

                ret_val = send(connectfd , msg, strlen(msg) , 0 );
                if ( ret_val > 0 )
                {
                    printf("📢  成功发送%d字节\n" , ret_val );                
                }
                else{
                    perror("send  error>>");
                    continue ;
                }           
            }

        }
    }
    
    close(connectfd);
    exit(2);
}