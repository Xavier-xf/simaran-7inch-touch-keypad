#include "layout_define.h"
#include "ringplay.h"

typedef enum
{
	MT_BOTTOM_MAIN_BTN_ID,
	MT_BOTTOM_HOME_BTN_ID,
	MT_BOTTOM_LEFT_BTN_ID,
	MT_BOTTOM_SELECT_BTN_ID,
	MT_BOTTOM_RIGHT_BTN_ID,
	MT_BOTTOM_BACK_BTN_ID,
	MT_TOP_TIME_BTN_ID,
	MT_TOP_BOTTOM_TOTAL_BTN,
} time_key_btn_module;

custom_area time_key_btn_area[MT_TOP_BOTTOM_TOTAL_BTN] =
	{
		{934, 0, 90, 600},
		{0, 40, 90, 80},
		{0, 150, 90, 80},
		{0, 260, 90, 80},
		{0, 370, 90, 80},
		{0, 480, 90, 80},
		{310, 35, 314, 29},
};

custom_area *commom_time_key_btn_area = time_key_btn_area;
lv_obj_t *top_time_parent = NULL;
static lv_obj_t *time_cont = NULL;
lv_obj_t *bottom_parent = NULL;
lv_obj_t *bottom_left_btn = NULL;
lv_obj_t *bottom_select_btn = NULL;
lv_obj_t *bottom_right_btn = NULL;
lv_obj_t *bottom_back_btn = NULL;
lv_obj_t *bottom_home_btn = NULL;
lv_obj_t *common_triangle_obj = NULL;
lv_obj_t *common_bule_triangle_obj = NULL;

void common_obj_null()
{
	top_time_parent = NULL;
	bottom_parent = NULL;
	bottom_left_btn = NULL;
	bottom_select_btn = NULL;
	bottom_right_btn = NULL;
	bottom_back_btn = NULL;
	bottom_home_btn = NULL;
	common_triangle_obj = NULL;
	common_bule_triangle_obj = NULL;
}

/***
** 日期: 2022-05-20 17:30
** 作者: leo.liu
** 函数作用：铃声结束的默认处理
** 返回参数说明：
***/
void ringplay_keysound_finish_default_func(int index)
{
	/***** 关闭功放 *****/
	// if (cur_layout_get() != pLAYOUT(memory_video) && cur_layout_get() != pLAYOUT(door_ring))
	// 	power_amplifier_enable(false);
}
void ringplay_keysound_start_default_func(int index)
{
	ring_volume_set(TOUCH_TONE_VOL);
}
/***
** 日期: 2022-04-28 09:53
** 作者: leo.liu
** 函数作用：控件被按下执行的回调函数
** 返回参数说明：
***/
void layout_obj_click_down_func(lv_obj_t *obj)
{
	//***** 控制声音流向 *****/
	touch_sound_play(ringplay_keysound_start_default_func, ringplay_keysound_finish_default_func);
}

/***
** 日期: 2022-05-21 08:12
** 作者: leo.liu
** 函数作用：门口机铃声开始
** 返回参数说明：
***/
void ringplay_doorcall_start_default_func(int index)
{
	MON_CH ch = monitor_channel_get();
	ring_volume_set(ch == MON_CH_DOOR1 ? user_data_get()->setting.door1_ring_volume : user_data_get()->setting.door2_ring_volume);
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
}
/***
** 日期: 2022-05-21 08:12
** 作者: leo.liu
** 函数作用：门口机铃声结束
** 返回参数说明：
***/
void ringplay_doorcall_finish_default_func(int index)
{
	/***** 开启功放 *****/
	power_amplifier_enable(false);
	MON_CH ch = monitor_channel_get();
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
}
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：door1 call 默认处理函数
** 返回参数说明：
***/
void layout_door1_call_default(void)
{
	monitor_valid_channel_set(MON_CH_DOOR1, true);
	if (cur_layout_get() != pLAYOUT(camera) /* && cur_layout_get() != pLAYOUT(intercom_talk) */)
	{

		intercom_state_set(INTERCOM_STATE_HUNG_UP);
		monitor_channel_set(MON_CH_DOOR1);
		monitor_enter_mask_set(MON_ENTER_CALL);
		goto_layout(pLAYOUT(camera));
		// if (hook_state_get() == false)
		// {
		// ringplay_play_form_index(user_data_get()->setting.door1_tone, 100, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
		// }
	}
}

