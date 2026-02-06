#include "layout_define.h"

typedef enum
{
	HOME_VOLUME_BTN_ID,
	HOME_INTERCOM_BTN_ID,
	HOME_SOUND_BTN_ID,

	HOME_LOCK_BTN_ID,
	HOME_ELEVATOR_BTN_ID,
	HOME_MONITOR_BTN_ID,

	HOME_CCTV_BTN_ID,
	HOME_SETTING_BTN_ID,
	HOME_RECORD_BTN_ID,

	HOME_TOTAL_BTN,
} home_btn_module;

static custom_area home_btn_area[HOME_TOTAL_BTN] =
	{
		{48, 16, 50, 50},	 // 静音
		{489, 75, 130, 207}, // 呼叫
		{624, 75, 131, 207}, // 音量设置

		{760, 75, 146, 208},  // 开锁
		{30, 287, 247, 265},  // 呼梯
		{282, 287, 155, 128}, // 监控

		{442, 287, 155, 128}, // CCTV
		{282, 420, 315, 132}, // 设置
		{602, 287, 304, 265}, // 浏览

};

static custom_area home_img_area[HOME_TOTAL_BTN] =
	{
		{0, 0, 50, 50},	  // 静音
		{37, 41, 60, 60}, // 呼叫
		{36, 41, 60, 60}, // 音量设置

		{42, 41, 60, 60}, // 开锁
		{82, 59, 84, 84}, // 呼梯s
		{48, 17, 60, 60}, // 监控

		{48, 17, 60, 60},  // CCTV
		{48, 36, 54, 54},  // 设置
		{108, 56, 97, 91}, // 浏览

};

static custom_area home_label_area_en[HOME_TOTAL_BTN] = {
	{0, 0, 0, 0},		// 静音
	{16, 138, 101, 29}, // 呼叫
	{29, 138, 73, 29},	// 音量设置
	{45, 138, 55, 29},	// 开锁
	{79, 174, 91, 28},	// 呼梯
	{35, 89, 88, 29},	// 监控
	{44, 89, 67, 29},	// CCTV
	{185, 51, 100, 50}, // 设置
	{97, 189, 115, 29}, // 浏览
};

static custom_area home_label_area_fa[HOME_TOTAL_BTN] = {
	{0, 0, 0, 0},		 // 静音
	{1, 138, 101, 29},	 // 呼叫
	{42, 138, 73, 29},	 // 音量设置
	{45, 138, 55, 29},	 // 开锁
	{105, 174, 91, 28},	 // 呼梯
	{50, 89, 88, 29},	 // 监控
	{26, 89, 67, 29},	 // CCTV
	{185, 51, 100, 50},	 // 设置
	{130, 189, 115, 29}, // 浏览
};

// 获取当前语言的标签区域
static custom_area *get_home_label_area(int btn_id)
{
	if (user_data_get()->setting.language == LANG_PERSIAN)
	{
		return &home_label_area_fa[btn_id];
	}
	else
	{
		return &home_label_area_en[btn_id];
	}
}

#define HOME_TIME_BTN_ID 10
#define HOME_SD_CARD_STATE_OBJ_ID 11
#define HOME_NEW_MEDIA_ICON_OBJ_ID 12

