/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-11 11:50
 * @LastEditTime   : 2023-03-15 16:32
 *******************************************************************/
#include "layout_define.h"

#define CAMERA_AUTO_RECORD_ENABLE 0 // 开启自动连续拍照、录像，测试用

#define CAMERA_TIMEOUT_DURATION 183 // 监控时长 240* 500ms
#define CAMERA_DISPLAY_DELAY 20		// 监控显示延时 * 100ms

enum
{
	RECORD_MODE_INIT = LAYOUT_SETTING_LANG_OFF_ID,
	RECORD_MODE_IMAGE = LAYOUT_RECORD_LANG_IMAGE_ID,
	RECORD_MODE_VIDEO = LAYOUT_RECORD_LANG_VIDEO_ID
};

/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：进入监控刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_1(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{934, 0, 90, 600}};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}

/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：点击拍照录像刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_2(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{934, 0, 90, 600},
		{320, 85, 300, 50}};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}
/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：点击开锁的刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_3(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{934, 0, 90, 600},
		{442, 230, 140, 140},
		{320, 85, 300, 50}};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}
/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：设置刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_4(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{934, 0, 90, 600},
		{115, 185, 750, 300}};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}

#define CAMERA_HEAD_CH_LABEL_ID 8			// 顶部通道标签
#define CAMERA_HEAD_TIME_LABEL_ID 9			// 顶部时间标签
#define CAMERA_HEAD_SDCADR_ICON_LABEL_ID 10 // 顶部SD卡图标

#define CAMERA_SETTING_WINDOW_ID 12
#define CAMERA_PROMPT_MESSAGE_LABEL_ID 13 // 提示消息的标签（拍照提示、录像提示）

#define CAMERA_BRIGHTNESS_CONT_ID 15		// 亮度设置的容器
#define CAMERA_COLOR_CONT_ID 16				// 色度设置的容器
#define CAMERA_CONTRAST_CONT_ID 17			// 对比度设置的容器
#define CAMERA_DISPLAY_DELAY_MASK_OBJ_ID 18 // 延时显示遮挡蒙版

#define CAMERA_MONITOR_COUNT_DOWN_ID 22 // 顶部监控倒计时

#define CAMERA_PROMPT_MESSAGE_ION_ID 20			// 开锁，录像的图片
#define CAMERA_PROMPT_MESSAGE_VIDEO_LABEL_ID 21 // 录像倒计时显示
#define CAMERA_OPEN_LOCK_ICON_ID 23
typedef enum
{
	CAMERA_TASK_TIME_DISP,
	CAMERA_TASK_NO_SDCARD_DISP,
	CAMERA_TASK_UNLOCK,
	CAMERA_TASK_RECORD_IMAGE,
	CAMERA_TASK_RECORD_VIDEO,
	CAMERA_TASK_MONITOR_COUNT,
	CAMERA_TASK_TOTAL,
} ticker_type;

typedef void (*ticker_func)(void);

typedef struct
{
	bool en;
	const int delay;
	int count;
	ticker_func handler_func;
} ticker_task_t;

typedef enum
{
	CAMERA_MODE_MONITOR = 0,
	CAMERA_MODE_ZOOM
} cam_mode_t;

static void layout_camera_door1_call_func(void);
static void layout_camera_door2_call_func(void);
static void layout_camera_callring_finish_default_func(int index);
static void layout_camera_click_down_func(lv_obj_t *obj);

static void camera_goto_monitor_mode(lv_obj_t *parent);

static void camera_record_photo_video(REC_MODE mode);
static void camera_setting_window_display_enable(bool en);
static void camera_display_delay_start(void);

static void camera_ticker_task_stop(ticker_type type);
static void camera_ticker_task_restart(ticker_type type);

static bool is_recording = false;
static bool is_opening = false;

static bool setting_win_diaplay_flag = true;

static cam_mode_t camera_mode = CAMERA_MODE_MONITOR;
static int camera_timeout_val = CAMERA_TIMEOUT_DURATION; // * 500ms
static int camera_display_delay = CAMERA_DISPLAY_DELAY;	 // * 100ms
static int camera_record_video_count_down = 15;
static lv_task_t *camera_display_delay_task_t = NULL;

static bool camera_enter_zoom = false;

static int current_adjust_mode = 0;
static lv_obj_t *current_focused_slider = NULL;

static void camera_change_door1_btn_up(lv_obj_t *obj)
{
	if (monitor_channel_get() == MON_CH_DOOR1)
	{
		return;
	}
	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	monitor_channel_set(MON_CH_DOOR1);
	goto_layout(pLAYOUT(camera));
}

static void camera_change_door2_btn_up(lv_obj_t *obj)
{
	if (monitor_channel_get() == MON_CH_DOOR2)
	{
		return;
	}
	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	monitor_channel_set(MON_CH_DOOR2);
	goto_layout(pLAYOUT(camera));
}

static void camera_change_cctv1_btn_up(lv_obj_t *obj)
{
	if (monitor_channel_get() == MON_CH_CCTV1)
	{
		return;
	}
	cctv_audio_video_enable_pin_ctrl(false);
	monitor_enter_mask_set(MON_ENTER_MANUAL_CCTV);
	monitor_channel_set(MON_CH_CCTV1);
	goto_layout(pLAYOUT(camera));
}

static void camera_change_cctv2_btn_up(lv_obj_t *obj)
{
	if (monitor_channel_get() == MON_CH_CCTV2)
	{
		return;
	}
	cctv_audio_video_enable_pin_ctrl(true);
	monitor_enter_mask_set(MON_ENTER_MANUAL_CCTV);
	monitor_channel_set(MON_CH_CCTV2);
	goto_layout(pLAYOUT(camera));
}