/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：door2 call 默认处理函数
** 返回参数说明：通道设置必须放在goto后面，
***/
void layout_door2_call_default(void)
{
	monitor_valid_channel_set(MON_CH_DOOR2, true);
	if (cur_layout_get() != pLAYOUT(camera) /* && cur_layout_get() != pLAYOUT(intercom_talk) */)
	{
		intercom_state_set(INTERCOM_STATE_HUNG_UP);
		monitor_channel_set(MON_CH_DOOR2);
		monitor_enter_mask_set(MON_ENTER_CALL);
		goto_layout(pLAYOUT(camera));
		// if (hook_state_get() == false)
		// {
		// ringplay_play_form_index(user_data_get()->setting.door2_tone, 100, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
		// }
	}
}

#define GATE_OPEN_DELAY 1000 // ms
#define ELEVATOR_CALL_DELAY 1000
static lv_task_t *gate_open_task_t = NULL;
static void gate_open_task(lv_task_t *task_t)
{
	gate_unlock_pin_ctrl(false);
	lv_task_del(gate_open_task_t);
	gate_open_task_t = NULL;
}

static lv_task_t *elevator_call_task_t = NULL;

static void elevator_call_task(lv_task_t *task_t)
{
	elevator_on_pin_ctrl(false);
	lv_task_del(elevator_call_task_t);
	elevator_call_task_t = NULL;
}
void open_ring_play_start_default_func(int index)
{
	ring_volume_set(OPEN_TONE_VOL);
}
void open_ring_play_finish_default_func(int index)
{
	// power_amplifier_enable(false);
}
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：室内机开锁键按下 默认处理函数
** 返回参数说明：
***/
void layout_gate_open_default(void)
{
	if (gate_open_task_t == NULL)
	{
		gate_unlock_pin_ctrl(true);
		ringplay_play_form_index(7, 100, open_ring_play_start_default_func, open_ring_play_finish_default_func, false);
		gate_open_task_t = lv_layout_task_create(gate_open_task, GATE_OPEN_DELAY, LV_TASK_PRIO_LOW, NULL);
		gate_open_task_t->clean_lock = false;
	}
}
/***
** 日期: 2022-05-20 14:29
** 作者: leo.liu
** 函数作用：室内机呼梯键按下 默认处理函数
** 返回参数说明：
***/
void layout_elevator_call_default(void)
{
	if (elevator_call_task_t == NULL)
	{
		elevator_on_pin_ctrl(true);
		ringplay_play_form_index(7, 100, open_ring_play_start_default_func, open_ring_play_finish_default_func, false);
		elevator_call_task_t = lv_layout_task_create(elevator_call_task, ELEVATOR_CALL_DELAY, LV_TASK_PRIO_LOW, NULL);
		elevator_call_task_t->clean_lock = false;
	}
}
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：hook state change 默认处理函数(听筒状态改变)
** 返回参数说明：
***/
bool layout_hook_state_change_default(unsigned int cmd, unsigned int arg)
{
	if (cur_layout_get() == pLAYOUT(logo))
	{
		return true;
	}
	else if (cur_layout_get() == pLAYOUT(standby))
	{
		return true;
	}
	else if (cur_layout_get() == pLAYOUT(camera))
	{
		if (cmd)
		{
			if (ringplay_ing_check() == true)
			{
				ringplay_play_stop();
			}
			// ringplay_play_stop();
			MON_CH ch = monitor_channel_get();
			if (ch == MON_CH_DOOR1)
			{
				printf("=========================>>>> door1 talking \n");
				door_audio_talk(AUDIO_CH_DOOR1);
				if (call_record_answered(CALL_DOOR_STATION_1))
				{
					printf("Call answered for door station 1\n");
				}
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
			extern void camera_timeout_value_reset(void);
			camera_timeout_value_reset();
			monitor_enter_mask_set(MON_ENTER_TALK);
		}
		else
		{
			printf("=============>>> hung up \n");
			goto_layout(pLAYOUT(standby));
		}
	}
	else if (cur_layout_get() == pLAYOUT(intercom_in))
	{
		if (cmd == true && intercom_state_get() != INTERCOM_STATE_HUNG_UP && intercom_state_get() != INTERCOM_STATE_IDLE)
		{
			ringplay_play_stop();
			call_record_answered(0);
			printf("=============>>> intercom in talking \n");
			goto_layout(pLAYOUT(intercom_talk));
		}
		else if (cmd == false)
		{
			printf("=============>>> 挂断 \n");
			intercom_state_set(INTERCOM_STATE_HUNG_UP);
			goto_layout(pLAYOUT(standby));
		}
	}
	else if (cur_layout_get() == pLAYOUT(intercom_out))
	{
		if (cmd == false)
		{
			printf("=============>>> 挂断 \n");
			intercom_state_set(INTERCOM_STATE_HUNG_UP);
			goto_layout(pLAYOUT(standby));
		}
	}
	else if (cur_layout_get() == pLAYOUT(intercom_talk))
	{
		if (cmd == false)
		{
			printf("=============>>> 挂断 \n");
			intercom_state_set(INTERCOM_STATE_HUNG_UP);
			goto_layout(pLAYOUT(standby));
		}
	}
	else
	{

		if (cmd == false)
		{
			printf("=============>>> 待机 \n");
			goto_layout(pLAYOUT(standby));
		}
	}
	return true;
}

/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：sd卡状态改变默认回调
** 返回参数说明：
***/
void layout_sdcard_state_change_default(void)
{
	if (media_sdcard_insert_check() == false)
	{
		user_data_get()->new_media_file_flag = false;
		user_data_get()->new_photo_file_flag = false;
		user_data_save();
	}
}

static MON_ENTER_FLG layout_monitor_enter_flag = MON_ENTER_MANUAL_DOOR;
/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：设置进入监控的标志位
** 返回参数说明：
***/
void monitor_enter_mask_set(MON_ENTER_FLG flg)
{
	layout_monitor_enter_flag = flg;
}

/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：获取进入监控的标志位
** 返回参数说明：
***/
MON_ENTER_FLG monitor_enter_mask_get(void)
{
	return layout_monitor_enter_flag;
}

/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道的亮度值
** 返回参数说明：
***/
int monitor_display_brightness_vol_get(void)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		return user_data_get()->camera.door1.bright;
	}
	else if (channel == MON_CH_DOOR2)
	{
		return user_data_get()->camera.door2.bright;
	}
	else if (channel == MON_CH_CCTV1)
	{
		return user_data_get()->camera.cctv1.bright;
	}
	else
	{
		return user_data_get()->camera.cctv2.bright;
	}
}

