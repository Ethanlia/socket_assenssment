/*
 * @Descripttion   : log function
 * @Author         : Ethan.liang
 * @Date           : 2022-07-18 09:20:36
 * @LastEditors:    
 * @LastEditTime: 2022-07-25 03:23:38
 */
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h>
#include "log.h"

dataType log_ret = -1;
dataType log_num = -1;
dataType log_cnt = 1;
dataType log_index = 0;
dataType log_sel = -1;
dataType line_len = 0;
dataType len = 0;

char str_temp[512] = {0};
char Date[512] = {0};

char linebuffer[QUEUEMAX][128]= {0};

#define LOGPATH "../log.txt"

uint64_t GetSysTimeCnt_ms(void)
{
    struct timeval tmp;
    gettimeofday(&tmp, NULL);
    return (tmp.tv_sec * 1000 + tmp.tv_usec / 1000);
}

int shell_cmd_send(char *cmd, char *res_vec, uint32_t len, uint32_t timeout)
{

    FILE  *fp = NULL;
    uint64_t timeout_tick = 0;

    if (cmd == NULL) {
        printf("popen cmd is NULL");    
        return -1;
    }

    fp = popen(cmd, "r");
    if (NULL == fp) {
        printf("exec cmd failed");
        return -1;
    }

    //清空缓冲区
    memset(res_vec, 0, len);
    timeout_tick = GetSysTimeCnt_ms();

    //收返回值
    while (fgets(&res_vec[strlen(res_vec)], len - strlen(res_vec) - 1, fp) != NULL)
    {
        //检查超时
        if (GetSysTimeCnt_ms() - timeout_tick > timeout) {
            break;
        }
        usleep(1000);
    }

    pclose(fp);
    return strlen(res_vec);
}

/**
 * @name         : QueueInit
 * @brief        :  初始化队列
 * @param         {P_Queue} log_que
 * @return        {P_Queue} log_que
 */
P_Queue QueueInit(P_Queue log_que)
{
    assert(log_que);                        //判断指针的有效性
                                            // log_que->_head->_index = 1;
    log_que->_head = log_que->_tail = NULL; //队头和队尾指向空指针
    log_que->_flag = 0;

    return log_que;
}

/**
 * @name         : QueueDestory
 * @brief        : 销毁队列
 * @param         {P_Queue} log_que
 * @return        {P_Queue}log_que
 */
P_Queue QueueDestory(P_Queue log_que)
{
    assert(log_que);
    QueueNode *cur = log_que->_head;
    while (cur)
    {
        QueueNode *next = cur->_next;
        free(cur);
        cur = next;
    }
    log_que->_head = log_que->_tail = NULL;

    return log_que;
}


/**
 * @name         : QueuePush
 * @brief        : 入队
 * @param         {P_Queue} log_que
 * @param         {dataType} log_num
 * @return        {P_Queue}log_que
 * @note         : 有BUG，入队时QueuePush写入index会导致QueueInit失败，只能在QueuePush函数外赋值，可能QueueInit写得有点问题
 */
P_Queue QueuePush(P_Queue log_que, dataType log_num)
{
    assert(log_que);
    P_Node newnode = (P_Node)malloc(1024); //为新节点申请内存空间
    if (newnode == NULL)                             //判断内存空间是否申请成功
    {
        printf("内存不足!\n");
        exit(-1);
    }
    
    newnode->_data = log_num;   //新节点储存数据
    newnode->_next = NULL;      //新节点的下一个指向NULL，即新节点作为队尾
    if (log_que->_head == NULL) //将新节点入队
    {
        log_que->_head = log_que->_tail = newnode;
    }
    else
    {
        log_que->_tail->_next = newnode;
        log_que->_tail = newnode;
    }
    return log_que;
}

/**
 * @name         : QueuePop
 * @brief        : 出队
 * @param         {P_Queue} log_que
 * @return        {P_Queue} log_que
 */
P_Queue QueuePop(P_Queue log_que)
{
    assert(log_que);
    assert(log_que->_head);

    if (log_que->_head == NULL)
    {
        // printf("%d\n",__LINE__);
        free(log_que->_head);
        log_que->_head = log_que->_tail = NULL;
    }
    else
    {
        QueueNode *next = log_que->_head->_next;
        // printf("%d\n",__LINE__);

        free(log_que->_head);
        // printf("%d\n",__LINE__);
        log_que->_head = next;
    }
    // printf("%d\n",__LINE__);

    
    return log_que;
}


/**
 * @name         : QueueFront
 * @brief        : 访问队首的元素
 * @param         {Queue} *log_que
 * @return        {Queue}  *(log_que->_head->_data)
 */
dataType QueueFront(Queue *log_que)
{
    assert(log_que);
    assert(log_que->_head);
    return log_que->_head->_data;
}

/**
 * @name         : QueueBack
 * @brief        : 访问队尾的元素
 * @param         {Queue} *log_que
 * @return        {Queue}  *(log_que->_tail->_data)
 */
dataType QueueBack(Queue *log_que)
{
    assert(log_que);
    assert(log_que->_tail);
    return log_que->_tail->_data;
}