static void door_call_default_ring_time_stop(lv_task_t *task)
{
	if (ringplay_ing_check() == true)
	{

		ringplay_play_stop();
	}
	lv_task_del(task);
}
static void door_call_auto_camere(lv_task_t *task)
{

	camera_record_photo_video(REC_MODE_AUTO);

	lv_task_del(task);
}
// 复位监控倒计时
void camera_timeout_value_reset(void)
{
#if CAMERA_AUTO_RECORD_ENABLE
	camera_timeout_val = 7200; // 3600;
#else
	camera_timeout_val = CAMERA_TIMEOUT_DURATION;
#endif
}
#if 1
static void camera_unlock_ringa_start_func(int index)
{
	MON_CH ch = monitor_channel_get();
	ring_volume_set(3);
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
}

static void camera_unlock_ring_finish_func(int index)
{
	power_amplifier_enable(false);
	MON_CH ch = monitor_channel_get();
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
}
#endif
static void camera_head_time_display_flush(void)
{
	lv_obj_t *time_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_TIME_LABEL_ID);
	struct tm tm = {0};
	// static struct tm prev_tm = {0};
	user_time_read(&tm);

	// if(prev_tm.tm_min != tm.tm_min)
	// {
	if (user_data_get()->setting.calendar == 0)
	{
		struct date temp_date =
			{
				.year = tm.tm_year,
				.month = tm.tm_mon,
				.day = tm.tm_mday};
		temp_date = gregorian2jalali(temp_date);
		lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d:%02d", temp_date.year, temp_date.month, temp_date.day, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else
	{
		lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d:%02d", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}

	lv_obj_set_pos(time_label, 315, 24);
	// }
	// prev_tm = tm;
}

// 监控倒计时时间
// static void camera_head_monitor_count_flush(void)
// {
// 	lv_obj_t *time_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_MONITOR_COUNT_DOWN_ID);

// 	lv_label_set_text_fmt(time_label, "%02dS", camera_timeout_val / 2);
// 	lv_obj_set_pos(time_label, 789, 24);
// }
static void camera_sdcard_state_display_flush(void)
{
	lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_SDCADR_ICON_LABEL_ID);

	if (media_sdcard_insert_check())
	{
		if (sdcard_icon_obj != NULL)
		{
			static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SD_YES_PNG);
			static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_SDCARD_ERROR_PNG);
			if (user_data_get()->sd_card_pattion)
			{
				lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
			}
			else
			{
				lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
			}
			lv_obj_set_hidden(sdcard_icon_obj, false);
			camera_ticker_task_stop(CAMERA_TASK_NO_SDCARD_DISP);
		}
	}
	else
	{
		if (sdcard_icon_obj != NULL)
		{
			static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_NO_SDCARD_PNG);
			lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
			lv_obj_set_hidden(sdcard_icon_obj, false);
			camera_ticker_task_restart(CAMERA_TASK_NO_SDCARD_DISP);
		}
	}
}

// 时间显示刷新任务
static void camera_head_time_display_task(void)
{
	// printf("========[%d]==========[%s]==\n",__LINE__,__func__);
	camera_head_time_display_flush();
}
// 监控倒计时刷新任务
// static void camera_monitor_count_dowm_task(void)
// {
// 	// printf("========[%d]==========[%s]==\n",__LINE__,__func__);
// 	camera_head_monitor_count_flush();
// }
// 无SD卡显示任务
static void camera_sdcard_display_task(void)
{
	// printf("========[%d]==========[%s]==\n",__LINE__,__func__);
	lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_SDCADR_ICON_LABEL_ID);
	if (sdcard_icon_obj != NULL && media_sdcard_insert_check() == false)
	{
		lv_obj_set_hidden(sdcard_icon_obj, !lv_obj_get_hidden(sdcard_icon_obj));
	}
}
// 开锁任务
static void camera_unlock_task(void)
{
	// printf("========[%d]==========[%s]==\n",__LINE__,__func__);
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_OPEN_LOCK_ICON_ID);
	lv_obj_set_hidden(obj, true);

	monitor_unlcok_close();
	is_opening = false;
	layout_monitor_refresh_2();
	camera_ticker_task_stop(CAMERA_TASK_UNLOCK);
}

// 抓拍结束时的任务
static void camera_record_image_end_task(void)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	lv_obj_t *obj2 = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_VIDEO_LABEL_ID);
	if (!is_opening && obj != NULL)
	{
		lv_obj_set_hidden(obj, true);
		lv_obj_set_hidden(obj1, true);
		lv_obj_set_hidden(obj2, true);
	}
	is_recording = false;
	user_data_get()->new_photo_file_flag = true;
	camera_ticker_task_stop(CAMERA_TASK_RECORD_IMAGE);
}
// 录视频时的倒计时的任务
static void camera_record_video_count_down_task(void)
{
	printf("========[%d]==========[%s]==\n", __LINE__, __func__);
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	lv_obj_t *obj2 = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_VIDEO_LABEL_ID);
	if (--camera_record_video_count_down < 0 ||
		media_sdcard_insert_check() == false ||
		video_record_status_get() == false ||
		video_input_state_get() == false)
	{
		if (!is_opening && obj != NULL)
		{
			lv_obj_set_hidden(obj, true);
			lv_obj_set_hidden(obj1, true);
			lv_obj_set_hidden(obj2, true);
		}
		if (media_sdcard_insert_check())
		{
			user_data_get()->new_media_file_flag = true;
		}
		camera_ticker_task_stop(CAMERA_TASK_RECORD_VIDEO);
		is_recording = false;
		record_video_close();
		return;
	}
	camera_ticker_task_restart(CAMERA_TASK_RECORD_VIDEO);
	if (!is_opening && obj2 != NULL && obj1 != NULL)
	{
		lv_obj_set_hidden(obj, true);
		lv_obj_set_hidden(obj1, false);
		lv_obj_set_hidden(obj2, false);
		lv_label_set_text_fmt(obj2, "%02d ", camera_record_video_count_down);
	}
}