/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置亮度值
** 返回参数说明：
***/
void monitor_display_brightness_vol_set(int vol)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		user_data_get()->camera.door1.bright = vol;
	}
	else if (channel == MON_CH_DOOR2)
	{
		user_data_get()->camera.door2.bright = vol;
	}
	else if (channel == MON_CH_CCTV1)
	{
		user_data_get()->camera.cctv1.bright = vol;
	}
	else if (channel == MON_CH_CCTV2)
	{
		user_data_get()->camera.cctv2.bright = vol;
	}
	user_data_save();
}

/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道对比度值
** 返回参数说明：
***/
int monitor_display_cont_vol_get(void)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		return user_data_get()->camera.door1.cont;
	}
	else if (channel == MON_CH_DOOR2)
	{
		return user_data_get()->camera.door2.cont;
	}
	else if (channel == MON_CH_CCTV1)
	{
		return user_data_get()->camera.cctv1.cont;
	}
	else
	{
		return user_data_get()->camera.cctv2.cont;
	}
}

/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置对比度值
** 返回参数说明：
***/
void monitor_display_cont_vol_set(int vol)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		user_data_get()->camera.door1.cont = vol;
	}
	else if (channel == MON_CH_DOOR2)
	{
		user_data_get()->camera.door2.cont = vol;
	}
	else if (channel == MON_CH_CCTV1)
	{
		user_data_get()->camera.cctv1.cont = vol;
	}
	else if (channel == MON_CH_CCTV2)
	{
		user_data_get()->camera.cctv2.cont = vol;
	}
	user_data_save();
}

/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道的色度值
** 返回参数说明：
***/
int monitor_display_color_vol_get(void)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		return user_data_get()->camera.door1.color;
	}
	else if (channel == MON_CH_DOOR2)
	{
		return user_data_get()->camera.door2.color;
	}
	else if (channel == MON_CH_CCTV1)
	{
		return user_data_get()->camera.cctv1.color;
	}
	else
	{
		return user_data_get()->camera.cctv2.color;
	}
}

/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置亮度值
** 返回参数说明：
***/
void monitor_display_color_vol_set(int vol)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		user_data_get()->camera.door1.color = vol;
	}
	else if (channel == MON_CH_DOOR2)
	{
		user_data_get()->camera.door2.color = vol;
	}
	else if (channel == MON_CH_CCTV1)
	{
		user_data_get()->camera.cctv1.color = vol;
	}
	else if (channel == MON_CH_CCTV2)
	{
		user_data_get()->camera.cctv2.color = vol;
	}
	user_data_save();
}

// 电源指示灯默认处理函数
// void power_led_handler_default_func(void)
// {
// 	static unsigned long long timestemp = 0;
// 	static bool led_state = false;
// 	unsigned long long curr_times = user_timestamp_get();
// 	if (curr_times - timestemp > 1000)
// 	{
// 		timestemp = curr_times;
// 		if ((user_data_get()->new_media_file_flag) || (user_data_get()->new_photo_file_flag))
// 		{
// 			led_state = !led_state;
// 			power_led_enable(led_state);
// 		}
// 		else
// 		{
// 			led_state = true;
// 			power_led_enable(true);
// 		}
// 	}
// }

