// /*******************************************************************
//  * @Descripttion   : 
//  * @version        : 1.0.0
//  * @Author         : wxj
//  * @Date           : 2022-11-11 11:50
//  * @LastEditTime   : 2022-11-28 16:32
// *******************************************************************/
// #include "layout_define.h"


// #define CAMERA_AUTO_RECORD_ENABLE 0//开启自动连续拍照、录像，测试用



// enum
// {
// 	RECORD_MODE_IMAGE = 0,
// 	RECORD_MODE_VIDEO
// };

// typedef enum 
// {
// 	CAMERA_HOME_BTN_ID,
// 	CAMERA_SWITCH_BTN_ID,
// 	CAMERA_RECORD_BTN_ID,
// 	CAMERA_ZOOM_BTN_ID,
// 	CAMERA_SETTING_BTN_ID,
// 	CAMERA_EXIT_BTN_ID,
// 	CAMERA_OPEN_BTN_ID,
// 	CAMERA_TOTAL_BTN,
// }camera_btn_module;

// static custom_area camera_btn_area[CAMERA_TOTAL_BTN] = 
// {
// 	{20, 218, 32, 32},
// 	{88, 218, 32, 32},
// 	{156, 218, 32, 32},
// 	{224, 218, 32, 32},
// 	{292, 218, 32, 32},
// 	{360, 218, 32, 32},
// 	{428, 218, 32, 32},
// };
// #define CAMERA_FUNC_BTN_BG_BLOCK_ID 8
// #define CAMERA_SETTING_WINDOW_ID 9
// #define CAMERA_SD_CARD_STATE_OBJ_ID 10
// #define CAMERA_PROMPT_MESSAGE_LABEL_ID 11 //提示消息的标签（开锁提示、录像提示）
// #define CAMERA_CHANNEL_LABEL_ID 13
// #define CAMERA_ZOOM_SCALE_LABEL_ID 14//缩放比例标签
// #define CAMERA_BRIGHTNESS_CONT_ID 15//亮度设置的容器
// #define CAMERA_COLOR_CONT_ID 16//色度设置的容器
// #define CAMERA_CONTRAST_CONT_ID 17//对比度设置的容器


// typedef enum
// {
// 	CAMERA_MODE_MONITOR = 0,
// 	CAMERA_MODE_ZOOM
// }cam_mode;


// static void layout_camera_door1_call_func(void);
// static void layout_camera_door2_call_func(void);
// static void layout_camera_click_down_func(lv_obj_t *obj);
// static void layout_camera_open_btn_func(void);
// static void camera_goto_zoom_mode(lv_obj_t *parent);
// static void camera_goto_monitor_mode(lv_obj_t *parent);
// static void camera_channel_switch(MON_CH ch);
// static void camera_func_btn_diaplay_enable(bool en);
// static void camera_btn_and_win_hidden_task_restart(void);
// static void camera_bg_btn_click_enable(bool en);
// static void camera_record_photo_video(REC_MODE mode);

// static bool is_recording = false;
// static bool is_opening = false;
// static bool func_btn_diaplay_flag = true;
// static bool setting_win_diaplay_flag = true;
// static cam_mode camera_mode = CAMERA_MODE_MONITOR;
// static int camera_timeout_val = 120;


// //复位监控倒计时
// void camera_timeout_value_reset(void)
// {
// #if CAMERA_AUTO_RECORD_ENABLE
// 	camera_timeout_val = 3600 * 8;
// #else
// 	camera_timeout_val = 120;
// #endif
// }

// static void camera_unlock_ringa_start_func(int index)
// {
// 	MON_CH ch = monitor_channel_get();
// 	ring_volume_set(ch == MON_CH_DOOR1 ? user_data_get()->setting.door1_ring_volume : user_data_get()->setting.door2_ring_volume);
// 	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
// }

// static void camera_unlock_ring_finish_func(int index)
// {
// 	power_amplifier_enable(false);
// 	MON_CH ch = monitor_channel_get();
// 	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
// }

// static void camera_unlock_task(lv_task_t * task_t)
// {
// 	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);

// 	if(!is_recording && obj != NULL)
// 	{
// 		lv_obj_set_hidden(obj, true);
// 	}		

// 	monitor_unlcok_close();
// 	is_opening = false;
// 	lv_task_del(task_t);
// }


// //倒计时任务
// static void camera_timeout_count_task(lv_task_t *task_t)
// {
// 	if(camera_timeout_val-- <= 0)
// 	{
// 		goto_layout(pLAYOUT(standby));
// 	}
// #if CAMERA_AUTO_RECORD_ENABLE
// 	printf("=================>>> timeout:[%d]\n", camera_timeout_val);
// 	camera_record_photo_video(REC_MODE_AUTO);
// #endif
// }
// static void camera_timeout_count_task_create(void)
// {
// 	lv_layout_task_create(camera_timeout_count_task, 1000, LV_TASK_PRIO_HIGH, NULL);
// }


// //时间显示刷新任务
// static void camera_head_time_display_task(lv_task_t *task_t)
// {
// 	if(camera_mode == CAMERA_MODE_ZOOM)
// 		return;

//     struct tm tm = {0};
// 	static struct tm prev_tm = {0};
//     user_time_read(&tm);

// 	if(prev_tm.tm_min != tm.tm_min)
// 	{
// 		if(user_data_get()->setting.calendar == 0)
// 		{
// 			struct date temp_date =
// 				{
// 					.year = tm.tm_year,
// 					.month = tm.tm_mon,
// 					.day = tm.tm_mday};
// 			temp_date = gregorian2jalali(temp_date);
// 			lv_label_set_text_fmt(task_t->user_data, "%04d-%02d-%02d   %02d:%02d", temp_date.year, temp_date.month, temp_date.day, tm.tm_hour, tm.tm_min);
// 		}
// 		else
// 		{
// 			lv_label_set_text_fmt(task_t->user_data, "%04d-%02d-%02d   %02d:%02d", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min);			
// 		}
// 		lv_obj_align(task_t->user_data, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
// 	}
// 	prev_tm = tm;
// }
// //SD卡显示任务
// static lv_task_t *camera_sdcard_display_task_t = NULL;
// static void camera_sdcard_display_task(lv_task_t *task_t)
// {
// 	if(camera_mode == CAMERA_MODE_ZOOM)
// 	{
// 		return;
// 	}
// 	if(media_sdcard_insert_check() == true)
// 	{
// 		lv_task_del(task_t);
// 		camera_sdcard_display_task_t = NULL;
// 	}
// 	else
// 	{
// 		lv_obj_set_hidden(task_t->user_data, !lv_obj_get_hidden(task_t->user_data));
// 	}
// }
// //SD卡状态显示
// static void camera_sdcard_state_display_func(void)
// {
// 	lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SD_CARD_STATE_OBJ_ID);

