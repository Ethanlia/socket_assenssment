/*
 * @Descripttion   : 
 * @Author         : Ethan.liang
 * @Date           : 2022-08-15 09:56:10
 * @LastEditors    : Do not Edit
 * @LastEditTime   : 2022-08-15 10:00:17
 */

#ifndef 	__DATA_H__
#define		__DATA_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

//冒泡排序
void bubble_sort(int arr[], int len);

// 获取最大值
dataType get_max(int arr[], int len);

//获取最小值
dataType get_min(int arr[], int len);

//中值滤波
float get_average(int arr[], int len);

void data_collection_window();

//收集数据
dataType data_collect( char * msg , int * data_buf);

//中值滤波输出
dataType median_filter(int * data_buf);

// dataType data_collection();

#endif