// 重新封装铃声播放函数
void ring_play(int index, int volume, ringplay_callback start, ringplay_callback finish, bool loop)
{
	if (0 == (monitor_channel_get() == MON_CH_DOOR1 ? user_data_get()->setting.door1_ring_volume : user_data_get()->setting.door2_ring_volume))
	{
		power_amplifier_enable(false);
	}
	else
	{
		power_amplifier_enable(true);
	}
	ringplay_play_form_index(index, volume, start, finish, loop);
}

/**
 * @brief 保存当前焦点对象的ID
 * @param save_id_ptr 用于保存焦点ID的变量指针
 * @param group 焦点组，传NULL使用默认组
 */
void lv_focus_save(unsigned int *save_id_ptr, lv_group_t *group)
{
	LV_ASSERT_NULL(save_id_ptr);

	if (group == NULL)
	{
		group = lv_group_get_default();
	}

	if (group != NULL)
	{
		lv_obj_t *focused_obj = lv_group_get_focused(group);
		if (focused_obj != NULL)
		{
			*save_id_ptr = lv_obj_get_id(focused_obj);
			printf("Focus saved: ID=%u\n", *save_id_ptr);
		}
		else
		{
			*save_id_ptr = 0; // 没有焦点对象，重置为0
			printf("No focused object to save\n");
		}
	}
	else
	{
		printf("No focus group available\n");
		*save_id_ptr = 0;
	}
}

/**
 * @brief 恢复之前保存的焦点
 * @param parent 父对象，通常是当前屏幕
 * @param saved_id 之前保存的焦点ID
 * @param group 焦点组，传NULL使用默认组
 * @return true 恢复成功，false 恢复失败
 */
lv_obj_t *lv_focus_restore(lv_obj_t *parent, unsigned int saved_id, lv_group_t *group)
{
	LV_ASSERT_NULL(parent);

	if (saved_id == 0)
	{ // 0为无效ID
		printf("No valid focus ID to restore\n");
	}

	if (group == NULL)
	{
		group = lv_group_get_default();
	}

	if (group == NULL)
	{
		printf("No focus group available for restore\n");
	}

	// 根据保存的ID查找当前界面的控件
	lv_obj_t *target_obj = lv_obj_get_child_form_id(parent, saved_id);
	lv_obj_get_parent(target_obj);

	lv_group_focus_obj(target_obj); // 恢复焦点
	printf("Focus restored to ID=%u\n", saved_id);
	return target_obj;
}