// 	if(sdcard_icon_obj != NULL)
// 	{
// 		static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SDCARD_INSERT_PNG);
// 		static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_NO_SDCARD_PNG);
// 		if(media_sdcard_insert_check() == true)
// 		{
// 			lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
// 		}
// 		else
// 		{
// 			lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
// 			user_data_get()->new_media_file_flag = false;
// 			if(camera_sdcard_display_task_t != NULL)
// 			{
// 				lv_task_del(camera_sdcard_display_task_t);
// 				camera_sdcard_display_task_t = NULL;
// 			}
// 			camera_sdcard_display_task_t = lv_layout_task_create(camera_sdcard_display_task, 1000, LV_TASK_PRIO_LOWEST, sdcard_icon_obj);
// 		}
// 		lv_obj_set_hidden(sdcard_icon_obj, false);
// 	}
// }

// static lv_task_t *camera_time_display_task_t = NULL;
// //头部标签创建
// static void camera_head_channel_and_time_label_create(lv_obj_t *parent)
// {
// 	lv_obj_t *ch_label = lv_label_create(parent, NULL);
// 	lv_obj_set_id(ch_label, CAMERA_CHANNEL_LABEL_ID);
// 	lv_obj_set_pos(ch_label, 20, 0);
// 	lv_label_set_text(ch_label, monitor_channel_get() == MON_CH_DOOR1 ? str_get(COMMON_LANG_CAM1_ID) : str_get(COMMON_LANG_CAM2_ID));
// 	lv_obj_set_style_local_text_color(ch_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFD500));
// 	lv_obj_set_style_local_text_font(ch_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(16));

// 	lv_obj_t *time_label = lv_label_create(parent, ch_label);
//     struct tm tm = {0};
//     user_time_read(&tm);
// 	if(user_data_get()->setting.calendar == 0)
// 	{
// 		struct date temp_date =
// 			{
// 				.year = tm.tm_year,
// 				.month = tm.tm_mon,
// 				.day = tm.tm_mday};
// 		temp_date = gregorian2jalali(temp_date);
// 		lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d", temp_date.year, temp_date.month, temp_date.day, tm.tm_hour, tm.tm_min);
// 	}
// 	else
// 	{
// 		lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min);
// 	}
// 	lv_obj_align(time_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
// 	if(camera_time_display_task_t != NULL)
// 	{
// 		lv_task_del(camera_time_display_task_t);
// 		camera_time_display_task_t = NULL;
// 	}
// 	camera_time_display_task_t = lv_layout_task_create(camera_head_time_display_task, 5000, LV_TASK_PRIO_LOWEST, time_label);
// }


// static void camera_sdcard_icon_create(lv_obj_t *parent)
// {
// 	lv_obj_t *sdcard_icon_obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_id(sdcard_icon_obj, CAMERA_SD_CARD_STATE_OBJ_ID);
// 	lv_obj_set_pos(sdcard_icon_obj, 442, 5);
// 	lv_obj_set_size(sdcard_icon_obj, 29, 21);

// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SDCARD_INSERT_PNG);
// 	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_NO_SDCARD_PNG);
// 	if(media_sdcard_insert_check() == true)
// 	{
// 		lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
// 	}
// 	else
// 	{
// 		lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
// 		if(camera_sdcard_display_task_t != NULL)
// 		{
// 			lv_task_del(camera_sdcard_display_task_t);
// 			camera_sdcard_display_task_t = NULL;
// 		}
// 		camera_sdcard_display_task_t = lv_layout_task_create(camera_sdcard_display_task, 1000, LV_TASK_PRIO_LOWEST, sdcard_icon_obj);
// 	}
// }


// //提示消息的标签创建（开锁提示、录像提示）
// static void camera_prompt_message_label_create(lv_obj_t *parent)
// {
// 	lv_obj_t *label = lv_label_create(parent, NULL);
// 	lv_obj_set_id(label, CAMERA_PROMPT_MESSAGE_LABEL_ID);
// 	lv_label_set_text(label, "");
// 	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 35);
// 	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFD500));
// 	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(16));
// 	lv_obj_set_hidden(label, true);
// }


// //监控视频参数的设置 按键 以及 滑块 创建
// static void camera_video_param_setting_btn_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, const char *str, obj_click_data *sub_btn_pdata, obj_click_data *add_btn_pdata, int value, unsigned int id)
// {
// 	lv_obj_t *cont = lv_cont_create(parent, NULL);
// 	lv_obj_set_id(cont, id);
// 	lv_obj_set_pos(cont, x, y);
// 	lv_obj_set_size(cont, 323, 38);

// 	lv_obj_set_style_local_value_str(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str);
// 	lv_obj_set_style_local_value_align(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
// 	lv_obj_set_style_local_value_font(cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(12));

// 	lv_obj_t *sub_btn = lv_obj_create(cont, NULL);
// 	lv_obj_set_id(sub_btn, 1);
// 	lv_obj_set_size(sub_btn, 22, 22);
// 	lv_obj_align(sub_btn, NULL, LV_ALIGN_IN_LEFT_MID, 79, 0);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SUB_PNG);
// 	lv_obj_set_style_local_pattern_image(sub_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
// 	lv_obj_set_ext_click_area(sub_btn, 10, 10, 6, 6);
// 	if(sub_btn_pdata != NULL)
// 		obj_click_event_listen(sub_btn, sub_btn_pdata);

