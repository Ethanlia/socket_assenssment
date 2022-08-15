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
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>

#include "log.h"
#include "data.h"

#define INFO "请连续采集10个数据 每个数据以, 分隔 如输入值为110, 10, 1, 5, 123, 48, 1, 2, 3, 99\n"


#define  QUECTEL_TIME_MAX_LEN     128
#define  QUECTEL_MSG_MAX_LEN      128
#define  SEND_CLIENT_LOG_OK       "log_ok\n"
#define  SEND_CLIENT_DATA_OK      "data_ok\n"
#define  SEND_CLIENT_FUNC_OK      "ok\n"
#define  SEND_CLIENT_FUNC_QUIT    "quit\n"


void func_select(int connfd , int listenfd , P_Queue Queue  )
{
    char * msg =  calloc(1, QUECTEL_MSG_MAX_LEN);

    char  msg_data[QUECTEL_TIME_MAX_LEN];
    char buff[QUECTEL_TIME_MAX_LEN];
    char buff_log[10][QUECTEL_MSG_MAX_LEN];

    int ret_val = recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
    int log_flag = 0, data_flag = 0;
    int data_buf [10] ;
    P_Queue log_que = Queue;
    struct sockaddr_in client_addr;

    /* 退出提示 */
    if (0 ==  ret_val)
    {
        printf("🚨  客户他，，，挂了》》》》\n");
        system("echo -e 'TCP Client quit at' $(date '+%Y-%m-%d %H:%M:%S') >> ../Connet.log");

        socklen_t  addrlen  = sizeof(struct sockaddr_in);
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        // // 重新连接
        // connfd = accept(listenfd , (struct sockaddr *)&client_addr, &addrlen);
        // if ( -1 == connfd )
        // {
        //     perror("accept error>>>");
        
        // }
        // //  当链接成功 则返回一个已经链接的套接字 
        // printf("📢  链接成功!!!\n"); 
        close(connfd);  
        system("killall server");
        exit(0);            
    }
    printf("msg:%s \n" , msg);
    sscanf(msg, "%d", &ret_val); 

    switch (ret_val) {
        /* log */
        case 1 :
            memset(buff, 0 , sizeof(buff));
            strcpy(buff, SEND_CLIENT_LOG_OK);
            send(connfd, buff, sizeof(buff),0);
            log_flag = 1;
            while ( log_flag )
            {                     
                log_window();
                memset(msg, 0 , sizeof(msg));
                ret_val = recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                printf("msg:%s \n" , msg);
                sscanf(msg, "%d", &ret_val);
                /* 保存log */
                if (1 == ret_val) {
                    recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                    // log_Load(log_que);
                    log_save(log_que, msg);
                    
                }
                /* 打印log */
                else if (2 == ret_val) {
                    log_Display(log_que, buff_log);
                    for (size_t i = 0; i < 10; i++){
                        /* code */
                        send(connfd, buff_log[i], sizeof(buff_log[i]),0);;
                    }              
                }
                /* 退出 */
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
                           
                }            
            break;
        /* 数据采集 */
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
                switch (ret_val) {
                case 1 :
                    printf(INFO);   
                    memset(msg_data, 0 , sizeof(msg_data));
                    recv(connfd , msg_data , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                    printf("msg:%s", msg_data);
                    data_collect(msg_data, data_buf);
                    send(connfd, buff, sizeof(buff),0);
                    break;
                case 2:
                    median_filter(data_buf);
                    sprintf(msg_data, "最大值：%d;\n最小值:%d;\n中值:%.3f\n",
                        get_max(data_buf, 10), get_min(data_buf, 10), 
                        get_average(data_buf, 10));
                    send(connfd, msg_data, sizeof(msg_data),0);
                    send(connfd, buff, sizeof(buff),0);
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
                
            }  
            break;
        /* 退出程序 */
        case 0 :
            system("echo -e 'TCP Client quit at' $(date '+%Y-%m-%d %H:%M:%S') >> ../Connet.log");
            // shutdown(connfd, SHUT_WR); //关闭套接字的输出流,并发送返回值
            // send(connfd , msg , strlen(msg), 0 );
            memset(buff, 0 , sizeof(buff));
            strcpy(buff, SEND_CLIENT_FUNC_QUIT);
            send(connfd, buff, sizeof(buff),0);
            read(connfd, buff, sizeof(buff)); 
            printf("Message from client: %s\n", buff);    //控制台打印接收到的字符串消息            //输入流仍可以接收数据
            sleep(1);
            exit(1);
            // continue ;
            break;
        
        default:
            break;
    }
            
}

int main(int argc, char *argv[])
{
    int listenfd = -1;
    int connfd = -1;

    int log_flag = 0, data_flag = 0;
    
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
    // server_addr.sin_port=inet_addr("10.55.36.137"); 
    server_addr.sin_port=htons(13910);
    
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
    
    /* 传输层发送心跳包 */
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

    
    pid_t id =  fork();
    /* 父进程：功能处理 */
    if (id > 0){
        
        strcpy(buff, SEND_CLIENT_FUNC_OK);
        send(connfd, buff, sizeof(buff),0);
        memset(buff, 0 , sizeof(buff));
        while (1)
        {
            ticks = time(NULL);
        
            // 把已连接套接字 + 标准输入描述符添加到 集合中
            fd_set set ; 
            FD_ZERO(&set);  // 清空集合
            FD_SET(connfd , &set); // 添加 套记字到集合中

            memset(msg, 0 , sizeof(msg));

            struct timeval timeout = {
                .tv_sec = 20 ,
                .tv_usec = 0 
            }; // 设置时间结构体为60秒

            // 设置多路复用 监听标准输入以及已链接套接字   ，   超时设置为60秒
            int ret_val = select( connfd + 1 , &set, NULL , NULL , &timeout);
            
            // send(connfd, SEND_CLIENT_FUNC_OK, sizeof(SEND_CLIENT_FUNC_OK),0); 
        
            // 超时
            if (0 == ret_val) {
                printf("⚠️  超时警告！！！\n");
                
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
                func_select(connfd, listenfd, log_que);
                
            }                                                                                                                                                                                                        
        }
        QueueDestory(log_que);

        close(connfd);
        close(listenfd);
        exit(0);
       
    }
    /* 子进程：监听心跳 */
    else if (id == 0 ) {    
        while (1)
        {
            sleep(1);
            /*父进程退出时，会收到SIGKILL信号*/
            prctl(PR_SET_PDEATHSIG,SIGKILL);
            // ticks = time(NULL);
            int stat;
             // 把已连接套接字 + 标准输入描述符添加到 集合中
            fd_set set ; 
            FD_ZERO(&set);  // 清空集合
            FD_SET(connfd , &set); // 添加 套记字到集合中

            struct timeval timeout = {
                .tv_sec = 60 ,
                .tv_usec = 0 
            }; // 设置时间结构体为60秒
            
            // 设置多路复用 监听标准输入以及已链接套接字   ，   超时设置为60秒
            int ret_val = select(connfd +1, &set , NULL , NULL , &timeout);
            
            memset(msg, 0 , sizeof(msg));
            if (0 == ret_val) { // 超时
                system("echo -e 'TCP Client quit at' $(date '+%Y-%m-%d %H:%M:%S') >> ../Connet.log");
            }

             // 检查哪个文件的数据到达
            if( FD_ISSET(connfd , &set)) {
                recv(connfd , msg , QUECTEL_MSG_MAX_LEN , 0); // 从套接字获得数据并发送
                // printf("msg:%s \n" , msg);
                int ret_val = strncmp(msg, "alive",strlen("alive"));
                // printf("ret = %d\n", ret_val);
                if (0 == strncmp(msg, "alive",strlen("alive"))) {                
                    printf("line:%d\n",__LINE__);
                    printf( "alive!\n");
                }
            }
          
        }   
    
    }
    return 0;
}