static void home_date_display(struct tm *time)
{
	static char str[32] = {0};
	if (user_data_get()->setting.calendar == 0)
	{
		struct date temp_date =
			{
				.year = time->tm_year + 1900,
				.month = time->tm_mon + 1,
				.day = time->tm_mday};
		temp_date = gregorian2jalali(temp_date);
		sprintf(str, "%04d-%02d-%02d   %02d:%02d:%02d", temp_date.year, temp_date.month, temp_date.day, time->tm_hour, time->tm_min, time->tm_sec);
	}
	else
	{
		sprintf(str, "%04d-%02d-%02d   %02d:%02d:%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
	}
	lv_obj_set_style_local_value_str(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str);
}

static void home_time_display_task(lv_task_t *task_t)
{
	time_t seconds = time(NULL);
	struct tm tm = {0};
	static struct tm prev_tm = {0};
	localtime_r(&seconds, &tm);

	if (prev_tm.tm_hour != tm.tm_hour || prev_tm.tm_min != tm.tm_min || prev_tm.tm_sec != tm.tm_sec || task_t == NULL)
	{
		home_date_display(&tm);
	}
	if (prev_tm.tm_year != tm.tm_year || prev_tm.tm_mon != tm.tm_mon || prev_tm.tm_mday != tm.tm_mday || task_t == NULL)
	{
		home_date_display(&tm);
	}

	prev_tm = tm;
}

bool top_time_date_text_create(lv_obj_t *parent) // 正上方时间显示 非sdandby时间
{
	/***** 创建时间容器 *****/
	time_cont = lv_cont_create(parent, NULL);
	lv_obj_set_pos(time_cont, 300, 35);
	lv_obj_set_size(time_cont, 314, 29);
	// lv_obj_align(time_cont, NULL, LV_ALIGN_IN_TOP_MID, -82, 10);

	/***** 创建日期,用value显示 *****/
	lv_obj_set_style_local_value_align(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
	lv_obj_set_style_local_value_ofs_x(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 40); //-27
	lv_obj_set_style_local_value_font(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
	lv_obj_set_style_local_value_color(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	home_date_display(&tm);

	lv_layout_task_create(home_time_display_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
	home_time_display_task(NULL);
	return true;
}

lv_obj_t *bottom_main(lv_obj_t *parent, custom_area *btn_area)
{
	lv_obj_t *bottom_parent = lv_obj_create(parent, NULL);
	lv_obj_set_pos(bottom_parent, btn_area->x, btn_area->y);
	lv_obj_set_size(bottom_parent, btn_area->w, btn_area->h);
	lv_obj_set_style_local_bg_color(bottom_parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0X000000));
	lv_obj_set_style_local_bg_opa(bottom_parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	return bottom_parent;
}

lv_obj_t *bottom_img_btn_create(lv_obj_t *bottom_parent, custom_area *btn_area, const void *icon_src)
{
	lv_obj_t *bottom_child = lv_obj_create(bottom_parent, NULL);
	lv_obj_set_pos(bottom_child, btn_area->x, btn_area->y);
	lv_obj_set_size(bottom_child, btn_area->w, btn_area->h);

	lv_obj_set_style_local_bg_color(bottom_child, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0X000000));
	lv_obj_set_style_local_bg_opa(bottom_child, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_bg_color(bottom_child, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_bg_opa(bottom_child, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_20);

	if (icon_src != NULL)
	{
		lv_obj_set_style_local_pattern_image(bottom_child, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
	}

	return bottom_child;
}
void common_btn_key_down(lv_key_t key)
{
	switch (key)
	{
	case LV_KEY_HOME:
		bottom_btn_opa_display(bottom_home_btn);
		break;
	case LV_KEY_ESC:
		bottom_btn_opa_display(bottom_back_btn);
		break;
	case LV_KEY_ENTER:
		bottom_btn_opa_display(bottom_select_btn);
		break;
	case LV_KEY_NEXT:
		bottom_btn_opa_display(bottom_right_btn);
		break;
	case LV_KEY_PREV:
		bottom_btn_opa_display(bottom_left_btn);
		break;
	default:

		break;
	}
}
void bottom_btn_opa_display(lv_obj_t *bottom_child)
{
	if (bottom_child != NULL)
		lv_obj_add_state(bottom_child, LV_STATE_PRESSED);
}

/**************************   按键按下效果隐藏   ***************************/
void bottom_btn_opa_hidden()
{
	// 底部左键判断
	if (bottom_left_btn != NULL)
	{
		lv_obj_clear_state(bottom_left_btn, LV_STATE_PRESSED);
	}
	// 底部选择键判断
	if (bottom_select_btn != NULL)
	{
		lv_obj_clear_state(bottom_select_btn, LV_STATE_PRESSED);
	}
	// 底部右键判断
	if (bottom_right_btn != NULL)
	{
		lv_obj_clear_state(bottom_right_btn, LV_STATE_PRESSED);
	}
	// 底部返回键判断
	if (bottom_back_btn != NULL)
	{
		lv_obj_clear_state(bottom_back_btn, LV_STATE_PRESSED);
	}
	// 底部主页键判断
	if (bottom_home_btn != NULL)
	{
		lv_obj_clear_state(bottom_home_btn, LV_STATE_PRESSED);
	}
}

void common_bottom_btn_create(lv_obj_t *bottom_parent)
{
	// 1、创建home按钮
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_COMMOM_HOME_PNG);
	bottom_home_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[1], &info1);

	// 2、创建left按钮
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_COMMOM_LEFT_PNG);
	bottom_left_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[2], &info2);

	// 3、创建select按钮
	static rom_bin_info info3 = rom_bin_info_get(ROM_UI_COMMOM_CONFIRM_PNG);
	bottom_select_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[3], &info3);

	// 4、创建right按钮
	static rom_bin_info info4 = rom_bin_info_get(ROM_UI_COMMOM_RIGHT_PNG);
	bottom_right_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[4], &info4);

	// 5、创建back按钮
	static rom_bin_info info5 = rom_bin_info_get(ROM_UI_COMMOM_BACK_PNG);
	bottom_back_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[5], &info5);
}