static int new_media_check_count = 0;
static bool backlight_enable_flag = false;
static unsigned int last_home_focus_id = 0;
// 在活动屏幕上显示背景图片
lv_obj_t *common_bg_display(lv_obj_t *parent)
{
	static rom_bin_info info = rom_bin_info_get(ROM_UI_BG_BG_PNG);
	lv_obj_set_style_local_pattern_image(parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	return parent;
}
// 图标按键创建
lv_obj_t *common_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string,
								obj_click_data *btn_pdata, const void *icon_src, int obj_id,
								custom_area img_area, custom_area label_area)
{
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_click(btn_obj, true);

	// 设置默认状态位置和大小
	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);

	// 添加到默认组
	lv_group_add_obj(lv_group_get_default(), btn_obj);

	// -------------------------- 默认状态样式 --------------------------
	lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40); // #00000066 约40%透明度
	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

	// -------------------------- 焦点状态样式 --------------------------
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_40);
	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x000000));

	// -------------------------- 按下状态样式 --------------------------
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_60);
	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, 2);
	// 创建图片对象
	if (icon_src != NULL)
	{
		lv_obj_t *img_obj = lv_img_create(btn_obj, NULL);
		lv_img_set_src(img_obj, icon_src);
		lv_obj_set_pos(img_obj, img_area.x, img_area.y);
		lv_obj_set_size(img_obj, img_area.w, img_area.h);
		lv_obj_set_style_local_image_opa(img_obj, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
		if (obj_id > 0)
		{
			lv_obj_set_id(img_obj, obj_id + 100);
		}
	}

	// 创建标签对象
	if (string != NULL)
	{
		lv_obj_t *label_obj = lv_label_create(btn_obj, NULL);
		lv_label_set_text(label_obj, string);
		lv_obj_set_pos(label_obj, label_area.x, label_area.y);
		lv_obj_set_size(label_obj, label_area.w, label_area.h);
		lv_obj_set_style_local_text_font(label_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_text_color(label_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	}
	if (obj_id > 0)
	{
		lv_obj_set_id(btn_obj, obj_id);
	}
	obj_click_event_listen(btn_obj, btn_pdata);

	return btn_obj;
}

// 声音图标按键创建
static lv_obj_t *Volume_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string,
									   obj_click_data *btn_pdata, const void *icon_src, int obj_id,
									   custom_area img_area, custom_area label_area)
{
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_click(btn_obj, true);

	// 设置默认状态位置和大小
	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);

	// 添加到默认组
	lv_group_add_obj(lv_group_get_default(), btn_obj);

	// -------------------------- 默认状态样式 --------------------------
	lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

	// -------------------------- 焦点状态样式 --------------------------
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_TRANSP);

	// -------------------------- 按下状态样式 --------------------------
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_60);
	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, 2);
	// 创建图片对象
	if (icon_src != NULL)
	{
		lv_obj_t *img_obj = lv_img_create(btn_obj, NULL);
		lv_img_set_src(img_obj, icon_src);
		lv_obj_set_pos(img_obj, img_area.x, img_area.y);
		lv_obj_set_size(img_obj, img_area.w, img_area.h);
		lv_obj_set_style_local_image_opa(img_obj, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
		if (obj_id > 0)
		{
			lv_obj_set_id(img_obj, obj_id + 100);
		}
	}

	// 创建标签对象
	if (string != NULL)
	{
		lv_obj_t *label_obj = lv_label_create(btn_obj, NULL);
		lv_label_set_text(label_obj, string);
		lv_obj_set_pos(label_obj, label_area.x, label_area.y);
		lv_obj_set_size(label_obj, label_area.w, label_area.h);
		lv_obj_set_style_local_text_font(label_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_text_color(label_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	}

	lv_obj_set_id(btn_obj, obj_id);

	obj_click_event_listen(btn_obj, btn_pdata);

	return btn_obj;
}
// 文件列表图标按键创建
lv_obj_t *photo_and_video_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src, bool underline, bool string_select)
{
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_ext_click_area(btn_obj, 12, 12, 10, 15);
	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);

	if (icon_src != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
		lv_obj_set_style_local_pattern_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	}

	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));

	// 2. 叠加透明度：默认低透明（图标显原始色），按下高透明（图标变深色）
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);	// 默认无叠加（原始色）
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色（深色）

	if (string != NULL)
	{
		lv_obj_set_style_local_value_str(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
		lv_obj_set_style_local_value_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_BOTTOM_MID);
		if (string_select)
			lv_obj_set_style_local_value_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xC52323));
		else
			lv_obj_set_style_local_value_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_value_ofs_y(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_value_font(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	}

	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
	lv_obj_set_style_local_border_side(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_RIGHT);
	// 创建底部下划线
	if (underline)
	{

		lv_obj_set_style_local_border_side(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_RIGHT);
		lv_obj_set_style_local_border_side(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_RIGHT);
	}

	if (btn_pdata != NULL)
	{
		// 添加到默认组
		lv_group_add_obj(lv_group_get_default(), btn_obj);
		obj_click_event_listen(btn_obj, btn_pdata);
	}

	return btn_obj;
}