// 	lv_obj_t *add_btn = lv_obj_create(cont, sub_btn);
// 	lv_obj_set_id(add_btn, 2);
// 	lv_obj_align(add_btn, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
// 	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_ADD_PNG);
// 	lv_obj_set_style_local_pattern_image(add_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
// 	lv_obj_set_ext_click_area(add_btn, 10, 10, 6, 6);
// 	if(add_btn_pdata != NULL)
// 		obj_click_event_listen(add_btn, add_btn_pdata);

// 	lv_obj_t *slider = lv_slider_create(cont, NULL);
// 	lv_obj_set_click(slider, false);
// 	lv_obj_set_size(slider, 160, 1);
//     lv_obj_align(slider, NULL, LV_ALIGN_IN_RIGHT_MID, -42, 0);
//     lv_slider_set_range(slider, 0, 8);
//     lv_slider_set_value(slider, value, LV_ANIM_OFF);
//     lv_obj_set_adv_hittest(slider, false);
//     lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x0A0A0A));
//     lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xD9D9D9));
//     lv_obj_set_style_local_pad_all(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 3);

// 	sub_btn->user_data = slider;
// 	add_btn->user_data = slider;
// 	cont->user_data = slider;
// }


// static void camera_brightness_adj_btn_up(lv_obj_t *obj)
// {
// 	int brightness = monitor_display_brightness_vol_get();
// 	if(obj == lv_obj_get_child_form_id(obj->parent, 1))
// 	{
// 		if(brightness > 0)
// 			brightness--;
// 	}
// 	else if(obj == lv_obj_get_child_form_id(obj->parent, 2))
// 	{
// 		if(brightness < 8)
// 			brightness++;
// 	}
// 	monitor_display_brightness_vol_set(brightness);
// 	display_bright_adj(brightness, INVALID_FORMAT);
// 	lv_slider_set_value((lv_obj_t *)obj->user_data, brightness, LV_ANIM_OFF);
// 	camera_btn_and_win_hidden_task_restart();
// }
// static void camera_color_adj_btn_up(lv_obj_t *obj)
// {
// 	int color = monitor_display_color_vol_get();
// 	if(obj == lv_obj_get_child_form_id(obj->parent, 1))
// 	{
// 		if(color > 0)
//         color--;
// 	}
// 	else if(obj == lv_obj_get_child_form_id(obj->parent, 2))
// 	{
// 		if(color < 8)
//         color++;
// 	}
// 	monitor_display_color_vol_set(color);
// 	display_color_adj(color, INVALID_FORMAT);
// 	lv_slider_set_value((lv_obj_t *)obj->user_data, color, LV_ANIM_OFF);
// 	camera_btn_and_win_hidden_task_restart();
// }
// static void camera_contrast_adj_btn_up(lv_obj_t *obj)
// {
// 	int contrast = monitor_display_cont_vol_get();
// 	if(obj == lv_obj_get_child_form_id(obj->parent, 1))
// 	{
// 		if(contrast > 0)
//         contrast--;
// 	}
// 	else if(obj == lv_obj_get_child_form_id(obj->parent, 2))
// 	{
// 		if(contrast < 8)
//         contrast++;
// 	}
// 	monitor_display_cont_vol_set(contrast);
// 	display_const_adj(contrast, INVALID_FORMAT);
// 	lv_slider_set_value((lv_obj_t *)obj->user_data, contrast, LV_ANIM_OFF);
// 	camera_btn_and_win_hidden_task_restart();
// }
// //监控设置窗口创建
// static void camera_setting_window_create(lv_obj_t *parent)
// {
// 	lv_obj_t *cont = lv_cont_create(parent, NULL);
// 	lv_obj_set_click(cont, false);
// 	lv_obj_set_id(cont, CAMERA_SETTING_WINDOW_ID);
// 	lv_obj_set_pos(cont, 52, 70);
// 	lv_obj_set_size(cont, 376, 128);
// 	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4F4F4F));
// 	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);

// 	static obj_click_data btn_data1 = obj_click_data_up_create(camera_brightness_adj_btn_up);
// 	static obj_click_data btn_data2 = obj_click_data_up_create(camera_color_adj_btn_up);
// 	static obj_click_data btn_data3 = obj_click_data_up_create(camera_contrast_adj_btn_up);
// 	camera_video_param_setting_btn_create(cont, 26, 7, str_get(LAYOUT_CAMERA_LANG_BRIGHTNESS_ID), &btn_data1, &btn_data1, monitor_display_brightness_vol_get(), CAMERA_BRIGHTNESS_CONT_ID);
// 	camera_video_param_setting_btn_create(cont, 26, 45, str_get(LAYOUT_CAMERA_LANG_COLOR_ID), &btn_data2, &btn_data2, monitor_display_color_vol_get(), CAMERA_COLOR_CONT_ID);
// 	camera_video_param_setting_btn_create(cont, 26, 84, str_get(LAYOUT_CAMERA_LANG_CONTRAST_ID), &btn_data3, &btn_data3, monitor_display_cont_vol_get(), CAMERA_CONTRAST_CONT_ID);
// }




// //创建camera界面功能按键的背景块
// static void camera_func_btn_bg_block_create(lv_obj_t * parent)
// {
// 	lv_obj_t *obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_click(obj, false);
// 	lv_obj_set_pos(obj, 0, 212);
// 	lv_obj_set_size(obj, 480, 60);
// 	lv_obj_set_id(obj, CAMERA_FUNC_BTN_BG_BLOCK_ID);
// 	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4F4F4F));
// 	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
// }


// //图标按键创建
// lv_obj_t *camera_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
// {
// 	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_ext_click_area(btn_obj, 12, 12, 10, 15);
// 	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
// 	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);
// 	if (icon_src != NULL)
// 	{
// 		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
// 		lv_obj_set_style_local_pattern_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
// 	}

// 	lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
// 	lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_RADIUS_CIRCLE);
// 	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x919191));
// 	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x919191));
// 	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
// 	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

// 	if (string != NULL)
// 	{
// 		lv_obj_set_style_local_value_str(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
// 		lv_obj_set_style_local_value_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
// 		lv_obj_set_style_local_value_ofs_y(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
// 		lv_obj_set_style_local_value_font(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(12));
// 	}

