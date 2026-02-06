#ifndef _VIDEO_INPUT_H_
#define _VIDEO_INPUT_H_
#include <stdbool.h>
#include "ak_common.h"
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include <pthread.h>

#define SUB_VIDEO_PIXEL_WIDTH 640
#define SUB_VIDEO_PIXEL_HIGHT 360

typedef struct
{
	char isp_conf_file[128];
	int main_width;
	int main_hgith;

	int sub_width;
	int sub_hight;

	int main_crop_width;
	int main_crop_hight;

	int main_crop_pos_x;
	int main_crop_pos_y;
	int fps;
} video_isp_param;

extern pthread_mutex_t video_input_mutex;
extern pthread_mutex_t video_main_display_mutex;
extern pthread_mutex_t video_sub_display_mutex;
/***
** 日期: 2022-05-12 15:44
** 作者: leo.liu
** 函数作用：关闭isp
** 返回参数说明：
***/
bool video_input_close(void);

/***
** 日期: 2022-05-12 15:21
** 作者: leo.liu
** 函数作用：打开isp
** 返回参数说明：
***/
bool video_input_open(video_isp_param *param);

/***
** 日期: 2022-05-12 16:24
** 作者: leo.liu
** 函数作用：初始化video onput 任务
** 返回参数说明：
***/
bool video_input_init(void);

/***
** 日期: 2022-05-13 17:48
** 作者: leo.liu
** 函数作用：获取常驻视频buffer
** 返回参数说明：
***/
unsigned char *video_input_resident_buffer_get(unsigned long long *timestamp);
/***
** 日期: 2022-05-13 17:48
** 作者: leo.liu
** 函数作用：获取常驻视频子buffer
** 返回参数说明：
***/
unsigned char *video_input_sub_resident_buffer_get(unsigned long long *timestamp);
/***
** 日期: 2022-05-14 16:16
** 作者: leo.liu
** 函数作用：设置过滤帧数
** 返回参数说明：
***/
void video_input_skip_frame_count_set(int count);

/***
** 日期: 2022-05-14 16:41
** 作者: leo.liu
** 函数作用：将数据写入视频buf
** 返回参数说明：
***/
void video_input_resident_buffer_write(unsigned char *data, int w, int h, int posx, int posy, int posw, int posh, AK_GP_FORMAT fmt);

/***
** 日期: 2022-05-14 17:34
** 作者: leo.liu
** 函数作用：清空buf
** 返回参数说明：
***/
void video_input_resident_bzero(void);
/***
**   日期:2022-06-11 11:36:39
**   作者: leo.liu
**   函数作用：设置将视频显示在指定的位置
**   参数说明:
***/
bool video_input_display_pos(int x, int y, int w, int h);

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：设置视频显示百分比倍数
**   参数说明:
***/
bool video_input_display_zoom_set(int zoom);

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：获取视频显示百分比倍数
**   参数说明:
***/
int video_input_display_zoom_get(void);

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：设置视频偏移显示
**   参数说明:
***/
bool video_input_display_offset_set(double offset_x, double offset_y);

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：获取视频偏移参数
**   参数说明:
***/
bool video_input_display_offset_get(double *offset_x, double *offset_y);

/***
** 日期: 2022-05-16 10:38
** 作者: leo.liu
** 函数作用：获取视频状态
** 返回参数说明：
***/
bool video_input_state_get(void);

#define video_input_lock() pthread_mutex_lock(&video_input_mutex)
#define video_input_unlock() pthread_mutex_unlock(&video_input_mutex)

#define video_main_display_lock() pthread_mutex_lock(&video_main_display_mutex)
#define video_main_display_unlock() pthread_mutex_unlock(&video_main_display_mutex)

#define video_sub_display_lock() pthread_mutex_lock(&video_sub_display_mutex)
#define video_sub_display_unlock() pthread_mutex_unlock(&video_sub_display_mutex)
#endif