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
#include <netinet/tcp.h>
#include <time.h>
#include <sys/time.h>
// #include <features.h>


#include "log.h"
#include "data.h"

#define  QUECTEL_TIME_MAX_LEN     128
#define  QUECTEL_MSG_MAX_LEN      128
#define  SEND_CLIENT_LOG_OK       "log_ok\n"
#define  SEND_CLIENT_DATA_OK      "data_ok\n"
#define  SEND_CLIENT_FUNC_OK      "ok\n"

// extern int data_buf ;



int main(int argc, char *argv[])
{
    int listenfd = -1;
    int connfd = -1;

    int log_flag = 0, data_flag = 0;;
    
    char buff[QUECTEL_TIME_MAX_LEN];
    char buff_log[10][QUECTEL_MSG_MAX_LEN];
    

    time_t ticks;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int sin_size;

    //创建tcp通信socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        printf("create socket error :%s %d",strerror(errno), errno);
        exit(0);
    }
    
    //绑定地址
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(9980);

    /* client_addr.sin_family=AF_INET;
    client_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    client_addr.sin_port=htons(9981); */
    
    // 绑定
    socklen_t  addrlen  = sizeof(struct sockaddr_in);
    if(bind(listenfd, (struct sockaddr *)(&server_addr), addrlen) == -1)
    {
        printf("bind error:%s %d",strerror(errno), errno);

        exit(0);            
    }
    
    if(listen(listenfd, 5) == -1)
    {
        printf("listen error:%s %d",strerror(errno), errno);
        exit(0);
    }

    connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addrlen );
    if(connfd == -1)
    {
        printf("accept error:%s %d",strerror(errno), errno);
        exit(0);        
    } 

     //  当链接成功 则返回一个已经链接的套接字 
    printf("📢  链接成功!!!\n");
    

    // int keepAlive = 1;    // 非0值，开启keepalive属性
    // int keepIdle = 60;    // 如该连接在60秒内没有任何数据往来,则进行此TCP层的探测
    // int keepInterval = 5; // 探测发包间隔为5秒
    // int keepCount = 3;        // 尝试探测的最多次数

    //  // 开启探活
    // if (-1 ==setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive)) ) {
    //     perror("SO_KEEPALIVE fault :");
    // }  
    // if (-1 == setsockopt(listenfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle))) {
    //     perror("TCP_KEEPIDLE fault:");
    // }
    // if (-1 == setsockopt(listenfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)) ) {
    //     perror("TCP_KEEPINTVL fault:");
    // }
    // if (-1 == setsockopt(listenfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)) ) {
    //     perror("TCP_KEEPCNT fault:");
    // }
     
    char * msg =  calloc(1, QUECTEL_MSG_MAX_LEN);
    char * buff_data =  calloc(1, QUECTEL_MSG_MAX_LEN);

    P_Queue log_que = calloc(1, 1024);
   
    QueueInit(log_que);

    // log_function(log_que);

    while (1)
    {
        ticks = time(NULL);
        // snprintf( buff, QUECTEL_TIME_MAX_LEN, "%s\r\n", ctime(&ticks) );
        // printf("buff=%s\r\n",buff);

        // 把已连接套接字 + 标准输入描述符添加到 集合中
        fd_set set ; 
        FD_ZERO(&set);  // 清空集合
        FD_SET(connfd , &set); // 添加 套记字到集合中
        FD_SET(STDIN_FILENO , &set); // 添加标准输入到集合中

        memset(msg, 0 , sizeof(msg));

        // 找到描述符最大值
        int max_fd = connfd > STDIN_FILENO ? connfd : STDIN_FILENO ;

        struct timeval timeout = {
            .tv_sec = 60 ,
            .tv_usec = 0 
        }; // 设置时间结构体为60秒

        // 设置多路复用 监听标准输入以及已链接套接字   ，   超时设置为60秒
        int ret_val = select( max_fd + 1 , &set, NULL , NULL , &timeout);
        memset(buff, 0 , sizeof(buff));
        strcpy(buff, SEND_CLIENT_FUNC_OK);
        send(connfd, buff, sizeof(buff),0);
        // send(connfd, SEND_CLIENT_FUNC_OK, sizeof(SEND_CLIENT_FUNC_OK),0); 
       
        // 超时
        if (0 == ret_val) {
            printf("⚠️  连接超时！！！\n");
            memset(msg, 0 , sizeof(msg));
            sprintf(msg, "alive");
            
            write(connfd, msg, sizeof(msg));
            
            // send(connfd , msg , strlen(msg), 0 );
            ret_val = read(connfd, buff, sizeof(buff));  
            if ( ret_val < 0 ) {
                close(connfd); //关闭套接字的输出流
            }
            else {
                printf("Message from client: %s\n", buff);    //控制台打印接收到的字符串消息            //输入流仍可以接收数据
                continue ;  
            }
            
        }
        else if (-1 == ret_val) {        // 出错
            perror("select error>>>>");
            continue ;
        }
        else{
            printf("📢  有数据到达...\n");
        }

        // 检查哪个文件的数据到达
        if( FD_ISSET(connfd , &set))
        {
    
            ret_val = recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
            if (0 ==  ret_val)
            {
                printf("🚨  客户他，，，挂了》》》》\n");

                // 重新连接
                connfd = accept(listenfd , (struct sockaddr *)&client_addr, &addrlen);
                if ( -1 == connfd )
                {
                    perror("accept error>>>");
                    continue ; 
                }
                //  当链接成功 则返回一个已经链接的套接字 
                printf("📢  链接成功!!!\n");

                
            }
            printf("msg:%s \n" , msg);
            sscanf(msg, "%d", &ret_val);
            // ret_val = strcmp(msg, "1");
            printf("ret_val: %d\n", ret_val);

            
            switch (ret_val)
            {
            case 1 :
                memset(buff, 0 , sizeof(buff));
                strcpy(buff, SEND_CLIENT_LOG_OK);
                send(connfd, buff, sizeof(buff),0);
                // send(connfd, SEND_CLIENT_LOG_OK, sizeof(SEND_CLIENT_LOG_OK),0); 
                log_flag = 1;
                while ( log_flag )
                {
                    /* code */                       
                    log_window();
                    memset(msg, 0 , sizeof(msg));
                    ret_val = recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                    printf("msg:%s \n" , msg);
                    sscanf(msg, "%d", &ret_val);
                    if (1 == ret_val) {
                        recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                        // log_Load(log_que);
                        log_save(log_que, msg);
                    }
                    else if (2 == ret_val) {
                        log_Display(log_que, buff_log);
                        for (size_t i = 0; i < 10; i++){
                            /* code */
                            send(connfd, buff_log[i], sizeof(buff_log[i]),0);
                            // write(connfd, buff_log[i], sizeof(buff_log[i]));
                            // usleep(5*1000*100);
                        }
                    }
                    else if (0 == ret_val) {
                        log_flag = 0;
                        printf("quit log_func\n");
                        memset(buff, 0 , sizeof(buff));
                        strcpy(buff, SEND_CLIENT_FUNC_OK);
                        send(connfd, buff, sizeof(buff),0);
                        break; 
                    }
                    usleep(5*1000*10);

                    memset(buff, 0 , sizeof(buff));
                    strcpy(buff, SEND_CLIENT_LOG_OK);
                    send(connfd, buff, sizeof(buff),0);
                    // send(connfd, SEND_CLIENT_LOG_OK, sizeof(SEND_CLIENT_LOG_OK),0);
                    // ret_val = recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                }
                // if( FD_ISSET(connfd , &set)){
                    
                // }
            
                break;

            case 2 :
                memset(buff, 0 , sizeof(buff));
                strcpy(buff, SEND_CLIENT_DATA_OK);
                send(connfd, buff, sizeof(buff),0);
                data_flag = 1;
                while (data_flag)
                {
                    data_collection_window();
                    memset(msg, 0 , sizeof(msg));
                    ret_val = recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                    printf("msg:%s \n" , msg);
                    sscanf(msg, "%d", &ret_val);
                    switch (ret_val)
                    {
                    case 1 :
                        memset(buff_data, 0 , sizeof(buff_data));
                        printf("%d\n",__LINE__);
                        recv(connfd , buff_data , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                        printf("%d\n",__LINE__);
                        printf("msg:%s", buff_data);
                        printf("%d\n",__LINE__);
                        data_collect(buff_data);
                        break;
                    case 2:
                        median_filter();
                        break;
                    case 0:
                        data_flag = 0;
                        printf("quit data_func\n");
                        memset(buff, 0 , sizeof(buff));
                        strcpy(buff, SEND_CLIENT_FUNC_OK);
                        send(connfd, buff, sizeof(buff),0);
                        continue;
                    default:
                        break;
                    }
                    usleep(5*1000*10);
                    memset(buff, 0 , sizeof(buff));
                    strcpy(buff, SEND_CLIENT_DATA_OK);
                    send(connfd, buff, sizeof(buff),0);
                }
                
                break;
                default:
                    break;
            }
            
        }
        
        
        if( FD_ISSET(STDIN_FILENO , &set))
        {
            memset(msg, 0, sizeof(msg));
            fgets(msg , QUECTEL_MSG_MAX_LEN , stdin);  // 获取标准输入数据
            send(connfd , msg , strlen(msg), 0 ); // 发送数据
        }
                                                                                                                                                                                                    
    }

    QueueDestory(log_que);
    
    close(connfd);
    close(listenfd);
    return 0;
 
}