// 	obj_click_event_listen(btn_obj, btn_pdata);
	
// 	return btn_obj;
// }







// static void camera_home_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(home));
	
// }
// //创建home按钮
// static void camera_home_btn_create(lv_obj_t * parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_home_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_HOME_BTN_ID], str_get(COMMON_LANG_HOME_ID), &btn_data, &info);
// 	lv_obj_set_id(btn, CAMERA_HOME_BTN_ID);
// }


// static void camera_switch_btn_up(lv_obj_t *obj)
// {
// 	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
// 	MON_CH ch = monitor_channel_get();
// 	ch = ch == MON_CH_DOOR1 ? MON_CH_DOOR2 : MON_CH_DOOR1;
// 	camera_channel_switch(ch);
// }

// //创建cam1&2按钮
// static void camera_switch_btn_create(lv_obj_t * parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_switch_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_CAM1_2_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_SWITCH_BTN_ID], str_get(LAYOUT_CAMERA_LANG_CAM1_2_ID), &btn_data, &info);
// 	lv_obj_set_id(btn, CAMERA_SWITCH_BTN_ID);
// }





// //抓拍结束时的任务
// static void camera_record_image_end_task(lv_task_t *task_t)
// {
// 	lv_task_del(task_t);
// 	if(camera_mode == CAMERA_MODE_ZOOM)
// 		return;

// 	if(!is_opening)
// 		lv_obj_set_hidden(task_t->user_data, true);
	
// 	is_recording = false;
// 	user_data_get()->new_media_file_flag = true;
// }
// //录视频时的倒计时的任务
// static void camera_record_video_count_down_task(lv_task_t *task_t)
// {
// 	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
// 	int *count = task_t->user_data;
// 	if(	--(*count) < 0 || 
// 		media_sdcard_insert_check() == false || 
// 		camera_mode == CAMERA_MODE_ZOOM || 
// 		video_record_status_get() == false || 
// 		video_input_state_get() == false)
// 	{
// 		record_video_close();
// 		lv_task_del(task_t);
// 		if(!is_opening && obj != NULL)
// 		{
// 			lv_obj_set_hidden(obj, true);
// 		}
// 		is_recording = false;
// 		user_data_get()->new_media_file_flag = true;
// 		return;
// 	}
// 	if(!is_opening && obj != NULL)
// 	{
// 		lv_label_set_text_fmt(obj, "%s %02d", str_get(LAYOUT_CAMERA_LANG_RECORD_VIDEO_ID), *count);
// 	}		
// }
// //抓拍或录像
// static void camera_record_photo_video(REC_MODE mode)
// {
// 	if(video_input_state_get() == false || is_recording == true)
// 		return;
// 	printf("=============>> record_mode : [%d] \n", user_data_get()->setting.record_mode);
// 	lv_obj_t *msg_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	
// 	if(user_data_get()->setting.record_mode == RECORD_MODE_IMAGE)
// 	{
// 		if(record_jpeg_start(mode) == true)
// 		{
// 			user_data_get()->media_disp_mode = 0;
// 			is_recording = true;
// 			lv_label_set_text(msg_label, str_get(LAYOUT_CAMERA_LANG_RECORD_IMAGE_ID));
// 			lv_obj_align_mid_x(msg_label, NULL, LV_ALIGN_CENTER, 0);
// 			lv_obj_set_hidden(msg_label, false);
// #if CAMERA_AUTO_RECORD_ENABLE
// 			lv_layout_task_create(camera_record_image_end_task, 200, LV_TASK_PRIO_LOWEST, msg_label);
// #else
// 			lv_layout_task_create(camera_record_image_end_task, 1000, LV_TASK_PRIO_LOWEST, msg_label);
// #endif
// 		}
// 	}
// 	else if(user_data_get()->setting.record_mode == RECORD_MODE_VIDEO)
// 	{
// 		if (media_sdcard_insert_check() == false)
// 		{
// 			if(record_jpeg_start(mode) == true)
// 			{
// 				user_data_get()->media_disp_mode = 0;
// 				is_recording = true;
// 				lv_label_set_text(msg_label, str_get(LAYOUT_CAMERA_LANG_RECORD_IMAGE_ID));
// 				lv_obj_align_mid_x(msg_label, NULL, LV_ALIGN_CENTER, 0);
// 				lv_obj_set_hidden(msg_label, false);
// #if CAMERA_AUTO_RECORD_ENABLE
// 				lv_layout_task_create(camera_record_image_end_task, 200, LV_TASK_PRIO_LOWEST, msg_label);
// #else
// 				lv_layout_task_create(camera_record_image_end_task, 1000, LV_TASK_PRIO_LOWEST, msg_label);
// #endif
// 			}
// 		}
// 		else
// 		{
// 			if(record_video_start(mode) == true)
// 			{
// 				user_data_get()->media_disp_mode = 1;
// 				is_recording = true;
// 				static int count;
// 				count = 15;
// 				lv_label_set_text_fmt(msg_label, "%s %02d", str_get(LAYOUT_CAMERA_LANG_RECORD_VIDEO_ID), count);
// 				lv_obj_align_mid_x(msg_label, NULL, LV_ALIGN_CENTER, 0);
// 				lv_obj_set_hidden(msg_label, false);
// 				lv_layout_task_create(camera_record_video_count_down_task, 1000, LV_TASK_PRIO_HIGH, &count);
// 			}
// 		}		
// 	}
// }
// static void camera_record_btn_up(lv_obj_t *obj)
// {
// 	camera_record_photo_video(REC_MODE_MANUAL);
// 	camera_btn_and_win_hidden_task_restart();
// }
// //创建record按钮
// static void camera_record_btn_create(lv_obj_t * parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_record_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_RECORD_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_RECORD_BTN_ID], str_get(COMMON_LANG_RECORD_ID), &btn_data, &info);
// 	lv_obj_set_id(btn, CAMERA_RECORD_BTN_ID);
// }






