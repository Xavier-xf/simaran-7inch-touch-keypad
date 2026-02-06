#ifndef _USER_FILE_H_
#define _USER_FILE_H_
#include <stdbool.h>

#define SD_BASE_PATH "/mnt/tf"
#define SD_MEDIA_PATH SD_BASE_PATH "/media/"
#define SD_VIDEO_PATH SD_BASE_PATH "/video/"
#define SD_PHOTO_PATH SD_BASE_PATH "/photo/"
#define SD_CALL_PATH SD_BASE_PATH "/call/" // 新增call记录路径
#define SD_BACKUP_PATH SD_BASE_PATH "/backup/"
#define FLASH_PHOTO_PATH "/app/data/photo/"
#define FLASH_CALL_PATH "/app/data/call/"

#define VIDEO_DOT ".AVI"
#define PHOTO_DOT ".JPG"
#define CALL_DOT ".CAL" // 新增call记录文件后缀

typedef enum
{
	FILE_TYPE_PHOTO,
	FILE_TYPE_VIDEO,
	FILE_TYPE_FLASH_PHOTO,
	FILE_TYPE_CALL, // 新增call类型
	FILE_TYPE_FLASH_CALL,
	FILE_TYPE_NONE
} file_type;

typedef struct
{
	char file_name[23];
	char mode;
	char ch;
	bool is_new;
	file_type type;
	bool answered;		  // 新增：是否接听
	int call_num;		  // 存储普通call_num（0<call_num<257），门口机场景设为0
	bool is_door_station; // 标志位：true=门口机，false=普通call_num
} file_info;

/***
** 日期: 2022-05-17 10:40
** 作者: leo.liu
** 函数作用：媒体文件初始化
** 返回参数说明：
***/
void media_file_list_init(void);
/***
** 日期: 2022-05-17 11:26
** 作者: leo.liu
** 函数作用：判断sd插入
** 返回参数说明：
***/
bool media_sdcard_insert_check(void);
/***
** 日期: 2022-05-17 14:40
** 作者: leo.liu
** 函数作用：判断sd文件是否已经满
** 返回参数说明：
***/
bool media_sdcard_full_check(void);
/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：创建一个文件
** 返回参数说明：
***/
bool media_file_create(file_type type, char ch, char mode, bool is_door_station, int call_num, char *file_path);
/***
** 日期: 2022-05-17 13:36
** 作者: leo.liu
** 函数作用：文件总数获取
** 返回参数说明：
***/
bool media_file_total_get(file_type type, int *total, int *new_total);
/***
** 日期: 2022-05-17 13:44
** 作者: leo.liu
** 函数作用：清除新文件标志
** 返回参数说明：
***/
bool media_file_new_clear(file_type type, int index);
/***
** 日期: 2022-05-17 13:53
** 作者: leo.liu
** 函数作用：删除一个文件
** 返回参数说明：
***/
bool media_file_delete(file_type type, int index);
/***
** 日期: 2022-05-17 13:54
** 作者: leo.liu
** 函数作用：判断该文件是否存在
** 返回参数说明：
***/
bool media_file_bad_check(const char *file);
/***
**   日期:2022-05-23 09:51:21
**   作者: leo.liu
**   函数作用：获取格式化sd卡状态
**   参数说明:
***/
bool media_format_sd_state(void);
/***
**   日期:2022-05-23 09:58:49
**   作者: leo.liu
**   函数作用：格式话sd卡
**   参数说明:
***/
void media_format_sd(void);
/***
**   日期:2022-05-23 14:34:43
**   作者: leo.liu
**   函数作用：获取文件信息
**   参数说明:
***/
const file_info *media_file_info_get(file_type type, int index);
/***
**   日期:2022-06-15 11:14:00
**   作者: leo.liu
**   函数作用：删除所有文件
**   参数说明:
 all,true:删除所有文件包括视频和照片
	 false:根据type删除指定的文件照片或这视频
***/
bool media_file_delete_all(file_type type, bool all);

bool sd_media_all_file_total_get(int *total, int *new_total);

int high_speed_media_file_total_get(file_type type);
/*******************************************************************
 * @brief  : 获取移动flash照片到SD卡的状态
 * @return  {*}
 *******************************************************************/
bool media_copy_flash_photo_to_sd_state(int *copied);
/*******************************************************************
 * @brief  : 开始移动flash照片到SD卡
 * @return  {*}
 *******************************************************************/
void media_copy_flash_photo_to_sd(void);

// 开始删除文件
void media_file_delete_all_start(file_type type);
// 删除文件状态
bool media_file_delete_all_state(void);
/***
**   日期:2022-06-15 11:14:00
**   作者: leo.liu
** 函数作用：更新call记录接听状态
** 返回参数说明：成功返回true，失败返回false
***/
bool media_file_call_answered_update(const char *file_name);

/***
**   日期:2022-06-15 11:14:00
**   作者: leo.liu
** 函数作用：创建call记录文件
** 返回参数说明：成功返回true，失败返回false
***/
bool media_call_record_create(char ch, int call_num, bool is_door_station, char *file_path);
#endif