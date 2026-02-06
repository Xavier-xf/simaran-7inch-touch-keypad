#ifndef LV_MSG_EVENT_H_
#define LV_MSG_EVENT_H_
#include <stdbool.h>
#include <stdio.h>
#include "../share/include/lvgl/lvgl.h"

typedef enum
{
	MSG_EVENT_CMD_DOOR1_CALL = 0X0000,
	MSG_EVENT_CMD_DOOR2_CALL,
	MSG_EVENT_CMD_SENSOR1_TRIGGER,
	MSG_EVENT_CMD_SENSOR2_TRIGGER,
	MSG_EVENT_CMD_SD_STATE,
	MSG_EVENT_CMD_INTERCOM,
	MSG_EVENT_CMD_TUYA,
	MSG_EVENT_CMD_CUSTOM,
	MSG_EVENT_CMD_GATE_OPEN,
	MSG_EVENT_CMD_KEY,
} MSG_CMD;

typedef enum
{
	TUYA_EVENT_CMD_VIDEO_START,
	TUYA_EVENT_CMD_VIDEO_STOP,
	TUYA_EVENT_CMD_AUDIO_START,
	TUYA_EVENT_CMD_ONLINE,
	TUYA_EVENT_CMD_CH_CHANGE,
	TUYA_EVENT_CMD_MOTION_ENBALE,
	TUYA_EVENT_CMD_DOOR_OPEN,
} TUYA_CMD;
/***
** 日期: 2022-04-25 16:13
** 作者: leo.liu
** 函数作用：layout页面属性
** 返回参数说明：
***/
typedef struct
{
	void (*enter)(void);
	void (*quit)(void);
} layout;
/***
** 日期: 2022-04-25 16:14
** 作者: leo.liu
** 函数作用：创建一个页面
** 返回参数说明：
***/
#define CREATE_LAYOUT(x) layout layout_##x = {            \
							 .enter = layout_##x##_enter, \
							 .quit = layout_##x##_quit};
/***
** 日期: 2022-04-25 16:14
** 作者: leo.liu
** 函数作用：进入此页面的第一个执行的函数
** 返回参数说明：
***/
#define LAYOUT_ENTER_FUNC(x) layout_##x##_enter(void)
/***
** 日期: 2022-04-25 16:14
** 作者: leo.liu
** 函数作用：退出此页面的最后一个函数
** 返回参数说明：
***/
#define LAYOUT_QUIT_FUNC(x) layout_##x##_quit(void)
/***
** 日期: 2022-04-25 16:15
** 作者: leo.liu
** 函数作用：页面地址
** 返回参数说明：
***/
#define pLAYOUT(x) &layout_##x
/***
** 日期: 2022-04-25 16:15
** 作者: leo.liu
** 函数作用：声明页面
** 返回参数说明：
***/
#define DEFINE_LAYOUT(x) extern layout layout_##x
/***
** 日期: 2022-04-25 16:31
** 作者: leo.liu
** 函数作用：页面跳转
** 返回参数说明：
***/
bool goto_layout(const layout *layout);
/***
**   日期:2022-05-23 13:41:22
**   作者: leo.liu
**   函数作用：获取当前页面
**   参数说明:
***/
const layout *cur_layout_get(void);
/***
** 日期: 2022-04-25 16:16
** 作者: leo.liu
** 函数作用：按钮相关属性
** 返回参数说明：
***/
typedef struct
{
	void (*down)(lv_obj_t *obj);
	void (*up)(lv_obj_t *obj);
	void (*anything_func)(lv_obj_t *obj, lv_event_t event);
} obj_click_data;

/***
** 日期: 2022-04-25 16:17
** 作者: leo.liu
** 函数作用：获取按钮相关属性
** 返回参数说明：
***/
#define obj_click_data_create(down_ex, up_ex) {.down = down_ex, \
											   .up = up_ex,     \
											   .anything_func = NULL};

#define obj_click_data_up_create(x) { \
	.down = NULL,                     \
	.up = x,                          \
	.anything_func = NULL};

#define obj_click_data_anything_create(x) { \
	.down = NULL,                           \
	.up = NULL,                             \
	.anything_func = x};

/***
** 日期: 2022-04-25 16:17
** 作者: leo.liu
** 函数作用：外部按键相关属性
** 返回参数说明：
***/
typedef struct
{
	lv_key_t key;
	void (*press_callback)(lv_key_t key);	   // 按下回调
	void (*release_callback)(lv_key_t key);	   // 抬手回调
	void (*long_press_callback)(lv_key_t key); // 长安回调
} key_binding_t;