void common_bottom_monitor_btn_create(lv_obj_t *bottom_parent)
{
	// 1、创建change按钮
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_COMMOM_CHANGE_PNG);
	bottom_home_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[1], &info1);

	// 2、创建photo按钮
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_COMMOM_PHOTO_PNG);
	bottom_left_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[2], &info2);

	// 3、创建video按钮
	static rom_bin_info info3 = rom_bin_info_get(ROM_UI_COMMOM_VIDEO_PNG);
	bottom_select_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[3], &info3);

	// 4、创建lock按钮
	static rom_bin_info info4 = rom_bin_info_get(ROM_UI_COMMOM_LOCK_PNG);
	bottom_right_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[4], &info4);

	// 5、创建back按钮
	static rom_bin_info info5 = rom_bin_info_get(ROM_UI_COMMOM_BACK_PNG);
	bottom_back_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[5], &info5);
}

void common_bottom_CCTV_btn_create(lv_obj_t *bottom_parent)
{
	// 1、创建change按钮
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_COMMOM_CHANGE_PNG);
	bottom_home_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[1], &info1);

	// 2、创建photo按钮
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_COMMOM_PHOTO_PNG);
	bottom_left_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[2], &info2);

	// 3、创建video按钮
	static rom_bin_info info3 = rom_bin_info_get(ROM_UI_COMMOM_VIDEO_PNG);
	bottom_select_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[3], &info3);

	// 5、创建back按钮
	static rom_bin_info info5 = rom_bin_info_get(ROM_UI_COMMOM_BACK_PNG);
	bottom_back_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[5], &info5);
}

void common_bottom_view_btn_create(lv_obj_t *bottom_parent)
{
	// 1、创建delete按钮
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_COMMOM_DELETE_PNG);
	bottom_home_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[1], &info1);

	// 2、创建left按钮
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_COMMOM_LEFT_PNG);
	bottom_left_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[2], &info2);

	// 3、创建stop按钮
	static rom_bin_info info3 = rom_bin_info_get(ROM_UI_COMMOM_STOP_PNG);
	bottom_select_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[3], &info3);

	// 4、创建right按钮
	static rom_bin_info info4 = rom_bin_info_get(ROM_UI_COMMOM_RIGHT_PNG);
	bottom_right_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[4], &info4);

	// 5、创建back按钮
	static rom_bin_info info5 = rom_bin_info_get(ROM_UI_COMMOM_BACK_PNG);
	bottom_back_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[5], &info5);
}

void common_bottom_view_select_btn_create(lv_obj_t *bottom_parent)
{
	// 1、创建delete按钮
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_COMMOM_DELETE_PNG);
	bottom_home_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[1], &info1);

	// 2、创建left按钮
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_COMMOM_LEFT_PNG);
	bottom_left_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[2], &info2);

	// 3、创建select按钮
	static rom_bin_info info3 = rom_bin_info_get(ROM_UI_COMMOM_CONFIRM_PNG);
	bottom_select_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[3], &info3);

	// 4、创建right按钮
	static rom_bin_info info4 = rom_bin_info_get(ROM_UI_COMMOM_RIGHT_PNG);
	bottom_right_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[4], &info4);

	// 5、创建back按钮
	static rom_bin_info info5 = rom_bin_info_get(ROM_UI_COMMOM_BACK_PNG);
	bottom_back_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[5], &info5);
}

void common_bottom_adujst_btn_create(lv_obj_t *bottom_parent)
{
	// 1、创建change按钮
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_COMMOM_CHANGE_PNG);
	bottom_home_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[1], &info1);

	// 2、创建left按钮
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_COMMOM_LEFT_PNG);
	bottom_left_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[2], &info2);

	// 3、创建select按钮
	static rom_bin_info info3 = rom_bin_info_get(ROM_UI_COMMOM_CONFIRM_PNG);
	bottom_select_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[3], &info3);

	// 4、创建right按钮
	static rom_bin_info info4 = rom_bin_info_get(ROM_UI_COMMOM_RIGHT_PNG);
	bottom_right_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[4], &info4);

	// 5、创建back按钮
	static rom_bin_info info5 = rom_bin_info_get(ROM_UI_COMMOM_BACK_PNG);
	bottom_back_btn = bottom_img_btn_create(bottom_parent, &commom_time_key_btn_area[5], &info5);
}