// static void camera_zoom_btn_up(lv_obj_t *obj)
// {
// 	camera_goto_zoom_mode(lv_scr_act());
// }
// //创建zoom按钮
// static void camera_zoom_btn_create(lv_obj_t * parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_zoom_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_ZOOM_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_ZOOM_BTN_ID], str_get(LAYOUT_CAMERA_LANG_ZOOM_ID), &btn_data, &info);
// 	lv_obj_set_id(btn, CAMERA_ZOOM_BTN_ID);
// }





// static void camera_setting_window_display_enable(bool en)
// {
// 	setting_win_diaplay_flag = en;
// 	if(en)
// 	{
// 		camera_bg_btn_click_enable(false);
// 	}
// 	else
// 	{
// 		camera_bg_btn_click_enable(true);
// 	}
// 	lv_obj_t *win = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SETTING_WINDOW_ID);
// 	if(win == NULL)
// 		return;
	
// 	lv_obj_t *cont = lv_obj_get_child_form_id(win, CAMERA_BRIGHTNESS_CONT_ID);
// 	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_brightness_vol_get(), LV_ANIM_OFF);
// 	cont = lv_obj_get_child_form_id(win, CAMERA_COLOR_CONT_ID);
// 	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_color_vol_get(), LV_ANIM_OFF);
// 	cont = lv_obj_get_child_form_id(win, CAMERA_CONTRAST_CONT_ID);
// 	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_cont_vol_get(), LV_ANIM_OFF);

// 	lv_obj_set_hidden(win, !en);
	
// }
// static void camera_setting_btn_up(lv_obj_t *obj)
// {
// 	camera_setting_window_display_enable(!setting_win_diaplay_flag);
// 	camera_btn_and_win_hidden_task_restart();
// }
// //创建setting按钮
// static void camera_setting_btn_create(lv_obj_t * parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_setting_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SETTING_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_SETTING_BTN_ID], str_get(COMMON_LANG_SETTING_ID), &btn_data, &info);
// 	lv_obj_set_id(btn, CAMERA_SETTING_BTN_ID);
// }







// static void camera_exit_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(standby));
// }
// //创建exit按钮
// static void camera_exit_btn_create(lv_obj_t * parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_exit_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_EXIT_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_EXIT_BTN_ID], str_get(COMMON_LANG_EXIT_ID), &btn_data, &info);
// 	lv_obj_set_id(btn, CAMERA_EXIT_BTN_ID);
// }






// static void camera_open_btn_up(lv_obj_t *obj)
// {
// 	if(is_opening)
// 		return;

// 	lv_obj_t *msg_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
// 	if(msg_label != NULL)
// 	{
// 		lv_label_set_text(msg_label, str_get(LAYOUT_CAMERA_LANG_DOOR_OPEN_ID));
// 		lv_obj_align_mid_x(msg_label, NULL, LV_ALIGN_CENTER, 0);
// 		lv_obj_set_hidden(msg_label, false);
// 	}
	
// 	MON_CH ch = monitor_channel_get();
// 	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
// 	monitor_unlock_open(0, ch);
// 	is_opening = true;
// 	ringplay_play_form_index(7, 100, camera_unlock_ringa_start_func, camera_unlock_ring_finish_func, false);
// 	camera_btn_and_win_hidden_task_restart();
// 	lv_layout_task_create(camera_unlock_task, 2000, LV_TASK_PRIO_MID, NULL);
// }
// //创建open按钮
// static void camera_open_btn_create(lv_obj_t * parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_open_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_OPEN_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_OPEN_BTN_ID], str_get(LAYOUT_CAMERA_LANG_OPEN_ID), &btn_data, &info);
// 	lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00CC6A));
// 	lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x009944));
// 	lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
// 	lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
// 	lv_obj_set_id(btn, CAMERA_OPEN_BTN_ID);
// }






// static void camera_zoom_back_btn_up(lv_obj_t *obj)
// {
// 	camera_goto_monitor_mode(lv_scr_act());
// }
// static void camera_zoom_back_btn_create(lv_obj_t *parent)
// {
//     setting_back_btn_create(parent, camera_zoom_back_btn_up);
// }


// //放大按键回调
// static void camera_zoom_in_btn_up(lv_obj_t *obj)
// {
// 	lv_obj_t *zoom_scale_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_ZOOM_SCALE_LABEL_ID);
// 	int zoom = video_input_display_zoom_get();
// 	if(zoom_scale_label != NULL && zoom <= 475 && zoom >= 100)
// 	{
// 		zoom += 25;
// 		lv_label_set_text_fmt(zoom_scale_label, "%d%%", zoom);
// 		lv_obj_align(zoom_scale_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 10);
// 		video_input_display_zoom_set(zoom);
// 	}
// }
// //缩小按键回调
// static void camera_zoom_out_btn_up(lv_obj_t *obj)
// {
// 	lv_obj_t *zoom_scale_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_ZOOM_SCALE_LABEL_ID);
// 	int zoom = video_input_display_zoom_get();
// 	if(zoom_scale_label != NULL && zoom >= 125 && zoom <= 500)
// 	{
// 		zoom -= 25;
// 		lv_label_set_text_fmt(zoom_scale_label, "%d%%", zoom);
// 		lv_obj_align(zoom_scale_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 10);
// 		video_input_display_zoom_set(zoom);
// 	}
// }
// static void camera_move_left_btn_up(lv_obj_t *obj)
// {
// 	double offset_x;
// 	double offset_y;
// 	video_input_display_offset_get(&offset_x, &offset_y);
// 	if(offset_x >= 5)
// 	{
// 		offset_x -= 5;
// 		video_input_display_offset_set(offset_x, offset_y);
// 	}
// }
// static void camera_move_right_btn_up(lv_obj_t *obj)
// {
// 	double offset_x;
// 	double offset_y;
// 	video_input_display_offset_get(&offset_x, &offset_y);
// 	if(offset_x <= 95)
// 	{
// 		offset_x += 5;
// 		video_input_display_offset_set(offset_x, offset_y);
// 	}
// }
// static void camera_move_up_btn_up(lv_obj_t *obj)
// {
// 	double offset_x;
// 	double offset_y;
// 	video_input_display_offset_get(&offset_x, &offset_y);
// 	if(offset_y >= 5)
// 	{
// 		offset_y -= 5;
// 		video_input_display_offset_set(offset_x, offset_y);
// 	}
// }
// static void camera_move_down_btn_up(lv_obj_t *obj)
// {
// 	double offset_x;
// 	double offset_y;
// 	video_input_display_offset_get(&offset_x, &offset_y);
// 	if(offset_y <= 95)
// 	{
// 		offset_y += 5;
// 		video_input_display_offset_set(offset_x, offset_y);
// 	}
// }
// //恢复
// static void camera_zoom_recover_btn_up(lv_obj_t *obj)
// {
// 	video_input_display_zoom_set(100);
// 	video_input_display_offset_set(0, 0);
// 	int zoom = video_input_display_zoom_get();

