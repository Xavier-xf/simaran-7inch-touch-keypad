#include "layout_define.h"
#include "video_input.h"
#include "audio_output.h"
#include "user_intercom.h"
#include "tuya/tuya_uuid_and_key.h"

extern bool LEO_FAST_ENTER_SYSTEM_FLAG;

unsigned long long calibrate_rtc_timestamp = 0;

/***
** 日期: 2022-04-25 17:08
** 作者: leo.liu
** 函数作用：logo图标显示
** 返回参数说明：
***/
static void logo_logo_icon_display(void)
{
	lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
	// static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_ENGLISH_LOGO_PNG);

	static rom_bin_info info = rom_bin_info_get(ROM_UI_BG_ENG_LOGO_PNG);
	// static rom_bin_info info3 = rom_bin_info_get(ROM_UI_BG_PERSIAN_LOGO_PNG);
	// if(user_data_get()->setting.language == LANG_ENGLISH)
	// {
	lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	// }
	// else
	// {
	// 	lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info3);
	// }

	lv_obj_set_size(obj, 386, 180);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	// lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);
}

// static void logo_software_version_label_create(void)
// {
// 	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
// 	int day = 0, month = 0, year = 0;
// 	get_curr_relesse_date(&day, &month, &year);
// 	lv_label_set_text_fmt(label, "compile time:%04d-%02d-%02d,%s", year, month, day, __TIME__);
// 	lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, -20);
// }

static void time_calibrate_task(lv_task_t *task)
{
	unsigned long long cur_timestamp = user_timestamp_get();
	if (abs(cur_timestamp - calibrate_rtc_timestamp) >= (4 * 60 * 60 * 1000))
	{
		struct tm tm;
		user_time_read(&tm);
		tm.tm_sec -= 1;
		user_time_set(&tm);
		calibrate_rtc_timestamp = user_timestamp_get();
		printf("%04d:%02d:%02d  %02d:%02d:%02d\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
}

/***
** 日期: 2022-05-18 09:23
** 作者: leo.liu
** 函数作用：定时器初始化
** 返回参数说明：
***/
static void layout_logo_loding_task(lv_task_t *task)
{
	user_time_init();
	/*****  tuya api初始化 *****/
	// tuya_api_init(TUYA_PID);
	/*****初始化isp *****/
	video_input_init();
	/***** 初始化声卡设备 *****/
	audio_output_init();

	/***** 初始化音频采集设备 *****/
	audio_input_init();

	/***** 初始化铃声设备 *****/
	ringplay_init();

	/***** 初始化jpg解码设备 *****/
	jpg_decode_init();
	/*****  初始化h264解码器 *****/
	// 		h264_decode_init();
	/***** 初始化mjpeg编码 *****/
	mjpeg_encode_init();
	/***** 初始化h264编码 *****/
	// 		h264_encode_init();
	/***** 初始化记录 *****/
	video_record_init();
	/*****  初始化播放 *****/
	video_play_init();
	/*****  户户通处理线程 *****/
	intercom_init();
	/***** gpio口初始化 *****/
	layout_gpio_init();

	/*****  注册按键音信息 *****/
	rom_bin_info info = rom_bin_info_get(ROM_UI_KEY_SOUND_PCM);
	touch_sound_rom_info_register(&info);
	/***** 注册控件按下回调函数 *****/
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
	ringplay_touchsound_mute_set(user_data_get()->setting.key_tone_enable ? false : true);
	/*****  call呼叫 *****/
	layout_door1_call_callback_register(layout_door1_call_default);
	layout_door2_call_callback_register(layout_door2_call_default);

	/*****  听筒状态改变 *****/
	layout_custom_event_callback_register(layout_hook_state_change_default);
	// /*****  电源指示灯处理 *****/
	// layout_power_led_handler_register(power_led_handler_default_func);

	/*****  sd卡状态改变默认回调 *****/
	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);

	media_file_list_init();
	/***** 初始化tp9950 *****/
	tp9950_init();

	if (user_data_get()->media_disp_mode && !media_sdcard_insert_check() && ((user_data_get()->new_media_file_flag == true) || (user_data_get()->new_photo_file_flag == true)))
		user_data_get()->new_media_file_flag = 0;
	user_data_get()->new_photo_file_flag = 0;

	calibrate_rtc_timestamp = user_timestamp_get();
	lv_task_t *time_task = lv_layout_task_create(time_calibrate_task, 60 * 1000, LV_TASK_PRIO_MID, NULL);
	time_task->clean_lock = false;

	if (LEO_FAST_ENTER_SYSTEM_FLAG == false)
	{
		usleep(1000 * 1000);
	}
	// extern void manual_enter_monitor_set(bool en);
	// manual_enter_monitor_set(false);

	/***** 开启待机侦测 *****/
	standby_timer_init(pLAYOUT(standby), 60000);
	standby_timer_restart(true);

	// if (user_data_get()->setting.time_display_enable == true)
	// {
	// 	goto_layout(pLAYOUT(standby));
	// }
	// else
	// {
	goto_layout(pLAYOUT(home));
	// }
}

// #include <time.h>
// #include <stdio.h>
// #include <unistd.h>
// unsigned long long start_time = 0;
// static void os_time_print_task(lv_task_t *task)
// {
// 	extern void print_time(void);
// 	print_time();
// }

/***
**   日期:2022-05-27 08:16:55
**   作者: leo.liu
**   函数作用：跳转进入系统
**   参数说明:
***/
static void layout_logo_enter_system(void)
{
	if (LEO_FAST_ENTER_SYSTEM_FLAG == false)
	{
		logo_logo_icon_display();
		// logo_software_version_label_create();
		lv_layout_task_create(layout_logo_loding_task, 200, LV_TASK_PRIO_MID, NULL);
	}
	else
	{
		layout_logo_loding_task(NULL);
	}

	// lv_task_t * task = lv_layout_task_create(os_time_print_task, 1000, LV_TASK_PRIO_MID, NULL);
	// task->clean_lock = false;
}

static void LAYOUT_ENTER_FUNC(logo)
{
	layout_logo_enter_system();
}

static void LAYOUT_QUIT_FUNC(logo)
{
}

CREATE_LAYOUT(logo);

void get_curr_relesse_date(int *day, int *month, int *year)
{
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
							"Sep", "Oct", "Nov", "Dec"};
	char temp[] = __DATE__;
	*year = atoi(temp + 7);
	*(temp + 6) = 0;
	*day = atoi(temp + 4);
	*(temp + 3) = 0;
	for (int i = 0; i < 12; i++)
	{
		if (!strcmp(temp, months[i]))
		{
			*month = i + 1;
			break;
		}
	}

	// Debug("Release Date : %d-%d-%d\n\r",*year,*month,*day);
}