ticker_task_t ticker_task[CAMERA_TASK_TOTAL] = {
	{false, 2, 2, camera_head_time_display_task},
	{false, 4, 4, camera_sdcard_display_task},
	{false, 2, 2, camera_unlock_task},
	{false, 2, 2, camera_record_image_end_task},
	{false, 2, 2, camera_record_video_count_down_task},
	// {false, 2, 2, camera_monitor_count_dowm_task},
};

static void camera_ticker_task_restart(ticker_type type)
{
	ticker_task[type].count = ticker_task[type].delay;
	ticker_task[type].en = true;
	// printf("=====================================>>>> %s : %d\n", __func__, type);
}

static void camera_ticker_task_stop(ticker_type type)
{
	if (type == CAMERA_TASK_TOTAL)
	{
		for (int i = 0; i < CAMERA_TASK_TOTAL; i++)
		{
			ticker_task[i].en = false;
		}
	}
	else
	{
		ticker_task[type].en = false;
	}
	// printf("=====================================>>>> %s : %d\n", __func__, type);
}

static void camera_ticker_task(lv_task_t *task_t)
{
#if CAMERA_AUTO_RECORD_ENABLE
	printf("=================>>> timeout:[%d]\n", camera_timeout_val);
	camera_record_photo_video(REC_MODE_AUTO);
#endif
	if (camera_timeout_val-- <= 0)
	{
		goto_layout(pLAYOUT(standby));
	}

	for (int i = 0; i < CAMERA_TASK_TOTAL; i++)
	{
		if (ticker_task[i].en)
		{
			ticker_task[i].count--;
			if (ticker_task[i].count <= 0)
			{
				ticker_task[i].handler_func();
			}
		}
	}
}

static void camera_ticker_task_create(void)
{
	lv_layout_task_create(camera_ticker_task, 500, LV_TASK_PRIO_HIGH, NULL);
}

// SD卡状态显示
static void camera_sdcard_state_change_func(void)
{
	camera_sdcard_state_display_flush();
	if (media_sdcard_insert_check() == false)
	{
		user_data_get()->new_media_file_flag = false;
		user_data_get()->new_photo_file_flag = false;
	}
}

static void camera_head_channel_label_create(lv_obj_t *parent)
{
	lv_obj_t *ch_obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(ch_obj, CAMERA_HEAD_CH_LABEL_ID);
	lv_obj_set_pos(ch_obj, 34, 24);
	MON_CH ch = monitor_channel_get();
	if ((ch == MON_CH_DOOR1) || (ch == MON_CH_DOOR2))
	{
		// lv_label_set_text(ch_obj, monitor_channel_get() == MON_CH_DOOR1 ? str_get(LAYOUT_HOME_LANG_DOOR1_ID) : str_get(LAYOUT_HOME_LANG_DOOR2_ID));
		lv_obj_set_style_local_value_str(ch_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, monitor_channel_get() == MON_CH_DOOR1 ? str_get(LAYOUT_HOME_LANG_DOOR1_ID) : str_get(LAYOUT_HOME_LANG_DOOR2_ID));
	}
	else if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		if (user_data_get()->setting.language == LANG_PERSIAN)
		{
			lv_obj_set_style_local_value_ofs_x(ch_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 30);
		}
		lv_obj_set_style_local_value_str(ch_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, monitor_channel_get() == MON_CH_CCTV1 ? str_get(LAYOUT_HOME_LANG_CCTV1_ID) : str_get(LAYOUT_HOME_LANG_CCTV2_ID));
		// lv_label_set_text(ch_obj, monitor_channel_get() == MON_CH_CCTV1 ? str_get(LAYOUT_HOME_LANG_CCTV1_ID) : str_get(LAYOUT_HOME_LANG_CCTV2_ID));
	}

	lv_obj_set_style_local_value_color(ch_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
	lv_obj_set_style_local_value_font(ch_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
}

static void camera_head_time_label_create(lv_obj_t *parent)
{
	lv_obj_t *time_label = lv_label_create(parent, NULL);
	lv_obj_set_id(time_label, CAMERA_HEAD_TIME_LABEL_ID);
	lv_obj_set_style_local_text_color(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
	lv_obj_set_style_local_text_font(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
	camera_head_time_display_flush();
	camera_ticker_task_restart(CAMERA_TASK_TIME_DISP);
}

// static void camera_head_monitor_count_label_create(lv_obj_t *parent)
// {
// 	lv_obj_t *countdown_label = lv_label_create(parent, NULL);
// 	lv_obj_set_id(countdown_label, CAMERA_MONITOR_COUNT_DOWN_ID);
// 	lv_obj_set_style_local_text_color(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
// 	lv_obj_set_style_local_text_font(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
// 	camera_head_monitor_count_flush();
// 	camera_ticker_task_restart(CAMERA_TASK_MONITOR_COUNT);
// }

static void camera_sdcard_icon_create(lv_obj_t *parent)
{
	lv_obj_t *sdcard_icon_obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(sdcard_icon_obj, CAMERA_HEAD_SDCADR_ICON_LABEL_ID);
	lv_obj_set_pos(sdcard_icon_obj, 862, 24);
	lv_obj_set_size(sdcard_icon_obj, 38, 38);
	camera_sdcard_state_display_flush();
}

// 提示消息的标签创建（开锁提示、录像提示）
static void camera_prompt_message_label_create(lv_obj_t *parent)
{
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_id(label, CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_label_set_text(label, "");
	lv_obj_set_pos(label, 380, 90);
	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0XFF453A));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));
	lv_obj_set_hidden(label, true);
}

