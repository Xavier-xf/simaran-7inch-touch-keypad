#include "user_data.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "stdio.h"
#include "stdlib.h"

#define USER_DATA_PATH "/app/data/user_data.cfg"

static user_data_info user_data;

static const user_data_info user_data_default = {

	.setting.language = 1,
	.setting.time_display_enable = true,
	.setting.clock_style = 0,
	.setting.intercom_receive_enable = true,

	.setting.calendar = 0,
	.setting.window_display_enable = false,
	.setting.record_mode = 0,
	.setting.door1_tone = 1,
	.setting.door2_tone = 1,
	.setting.door1_ring_volume = 2,
	.setting.door2_ring_volume = 2,
	.setting.key_tone_enable = true,
	.setting.inter_ring_volume = 2,
	.setting.ring_time = 5,

	.motion = {
		.enable = false,
		.select_camera = 0,
		.saving_fmt = 0,
		.sensivity = 1,
		.timer_en = false,
		.lcd_en = false,
		.record_duration = 10,
		.start =
			{
				.tm_year = 2023,
				.tm_mon = 1,
				.tm_mday = 1,
				.tm_hour = 0,
				.tm_min = 0,
				.tm_sec = 0},
		.end =
			{
				.tm_year = 2030,
				.tm_mon = 1,
				.tm_mday = 1,
				.tm_hour = 0,
				.tm_min = 0,
				.tm_sec = 0}},

	.camera.door1.bright = 4,
	.camera.door1.color = 4,
	.camera.door1.cont = 4,

	.camera.door2.bright = 4,
	.camera.door2.color = 4,
	.camera.door2.cont = 4,

	.camera.door.bright = 4,
	.camera.door.color = 4,
	.camera.door.cont = 4,

	.camera.cctv1.bright = 4,
	.camera.cctv1.color = 4,
	.camera.cctv1.cont = 4,

	.camera.cctv2.bright = 4,
	.camera.cctv2.color = 4,
	.camera.cctv2.cont = 4,

	.media_disp_mode = 0,

	.new_media_file_flag = false,
	.new_call_record_flag = false,

	.etc = {
		.language = 0,
		.deive_id = 0,
		.door_1_open_time = 1,
		.door_2_open_time = 1,
		.password = {"1234"},
	},
	.alarm = {
		.auto_record = false,
		.alarm_1_enable = false,
		.alarm_1_trigger = false,
		.alarm_2_enable = false,
		.alarm_2_trigger = false,
	},
	.device_id = 0x01,

	.upgrade_success_flag = false,

	.password = 12345,

	.room_no_flag = false,

	.sd_card_pattion = false,

	.new_photo_file_flag = false,

	// .change_channel_enable = false,
};

bool user_data_save(void)
{
	int fd = open(USER_DATA_PATH, O_WRONLY | O_CREAT);
	if (fd < 0)
	{
		printf("write open %s fail \n", USER_DATA_PATH);
		return false;
	}

	write(fd, &user_data, sizeof(user_data_info));

	close(fd);
	system("sync");
	return true;
}