/***
** 日期: 2022-04-25 16:17
** 作者: leo.liu
** 函数作用：设置外部按键相关属性
** 返回参数说明：
***/
#define KEY_BIND(key_val, press_func, release_func) \
	{key_val, press_func, release_func, NULL}

#define KEY_BIND_PRESS_ONLY(key_val, press_func) \
	{key_val, press_func, NULL, NULL}

#define KEY_BIND_RELEASE_ONLY(key_val, release_func) \
	{key_val, NULL, release_func, NULL}

#define KEY_BIND_LONG_PRESS(key_val, press_func, release_func, long_press_func) \
	{key_val, press_func, release_func, long_press_func}

#define KEY_BIND_PRESS_LONG_PRESS(key_val, press_func, long_press_func) \
	{key_val, press_func, NULL, long_press_func}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define LAYOUT_KEY_BINDINGS(bindings) layout_key_bind(bindings, ARRAY_SIZE(bindings))
/***
** 日期: 2022-04-25 16:18
** 作者: leo.liu
** 函数作用：监听此控件的点击事件
** 返回参数说明：
***/
void obj_click_event_listen(lv_obj_t *obj, const obj_click_data *click_data);

/***
** 日期: 2022-05-12 10:34
** 作者: leo.liu
** 函数作用：按键绑定
** 返回参数说明：
***/
void layout_key_bind(const key_binding_t *bindings, int count);

/***
** 日期: 2022-05-12 10:34
** 作者: leo.liu
** 函数作用：按键解绑
** 返回参数说明：
***/
void layout_key_unbind(void);

/***
** 日期: 2022-04-28 09:49
** 作者: leo.liu
** 函数作用：注册控件被按下执行的函数
** 返回参数说明：
***/
bool lv_obj_click_down_callback_register(void (*callback)(lv_obj_t *));
/***
** 日期: 2022-04-25 16:13
** 作者: leo.liu
** 函数作用：lv任务消息任务初始化
** 返回参数说明：
***/
void lv_event_task_init(void);
/***
** 日期: 2022-05-12 10:28
** 作者: leo.liu
** 函数作用：注册door1呼叫执行函数
** 返回参数说明：
***/
bool layout_door1_call_callback_register(void (*callback)(void));
/***
** 日期: 2022-05-12 10:28
** 作者: leo.liu
** 函数作用：注册door2呼叫执行函数
** 返回参数说明：
***/
bool layout_door2_call_callback_register(void (*callback)(void));
/***
** 日期: 2022-05-12 10:28
** 作者: leo.liu
** 函数作用：注册固定按钮执行函数
** 返回参数说明：
***/
bool layout_adc_key_register(void (*callback)(unsigned int arg));

/***
** 日期: 2022-05-17 14:47
** 作者: leo.liu
** 函数作用：sdcard状态注册
** 返回参数说明：
***/
bool lyaout_sd_state_callback_register(void (*callback)(void));
/***
** 日期: 2022-05-12 10:34
** 作者: leo.liu
** 函数作用：发送doo1 call event
** 返回参数说明：
***/
bool lv_msg_send_cmd(unsigned int cmd, unsigned int arg1, unsigned int arg2);
/***
**   日期:2022-05-28 08:07:55
**   作者: leo.liu
**   函数作用：警报1执行注册
**   参数说明:
***/
bool layout_alarm1_trigger_callback_register(void (*callback)(void));
/***
**   日期:2022-05-28 08:07:55
**   作者: leo.liu
**   函数作用：警报1执行注册
**   参数说明:
***/
bool layout_alarm2_trigger_callback_register(void (*callback)(void));
/***
**   日期:2022-06-07 15:37:04
**   作者: leo.liu
**   函数作用：tuya事件执行注册
**   参数说明:
***/
bool layout_tuya_event_callback_register(bool (*callback)(TUYA_CMD cmd, int arg));
/***
**   日期:2022-05-31 08:46:12
**   作者: leo.liu
**   函数作用：注册处理intercom事件
**   参数说明:
***/
bool layout_intercom_event_callback_register(bool (*callback)(unsigned int send_id, unsigned int cmd));
/***
**   日期:2022-05-31 08:46:12
**   作者: leo.liu
**   函数作用：注册自定义事件
**   参数说明:
***/
bool layout_custom_event_callback_register(bool (*callback)(unsigned int cmd, unsigned int arg));

/***
** 日期: 2022-05-12 10:28
** 作者: leo.liu
** 函数作用：电源指示灯处理函数注册
** 返回参数说明：
***/
bool layout_power_led_handler_register(void (*callback)(void));
#endif