// 录像倒计时显示
static void camera_prompt_message_per_label_create(lv_obj_t *parent)
{
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_id(label, CAMERA_PROMPT_MESSAGE_VIDEO_LABEL_ID);
	lv_label_set_text(label, "");
	lv_obj_set_pos(label, 794, 27);
	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFD500));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(16));
	lv_obj_set_hidden(label, true);
}

// 提示消息的ion创建（开锁提示、录像提示）
static void camera_prompt_message_ion_create(lv_obj_t *parent)
{
	lv_obj_t *obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(obj, CAMERA_PROMPT_MESSAGE_ION_ID);
	lv_obj_set_size(obj, 48, 48);
	lv_obj_set_pos(obj, 784, 16);
	lv_obj_set_hidden(obj, true);
}

// 开锁提示图片
static void camera_open_lock_prompt_icon_create(lv_obj_t *parent)
{
	lv_obj_t *obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(obj, CAMERA_OPEN_LOCK_ICON_ID);
	lv_obj_set_size(obj, 140, 140);
	lv_obj_set_pos(obj, 442, 230);
	lv_obj_set_hidden(obj, true);
}

// 监控视频参数的设置 滑块创建（
static void camera_video_param_setting_btn_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, const char *str, obj_click_data *slider_data, int value, unsigned int id)
{
	lv_obj_t *cont = lv_cont_create(parent, NULL);
	lv_obj_set_id(cont, id);
	lv_obj_set_pos(cont, x, y);
	lv_obj_set_size(cont, 700, 91);

	lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str);

	lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -180);
	lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	// 默认状态文字颜色
	lv_obj_set_style_local_text_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	// 按下状态文字变蓝
	lv_obj_set_style_local_text_color(cont, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x007AFF));
	lv_obj_set_style_local_text_color(cont, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x007AFF));
	// 创建可交互的滑块
	lv_obj_t *slider = lv_slider_create(cont, NULL);
	lv_group_add_obj(lv_group_get_default(), slider);
	lv_obj_set_click(slider, true); // 允许滑块被点击拖动
	lv_obj_set_size(slider, 298, 12);
	lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 66, 0);
	lv_slider_set_range(slider, 0, 8);
	lv_slider_set_value(slider, value, LV_ANIM_ON);
	lv_obj_set_adv_hittest(slider, false);
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x0A0A0A));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xD9D9D9));
	// 滑块按下状态样式（变蓝）
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_PRESSED, lv_color_hex(0x007AFF));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_PRESSED, lv_color_hex(0x007AFF));
	lv_obj_set_style_local_pad_all(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 3);

	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_FOCUSED, lv_color_hex(0x007AFF));
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, lv_color_hex(0x007AFF));
	lv_obj_set_style_local_pad_all(slider, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, 3);

	// 绑定滑块事件回调
	if (slider_data != NULL)
		obj_click_event_listen(slider, slider_data);

	// 保持数据关联
	cont->user_data = slider;
	slider->user_data = cont;
}

// 亮度调节滑块回调
static void camera_brightness_adj_btn_up(lv_obj_t *obj)
{
	if (current_adjust_mode)
	{
		// 获取滑块当前值
		int brightness = lv_slider_get_value(obj);
		printf("=============>>>>>>>>>>>>>[%d]\n", brightness);
		// 设置亮度值并更新显示
		monitor_display_brightness_vol_set(brightness);
		display_bright_adj(brightness, INVALID_FORMAT);
		common_btn_bule_triangle_hidden();
		lv_group_focus_freeze(lv_group_get_default(), false);
		current_focused_slider = NULL;
		current_adjust_mode = false;
	}
	else
	{
		current_adjust_mode = 1;
		lv_group_focus_freeze(lv_group_get_default(), true);
		current_focused_slider = lv_group_get_focused(lv_group_get_default());
		common_btn_bule_triangle_display(current_focused_slider);
	}
}

// 色彩调节滑块回调
static void camera_color_adj_btn_up(lv_obj_t *obj)
{
	if (current_adjust_mode)
	{
		// 获取滑块当前值
		int color = lv_slider_get_value(obj);
		printf("=============>>>>>>>>>>>>>[%d]\n", color);
		// 设置色彩值并更新显示
		monitor_display_color_vol_set(color);
		display_color_adj(color, INVALID_FORMAT);
		// 恢复默认样式
		lv_obj_clear_state(obj, LV_STATE_PRESSED);
		common_btn_bule_triangle_hidden();
		lv_group_focus_freeze(lv_group_get_default(), false);
		current_focused_slider = NULL;
		current_adjust_mode = false;
	}
	else
	{
		current_adjust_mode = 2;
		lv_group_focus_freeze(lv_group_get_default(), true);
		current_focused_slider = lv_group_get_focused(lv_group_get_default());
		common_btn_bule_triangle_display(current_focused_slider);
	}
}

// 对比度调节滑块回调
static void camera_contrast_adj_btn_up(lv_obj_t *obj)
{
	if (current_adjust_mode)
	{
		// 获取滑块当前值
		int contrast = lv_slider_get_value(obj);
		printf("=============>>>>>>>>>>>>>[%d]\n", contrast);
		// 设置对比度值并更新显示
		monitor_display_cont_vol_set(contrast);
		display_const_adj(contrast, INVALID_FORMAT);
		// 恢复默认样式
		lv_obj_clear_state(obj, LV_STATE_PRESSED);
		common_btn_bule_triangle_hidden();
		lv_group_focus_freeze(lv_group_get_default(), false);
		current_focused_slider = NULL;
		current_adjust_mode = false;
	}
	else
	{
		current_adjust_mode = 3;
		lv_group_focus_freeze(lv_group_get_default(), true);
		current_focused_slider = lv_group_get_focused(lv_group_get_default());
		common_btn_bule_triangle_display(current_focused_slider);
	}
}

