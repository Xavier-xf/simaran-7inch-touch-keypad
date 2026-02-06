#ifndef _MOTION_DETECTION_H_
#define _MOTION_DETECTION_H_
#include <stdbool.h>
/***
**   日期:2022-06-09 09:07:48
**   作者: leo.liu
**   函数作用：移动侦测初始化
**   参数说明:
***/
bool motion_detection_init(void);
/***
**   日期:2022-06-09 09:08:19
**   作者: leo.liu
**   函数作用：移动侦测销毁
**   参数说明:
***/
bool motion_detection_destory(void);
/***
**   日期:2022-06-09 09:08:41
**   作者: leo.liu
**   函数作用：开启侦测
**   参数说明:
***/
bool motion_detection_start(void);
/***
**   日期:2022-06-09 09:08:59
**   作者: leo.liu
**   函数作用：移动侦测停止
**   参数说明:
***/
bool motion_detection_stop(void);
/***
**   日期:2022-06-09 09:09:20
**   作者: leo.liu
**   函数作用：移动侦测检测是否移动
**   参数说明:
***/
bool motion_detection_check(void);
/***
**   日期:2022-06-09 09:09:43
**   作者: leo.liu
**   函数作用：移动侦测设置灵敏度
**   参数说明:
*    sensivity:取值在0-1200 每个8x8的宏块中有一个移动就+1,在320x240的像素块 中总共有1200个宏块
*    threshold:取值范围在0-255
***/
bool motion_detection_sensivity(int sensivity, int threshold);
#endif