#ifndef _VIDEO_PLAY_H_
#define _VIDEO_PLAY_H_
#include <stdbool.h>
typedef enum
{
	VIDEO_PLAY_STATE_IDLE,
	VIDEO_PLAY_STATE_PLAY,
	VIDEO_PLAY_STATE_PAUSE
} VIDEO_PLAY_STATUS;

/***
**   日期:2022-05-24 14:27:18
**   作者: leo.liu
**   函数作用：视频播放初始化
**   参数说明:
***/
bool video_play_init(void);
/***
**   日期:2022-05-24 14:27:45
**   作者: leo.liu
**   函数作用：视频开始播放
**   参数说明:
***/
bool video_play_start(const char *file);
/***
**   日期:2022-05-24 14:28:35
**   作者: leo.liu
**   函数作用：视频停止播放
**   参数说明:
***/
bool video_play_stop(void);
/***
**   日期:2022-05-24 14:28:51
**   作者: leo.liu
**   函数作用：暂停播放
**   参数说明:
***/
bool video_play_pause(void);
/***
**   日期:2022-05-24 14:30:09
**   作者: leo.liu
**   函数作用：获取视频播放状态
**   参数说明:
***/
VIDEO_PLAY_STATUS video_play_status_get(void);
/***
**   日期:2022-05-24 14:32:04
**   作者: leo.liu
**   函数作用：获取播放时长
**   参数说明:
***/
bool video_play_duration_get(int *cur, int *total);
/***
**   日期:2022-06-11 11:41:27
**   作者: leo.liu
**   函数作用：播放显示位置设置
**   参数说明:
***/
bool video_play_display_pos(int x, int y, int w, int h);
/**
 * 功能：非播放状态下获取视频总时长（单位：毫秒）
 * 参数：file - 视频文件路径（如 "/sdcard/video/20240520_1530.avi"）
 * 返回：成功返回总时长（毫秒），失败返回 0
 */
int video_get_duration_without_play(const char *file);

#endif