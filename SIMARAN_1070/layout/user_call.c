#include "user_call.h"
#include "user_time.h"
#include "user_data.h"
#include <string.h>
#include <stdio.h>

// 当前活跃的call记录文件名
static char current_call_file[128] = {0}; // 门口机的当前call记录

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：开始一个call记录
** 参数说明：
**   door_station: 门口机编号 (1或2)
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_start(bool is_door_station, call_door_station door_station, int call_num)
{
    memset(current_call_file, 0, sizeof(current_call_file));
    char file_path[128] = {0};
    char ch = 0;
    int target_call_num = 0;

    // 1. 参数合法性校验
    if (is_door_station)
    {
        // 门口机场景：仅支持1/2
        if (door_station != CALL_DOOR_STATION_1 && door_station != CALL_DOOR_STATION_2)
        {
            printf("Invalid door station number: %d\n", door_station);
            return false;
        }
        ch = (door_station == CALL_DOOR_STATION_1) ? '1' : '2'; // 沿用原有门口机编号
    }
    else
    {
        // 普通call_num场景：校验0<call_num<257
        if (call_num <= 0 || call_num >= 257)
        {
            printf("Invalid call_num: %d (must be 1~256)\n", call_num);
            return false;
        }
        target_call_num = call_num;
        ch = '0'; // 普通场景用ch=0区分，避免与门口机1/2冲突
    }

    // 创建call记录文件
    if (!media_call_record_create(ch, target_call_num, is_door_station, file_path))
    {
        printf("Failed to create call record for door station %d\n", door_station);
        return false;
    }
    int fd = open(file_path, O_CREAT | O_WRONLY);
    if (fd >= 0)
    {
        close(fd);
    }
    media_file_bad_check(file_path);

    strncpy(current_call_file, file_path, sizeof(current_call_file) - 1);

    printf("Call record started for door station %d: %s\n", door_station, file_path);

    // 标记有新的call记录
    system("sync");
    user_data_get()->new_call_record_flag = true;
    user_data_save();

    return true;
}

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：标记call为已接听
** 参数说明：
**   door_station: 门口机编号 (1或2)
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_answered(call_door_station door_station)
{
    char *current_file = NULL;

    current_file = current_call_file;

    // 检查是否有活跃的call记录
    if (strlen(current_file) == 0)
    {
        printf("No active call record for door station %d\n", door_station);
        return false;
    }

    // 提取文件名（不包含路径）
    const char *file_name = strrchr(current_file, '/');
    if (file_name == NULL)
    {
        file_name = current_file;
    }
    else
    {
        file_name++; // 跳过'/'
    }

    // 更新接听状态
    if (!media_file_call_answered_update(file_name))
    {
        printf("Failed to update call answered status for door station %d\n", door_station);
        return false;
    }

    printf("Call answered for door station %d: %s\n", door_station, file_name);

    // 标记数据有更新
    user_data_get()->new_call_record_flag = true;
    user_data_save();

    return true;
}

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：结束call记录
** 参数说明：
**   door_station: 门口机编号 (1或2)
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_end(call_door_station door_station)
{

    memset(current_call_file, 0, sizeof(current_call_file));

    printf("Call record ended for door station %d\n", door_station);
    return true;
}

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：获取call记录总数
** 参数说明：
**   total: 总记录数
**   new_total: 新记录数
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_total_get(int *total, int *new_total)
{
    return media_file_total_get(FILE_TYPE_FLASH_CALL, total, new_total);
}

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：清除call记录的新标志
** 参数说明：
**   index: 记录索引
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_new_clear(int index)
{
    return media_file_new_clear(FILE_TYPE_FLASH_CALL, index);
}

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：删除一个call记录
** 参数说明：
**   index: 记录索引
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_delete(int index)
{
    return media_file_delete(FILE_TYPE_FLASH_CALL, index);
}

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：获取call记录信息
** 参数说明：
**   index: 记录索引
** 返回参数说明：成功返回文件信息指针，失败返回NULL
***/
const file_info *call_record_info_get(int index)
{
    return media_file_info_get(FILE_TYPE_FLASH_CALL, index);
}

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：删除所有call记录
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_delete_all(void)
{
    return media_file_delete_all(FILE_TYPE_FLASH_CALL, false);
}