// 监控设置窗口创建
static void camera_setting_window_create(lv_obj_t *parent)
{
	lv_obj_t *cont = lv_cont_create(parent, NULL);
	lv_obj_set_click(cont, false);
	lv_obj_set_id(cont, CAMERA_SETTING_WINDOW_ID);
	lv_obj_set_pos(cont, 160, 185);
	lv_obj_set_size(cont, 704, 273);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4F4F4F));
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
	lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 30);

	// 为每个滑块创建独立的事件数据并绑定对应的回调函数
	static obj_click_data slider_data1 = obj_click_data_up_create(camera_brightness_adj_btn_up);
	static obj_click_data slider_data2 = obj_click_data_up_create(camera_color_adj_btn_up);
	static obj_click_data slider_data3 = obj_click_data_up_create(camera_contrast_adj_btn_up);

	// 创建三个参数的滑块控件
	camera_video_param_setting_btn_create(cont, 0, 21, str_get(LAYOUT_CAMERA_LANG_BRIGHTNESS_ID), &slider_data1, monitor_display_brightness_vol_get(), CAMERA_BRIGHTNESS_CONT_ID);
	camera_video_param_setting_btn_create(cont, 0, 92, str_get(LAYOUT_CAMERA_LANG_COLOR_ID), &slider_data2, monitor_display_color_vol_get(), CAMERA_COLOR_CONT_ID);
	camera_video_param_setting_btn_create(cont, 0, 164, str_get(LAYOUT_CAMERA_LANG_CONTRAST_ID), &slider_data3, monitor_display_cont_vol_get(), CAMERA_CONTRAST_CONT_ID);
}
// extern bool video_format_is_invalid(void);

// 图标按键创建
lv_obj_t *camera_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
{
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_ext_click_area(btn_obj, 12, 12, 10, 15);
	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);

	// 使对象可聚焦
	lv_obj_set_click(btn_obj, true);
	lv_obj_add_protect(btn_obj, LV_PROTECT_CLICK_FOCUS);
	lv_group_add_obj(lv_group_get_default(), btn_obj);
	if (icon_src != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
		lv_obj_set_style_local_pattern_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	}

	// lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	// lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_RADIUS_CIRCLE);
	// 1. 图标颜色叠加：默认和按下都叠加黑色（0x000000）
	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));

	// 2. 叠加透明度：默认低透明（图标显原始色），按下高透明（图标变深色）
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);	// 默认无叠加（原始色）
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色（深色）

	if (string != NULL)
	{
		lv_obj_set_style_local_value_str(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
		lv_obj_set_style_local_value_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
		lv_obj_set_style_local_value_font(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	}

	obj_click_event_listen(btn_obj, btn_pdata);

	return btn_obj;
}

// 抓拍或录像
static void camera_record_photo_video(REC_MODE mode)
{
	if (video_input_state_get() == false || is_recording == true)
		return;

	printf("=============>> record_mode : [%d] \n", user_data_get()->setting.record_mode);
	lv_obj_t *msg_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_obj_t *sec_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_VIDEO_LABEL_ID);
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	layout_monitor_refresh_2();

	if (user_data_get()->setting.record_mode == RECORD_MODE_IMAGE || media_sdcard_insert_check() == false)
	{
		if (record_jpeg_start(mode) == true)
		{
			printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
			user_data_get()->media_disp_mode = 0;
			is_recording = true;
			static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_PHOTO_RECORD_PNG);
			lv_obj_set_style_local_pattern_image(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
			lv_label_set_text(msg_label, str_get(LAYOUT_CAMERA_LANG_RECORD_IMAGE_ID));
			// lv_obj_align_mid_x(msg_label, NULL, LV_ALIGN_CENTER, 0);
			lv_obj_set_hidden(msg_label, false);
			lv_obj_set_hidden(obj, false);
			printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
			camera_ticker_task_restart(CAMERA_TASK_RECORD_IMAGE);
			printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
		}
	}
	else if (user_data_get()->setting.record_mode == RECORD_MODE_VIDEO)
	{
		if (record_video_start(mode) == true)
		{
			user_data_get()->media_disp_mode = 1;
			is_recording = true;
			camera_record_video_count_down = 15;
			lv_label_set_text_fmt(msg_label, "%s", str_get(LAYOUT_CAMERA_LANG_RECORD_VIDEO_ID));

			static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_VIDEO_RECORD_PNG);
			lv_obj_set_style_local_pattern_image(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);

			lv_label_set_text_fmt(sec_label, "%d ", camera_record_video_count_down);

			lv_obj_set_hidden(msg_label, false);
			lv_obj_set_hidden(obj, false);
			lv_obj_set_hidden(sec_label, false);
			camera_ticker_task_restart(CAMERA_TASK_RECORD_VIDEO);
		}
	}
}
static void camera_record_photo_video_task(lv_task_t *task)
{

	camera_record_photo_video(REC_MODE_AUTO);

	lv_task_del(task);
}

static void camera_setting_window_display_enable(bool en)
{
	lv_obj_t *win = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SETTING_WINDOW_ID);
	if (win == NULL)
		return;

	lv_obj_t *cont = lv_obj_get_child_form_id(win, CAMERA_BRIGHTNESS_CONT_ID);
	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_brightness_vol_get(), LV_ANIM_OFF);
	cont = lv_obj_get_child_form_id(win, CAMERA_COLOR_CONT_ID);
	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_color_vol_get(), LV_ANIM_OFF);
	cont = lv_obj_get_child_form_id(win, CAMERA_CONTRAST_CONT_ID);
	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_cont_vol_get(), LV_ANIM_OFF);

	layout_monitor_refresh_4();

	lv_obj_set_hidden(win, !en);
}

