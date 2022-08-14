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



int main(int argc, char *argv[])
{
    int connectfd = -1;
    int connfd = -1;
    int read_ret,send_ret,log_ret;
    int log_num;
    int time_cnt = 0;
    


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
        snprintf( buff_msg, QUECTEL_TIME_MAX_LEN, "hello ,This is %s\n", Addr );
        send(connectfd,buff_msg,QUECTEL_TIME_MAX_LEN,0);
        
    }

    char * msg =  calloc(1, QUECTEL_MSG_MAX_LEN);

    pid_t id =  fork();
    if ( id > 0 )  {
        int stat;
        
        struct timeval timeout = {
            .tv_sec = 60 ,
            .tv_usec = 0 
        }; // 设置时间结构体为20秒

        while ( 1 )
        {
            /* code */
            int ret_val = select( 0 , NULL, NULL , NULL , &timeout);
            if ( 0 == ret_val ) {
                // printf("⚠️  超时警告！！！\n");

                for (size_t i = 0; i < 3; i++){
                    /* code */
                    memset(msg, 0, sizeof(msg));
                    strcpy(msg, CLIENT_RESPOND_ALIVE);
                    send(connectfd, msg, sizeof(msg), 0);
                    sleep(1);
                }
                memset(msg, 0, sizeof(msg));
                recv(connectfd , msg , QUECTEL_MSG_MAX_LEN , 0);

                if (0 == strncmp(msg, "alive!",strlen("alive!"))){
                    printf("%s\n",msg);
                    continue;
                }
                
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
                .tv_sec = 20 ,
                .tv_usec = 0 
            }; // 设置时间结构体为20秒

            // 设置多路复用 监听标准输入以及已链接套接字   ， 超时设置为20秒
            int ret_val = select( max_fd + 1 , &set, NULL , NULL , &timeout);
            // usleep(5*1000*100);
            
            // pthread_join(tid_Beat, NULL);
            if (0 == ret_val)  // 超时
            {
                printf("⚠️  超时警告！！！\n");
                time_cnt += 1;
                if (3 == time_cnt) {
                    for (size_t i = 0; i < 5; i++)
                    {
                        memset(msg, 0, sizeof(msg));
                        strcpy(msg, CLIENT_RESPOND_ALIVE);
                        send(connectfd, msg, sizeof(msg), 0);
                        sleep(1);
                        /* code */
                    }
                    
                    
                }
                continue ;
            }else if (-1 == ret_val) // 出错
            {
                perror("select error>>>>");
                continue ;
            }

            // 检查哪个文件的数据到达
            if( FD_ISSET(connectfd , &set))
            {
                ret_val = recv(connectfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                if (0 ==  ret_val)
                {
                    // printf("🚨  服务器他，，，挂了》》》》\n");
                    printf("📢  接收到关闭指令,关闭连接...\n");
                    write(connectfd, "Thank you", 10);  //向服务器端发送表示感谢的消息，若服务器端未关闭输入流，则可接收到此消息ret_val
                    close(connectfd);

                    sleep(1);
                   exit(1);
                    // break;             
                }      

                printf("msg:%s \n" , msg);   
                // ret_val = strncmp(msg, "ok",strlen("ok"));
                // printf("ret_val: %d\n", read_ret);
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
                            ret_val = send(connectfd , msg, strlen(msg) , 0 );
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
                    continue;
                }
                else if (0 == strncmp(msg, "ok",strlen("ok"))) {
                    /* code */
                    func_window();
                    continue;
                }
                else if ( 0 == strncmp(msg, "quit",strlen("quit")) ) {
                    shutdown(connectfd, SHUT_RD); //关闭套接字的输入流,并发送返回值
                    // printf("📢  接收到关闭指令,关闭连接...\n");
                    write(connectfd, "Thank you", 10);  //向服务器端发送表示感谢的消息，若服务器端未关闭输入流，则可接收到此消息ret_val
                    exit(0);
                    continue;
                }
                    
                if (0 == strncmp(msg, "alive",sizeof("alive"))){
                
                    // printf("line:%d\n",__LINE__);
                    sprintf(msg, "alive!");
                    usleep(5*1000*10);
                    send(connectfd , msg, strlen(msg) , 0 );//响应心跳探活
                    continue;
                    // printf("line:%d\n",__LINE__);
                    // write(connectfd, msg,sizeof(msg));  //响应心跳探活
                }           
            }

            // 为所欲为 
            if(FD_ISSET(STDIN_FILENO, &set))
            {
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