// SD卡状态显示
static void home_sdcard_state_display_func(void)
{
	lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), HOME_SD_CARD_STATE_OBJ_ID);

	if (sdcard_icon_obj != NULL)
	{
		static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_SDCARD_INSERT_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_NO_SDCARD_PNG);
		static rom_bin_info info2 = rom_bin_info_get(ROM_UI_HOME_SDCARD_ERROR_PNG);
		if (media_sdcard_insert_check() == true)
		{
			printf("==========sd_card_pattion==[%d]\n", user_data_get()->sd_card_pattion);
			if (user_data_get()->sd_card_pattion)
			{
				lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info2);
			}
			else
			{
				lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
			}
			user_data_get()->new_photo_file_flag = false;
		}
		else
		{
			lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
			user_data_get()->new_media_file_flag = false;
			user_data_get()->new_photo_file_flag = false;
			lv_obj_t *new_media_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), HOME_NEW_MEDIA_ICON_OBJ_ID);
			if (new_media_icon_obj != NULL)
				lv_obj_del(new_media_icon_obj);
		}
	}
}

// SD卡图标创建
static void home_sdcard_icon_create(lv_obj_t *parent)
{

	lv_obj_t *sdcard_icon_obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(sdcard_icon_obj, HOME_SD_CARD_STATE_OBJ_ID);
	lv_obj_set_pos(sdcard_icon_obj, 122, 16);
	lv_obj_set_size(sdcard_icon_obj, 48, 48);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_SDCARD_INSERT_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_HOME_NO_SDCARD_PNG);
	static rom_bin_info info4 = rom_bin_info_get(ROM_UI_HOME_SDCARD_ERROR_PNG);
	if (media_sdcard_insert_check() == true)
	{
		if (user_data_get()->sd_card_pattion)
		{
			lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info4);
		}
		else
		{
			lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
		}
	}
	else
	{
		lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	}
}
static void home_Volume_btn_up(lv_obj_t *obj)
{
	static bool mute_flag = false;
	mute_flag = !mute_flag;
	ringplay_force_mute_set(mute_flag);

	lv_obj_t *btn_obj = lv_obj_get_child_form_id(lv_scr_act(), HOME_VOLUME_BTN_ID);
	if (btn_obj != NULL)
	{
		lv_obj_t *img_obj = lv_obj_get_child(btn_obj, 0);
		if (img_obj != NULL)
		{
			static rom_bin_info mute_info = rom_bin_info_get(ROM_UI_HOME_MUTE_PNG);
			static rom_bin_info vol_info = rom_bin_info_get(ROM_UI_HOME_VOLUME_PNG);

			if (ringplay_force_mute_get())
			{
				lv_img_set_src(img_obj, &mute_info); // 静音图
			}
			else
			{
				lv_img_set_src(img_obj, &vol_info); // 音量图
			}
		}
	}
}
// 声音图标创建
static void home_Volume_icon_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_Volume_btn_up);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_VOLUME_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_UI_HOME_MUTE_PNG);

	Volume_img_btn_create(parent,
						  home_btn_area[HOME_VOLUME_BTN_ID],
						  NULL,
						  &btn_data,
						  ringplay_force_mute_get() ? &info2 : &info1,
						  HOME_VOLUME_BTN_ID,
						  home_img_area[HOME_VOLUME_BTN_ID],
						  *get_home_label_area(HOME_VOLUME_BTN_ID));
}
static void home_intercom_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(intercom));
}
// 创建intercom按钮
static void home_intercom_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_intercom_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_INTERCOM_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_INTERCOM_BTN_ID], str_get(COMMON_LANG_INTERCOM_ID), &btn_data, &info, HOME_INTERCOM_BTN_ID,
						  home_img_area[HOME_INTERCOM_BTN_ID], *get_home_label_area(HOME_INTERCOM_BTN_ID));
}

