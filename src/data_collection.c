
#include "log.h"
#include "data.h"

int collect_ret = 0;
int data_buf [10] = {0};

#define INFO "请连续采集10个数据 每个数据以, 分隔 如输入值为110, 10, 1, 5, 123, 48, 1, 2, 3, 99\n"

//冒泡排序
void bubble_sort(int arr[], int len) 
{
	int i, j, temp;

	for (i = 0; i < len - 1; i++)

    //升序排法，前一个数和后一个数比较，如果前数大则与后一个数换位置
    for (j = 0; j < len - 1 - i; j++)

    if (arr[j] > arr[j + 1]) 
    {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
    }
}

// 获取最大值
dataType get_max(int arr[], int len)
{
	return arr[len - 1];
}

//获取最小值
dataType get_min(int arr[], int len)
{
	return arr[0];
}

//中值滤波
float get_average(int arr[], int len)
{
	float data_ret = 0;
	for (size_t i = 1; i < len-1; i++)
	{
		data_ret = data_ret + arr[i];
	}
	return (data_ret/(len-2));
}

void data_collection_window()
{
    // system("clear");
    printf("\n|************数据采集界面***********|\n");
    printf("|       1:采集数据                  |\n");
    printf("|       2:输出最大 最小 平均值      |\n");
    printf("|       0:返回上一级菜单            |\n");
    printf("|***********************************|\n");
}

dataType data_collect()
{
    
    printf(INFO);
    scanf("%d[^,]%d[^,]%d[^,]%d[^,]%d[^,]%d[^,]%d[^,]%d[^,]%d[^,]%d",
        &data_buf[0], &data_buf[1], &data_buf[2], &data_buf[3], &data_buf[4],
        &data_buf[5], &data_buf[6], &data_buf[7], &data_buf[8], &data_buf[9]
    );

    bubble_sort(data_buf, 10);

    for (size_t i = 0; i < 10; i++)
    {
        printf("%d\t ", data_buf[i]);
    }
       
}

dataType median_filter()
{
    printf( "最大值：%d;\n最小值:%d;\n", get_max(data_buf, 10),get_min(data_buf, 10) );
    
    printf("中值:%.3f\n", get_average(data_buf, 10));
}

dataType data_collection()
{
    dataType collect_sel = -1;

    while (1)
    {
        data_collection_window();

        collect_ret = scanf("%d", &collect_sel);
        if (-1 == collect_ret)
        {
            printf("请重新输入!\n");
            return collect_sel;
        }
        else
        {
            getchar();
        }

        switch (collect_sel)
        {
            case 1:
                data_collect();

                continue;
                //break;

            case 2:
                
                median_filter();

                continue;
                //break;

            default:

                return 0;
        }
    }
    
}