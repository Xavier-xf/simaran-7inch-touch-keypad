#ifndef _USER_CALL_H_
#define _USER_CALL_H_

#include "layout_define.h"
#include "user_file.h"

// Call事件类型
typedef enum
{
    CALL_EVENT_START,    // Call开始
    CALL_EVENT_ANSWERED, // 接听
    CALL_EVENT_END       // Call结束
} call_event_type;

// 门口机编号
typedef enum
{
    CALL_DOOR_STATION_1 = 1, // 门口机1
    CALL_DOOR_STATION_2 = 2  // 门口机2
} call_door_station;

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：开始一个call记录
** 参数说明：
**   door_station: 门口机编号 (1或2)
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_start(bool is_door_station, call_door_station door_station, int call_num);

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：标记call为已接听
** 参数说明：
**   door_station: 门口机编号 (1或2)
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_answered(call_door_station door_station);

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：结束call记录
** 参数说明：
**   door_station: 门口机编号 (1或2)
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_end(call_door_station door_station);

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：获取call记录总数
** 参数说明：
**   total: 总记录数
**   new_total: 新记录数
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_total_get(int *total, int *new_total);

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：清除call记录的新标志
** 参数说明：
**   index: 记录索引
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_new_clear(int index);

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：删除一个call记录
** 参数说明：
**   index: 记录索引
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_delete(int index);

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：获取call记录信息
** 参数说明：
**   index: 记录索引
** 返回参数说明：成功返回文件信息指针，失败返回NULL
***/
const file_info *call_record_info_get(int index);

/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：删除所有call记录
** 返回参数说明：成功返回true，失败返回false
***/
bool call_record_delete_all(void);

#endif