#define user_data_check_range_out(cur, min, max)            \
	if ((user_##data.cur < min) || (user_##data.cur > max)) \
	{                                                       \
		user_##data.cur = user_##data##_default.cur;        \
		printf("user data error \n");                       \
	}

#define user_data_motion_check_range_out(x, min, max) user_data_check_range_out(motion.x, min, max)

#define user_data_audio_check_range_out(x, min, max) user_data_check_range_out(audio.x, min, max)

#define user_data_display_check_range_out(x, min, max) user_data_check_range_out(display.x, min, max)

#define user_data_etc_check_range_out(x, min, max) user_data_check_range_out(etc.x, min, max)

#define user_data_alarm_check_range_out(x, min, max) user_data_check_range_out(alarm.x, min, max)
/***
** 日期: 2022-05-05 08:47
** 作者: leo.liu
** 函数作用：检验数据是否合法
** 返回参数说明：
***/
static void user_data_check_valid(void)
{
	// /***** 自动记录模式 *****/
	// user_data_check_range_out(auto_record_mode, 0, 2);
	// user_data_check_range_out(wifi_enable, 0, 1);
	// /***** 移动侦测 *****/
	// user_data_motion_check_range_out(enable, 0, 1);
	// user_data_motion_check_range_out(select_camera, 0, 3);
	// user_data_motion_check_range_out(saving_fmt, 0, 1);
	// user_data_motion_check_range_out(sensivity, 0, 2);
	// user_data_motion_check_range_out(timer_en, 0, 1);
	// user_data_motion_check_range_out(lcd_en, 0, 1);

	// user_data_motion_check_range_out(start.tm_year, 2021, 2037);
	// user_data_motion_check_range_out(start.tm_mon, 1, 12);
	// user_data_motion_check_range_out(start.tm_mday, 1, 31);
	// user_data_motion_check_range_out(start.tm_hour, 0, 23);
	// user_data_motion_check_range_out(start.tm_min, 0, 59);
	// user_data_motion_check_range_out(start.tm_sec, 0, 59);

	// user_data_motion_check_range_out(end.tm_year, 2021, 2037);
	// user_data_motion_check_range_out(end.tm_mon, 1, 12);
	// user_data_motion_check_range_out(end.tm_mday, 1, 31);
	// user_data_motion_check_range_out(end.tm_hour, 0, 23);
	// user_data_motion_check_range_out(end.tm_min, 0, 59);
	// user_data_motion_check_range_out(end.tm_sec, 0, 59);

	// /***** audio *****/
	// user_data_audio_check_range_out(door1_tone, 1, 6);
	// user_data_audio_check_range_out(door2_tone, 1, 6);

	// user_data_audio_check_range_out(door_ring_volume, 0, 3);
	// user_data_audio_check_range_out(inter_ring_volume, 0, 3);

	// user_data_audio_check_range_out(door_talk_volume, 1, 3);
	// user_data_audio_check_range_out(inter_talk_volume, 1, 3);

	// /***** display *****/
	// user_data_display_check_range_out(standby_mode, 0, 1);
	// user_data_display_check_range_out(lcd_brigtness, 0, 100);
	// user_data_display_check_range_out(frame_time_en, 0, 1);
	// user_data_display_check_range_out(frame_time_start, 0, 1440);
	// user_data_display_check_range_out(frame_time_end, 0, 1440);
	// user_data_display_check_range_out(night_mode, 0, 1);
	// user_data_display_check_range_out(night_time_start, 0, 1440);
	// user_data_display_check_range_out(night_time_end, 0, 1440);

	// user_data_display_check_range_out(frame_list, 0, 0x1F);
	// user_data_display_check_range_out(frame_background, 0, 1);

	// user_data_display_check_range_out(door1.bright, 0, 20);
	// user_data_display_check_range_out(door1.cont, 0, 20);
	// user_data_display_check_range_out(door1.color, 0, 20);

	// user_data_display_check_range_out(door2.bright, 0, 20);
	// user_data_display_check_range_out(door2.cont, 0, 20);
	// user_data_display_check_range_out(door2.color, 0, 20);

	// user_data_display_check_range_out(cctv1.bright, 0, 20);
	// user_data_display_check_range_out(cctv1.cont, 0, 20);
	// user_data_display_check_range_out(cctv1.color, 0, 20);

	// user_data_display_check_range_out(cctv2.bright, 0, 20);
	// user_data_display_check_range_out(cctv2.cont, 0, 20);
	// user_data_display_check_range_out(cctv2.color, 0, 20);

	// /***** etc *****/
	// user_data_etc_check_range_out(language, 0, 9);
	// user_data_etc_check_range_out(deive_id, 0, 3);
	// user_data_etc_check_range_out(open_the_door, 0, 1);
	// user_data_etc_check_range_out(door1_open_door_mode, 0, 1);
	// user_data_etc_check_range_out(door2_lock_num, 1, 2);
	// user_data_etc_check_range_out(password[0], '0', '9');
	// user_data_etc_check_range_out(password[1], '0', '9');
	// user_data_etc_check_range_out(password[2], '0', '9');
	// user_data_etc_check_range_out(password[3], '0', '9');
	// /*****  alarm *****/
	// user_data_alarm_check_range_out(auto_record, 0, 1);
	// user_data_alarm_check_range_out(alarm_1_enable, 0, 1);
	// user_data_alarm_check_range_out(alarm_1_trigger, 0, 1);
	// user_data_alarm_check_range_out(alarm_2_enable, 0, 1);
	// user_data_alarm_check_range_out(alarm_2_trigger, 0, 1);
}

bool user_data_init(void)
{
	int fd = open(USER_DATA_PATH, O_RDONLY);
	if (fd < 0)
	{
		printf("read open %s fail \n", USER_DATA_PATH);
		user_data = user_data_default;
		return false;
	}
	read(fd, &user_data, sizeof(user_data_info));

	close(fd);
	user_data_check_valid();
	return true;
}

user_data_info *user_data_get(void)
{
	return &user_data;
}

void user_data_reset(void)
{
	user_data = user_data_default;
	user_data_save();
	system("sync");
}