static void camera_bg_btn_up(lv_obj_t *obj)
{
}
// 背景的点击使能
static void camera_bg_btn_click_enable(void)
{
	static obj_click_data bg_btn_data = obj_click_data_up_create(camera_bg_btn_up);

	obj_click_event_listen(lv_scr_act(), &bg_btn_data);
	lv_group_add_obj(lv_group_get_default(), lv_scr_act());
}
// 如果是Screen adjust设置。
static void camera_screen_adjust_enable(bool en)
{
	if (en)
	{
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_SDCADR_ICON_LABEL_ID), true);
		camera_setting_window_display_enable(true); /*win容器和亮度等的ui显示与否*/
	}
}

static void camera_key_up_home(lv_key_t key)
{
	MON_CH ch = monitor_channel_get();
	switch (ch)
	{
	case MON_CH_DOOR1:
		camera_change_door2_btn_up(NULL);
		break;
	case MON_CH_DOOR2:
		camera_change_door1_btn_up(NULL);
		break;
	case MON_CH_CCTV1:
		camera_change_cctv2_btn_up(NULL);
		break;
	case MON_CH_CCTV2:
		camera_change_cctv1_btn_up(NULL);
		break;
	default:
		break;
	}
}
static void camera_key_up_prev(lv_key_t key)
{
	if (video_input_state_get() == false || is_recording == true)
		return;

	layout_monitor_refresh_2();
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	lv_obj_t *msg_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	if (record_jpeg_start(REC_MODE_MANUAL) == true)
	{
		printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
		user_data_get()->media_disp_mode = 0;
		is_recording = true;

		static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_PHOTO_RECORD_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);

		lv_label_set_text(msg_label, str_get(LAYOUT_CAMERA_LANG_RECORD_IMAGE_ID));

		// lv_obj_align_mid_x(msg_label, NULL, LV_ALIGN_CENTER, 0);
		lv_obj_set_hidden(msg_label, false);
		lv_obj_set_hidden(obj, false);
		printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
		camera_ticker_task_restart(CAMERA_TASK_RECORD_IMAGE);
		printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
	}
}

static void camera_key_up_enter(lv_key_t key)
{
	if (video_input_state_get() == false || is_recording == true)
		return;

	lv_obj_t *msg_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_obj_t *sec_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_VIDEO_LABEL_ID);
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	layout_monitor_refresh_2();
	if (record_video_start(REC_MODE_MANUAL) == true)
	{
		user_data_get()->media_disp_mode = 1;
		is_recording = true;
		camera_record_video_count_down = 15;
		lv_label_set_text_fmt(msg_label, "%s", str_get(LAYOUT_CAMERA_LANG_RECORD_VIDEO_ID));

		static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_VIDEO_RECORD_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);

		lv_label_set_text_fmt(sec_label, "%d ", camera_record_video_count_down);

		lv_obj_set_hidden(msg_label, false);
		lv_obj_set_hidden(obj, false);
		lv_obj_set_hidden(sec_label, false);
		camera_ticker_task_restart(CAMERA_TASK_RECORD_VIDEO);
	}
}

static void camera_key_up_next(lv_key_t key)
{
	MON_CH ch = monitor_channel_get();
	if ((is_opening) || (ch == MON_CH_CCTV1 || ch == MON_CH_CCTV2))
		return;
	layout_monitor_refresh_3();
	lv_obj_t *lock_icon = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_OPEN_LOCK_ICON_ID);
	if (lock_icon != NULL)
	{

		static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_OPEN_DOOR_PNG);
		lv_obj_set_style_local_pattern_image(lock_icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_hidden(lock_icon, false);
	}
	camera_ticker_task_restart(CAMERA_TASK_UNLOCK);

	is_opening = true;
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
	monitor_unlock_open(0, ch);
	ringplay_play_form_index(7, 100, camera_unlock_ringa_start_func, camera_unlock_ring_finish_func, false);
}

static void camera_key_up_esc(lv_key_t key)
{
	if (user_data_get()->setting.window_display_enable)
	{
		goto_layout(pLAYOUT(setting));
	}
	else
	{
		goto_layout(pLAYOUT(home));
	}
}
// 按键绑定表
static const key_binding_t camera_key_bindings[] = {
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, camera_key_up_home),
	KEY_BIND(LV_KEY_PREV, common_btn_key_down, camera_key_up_prev),
	KEY_BIND(LV_KEY_ENTER, common_btn_key_down, camera_key_up_enter),
	KEY_BIND(LV_KEY_NEXT, common_btn_key_down, camera_key_up_next),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, camera_key_up_esc),

};

static void camera_adjust_key_long_down_next(lv_key_t key)
{
	if (current_focused_slider)
	{
		int value = lv_slider_get_value(current_focused_slider);

		if (value < 8)
		{
			lv_slider_set_value(current_focused_slider, ++value, LV_ANIM_ON);
			switch (current_adjust_mode)
			{
			case 1:
				monitor_display_brightness_vol_set(value);
				display_bright_adj(value, INVALID_FORMAT);
				break;

			case 2:
				monitor_display_color_vol_set(value);
				display_color_adj(value, INVALID_FORMAT);
				break;

			case 3:
				monitor_display_cont_vol_set(value);
				display_const_adj(value, INVALID_FORMAT);
				break;
			default:
				return;
			}
		}
	}
}
static void camera_adjust_key_down_next(lv_key_t key)
{
	common_btn_key_down(key);
	camera_adjust_key_long_down_next(key);
}

static void camera_adjust_key_long_down_prev(lv_key_t key)
{
	if (current_focused_slider)
	{
		int value = lv_slider_get_value(current_focused_slider);

		if (value > 0)
		{
			lv_slider_set_value(current_focused_slider, --value, LV_ANIM_ON);
			switch (current_adjust_mode)
			{
			case 1:
				monitor_display_brightness_vol_set(value);
				display_bright_adj(value, INVALID_FORMAT);
				break;

			case 2:
				monitor_display_color_vol_set(value);
				display_color_adj(value, INVALID_FORMAT);
				break;

			case 3:
				monitor_display_cont_vol_set(value);
				display_const_adj(value, INVALID_FORMAT);
				break;

			default:
				return;
			}
		}
	}
}