/**
 * @name         : QueueEmpty
 * @brief        : 返回1是空，返回0是非空
 * @param         {Queue} *log_que
 * @return        1 --> log_que->_head == NULL,0 --> log_que->_head != NULL
 */
dataType QueueEmpty(Queue *log_que)
{
    assert(log_que);
    return log_que->_head == NULL ? 1 : 0;
}

// 获取数据的个数
/**
 * @name         : QueueSize
 * @brief        : 获取数据的个数
 * @param         {Queue} *log_que
 * @return        {int} size
 */
dataType QueueSize(Queue *log_que)
{
    assert(log_que);
    QueueNode *cur = log_que->_head;
    int size = 0;
    while (cur)
    {
        ++size;
        cur = cur->_next;
    }
    return size;
}

char delchar( char *str, char c )
{
    int i,j;
    for(i=j=0;str[i]!='\0';i++) {
        //判断是否有和待删除字符一样的字符
        if(str[i]!=c) {
            str[j++]=str[i];
        }
    }
    str[j]='\0';//字符串结束

    return *str;
}

/**
 * @name         : log_Load
 * @brief        : 加载log文件
 * @param         {dataType} log_index
 * @return        {int} log_index
 */
dataType log_Load(Queue *log_que )
{
    FILE *fp;
    
    Queue * que = (Queue *) log_que;
    dataType  time = 0;
    dataType  ret = 0;
    dataType  load_date[256]  = {0};
    dataType  load_time[256]  = {0};
    dataType  num ; 


    // char *p;
    char date_tmp[128] = {"0"};
    char time_tmp[128] = {"0"};
    // log_que = calloc(1, 1024);
    // P_Queue date = (P_Queue) log_que;
    
    if ( NULL != que->_flag) {
        return que->_tail->_index;
    }

    printf("reloading...\n");
    
    dataType n = 0;
    
    fp = fopen(LOGPATH, "a+");
       

    char *buff = (char *)malloc(64 * 64);

    /* 读取时间戳 */
    while ( fgets(buff, 63, fp) )
    {
        printf("buff:%s\n",buff);
        // printf("%d\n",__LINE__);
        sscanf(buff, "%s %s %s %s %s %d", str_temp, date_tmp, time_tmp, str_temp, str_temp, &num);
        printf("str_temp:%s, date_tmp:%s, time_tmp:%s, num:%d\n ", str_temp, date_tmp, time_tmp, num);
        
        QueuePush(que, num);
        que->_tail->_data = num;

        memset(log_que->_tail->_time, 0, sizeof(log_que));
        sprintf(log_que->_tail->_time, "[ %s %s ]", date_tmp, time_tmp);
        printf("time: %s\n",  log_que->_tail->_time);
        printf("QueueFront:%s, num:%d\n", log_que->_head->_time, QueueFront(log_que));
        
        date_tmp[strlen(date_tmp)] = delchar(date_tmp, '-');
        time_tmp[strlen(time_tmp)] = delchar(time_tmp, ':');
        printf("date_tmp:%s,time_tmp:%s\n", date_tmp, time_tmp);

        sscanf(date_tmp, "%d",  &(load_date[n]));
        sscanf(time_tmp, "%d",  &(load_time[n]));
        printf("date_tmp:%d,time_tmp:%d\n", load_date[n], load_time[n]);

        n ++;

        memset(buff, 0, sizeof(buff));
    }

    free(buff);
    ret = fclose(fp);
    if ( EOF == ret)
    {
        perror("fclose error>>>>");
    }

    (que->_flag) = que->_head;
    int min=0 , cnt = 0;

    /* 寻找最小时间戳 */
    for (u_int32_t i = 0; i < QUEUEMAX; i++){
        /* 从第二位数开始判断 */
        if(load_date[i] < load_date[min]){
            printf("load_date[%d] = %d\n", i, load_date[i]);
            min = i;    
            cnt ++;
        }
        printf("load_date min:%d\n", min);
    }
    
    // printf("cnt:%d\n", cnt);
    if (0 == cnt) {
         for (int i = 1; i < QUEUEMAX ; i++){
            if(load_time[i] < load_time[min]) {
                printf("load_time[%d] = %d\n", i, load_time[i]);
                min = i;                 
            }
            printf("load_time min:%d\n", min);
        }
        printf("最小值为:%d,下标为：%d",load_time[min], min);
        printf("\nDone\n");
        return min;
    }
    
    printf("最小值为:%d,下标为：%d",load_date[min], min);
    printf("\nDone\n");
    return min;  
}

/**
 * @name         : log_index
 * @brief        : 保存log文件
 * @param         {P_Queue} log_que
 * @return        {P_Queue} log_que
 */