// 	lv_obj_t *zoom_scale_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_ZOOM_SCALE_LABEL_ID);
// 	lv_label_set_text_fmt(zoom_scale_label, "%d%%", zoom);
// 	lv_obj_align(zoom_scale_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 10);
// }
// //进入缩放模式
// void camera_goto_zoom_mode(lv_obj_t *parent)
// {
// 	camera_mode = CAMERA_MODE_ZOOM;
// 	lv_obj_clean(parent);
// 	camera_bg_btn_click_enable(false);
// 	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
// 	camera_zoom_back_btn_create(parent);
	
// 	//缩放比例标签
// 	lv_obj_t *zoom_scale_label = lv_label_create(parent, NULL);
// 	lv_obj_set_id(zoom_scale_label, CAMERA_ZOOM_SCALE_LABEL_ID);
// 	lv_obj_set_style_local_text_font(zoom_scale_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(16));
// 	lv_obj_set_style_local_text_color(zoom_scale_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFD500));
// 	int zoom = video_input_display_zoom_get();
// 	lv_label_set_text_fmt(zoom_scale_label, "%d%%", zoom);
// 	lv_obj_align(zoom_scale_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 10);

// 	//放大按键
// 	lv_obj_t *zoom_in_btn = lv_obj_create(parent, NULL);
// 	lv_obj_set_size(zoom_in_btn, 41, 36);
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_zoom_in_btn_up);
// 	obj_click_event_listen(zoom_in_btn, &btn_data);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_ZOOM_IN_PNG);
// 	lv_obj_set_style_local_pattern_image(zoom_in_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
// 	lv_obj_set_pos(zoom_in_btn, 334, 223);
// 	lv_obj_set_ext_click_area(zoom_in_btn, 10, 10, 10, 10);

// 	//缩小按键
// 	lv_obj_t *zoom_out_btn = lv_obj_create(parent, zoom_in_btn);
// 	static obj_click_data btn_data1 = obj_click_data_up_create(camera_zoom_out_btn_up);
// 	obj_click_event_listen(zoom_out_btn, &btn_data1);
// 	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_ZOOM_OUT_PNG);
// 	lv_obj_set_style_local_pattern_image(zoom_out_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
// 	lv_obj_set_pos(zoom_out_btn, 410, 223);
// 	lv_obj_set_ext_click_area(zoom_out_btn, 10, 10, 10, 10);

// 	lv_obj_t *cont = lv_cont_create(parent, NULL);
// 	lv_obj_set_size(cont, 116, 116);
// 	lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
// 	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
// 	lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_RIGHT, -28, 68);

// 	//左移按键
// 	lv_obj_t *left_btn = lv_obj_create(cont, NULL);
// 	lv_obj_set_size(left_btn, 18, 21);
// 	static obj_click_data left_btn_data = obj_click_data_up_create(camera_move_left_btn_up);
// 	obj_click_event_listen(left_btn, &left_btn_data);
// 	static rom_bin_info left_btn_info = rom_bin_info_get(ROM_UI_CAMERA_LEFT_PNG);
// 	lv_obj_set_style_local_pattern_image(left_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &left_btn_info);
// 	lv_obj_set_style_local_pattern_recolor(left_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x46CC00));
// 	lv_obj_set_style_local_pattern_recolor(left_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	lv_obj_set_style_local_pattern_recolor_opa(left_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
// 	lv_obj_align(left_btn, NULL, LV_ALIGN_IN_LEFT_MID, 12, 0);
// 	lv_obj_set_ext_click_area(left_btn, 12, 3, 10, 10);

// 	//右移按键
// 	lv_obj_t *right_btn = lv_obj_create(cont, left_btn);
// 	static obj_click_data right_btn_data = obj_click_data_up_create(camera_move_right_btn_up);
// 	obj_click_event_listen(right_btn, &right_btn_data);
// 	static rom_bin_info right_btn_info = rom_bin_info_get(ROM_UI_CAMERA_RIGHT_PNG);
// 	lv_obj_set_style_local_pattern_image(right_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &right_btn_info);
// 	lv_obj_align(right_btn, NULL, LV_ALIGN_IN_RIGHT_MID, -12, 0);
// 	lv_obj_set_ext_click_area(right_btn, 3, 12, 10, 10);
	
// 	//上移按键
// 	lv_obj_t *up_btn = lv_obj_create(cont, left_btn);
// 	lv_obj_set_size(up_btn, 21, 18);
// 	static obj_click_data up_btn_data = obj_click_data_up_create(camera_move_up_btn_up);
// 	obj_click_event_listen(up_btn, &up_btn_data);
// 	static rom_bin_info up_btn_info = rom_bin_info_get(ROM_UI_CAMERA_UP_PNG);
// 	lv_obj_set_style_local_pattern_image(up_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &up_btn_info);
// 	lv_obj_set_style_local_pattern_recolor(up_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x46CC00));
// 	lv_obj_set_style_local_pattern_recolor(up_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	lv_obj_set_style_local_pattern_recolor_opa(up_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
// 	lv_obj_align(up_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 12);
// 	lv_obj_set_ext_click_area(up_btn, 10, 10, 12, 3);

// 	//下移按键
// 	lv_obj_t *down_btn = lv_obj_create(cont, up_btn);
// 	static obj_click_data down_btn_data = obj_click_data_up_create(camera_move_down_btn_up);
// 	obj_click_event_listen(down_btn, &down_btn_data);
// 	static rom_bin_info down_btn_info = rom_bin_info_get(ROM_UI_CAMERA_DOWN_PNG);
// 	lv_obj_set_style_local_pattern_image(down_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &down_btn_info);
// 	lv_obj_align(down_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
// 	lv_obj_set_ext_click_area(down_btn, 10, 10, 3, 12);