static void camera_adjust_key_down_prev(lv_key_t key)
{
	common_btn_key_down(key);
	camera_adjust_key_long_down_prev(key);
}

static void camera_adjust_key_up_home(lv_key_t key)
{
	setting_win_diaplay_flag = true;
	MON_CH ch = monitor_channel_get();
	switch (ch)
	{
	case MON_CH_DOOR1:
		camera_change_door2_btn_up(NULL);
		break;
	case MON_CH_DOOR2:
		camera_change_cctv1_btn_up(NULL);
		break;
	case MON_CH_CCTV1:
		camera_change_cctv2_btn_up(NULL);
		break;
	case MON_CH_CCTV2:
		camera_change_door1_btn_up(NULL);
		break;
	default:
		break;
	}
}
static void camera_adjust_key_up_esc(lv_key_t key)
{
	if (current_adjust_mode)
	{
		lv_group_focus_freeze(lv_group_get_default(), false);
		common_btn_bule_triangle_hidden();
		current_focused_slider = NULL;
		current_adjust_mode = false;
	}
	else
	{
		goto_layout(pLAYOUT(setting));
	}
}
// 按键绑定表
static const key_binding_t camera_adjust_key_bindings[] = {
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, camera_adjust_key_up_home),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_PREV, camera_adjust_key_down_prev, camera_adjust_key_long_down_prev),
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_NEXT, camera_adjust_key_down_next, camera_adjust_key_long_down_next),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, camera_adjust_key_up_esc),
};

// 进入监控模式
static void camera_goto_monitor_mode(lv_obj_t *parent)
{
	camera_mode = CAMERA_MODE_MONITOR; /*选择是进入平常模式*/

	video_input_display_zoom_set(100);
	video_input_display_offset_set(0, 0);
	lv_obj_clean(parent);

	camera_head_channel_label_create(parent); /*顶部通道label的显示*/
	camera_head_time_label_create(parent);	  /*顶部时间label的显示*/
	camera_sdcard_icon_create(parent);		  /*顶部SD卡ui状态的显示*/
	// camera_head_monitor_count_label_create(parent); /*顶部监控倒计时的显示*/
	camera_prompt_message_label_create(parent);		/*录像/抓拍之后label的显示*/
	camera_prompt_message_ion_create(parent);		/*开锁/录像/抓拍之后ui的显示*/
	camera_prompt_message_per_label_create(parent); /*录像倒计时label的显示*/

	camera_open_lock_prompt_icon_create(parent); /*开锁icon的显示*/

	MON_CH ch = monitor_channel_get();
	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/

	if (user_data_get()->setting.window_display_enable)
	{
		common_bottom_adujst_btn_create(bottom_parent);
		camera_setting_window_create(parent); /*创建一个容器和在之上创建亮度等的ui*/
		LAYOUT_KEY_BINDINGS(camera_adjust_key_bindings);
	}
	else
	{
		if (ch == MON_CH_DOOR1 || ch == MON_CH_DOOR2)
			common_bottom_monitor_btn_create(bottom_parent); /*按键ui显示*/
		else if (ch == MON_CH_CCTV1 || ch == MON_CH_CCTV2)
			common_bottom_CCTV_btn_create(bottom_parent); /*按键ui显示*/

		camera_bg_btn_click_enable();			  /*背景的点击使能*/
		LAYOUT_KEY_BINDINGS(camera_key_bindings); // 绑定当前页面的按键
	}

	camera_setting_window_display_enable(false); /*win容器和亮度等的ui显示与否*/

	layout_monitor_refresh_1();

	lyaout_sd_state_callback_register(camera_sdcard_state_change_func);
}

