/*
 * @Descripttion   : 
 * @Author         : Ethan.liang
 * @Date           : 2022-07-24 21:17:28
 * @LastEditors    : Do not Edit
 * @LastEditTime   : 2022-08-06 19:01:21
 */
#ifndef 	__LOG_H__
#define		__LOG_H__

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>


typedef int  dataType;

#define QUEUEMAX 10
#define MAX_DATA 1024

// char log_buffer[100][1024] = {0};
typedef struct QueueNode
{
    dataType _data; //用来存储数据
    dataType _index;
    
    char    _time[512];
    
    
    struct QueueNode *_next; //用来指向下一个结构体
} QueueNode, *P_Node;

typedef struct Queue
{
    QueueNode *_flag; //读取文件标志
    QueueNode *_head; //存放整个队列的队头
    QueueNode *_tail; //存放整个队列的队尾
} Queue, *P_Queue;

//初始化
P_Queue QueueInit(P_Queue log_que);

//销毁
P_Queue QueueDestory(P_Queue log_que);

//入队
P_Queue QueuePush(P_Queue log_que, dataType log_num) ;

//出队
P_Queue QueuePop(P_Queue log_que);

//访问队首的元素
dataType QueueFront(Queue *log_que);

//访问对尾的元素
dataType QueueBack(Queue *log_que);

//返回1是空，返回0是非空
dataType QueueEmpty(Queue *log_que);

//获取数据的个数
dataType QueueSize(Queue *log_que);

// 读取log文件
dataType log_Load(Queue *log_que);

// 保存log文件
P_Queue log_save(P_Queue log_que, char * num);

// log加载界面
dataType log_window();

// 
P_Queue log_Display(P_Queue log_que, char (*linebuffer)[128]);

int log_function(P_Queue log_que);

#endif