// 	//复原按键
// 	lv_obj_t *recover_btn = lv_obj_create(cont, NULL);
// 	lv_obj_set_size(recover_btn, 48, 48);
// 	static obj_click_data recover_btn_data = obj_click_data_up_create(camera_zoom_recover_btn_up);
// 	obj_click_event_listen(recover_btn, &recover_btn_data);
// 	lv_obj_set_style_local_radius(recover_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
// 	lv_obj_set_style_local_bg_color(recover_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	lv_obj_set_style_local_bg_color(recover_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x46CC00));
// 	lv_obj_set_style_local_bg_opa(recover_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
// 	lv_obj_align(recover_btn, NULL, LV_ALIGN_CENTER, 0, 0);
// }

// static lv_task_t *camera_btn_and_win_hidden_task_t = NULL;
// //功能键隐藏任务10s
// static void camera_btn_and_win_hidden_task(lv_task_t *task_t)
// {
// 	if(setting_win_diaplay_flag)
// 	{
// 		camera_setting_window_display_enable(false);
// 		return;
// 	}
// 	else if(func_btn_diaplay_flag)
// 	{
// 		camera_func_btn_diaplay_enable(false);
// 	}
// 	lv_task_del(task_t);
// 	camera_btn_and_win_hidden_task_t = NULL;
// }

// //图标隐藏任务重新倒计时
// static void camera_btn_and_win_hidden_task_restart(void)
// {
// 	if(camera_btn_and_win_hidden_task_t != NULL)
// 	{
// 		lv_task_del(camera_btn_and_win_hidden_task_t);
// 		camera_btn_and_win_hidden_task_t = NULL;
// 	}
// 	camera_btn_and_win_hidden_task_t = lv_layout_task_create(camera_btn_and_win_hidden_task, 10000, LV_TASK_PRIO_MID, NULL);
// }

// //功能键显示使能
// static void camera_func_btn_diaplay_enable(bool en)
// {
// 	func_btn_diaplay_flag = en;
// 	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_FUNC_BTN_BG_BLOCK_ID);
// 	if(obj == NULL)
// 		return;
// 	lv_obj_set_hidden(obj, !en);
// 	for (int i = 0; i < 6; i++)
// 	{
// 		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), i), !en);
// 	}
// }

// static void camera_bg_btn_up(lv_obj_t *obj)
// {
// 	camera_func_btn_diaplay_enable(!func_btn_diaplay_flag);
// 	camera_btn_and_win_hidden_task_restart();
// }
// //背景的点击使能
// static void camera_bg_btn_click_enable(bool en)
// {
// 	if(en)
// 	{
// 		static obj_click_data bg_btn_data = obj_click_data_up_create(camera_bg_btn_up);
// 		obj_click_event_listen(lv_scr_act(), &bg_btn_data);
// 	}
// 	else
// 	{
// 		obj_click_event_listen(lv_scr_act(), NULL);
// 	}
// }



// //进入监控模式
// static void camera_goto_monitor_mode(lv_obj_t *parent)
// {
// 	camera_mode = CAMERA_MODE_MONITOR;
// 	video_input_display_zoom_set(100);
// 	video_input_display_offset_set(0, 0);
// 	lv_obj_clean(parent);
// 	camera_bg_btn_click_enable(true);

// 	camera_head_channel_and_time_label_create(parent);
// 	camera_sdcard_icon_create(parent);
// 	camera_prompt_message_label_create(parent);
// 	camera_func_btn_bg_block_create(parent);
// 	camera_home_btn_create(parent);
// 	camera_switch_btn_create(parent);
// 	camera_record_btn_create(parent);
// 	camera_zoom_btn_create(parent);
// 	camera_setting_btn_create(parent);
// 	camera_exit_btn_create(parent);
// 	camera_open_btn_create(parent);
// 	camera_func_btn_diaplay_enable(true);

// 	camera_setting_window_create(parent);
// 	camera_setting_window_display_enable(false);

// 	camera_btn_and_win_hidden_task_restart();

// 	lyaout_sd_state_callback_register(camera_sdcard_state_display_func);
// }

// //自动录像任务
// static lv_task_t *camera_auto_record_task_t = NULL;
// static void camera_auto_record_task(lv_task_t *task_t)
// {
// 	if(video_input_state_get() == true)
// 	{
// 		lv_task_del(task_t);
// 		camera_auto_record_task_t = NULL;
// 		if(camera_mode == CAMERA_MODE_ZOOM)
// 			return;
// 		camera_record_photo_video(REC_MODE_AUTO);
// 	}
// }


// static void camera_auto_record_task_create(void)
// {
// 	if(user_data_get()->setting.auto_record_enable == true)
// 	{
// 		if(camera_auto_record_task_t != NULL)
// 		{
// 			lv_task_del(camera_auto_record_task_t);
// 			camera_auto_record_task_t = NULL;
// 		}
// 		camera_auto_record_task_t = lv_layout_task_create(camera_auto_record_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
// 	}		
// }


// static void LAYOUT_ENTER_FUNC(camera)
// {
// 	standby_timer_close();
// 	backlight_enable(true);
// 	is_recording = false;
// 	is_opening = false;

// 	lv_obj_t *parent = lv_scr_act();
// 	lv_obj_set_style_local_pattern_image(parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);

// 	camera_goto_monitor_mode(parent);
	
// 	layout_door1_call_callback_register(layout_camera_door1_call_func);
// 	layout_door2_call_callback_register(layout_camera_door2_call_func);
// 	lv_obj_click_down_callback_register(layout_camera_click_down_func);
// 	layout_gate_open_callback_register(layout_camera_open_btn_func);

// 	if(monitor_enter_mask_get() == MON_ENTER_CALL)
// 		camera_auto_record_task_create();