static void LAYOUT_ENTER_FUNC(camera)
{
	layout_monitor_refresh_1();
	MON_CH ch = monitor_channel_get();

	if (ch == MON_CH_DOOR1)
	{
		if (user_data_get()->setting.door1_ring_volume == 0)
		{
			power_amplifier_enable(false);
		}
	}
	else if (ch == MON_CH_DOOR2)
	{
		if (user_data_get()->setting.door2_ring_volume == 0)
		{
			power_amplifier_enable(false);
		}
	}

	camera_enter_zoom = false;
	current_adjust_mode = false;
	setting_win_diaplay_flag = false;
	current_focused_slider = NULL;
	monitor_open(true, 0x03);
	printf("==============[%d]:[%s]\n", __LINE__, __func__);
	audio_input_capture_enable(true); // 延时打开ai，ai打开的同时，铃声开始播放，会有顿一下
	jpg_encode_capture_enable(true);

	standby_timer_close();
	is_recording = false;
	is_opening = false;

	lv_obj_t *parent = lv_scr_act();
	lv_obj_set_style_local_pattern_image(parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);

	camera_goto_monitor_mode(parent);
	camera_screen_adjust_enable(user_data_get()->setting.window_display_enable);
	/* 主界面插卡的时候进入监控，有ui残留 */
	fb_gui_layer_rect_fill(0x00, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX);

	layout_door1_call_callback_register(layout_camera_door1_call_func);
	layout_door2_call_callback_register(layout_camera_door2_call_func);
	lv_obj_click_down_callback_register(layout_camera_click_down_func);

	ak_sleep_ms(500); // 延时播放铃声，ai打开的同时，铃声开始播放，会有顿一下

	camera_timeout_value_reset();
	camera_ticker_task_create();
	camera_display_delay_start();

	if ((monitor_enter_mask_get() == MON_ENTER_CALL))
	{
		MON_CH ch = monitor_channel_get();
		if (call_record_start(true, ch, 0))
		{
			printf("Call record created for door station 1\n");
		}
		if (ch == MON_CH_DOOR1 && user_data_get()->setting.ring_time != 0)
		{
			ringplay_play_form_index(user_data_get()->setting.door1_tone, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, true);
		}
		else if (ch == MON_CH_DOOR2 && user_data_get()->setting.ring_time != 0)
		{
			ringplay_play_form_index(user_data_get()->setting.door2_tone, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, true);
		}
		lv_layout_task_create(door_call_default_ring_time_stop, user_data_get()->setting.ring_time * 1000, LV_TASK_PRIO_MID, NULL);
		if (user_data_get()->setting.ring_time == 0 || ringplay_force_mute_get() == true)
		{
			lv_layout_task_create(door_call_auto_camere, 3000, LV_TASK_PRIO_MID, NULL);
		}
	}
	if (hook_state_get() == true)
	{
		if (ch == MON_CH_DOOR1)
		{
			printf("=========================>>>> door1 talking \n");
			if (ringplay_ing_check() == true)
			{
				ringplay_play_stop();
			}
			if (call_record_answered(CALL_DOOR_STATION_1))
			{
				printf("Call answered for door station 1\n");
			}
			door_audio_talk(AUDIO_CH_DOOR1);
		}
		else if (ch == MON_CH_DOOR2)
		{
			printf("=========================>>>> door2 talking \n");
			if (call_record_answered(CALL_DOOR_STATION_2))
			{
				printf("Call answered for door station 1\n");
			}
			door_audio_talk(AUDIO_CH_DOOR2);
		}
		monitor_enter_mask_set(MON_ENTER_TALK);
	}
}
static void LAYOUT_QUIT_FUNC(camera)
{
	common_obj_null();
	ringplay_play_stop();
	video_input_display_zoom_set(100);
	video_input_display_offset_set(0, 0);
	layout_door1_call_callback_register(layout_door1_call_default);
	layout_door2_call_callback_register(layout_door2_call_default);
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
	camera_ticker_task_stop(CAMERA_TASK_TOTAL);
	audio_input_capture_enable(false);
	camera_bg_btn_click_enable();

	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);

	door_audio_talk(AUDIO_CH_CLOSE);
	monitor_close();
	record_video_close();
	record_jpeg_close();

	call_record_end(CALL_DOOR_STATION_1);
	call_record_end(CALL_DOOR_STATION_2);
	if (!setting_win_diaplay_flag)
		user_data_get()->setting.window_display_enable = false;
	user_data_save();
	standby_timer_restart(true);
	monitor_unlcok_close();
	camera_display_delay_task_t = NULL;
}

// 监控界面点击按键音处理函数
static void layout_camera_click_down_func(lv_obj_t *obj)
{
}

// 监控界面door1 call机处理函数
static void layout_camera_door1_call_func(void)
{
	monitor_valid_channel_set(MON_CH_DOOR1, true);
	monitor_enter_mask_set(MON_ENTER_CALL);
	MON_CH ch = monitor_channel_get();
	if (ch != MON_CH_DOOR1)
	{
		monitor_channel_set(MON_CH_DOOR1);
		goto_layout(pLAYOUT(camera));
		return;
	}
	if (camera_enter_zoom == true)
	{
		camera_enter_zoom = false;
		camera_goto_monitor_mode(lv_scr_act());
	}
	if (user_data_get()->setting.ring_time != 0)
		ringplay_play_form_index(user_data_get()->setting.door1_tone, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, false);
}

// 监控界面door2 call机处理函数
static void layout_camera_door2_call_func(void)
{
	monitor_valid_channel_set(MON_CH_DOOR2, true);
	monitor_enter_mask_set(MON_ENTER_CALL);
	MON_CH ch = monitor_channel_get();
	if (ch != MON_CH_DOOR2)
	{
		monitor_channel_set(MON_CH_DOOR2);
		goto_layout(pLAYOUT(camera));
		return;
	}
	if (camera_enter_zoom == true)
	{
		camera_enter_zoom = false;
		camera_goto_monitor_mode(lv_scr_act());
	}
	if (user_data_get()->setting.ring_time != 0)
		ringplay_play_form_index(user_data_get()->setting.door2_tone, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, false);
}

static void layout_camera_callring_finish_default_func(int index)
{
	power_amplifier_enable(false);
	MON_CH ch = monitor_channel_get();
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
	if ((user_data_get()->setting.record_mode != RECORD_MODE_INIT) && (camera_enter_zoom == false))
	{
		lv_layout_task_create(camera_record_photo_video_task, 1000, LV_TASK_PRIO_MID, NULL);
	}
}

// 监控延时显示任务
static void camera_display_delay_task(lv_task_t *task_t)
{
	if (--camera_display_delay < 0 || video_input_state_get())
	{
		lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_DISPLAY_DELAY_MASK_OBJ_ID);
		if (obj != NULL)
		{
			lv_obj_del(obj);
		}
		lv_task_del(task_t);
		camera_display_delay_task_t = NULL;
		backlight_enable(true);
	}
}
// 监控延时显示开始
static void camera_display_delay_start(void)
{
	backlight_enable(false);
	camera_display_delay = CAMERA_DISPLAY_DELAY;
	if (camera_display_delay_task_t == NULL)
	{
		camera_display_delay_task_t = lv_layout_task_create(camera_display_delay_task, 100, LV_TASK_PRIO_HIGH, NULL);
		lv_obj_t *mask_obj = lv_obj_create(lv_scr_act(), NULL);
		lv_obj_set_id(mask_obj, CAMERA_DISPLAY_DELAY_MASK_OBJ_ID);
		lv_obj_set_pos(mask_obj, 0, 0);
		lv_obj_set_size(mask_obj, 1024, 600);
	}
}

CREATE_LAYOUT(camera);