static void home_sound_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_sound));
}
// 创建sound按钮
static void home_sound_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_sound_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_SOUND_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_SOUND_BTN_ID], str_get(LAYOUT_SETTING_LANG_SOUND_ID), &btn_data, &info, HOME_SOUND_BTN_ID,
						  home_img_area[HOME_SOUND_BTN_ID], *get_home_label_area(HOME_SOUND_BTN_ID));
}
static void success_promtp_image_task(lv_task_t *task_t)
{
	lv_obj_t *btn_obj = lv_obj_get_child_form_id(lv_scr_act(), HOME_LOCK_BTN_ID);
	if (btn_obj != NULL)
	{
		lv_obj_t *img_obj = lv_obj_get_child_form_id(btn_obj, HOME_LOCK_BTN_ID + 100);
		if (img_obj != NULL)
		{
			static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_LOCK_CLOSE_PNG);
			lv_img_set_src(img_obj, &info);
		}
	}
	lv_task_del(task_t);
}
static void home_lock_btn_up(lv_obj_t *obj)
{
	lv_obj_t *btn_obj = lv_obj_get_child_form_id(lv_scr_act(), HOME_LOCK_BTN_ID);
	if (btn_obj != NULL)
	{
		lv_obj_t *img_obj = lv_obj_get_child_form_id(btn_obj, HOME_LOCK_BTN_ID + 100);
		if (img_obj != NULL)
		{
			static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_LOCK_SUCCESS_PNG);
			lv_img_set_src(img_obj, &info);
		}
	}
	layout_gate_open_default();
	lv_layout_task_create(success_promtp_image_task, 1200, LV_TASK_PRIO_LOW, NULL);
}
// 创建lock按钮
static void home_lock_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_lock_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_LOCK_CLOSE_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_LOCK_BTN_ID], str_get(LAYOUT_HOME_LANG_LOCK_ID), &btn_data, &info, HOME_LOCK_BTN_ID,
						  home_img_area[HOME_LOCK_BTN_ID], *get_home_label_area(HOME_LOCK_BTN_ID));
}

static void home_elevator_btn_up(lv_obj_t *obj)
{

	layout_elevator_call_default();
	lv_layout_task_create(success_promtp_image_task, 1200, LV_TASK_PRIO_LOW, NULL);
}
// 创建elevator按钮
static void home_elevator_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_elevator_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_ELEVATOR_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_ELEVATOR_BTN_ID], str_get(LAYOUT_HOME_LANG_ELEVATOR_ID), &btn_data, &info, HOME_ELEVATOR_BTN_ID,
						  home_img_area[HOME_ELEVATOR_BTN_ID], *get_home_label_area(HOME_ELEVATOR_BTN_ID));
}

static void home_monitor_btn_up(lv_obj_t *obj)
{
	monitor_channel_set(MON_CH_DOOR1);
	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	// manual_enter_monitor = true;
	goto_layout(pLAYOUT(camera));
}
// 创建monitor按钮
static void home_monitor_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_monitor_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_CAMERA_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_MONITOR_BTN_ID], str_get(LAYOUT_HOME_LANG_MINOTOR_ID), &btn_data,
						  &info, HOME_MONITOR_BTN_ID, home_img_area[HOME_MONITOR_BTN_ID], *get_home_label_area(HOME_MONITOR_BTN_ID));
}

