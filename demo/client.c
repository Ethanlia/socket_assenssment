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

#define QUECTEL_TIME_MAX_LEN      128
#define QUECTEL_MSG_MAX_LEN       128

int main(int argc, char *argv[])
{
    int connectfd = -1;
    int connfd = -1;
    int read_ret;

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
    server_addr.sin_port=htons(9980);
    // server_addr.sin_port=inet_addr("10.55.20.110"); 
    
    client_addr.sin_family=AF_INET;
    // client_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    client_addr.sin_addr.s_addr=inet_addr("10.55.32.245");
    client_addr.sin_port=htons(9981);
    
    /* // 绑定

    if(bind(connectfd, (struct sockaddr *)(&client_addr), addrlen) == -1)
    {
        printf("bind error:%s %d",strerror(errno), errno);exit(0);
    } */
    
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
    
    

    while (1)
    {
        // 把已连接套接字 + 标准输入描述符添加到 集合中
        fd_set set ; 
        FD_ZERO(&set);  // 清空集合
        FD_SET(connectfd , &set); // 添加 套记字到集合中
        FD_SET(STDIN_FILENO , &set); // 添加标准输入到集合中

        // 找到描述符最大值
        int max_fd = connectfd > STDIN_FILENO ? connectfd : STDIN_FILENO ;

        struct timeval timeout = {
            .tv_sec = 20 ,
            .tv_usec = 0 
        }; // 设置时间结构体为20秒

        // 设置多路复用 监听标准输入以及已链接套接字   ， 超时设置为20秒
        int ret_val = select( max_fd + 1 , &set, NULL , NULL , &timeout);
        if (0 == ret_val)  // 超时
        {
            printf("⚠️  超时警告！！！\n");
            continue ;
        }else if (-1 == ret_val) // 出错
        {
            perror("select error>>>>");
            continue ;
        }
        else{
            printf("📢  有数据发出...\n");
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
                return 0;
                // break;             
            }           
        }

        // 为所欲为 接收数据
        if(FD_ISSET(STDIN_FILENO, &set))
        {
            bzero(msg , QUECTEL_MSG_MAX_LEN );
            fgets(msg , QUECTEL_MSG_MAX_LEN , stdin);

            ret_val = send(connectfd , msg, QUECTEL_MSG_MAX_LEN , 0 );
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
     
    close(connectfd);
    return 0;
}