P_Queue log_save(P_Queue log_que, char * num)
{
    struct timeval      tv_log;
    struct  tm         *tm_ptr;

    char date_tmp[128] = {"0"};
    char time_tmp[128] = {"0"};       

    gettimeofday(&tv_log, NULL);
 
    tm_ptr = localtime(&tv_log.tv_sec);

    sscanf(num, "%d", &log_num);

    FILE *fp;
    fp = fopen(LOGPATH, "r+");
    if (NULL == fp)
    {
        perror("open log.txt error>>>");
        exit(1);
    }
    printf("%d\n",__LINE__);

    dataType n = 0;
    
    /* 判断log文件写入条数 */
    while ( NULL != fgets(linebuffer[n], MAX_DATA, fp) )
    {  
        n ++;
        log_cnt = n;
        printf("log_cnt:%d\n", log_cnt);
    }
    // printf("line:%d\n",__LINE__);
    
    
    fclose(fp);
    printf("log_cnt:%d\n", log_cnt );
    if (log_cnt < QUEUEMAX)
    {
        QueuePush(log_que, log_num);
      
        log_que->_tail->_data = log_num;
 
        memset(time_tmp, 0 , sizeof(time_tmp));
        snprintf(time_tmp, sizeof(time_tmp) ,
            "[ %d-%02d-%02d %02d:%02d:%02d ]",
            1900+tm_ptr->tm_year, 1+tm_ptr->tm_mon, tm_ptr->tm_mday, 
            tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
        
        sprintf(str_temp, "echo  '%s get_num= %d' >> %s ", time_tmp, log_que->_tail->_data, LOGPATH);        
        system(str_temp);
        memset(str_temp, 0, sizeof(str_temp));
        // printf("QueueBack:%d, num:%d\n ", log_que->_tail->_index, QueueBack(log_que));
        // printf("QueueFront:%d, num:%d\n", log_que->_head->_index, QueueFront(log_que));     
    }
    else 
    {
        log_index = log_Load(log_que);
        QueuePop(log_que);
        QueuePush(log_que, log_num);

        /* 保存元素到队列 */
        log_que->_tail->_index = log_index + 1;
        log_que->_tail->_index = log_index+1;
        if (log_que->_tail->_index > QUEUEMAX) {
            log_que->_tail->_index -= QUEUEMAX;
        }
        log_que->_tail->_data = log_num;

        printf("log_index:%d\n", log_que->_tail->_index);

        memset(time_tmp, 0 , sizeof(time_tmp));
        snprintf(time_tmp, sizeof(time_tmp) ,
            "[ %d-%02d-%02d %02d:%02d:%02d ]",
            1900+tm_ptr->tm_year, 1+tm_ptr->tm_mon, tm_ptr->tm_mday, 
            tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec
        );
        
        // 使用sed 命令修改log
        sprintf(str_temp, "  sed -i '%dc %s get_num= %d ' %s ",log_que->_tail->_index , time_tmp, log_que->_tail->_data, LOGPATH);
        printf("str_temp = %s\n", str_temp);
        system(str_temp);
        memset(str_temp, 0, sizeof(str_temp));
    }

    return log_que;
}

/* 打印log */
/**
 * @name         : log_Display
 * @brief        : 打印log
 */
P_Queue log_Display(P_Queue log_que, char (*linebuffer)[128] )
{
    int n = 0;
    FILE *fp = fopen(LOGPATH, "r");
    if (NULL == fp)
    {
        perror("open log.txt error>>>");
        exit(1);
    }

    dataType  index_read = 0;
    dataType  num_read   = 0;

    while ( NULL != fgets(linebuffer[n], MAX_DATA, fp) )
    {

        n ++;
        log_cnt = n;
        printf("log_cnt:%d\n", log_cnt);
        
    }
    
    if ( log_cnt < QUEUEMAX) {
        // sprintf(str_temp, "cat ../log.txt");
        // printf("str_temp = %s\n", str_temp);
        for (int i = 1; i <= n; i++) {
            sprintf(str_temp, " sed -n '%dp' ../log.txt", i);
            log_ret = shell_cmd_send(str_temp, linebuffer[i-1], 128, 100);
            if (log_ret > 0) {
                printf("linebuffer: %s\n", linebuffer[i-1]);
            }
            else  {
                printf("shell_cmd_send err");
        
            }
            memset(str_temp, 0, sizeof(str_temp));  
        }
    }
    else{

        log_index = log_Load(log_que) + 1;
        printf("log_index:%d\n", log_index);

        for (size_t i = 0; i < QUEUEMAX; i++){
            if (QUEUEMAX < log_index) {
                log_index -= QUEUEMAX ;
            }                                       
            sprintf(str_temp, " sed -n '%dp' ../log.txt", log_index);

            log_ret = shell_cmd_send(str_temp, linebuffer[i], 128, 100);
            if (log_ret > 0) {
                // printf("log_ret:%d\n", log_ret);
                printf("%s\n", linebuffer[i]);
            }
            else  {
                printf("shell_cmd_send err");
        
            }
            memset(str_temp, 0, sizeof(str_temp));  
            log_index ++;
        }      
    }
    fclose(fp);
}
/**
 * @name         : log_window
 * @brief        : log加载界面
 * @return        {*}
 */
dataType log_window()
{
    // system("clear");
    printf("|***********log功能界面************|\n");
    printf("|       1:文件保存log功能          |\n");
    printf("|       2:文件log打印功能          |\n");
    printf("|       0:返回上一级菜单           |\n");
    printf("|**********************************|\n");
}