static void home_cctv_btn_up(lv_obj_t *obj)
{
	cctv_audio_video_enable_pin_ctrl(false);
	monitor_channel_set(MON_CH_CCTV1);
	monitor_enter_mask_set(MON_ENTER_MANUAL_CCTV);
	// manual_enter_monitor = true;
	goto_layout(pLAYOUT(camera));
}
// 创建CCTV按钮
static void home_cctv_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_cctv_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_CCTV_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_CCTV_BTN_ID], str_get(LAYOUT_HOME_LANG_CCTV_ID), &btn_data, &info, HOME_CCTV_BTN_ID,
						  home_img_area[HOME_CCTV_BTN_ID], *get_home_label_area(HOME_CCTV_BTN_ID));
}

static void home_setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}
// 创建setting按钮
static void home_setting_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_setting_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_SETTING_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_SETTING_BTN_ID], str_get(COMMON_LANG_SETTING_ID), &btn_data, &info, HOME_SETTING_BTN_ID,
						  home_img_area[HOME_SETTING_BTN_ID], *get_home_label_area(HOME_SETTING_BTN_ID));
}

static void home_record_btn_up(lv_obj_t *obj)
{

	if (media_sdcard_insert_check() == true)
	{

		user_data_get()->new_media_file_flag = false;

		goto_layout(pLAYOUT(video_list));
	}
	else
	{
		user_data_get()->new_photo_file_flag = false;
		goto_layout(pLAYOUT(photo_list));
	}
}
// 创建recording按钮
static void home_memory_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_record_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_RECORD_PNG);
	lv_obj_t *btn = common_img_btn_create(parent, home_btn_area[HOME_RECORD_BTN_ID], str_get(LAYOUT_HOME_LANG_RECORD_ID), &btn_data, &info, HOME_RECORD_BTN_ID,
										  home_img_area[HOME_RECORD_BTN_ID], *get_home_label_area(HOME_RECORD_BTN_ID));

	lv_obj_t *icon_obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(icon_obj, HOME_NEW_MEDIA_ICON_OBJ_ID);
	lv_obj_set_click(icon_obj, false);
	lv_obj_set_size(icon_obj, 40, 27);
	// 设置圆角样式，值越大圆角越明显
	lv_obj_set_style_local_radius(icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_NEW_MEDIA_PNG);
	lv_obj_set_style_local_pattern_image(icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
	lv_obj_align(icon_obj, btn, LV_ALIGN_IN_TOP_RIGHT, -57, 39);
	if ((user_data_get()->new_media_file_flag == true) || (user_data_get()->new_photo_file_flag == true) || (user_data_get()->new_call_record_flag == true))
	{
		lv_obj_set_hidden(icon_obj, false);
	}
	else
	{
		lv_obj_set_hidden(icon_obj, true);
	}
}

static lv_obj_t *time_cont = NULL;
static lv_obj_t *hour_label = NULL;
static lv_obj_t *min_label = NULL;
static lv_obj_t *colon_label = NULL;
static lv_obj_t *week_label = NULL;
// 时间显示
static void home_time_display(struct tm *time)
{
	lv_label_set_text_fmt(hour_label, "%02d", time->tm_hour);
	lv_label_set_text_fmt(min_label, "%02d", time->tm_min);
}
// 日期显示
static void home_date_display(struct tm *time)
{
	static char str[32] = {0};

	lv_label_set_text_fmt(week_label, "%s", str_get(LAYOUT_HOME_LANG_ID_WEEK_1 + time->tm_wday - 1));

	if (user_data_get()->setting.calendar == 0)
	{
		struct date temp_date =
			{
				.year = time->tm_year,
				.month = time->tm_mon,
				.day = time->tm_mday};
		temp_date = gregorian2jalali(temp_date);
		sprintf(str, "%04d-%02d-%02d", temp_date.year, temp_date.month, temp_date.day);
	}
	else
	{
		sprintf(str, "%04d-%02d-%02d", time->tm_year, time->tm_mon, time->tm_mday);
	}
	lv_obj_set_style_local_value_str(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str);
}
// 时间和日期的刷新任务
static void home_time_display_task(lv_task_t *task_t)
{
	struct tm tm = {0};
	static struct tm prev_tm = {0};
	user_time_read(&tm);

	if (prev_tm.tm_min != tm.tm_min || task_t == NULL)
	{
		home_time_display(&tm);
	}
	if (prev_tm.tm_mday != tm.tm_mday || task_t == NULL)
	{
		home_date_display(&tm);
	}
	lv_obj_set_hidden(colon_label, !lv_obj_get_hidden(colon_label));
	prev_tm = tm;
}

