#ifndef _USER_DATA_H_
#define _USER_DATA_H_
#include "stdbool.h"
#include <time.h>
#include <stdint.h>

typedef struct
{
	bool enable;
	/***** 0:door1 ,1:door2 --- 3:cctv2 *****/
	char select_camera;
	/***** 0:photo 1:video *****/
	char saving_fmt;
	/***** 灵敏度 0：low,1:middle,2:height *****/
	char sensivity;
	/***** 定时器 ,启用了时间段控制*****/
	bool timer_en;
	/***** lcd *****/
	bool lcd_en;
	/*****录像时长*****/
	int record_duration;
	struct tm start;
	struct tm end;
} user_motion_info;

typedef struct
{
	int bright;
	int cont;
	int color;
} isp_info;

typedef struct
{
	/***** 0:english *****/
	char language;
	/***** 0：id1  1：id2 *****/
	char deive_id;

	unsigned char door_1_open_time;
	unsigned char door_2_open_time;

	/*****  password *****/
	char password[5];

} user_etc_info;

typedef struct
{
	uint8_t language;

	bool time_display_enable;
	uint8_t clock_style;
	bool intercom_receive_enable;
	// 0->波斯日历 1->西方日历
	uint8_t calendar;

	bool window_display_enable;

	uint8_t record_mode;

	int door1_tone;
	int door2_tone;
	// min:0 max:4
	int door1_ring_volume;
	int door2_ring_volume;
	bool key_tone_enable;
	int ring_time;

	int inter_ring_volume;
} user_setting_info;

typedef struct
{
	isp_info door1;
	isp_info door2;
	isp_info door;
	isp_info cctv1;
	isp_info cctv2;
} user_camera_info;

typedef struct
{

	bool auto_record;

	bool alarm_1_enable;
	bool alarm_1_trigger;

	bool alarm_2_enable;
	bool alarm_2_trigger;

} user_alarm_info;

typedef struct
{
	user_setting_info setting;
	user_camera_info camera;
	bool media_disp_mode;
	bool new_media_file_flag;
	bool new_call_record_flag;

	user_motion_info motion;
	user_etc_info etc;
	user_alarm_info alarm;

	/*****  password *****/
	unsigned int password;
	bool sd_card_pattion;
	unsigned int device_id;
	bool room_no_flag;
	bool upgrade_success_flag;
	bool new_photo_file_flag;
	// bool change_channel_enable;
} user_data_info;

bool user_data_save(void);
bool user_data_init(void);
user_data_info *user_data_get(void);
void user_data_reset(void);

#endif