/**************************   三角形图标  ***************************/
void common_btn_triangle_display(lv_obj_t *child_1)
{
	if (child_1 == NULL)
		return;

	// 获取按钮的父容器
	lv_obj_t *parent = lv_obj_get_parent(child_1);
	if (parent == NULL)
		parent = child_1; // 按钮无父容器，直接用按钮当父

	// 创建专门的图片对象
	common_triangle_obj = lv_img_create(parent, NULL); // 父对象改为 parent，与按钮同级
	if (common_triangle_obj == NULL)
		return;

	// 设置图片源
	static rom_bin_info info = rom_bin_info_get(ROM_UI_COMMOM_TRIANGLE_PNG);
	lv_img_set_src(common_triangle_obj, &info); // 图片对象

	//  设置图片大小
	lv_obj_set_size(common_triangle_obj, 21, 25);

	// 对齐：相对于按钮左外侧垂直居中
	lv_obj_align(common_triangle_obj, child_1, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}
void common_btn_triangle_hidden()
{
	if (common_triangle_obj != NULL)
	{
		lv_obj_del(common_triangle_obj);
		common_triangle_obj = NULL;
	}
}

/**************************  蓝色三角形图标  ***************************/
void common_btn_bule_triangle_display(lv_obj_t *child_1)
{
	if (child_1 == NULL)
		return;

	// 获取按钮的父容器
	lv_obj_t *parent = lv_obj_get_parent(child_1);
	if (parent == NULL)
		parent = child_1; // 按钮无父容器，直接用按钮当父

	// 创建专门的图片对象
	common_bule_triangle_obj = lv_img_create(parent, NULL); // 父对象改为 parent，与按钮同级
	if (common_bule_triangle_obj == NULL)
		return;

	// 设置图片源
	static rom_bin_info info = rom_bin_info_get(ROM_UI_COMMOM_BULE_TRIANGLE_PNG);
	lv_img_set_src(common_bule_triangle_obj, &info); // 图片对象

	//  设置图片大小
	lv_obj_set_size(common_bule_triangle_obj, 20, 24);

	// 对齐：相对于按钮左外侧垂直居中
	lv_obj_align(common_bule_triangle_obj, child_1, LV_ALIGN_OUT_LEFT_MID, -182, 0);
}

void common_btn_bule_triangle_hidden()
{
	if (common_bule_triangle_obj != NULL)
	{
		lv_obj_del(common_bule_triangle_obj);
		common_bule_triangle_obj = NULL;
	}
}

/**
 * @brief  照片列表焦点移动控制（支持向上/向下）
 * @param  container_id: 父容器ID（对应PHOTO_LIST_PHOTO_PAGE_BTN_ID）
 * @param  focused_id_ptr: 当前聚焦ID的指针（函数内部修改并同步外部状态）
 * @param  total_count: 子对象总数量（必须大于0）
 * @param  direction: 移动方向（LV_FOCUS_DIR_UP：往上，LV_FOCUS_DIR_DOWN：往下）
 * @return 成功聚焦的对象指针，失败返回NULL
 */
lv_obj_t *lv_photo_list_move_focus(uint32_t container_id, int *focused_id_ptr, int total_count, bool direction)
{
	// 参数合法性检查
	if (focused_id_ptr == NULL || total_count <= 0)
	{
		printf("焦点移动失败：参数无效（聚焦ID指针为空或总数量不合法）\n");
		return NULL;
	}

	lv_obj_t *focused_btn = NULL;
	// 找到指定ID的父容器
	lv_obj_t *image_page = lv_obj_get_child_form_id(lv_scr_act(), container_id);
	if (image_page == NULL)
	{
		printf("焦点移动失败：未找到ID为%d的父容器\n", container_id);
		return NULL;
	}

	int old_focused_id = *focused_id_ptr;
	int new_focused_id = old_focused_id;

	// 获取当前页面实际存在的控件ID范围
	int min_id = -1, max_id = -1;
	int page_control_ids[6] = {-1, -1, -1, -1, -1, -1}; // 存储当前页面所有控件ID
	int page_control_count = 0;

	lv_obj_t *child = lv_obj_get_child_back(image_page, NULL);
	while (child != NULL)
	{
		int child_id = lv_obj_get_id(child);
		if (min_id == -1 || child_id < min_id)
			min_id = child_id;
		if (max_id == -1 || child_id > max_id)
			max_id = child_id;

		// 保存当前页面的所有控件ID
		if (page_control_count < 6)
		{
			page_control_ids[page_control_count++] = child_id;
		}

		child = lv_obj_get_child_back(image_page, child);
	}

	// 如果没有找到任何子控件，直接返回
	if (min_id == -1 || max_id == -1)
	{
		printf("焦点移动失败：当前页面没有任何子控件\n");
		return NULL;
	}

	printf("当前页面控件ID范围: %d - %d, 当前焦点: %d, 页面控件: ", min_id, max_id, old_focused_id);
	for (int i = 0; i < page_control_count; i++)
	{
		printf("%d ", page_control_ids[i]);
	}
	printf("\n");

	// 计算总页数
	int page_size = 6;
	int total_pages = (total_count + page_size - 1) / page_size;

	// 如果是只有一页的情况，简化处理逻辑
	if (total_pages == 1)
	{
		if (direction == true) // 往下移动
		{
			// 在当前页面内按ID顺序向下移动（ID递增）
			int next_id = -1;
			for (int i = 0; i < page_control_count; i++)
			{
				if (page_control_ids[i] > old_focused_id)
				{
					if (next_id == -1 || page_control_ids[i] < next_id)
					{
						next_id = page_control_ids[i];
					}
				}
			}

			if (next_id != -1)
			{
				// 在当前页面找到了下一个ID
				new_focused_id = next_id;
			}
			else
			{
				// 当前页面没有更大的ID，回到第一个
				new_focused_id = min_id;
			}
		}
		else // 往上移动
		{
			// 在当前页面内按ID顺序向上移动（ID递减）
			int prev_id = -1;
			for (int i = 0; i < page_control_count; i++)
			{
				if (page_control_ids[i] < old_focused_id)
				{
					if (prev_id == -1 || page_control_ids[i] > prev_id)
					{
						prev_id = page_control_ids[i];
					}
				}
			}

			if (prev_id != -1)
			{
				// 在当前页面找到了上一个ID
				new_focused_id = prev_id;
			}
			else
			{
				// 当前页面没有更小的ID，回到最后一个
				new_focused_id = max_id;
			}
		}
	}
	else
	{
		// 多页情况，使用原有逻辑
		// 计算当前焦点所在的页
		int current_page = 0;
		for (int i = 0; i < page_control_count; i++)
		{
			if (page_control_ids[i] == old_focused_id)
			{
				// 根据控件ID判断所在页
				current_page = total_pages - (page_control_ids[i] / page_size);
				break;
			}
		}

		if (direction == true) // 往下移动
		{
			// 在当前页面内按ID顺序向下移动（ID递增）
			int next_id = -1;
			for (int i = 0; i < page_control_count; i++)
			{
				if (page_control_ids[i] > old_focused_id)
				{
					if (next_id == -1 || page_control_ids[i] < next_id)
					{
						next_id = page_control_ids[i];
					}
				}
			}

			if (next_id != -1)
			{
				// 在当前页面找到了下一个ID
				new_focused_id = next_id;
			}
			else
			{
				// 当前页面没有更大的ID，需要翻页

				// 如果是最后一页（控件ID最小的一页），回到第一页（控件ID最大的一页）
				if (current_page == total_pages)
				{
					// 找到整个列表中最大的ID
					new_focused_id = total_count - 1;
				}
				else
				{
					// 否则翻到上一页（控件ID更大的一页）
					// 这里需要外部翻页逻辑配合，我们先尝试在当前页面循环
					new_focused_id = min_id; // 回到当前页最小ID
				}
			}
		}
		else // 往上移动
		{
			// 在当前页面内按ID顺序向上移动（ID递减）
			int prev_id = -1;
			for (int i = 0; i < page_control_count; i++)
			{
				if (page_control_ids[i] < old_focused_id)
				{
					if (prev_id == -1 || page_control_ids[i] > prev_id)
					{
						prev_id = page_control_ids[i];
					}
				}
			}

			if (prev_id != -1)
			{
				// 在当前页面找到了上一个ID
				new_focused_id = prev_id;
			}
			else
			{
				// 当前页面没有更小的ID，需要翻页

				// 如果是第一页（控件ID最大的一页），回到最后一页（控件ID最小的一页）
				if (current_page == 1)
				{
					// 找到整个列表中最小的ID
					new_focused_id = 0;
				}
				else
				{
					// 否则翻到下一页（控件ID更小的一页）
					// 这里需要外部翻页逻辑配合，我们先尝试在当前页面循环
					new_focused_id = max_id; // 回到当前页最大ID
				}
			}
		}
	}

	// 更新聚焦ID
	*focused_id_ptr = new_focused_id;

	// 获取目标聚焦对象
	focused_btn = lv_obj_get_child_form_id(image_page, new_focused_id);

	// 设置焦点并输出日志
	if (focused_btn != NULL)
	{
		lv_group_focus_obj(focused_btn);
		printf("焦点已%s移动：ID %d -> %d\n",
			   (direction == false ? "向上" : "向下"),
			   old_focused_id, new_focused_id);
	}
	else
	{
		// 如果找不到目标对象，恢复原来的焦点ID
		*focused_id_ptr = old_focused_id;
		printf("焦点移动失败：未找到子对象（目标ID：%d）\n", new_focused_id);

		// 尝试恢复原来的焦点
		focused_btn = lv_obj_get_child_form_id(image_page, old_focused_id);
		if (focused_btn != NULL)
		{
			lv_group_focus_obj(focused_btn);
			printf("焦点已恢复到原来的ID：%d\n", old_focused_id);
		}
	}

	return focused_btn;
}