// 时间日期按钮创建函数
lv_obj_t *home_time_date_btn_create(lv_obj_t *parent, obj_click_data *btn_pdata)
{
	// 创建主按钮容器
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_click(btn_obj, true);

	// 设置按钮位置和大小
	lv_obj_set_pos(btn_obj, 30, 75);
	lv_obj_set_size(btn_obj, 454, 207);

	// 添加到默认组
	lv_group_add_obj(lv_group_get_default(), btn_obj);

	// -------------------------- 按钮样式 --------------------------
	lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

	// 焦点状态样式
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_40);
	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x000000));

	// 按下状态样式
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_60);
	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, 2);

	obj_click_event_listen(btn_obj, btn_pdata);

	return btn_obj;
}
// 创建时间显示对象
static bool home_time_date_text_create_in_btn(lv_obj_t *btn_obj)
{
	/***** 创建时间容器 *****/
	time_cont = lv_cont_create(btn_obj, NULL);
	lv_obj_set_size(time_cont, 280, 107);
	if (user_data_get()->setting.language == LANG_ENGLISH)
	{
		lv_obj_set_pos(time_cont, 90, 34);
	}
	else
	{
		lv_obj_set_pos(time_cont, 70, 34);
	}

	/***** 创建小时label控件 *****/
	hour_label = lv_label_create(time_cont, NULL);
	if (hour_label == NULL)
	{
		printf("home create time(hour) label failed \n");
		return false;
	}
	lv_obj_set_style_local_text_font(hour_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(80));
	lv_obj_set_style_local_text_letter_space(hour_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_color(hour_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_label_set_align(hour_label, LV_LABEL_ALIGN_CENTER);

	/***** 创建分钟label控件 *****/
	min_label = lv_label_create(time_cont, hour_label);
	if (min_label == NULL)
	{
		printf("home create time(min) label failed \n");
		return false;
	}

	/***** 创建":"文本控件 *****/
	colon_label = lv_label_create(time_cont, hour_label);
	if (colon_label == NULL)
	{
		printf("home create time(:) obj failed \n");
		return false;
	}
	lv_label_set_text(colon_label, ":");
	lv_obj_set_style_local_text_color(colon_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

	/***** 创建日期,用value显示 *****/
	lv_obj_set_style_local_value_align(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_BOTTOM_RIGHT);
	lv_obj_set_style_local_value_font(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_set_style_local_value_letter_space(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_value_color(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

	if (user_data_get()->setting.language == LANG_ENGLISH)
	{
		lv_obj_set_style_local_value_ofs_x(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
	}
	else
	{
		lv_obj_set_style_local_value_ofs_x(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
	}
	lv_obj_set_style_local_value_ofs_y(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 40);

	/***** 创建星期标签 *****/
	week_label = lv_label_create(btn_obj, NULL);
	if (week_label == NULL)
	{
		printf("home create week obj failed \n");
		return false;
	}
	lv_obj_set_style_local_text_font(week_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_set_style_local_text_color(week_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

	struct tm tm = {0};
	user_time_read(&tm);
	home_time_display(&tm);
	home_date_display(&tm);
	lv_obj_set_hidden(colon_label, true);

	// 对齐所有元素
	lv_obj_align(colon_label, time_cont, LV_ALIGN_IN_TOP_MID, 0, 0);

	if (user_data_get()->setting.language == LANG_ENGLISH)
	{
		lv_obj_align(week_label, time_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 30, 0);
	}
	else
	{
		lv_obj_align(week_label, time_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 40, 0);
	}

	lv_obj_align(hour_label, colon_label, LV_ALIGN_OUT_LEFT_MID, -20, 0);
	lv_obj_align(min_label, colon_label, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

	lv_layout_task_create(home_time_display_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
	home_time_display_task(NULL);

	return true;
}

static void home_time_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_time));
}
// 创建time按钮
static void home_time_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_time_btn_up);
	lv_obj_t *time_date_btn = home_time_date_btn_create(parent, &btn_data);
	lv_obj_set_id(time_date_btn, HOME_TIME_BTN_ID);
	home_time_date_text_create_in_btn(time_date_btn);
}

static void new_media_check_task(lv_task_t *task_t)
{
	if (--new_media_check_count <= 0)
	{
		lv_task_del(task_t);
	}
	else if ((user_data_get()->new_media_file_flag == true) || (user_data_get()->new_photo_file_flag == true))
	{
		lv_obj_t *new_media_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), HOME_NEW_MEDIA_ICON_OBJ_ID);
		if (new_media_icon_obj != NULL)
			lv_obj_set_hidden(new_media_icon_obj, false);

		lv_task_del(task_t);
	}
}

static void home_backlight_enable_task(lv_task_t *task_t)
{
	backlight_enable_flag = true;
	backlight_enable(true);

	lv_task_del(task_t);
}
static void home_key_up_home(lv_key_t key)
{
	printf("Home key pressed\n");
	if (cur_layout_get() != pLAYOUT(home))
	{
		goto_layout(pLAYOUT(home));
	}
}
static void home_key_up_esc(lv_key_t key)
{
	printf("ESC key pressed\n");
	goto_layout(pLAYOUT(standby));
}

// 按键绑定表
static const key_binding_t home_key_bindings[] = {
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, home_key_up_home),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, home_key_up_esc),
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),

};

static void LAYOUT_ENTER_FUNC(home)
{
	// standby_timer_close();
	backlight_enable_flag = false;
	power_amplifier_enable(true);

	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(home_key_bindings);

	lv_obj_t *parent = common_bg_display(lv_scr_act());

	home_sdcard_icon_create(parent); /*sd卡ui显示*/
	home_Volume_icon_create(parent); /*声音ui显示*/

	home_time_btn_create(parent);	  // 时间按钮ui显示
	home_intercom_btn_create(parent); /*内线通话ui显示*/
	home_sound_btn_create(parent);	  /*声音设置ui显示 */

	home_lock_btn_create(parent);	  /*开锁ui显示*/
	home_elevator_btn_create(parent); /*呼梯ui显示*/

	home_monitor_btn_create(parent); /*监控ui显示*/
	home_cctv_btn_create(parent);	 /*CCTVui显示*/

	home_setting_btn_create(parent); /*设置按钮ui显示*/

	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_btn_create(bottom_parent);						   /*按键ui显示*/

	if ((user_data_get()->new_media_file_flag == false) || (user_data_get()->new_photo_file_flag == false))
	{
		new_media_check_count = 20;
		lv_layout_task_create(new_media_check_task, 50, LV_TASK_PRIO_MID, NULL);
	}

	home_memory_btn_create(parent); /*浏览ui显示*/

	// 恢复上次焦点
	lv_focus_restore(parent, last_home_focus_id, NULL);

	lyaout_sd_state_callback_register(home_sdcard_state_display_func); /*SD卡回调*/

	lv_layout_task_create(home_backlight_enable_task, 350, LV_TASK_PRIO_MID, NULL);
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
}

static void LAYOUT_QUIT_FUNC(home)
{
	common_obj_null();
	if (backlight_enable_flag == false)
		backlight_enable(true);

	// 保存当前焦点控件的ID
	lv_focus_save(&last_home_focus_id, NULL);

	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
	user_data_save();
}

CREATE_LAYOUT(home);