// 	audio_input_capture_enable(true);
// 	monitor_open(true, 0x03);
// 	MON_CH ch = monitor_channel_get();
// 	if(hook_state_get() == true)
// 	{
// 		if (ch == MON_CH_DOOR1)
// 		{
// 			printf("=========================>>>> door1 talking \n");
// 			door_audio_talk(AUDIO_CH_DOOR1);
// 		}
// 		else if (ch == MON_CH_DOOR2)
// 		{
// 			printf("=========================>>>> door2 talking \n");
// 			door_audio_talk(AUDIO_CH_DOOR2);
// 		}
// 		monitor_enter_mask_set(MON_ENTER_TALK);
// 	}
// 	camera_timeout_value_reset();
// 	camera_timeout_count_task_create();
// }
// static void LAYOUT_QUIT_FUNC(camera)
// {
// 	layout_door1_call_callback_register(layout_door1_call_default);
// 	layout_door2_call_callback_register(layout_door2_call_default);
// 	lv_obj_click_down_callback_register(layout_obj_click_down_func);
// 	layout_gate_open_callback_register(NULL);
	
// 	audio_input_capture_enable(false);
// 	camera_bg_btn_click_enable(false);

// 	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
	
// 	record_jpeg_close();
// 	record_video_close();
// 	ringplay_play_stop();
	
// 	door_audio_talk(AUDIO_CH_CLOSE);
// 	monitor_close();

// 	user_data_save();
// 	standby_timer_restart(true);
// 	monitor_unlcok_close();	

// 	camera_sdcard_display_task_t = NULL;
// 	camera_time_display_task_t = NULL;
// 	camera_btn_and_win_hidden_task_t = NULL;
// 	camera_auto_record_task_t = NULL;
// }

// //监控界面点击按键音处理函数
// static void layout_camera_click_down_func(lv_obj_t *obj)
// {
// 	if(hook_state_get() == true)
// 	{
// 		return;
// 	}
// 	MON_CH ch = monitor_channel_get();
// 	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
// 	touch_sound_play(NULL, NULL);
// }

// //监控界面door1 call机处理函数
// static void layout_camera_door1_call_func(void)
// {
// 	monitor_valid_channel_set(MON_CH_DOOR1, true);

// 	// if(hook_state_get() == false)
// 	// {
// 	// 	ringplay_play_form_index(user_data_get()->setting.door1_tone, 100, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
// 	// 	monitor_enter_mask_set(MON_ENTER_CALL);
// 	// 	if(user_data_get()->setting.auto_record_enable == true)
// 	// 	{
// 	// 		camera_record_photo_video(REC_MODE_AUTO);
// 	// 	}
// 	// }

// 	MON_CH ch = monitor_channel_get();
// 	if (ch != MON_CH_DOOR1)
// 	{
// 		monitor_enter_mask_set(MON_ENTER_CALL);
// 		camera_channel_switch(MON_CH_DOOR1);
// 		camera_auto_record_task_create();
// 	}
// 	else
// 	{
		
// 		monitor_enter_mask_set(MON_ENTER_CALL);
// 		if(user_data_get()->setting.auto_record_enable == true)
// 		{
// 			camera_record_photo_video(REC_MODE_AUTO);
// 		}
// 	}
// 	ringplay_play_form_index(user_data_get()->setting.door1_tone, 100, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
// }

// //监控界面door2 call机处理函数
// static void layout_camera_door2_call_func(void)
// {
// 	monitor_valid_channel_set(MON_CH_DOOR2, true);

// 	// if(hook_state_get() == false)
// 	// {
// 	// 	ringplay_play_form_index(user_data_get()->setting.door2_tone, 100, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
// 	// 	monitor_enter_mask_set(MON_ENTER_CALL);
// 	// 	if(user_data_get()->setting.auto_record_enable == true)
// 	// 	{
// 	// 		camera_record_photo_video(REC_MODE_AUTO);
// 	// 	}
// 	// }

// 	MON_CH ch = monitor_channel_get();
// 	if (ch != MON_CH_DOOR2)
// 	{
// 		monitor_enter_mask_set(MON_ENTER_CALL);
// 		camera_channel_switch(MON_CH_DOOR2);
// 		camera_auto_record_task_create();
// 	}
// 	else
// 	{
// 		ringplay_play_form_index(user_data_get()->setting.door2_tone, 100, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
// 		monitor_enter_mask_set(MON_ENTER_CALL);
// 		if(user_data_get()->setting.auto_record_enable == true)
// 		{
// 			camera_record_photo_video(REC_MODE_AUTO);
// 		}
// 	}
// 	ringplay_play_form_index(user_data_get()->setting.door2_tone, 100, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
// }

// //监控通道切换
// static void camera_channel_switch(MON_CH ch)
// {
// 	if(camera_mode == CAMERA_MODE_ZOOM)
// 		camera_goto_monitor_mode(lv_scr_act());

// 	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
// 	if(obj != NULL)
// 		lv_obj_set_hidden(obj, true);

// 	obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_CHANNEL_LABEL_ID);
// 	if(obj != NULL)
// 		lv_label_set_text(obj, ch == MON_CH_DOOR1 ? str_get(COMMON_LANG_CAM1_ID) : str_get(COMMON_LANG_CAM2_ID));

// 	camera_setting_window_display_enable(false);

// 	record_jpeg_close();
// 	record_video_close();
// 	monitor_unlcok_close();
// 	monitor_channel_set(ch);
// 	monitor_open(true, 0x03);
	
// 	camera_timeout_value_reset();
// 	camera_btn_and_win_hidden_task_restart();

// 	if(hook_state_get() == true)
// 	{
// 		if (ch == MON_CH_DOOR1)
// 		{
// 			printf("=========================>>>> door1 talking \n");
// 			door_audio_talk(AUDIO_CH_DOOR1);
// 		}
// 		else if (ch == MON_CH_DOOR2)
// 		{
// 			printf("=========================>>>> door2 talking \n");
// 			door_audio_talk(AUDIO_CH_DOOR2);
// 		}
// 		monitor_enter_mask_set(MON_ENTER_TALK);
// 	}
// }


// static void layout_camera_open_btn_func(void)
// {
// 	camera_open_btn_up(NULL);
// }

// CREATE_LAYOUT(camera);