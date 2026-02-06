/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-09 18:00
 * @LastEditTime   : 2022-11-24 14:59
 *******************************************************************/
#include "layout_define.h"

typedef enum
{
	SETTING_LANGUAGE_BTN_ID,		 // 语言设置
	SETTING_AUTO_RECORD_BTN_ID,		 // 自动录像
	SETTING_MOTION_DETECTION_BTN_ID, // 运动检测
	SETTING_ROOM_NO_BTN_ID,			 // 房间号设置
	SETTING_FACTORY_RESET_BTN_ID,	 // 恢复出厂设置
	SETTING_FORMAT_SD_BTN_ID,		 // 格式化SD卡
	SETTING_BACKUP_PHOTOS_BTN_ID,	 // 备份照片
	SETTING_SCREEN_ADJUST_BTN_ID,	 // 屏幕调整
	SETTING_VERSION_BTN_ID,			 // 版本号

	SETTING_ADJUST_DOOR1_BTN_ID,
	SETTING_ADJUST_DOOR2_BTN_ID,
	SETTING_ADJUST_CCTV1_BTN_ID,
	SETTING_ADJUST_CCTV2_BTN_ID,

	SETTING_TOTAL_BTN, // 按钮总数（占位）
} setting_btn_module;

#define FACTORY_RESET_LOADER_ID 0x10
#define SETTING_FORMAT_SD_LOADER_ID 0x11
#define SETTING_BACKUP_PHOTOS_LOADER_ID 0x12
#define LAYOUT_SETTING_ADJUST_OBJ_MSG_ID 0X13
#define LAYOUT_SETTING_ADJUST_OBJ_BTNMATRIX_ID 0X14

static unsigned int last_setting_focus_id = 0;
static bool is_from_dialog = false;

static int start_angle = 270;			 // 起始角度
static bool setting_change_flag = false; // 修改标记（退出时判断是否保存）

extern void lv_ft_font_set_type(int type);
/***** 重新初始化字庫 *****/
extern void lv_font_afresh_init(void);

static void setting_key_up_home(lv_key_t key)
{
	printf("setting Home key pressed\n");
	if (cur_layout_get() != pLAYOUT(home))
	{
		goto_layout(pLAYOUT(home));
	}
}
static void setting_key_up_esc(lv_key_t key)
{
	printf("setting ESC key pressed\n");
	if (is_from_dialog == true)
	{
		goto_layout(pLAYOUT(setting));
	}
	else
	{
		goto_layout(pLAYOUT(home));
	}
}
static void setting_direction_key_down(lv_key_t key)
{
	common_btn_key_down(key);
	common_btn_triangle_hidden();
	common_btn_triangle_display(lv_group_get_focused(lv_group_get_default()));
}
static void setting_direction_key_long_down(lv_key_t key)
{
	common_btn_triangle_hidden();
	common_btn_triangle_display(lv_group_get_focused(lv_group_get_default()));
}

// 按键绑定表
static const key_binding_t setting_key_bindings[] = {
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, setting_key_up_home),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, setting_key_up_esc),
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_NEXT, setting_direction_key_down, setting_direction_key_long_down),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_PREV, setting_direction_key_down, setting_direction_key_long_down),

};

// 按键绑定表
static const key_binding_t setting_key_Checkbox_bindings[] = {
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, setting_key_up_home),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, setting_key_up_esc),
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),
};

/***
** 日期: 2024-05-20 10:30
** 作者: leo.liu
** 函数作用：创建屏幕调整选项按钮
** 返回参数说明：成功创建返回true
***/
static bool create_adjust_option_button(lv_obj_t *parent, int width, int height, int x, int y,
										obj_click_data *btn_pdata, const char *string, int btn_id)
{
	static rom_bin_info image_no = rom_bin_info_get(ROM_UI_SETTING_CIRCLE_OPTION_NO_PNG);
	static rom_bin_info image_ck = rom_bin_info_get(ROM_UI_SETTING_CIRCLE_OPTION_CK_PNG);

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_size(btn, width, height);
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_id(btn, btn_id);

	// 设置按钮样式
	lv_obj_set_style_local_radius(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x40, 0x40, 0x40));
	lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	// lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x47, 0x49, 0x4a));
	lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_40);
	lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));

	// 设置按钮图片 - 默认使用未选中状态的图片
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &image_no);
	lv_obj_set_style_local_pattern_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
	lv_obj_set_style_local_pattern_recolor(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	// 设置选中状态的图片
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, &image_ck);
	lv_obj_set_style_local_pattern_align(btn, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, LV_ALIGN_IN_LEFT_MID);
	lv_obj_set_style_local_pattern_recolor(btn, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, LV_COLOR_WHITE);

	if (string != NULL)
	{
		// 创建标签
		lv_obj_t *label = lv_label_create(btn, NULL);
		lv_label_set_text(label, string);
		lv_obj_set_pos(label, 20, (height - 20) / 2); // 标签在图片右侧，左边距40
		lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	}

	// 添加到焦点组并设置点击事件
	lv_group_add_obj(lv_group_get_default(), btn);
	obj_click_event_listen(btn, btn_pdata);

	return true;
}

/***
** 日期: 2024-05-20 10:30
** 作者: leo.liu
** 函数作用：更新按钮选中状态
** 返回参数说明：
***/
static void update_adjust_button_state(lv_obj_t *btn, bool selected)
{
	if (selected)
	{
		lv_obj_add_state(btn, LV_STATE_CHECKED);
		lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0081DC));
	}
	else
	{
		lv_obj_clear_state(btn, LV_STATE_CHECKED);
		lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x40, 0x40, 0x40));
	}
}

// 屏幕调整选项按钮点击回调
static void setting_adjust_option_btn_up(lv_obj_t *obj)
{
	uint16_t btn_id = lv_obj_get_id(obj);

	// 清除所有选项的选中状态
	lv_obj_t *parent = lv_obj_get_parent(obj);
	for (int i = 0; i < 4; i++)
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(parent, SETTING_ADJUST_DOOR1_BTN_ID + i);
		if (btn)
		{
			update_adjust_button_state(btn, false);
		}
	}

	// 设置当前按钮为选中状态
	update_adjust_button_state(obj, true);

	// 保存选中的通道
	MON_CH selected_channel = btn_id - SETTING_ADJUST_DOOR1_BTN_ID + 1;
	monitor_channel_set(selected_channel);
}

static void setting_language_left_arrow_click(lv_obj_t *obj)
{
	// 左箭头点击处理 - 上一个语言
	printf("Left arrow clicked - previous language\n");
	setting_change_flag = true;
	int current_lang = user_data_get()->setting.language;

	if (current_lang == LANG_ENGLISH)
	{
		user_data_get()->setting.language = LANG_PERSIAN;
	}
	else if (current_lang == LANG_PERSIAN)
	{
		user_data_get()->setting.language = LANG_ENGLISH;
	}

	lv_ft_font_set_type(user_data_get()->setting.language);
	lv_font_afresh_init();
	user_data_save();
	printf("Language changed to: %d\n", user_data_get()->setting.language);
	goto_layout(pLAYOUT(setting));
}

static bool setting_language_btn_create(void)
{
	static obj_click_data left_click_data = obj_click_data_up_create(setting_language_left_arrow_click);

	lv_obj_t *btn_obj = setting_right_btn_base_create(NULL, 127, 84 + (50 * 0), 680, 50,
													  str_get(LAYOUT_HOME_LANG_LANGUAGE_ID),
													  !user_data_get()->setting.language ? "English" : "بالعربية", // 当前语言显示
													  &left_click_data,
													  SETTING_LANGUAGE_BTN_ID);
	lv_obj_set_id(btn_obj, SETTING_LANGUAGE_BTN_ID);
	return true;
}

/***
** 日期: 2024-05-20 10:30
** 作者: leo.liu
** 函数作用：获取自动录像模式的显示字符串
** 返回参数说明：返回当前自动录像模式的字符串指针（关闭/视频/照片）
***/
static const char *setting_auto_record_mode_string_get(void)
{

	// 已开启，根据auto_record_mode判断是视频还是照片模式
	switch (user_data_get()->setting.record_mode)
	{
	case LAYOUT_SETTING_LANG_OFF_ID:				 // 失能
		return str_get(LAYOUT_SETTING_LANG_OFF_ID);	 // 复用现有guanbi模式字符串
	case LAYOUT_RECORD_LANG_IMAGE_ID:				 // 照片模式
		return str_get(LAYOUT_RECORD_LANG_IMAGE_ID); // 复用现有照片模式字符串
	case LAYOUT_RECORD_LANG_VIDEO_ID:				 // 视频模式
		return str_get(LAYOUT_RECORD_LANG_VIDEO_ID); // 复用现有视频模式字符串

	default: // 异常mode值：默认返回“关闭”
		return str_get(LAYOUT_SETTING_LANG_OFF_ID);
	}
}
static void setting_auto_record_id_btn_up(lv_obj_t *obj)
{
	printf("Right arrow clicked - previous auto_record_id\n");
	setting_change_flag = true;
	// 切换上一个模式
	switch (user_data_get()->setting.record_mode)
	{
	case LAYOUT_SETTING_LANG_OFF_ID: // 当前是失能模式→切换到照片模式
		user_data_get()->setting.record_mode = LAYOUT_RECORD_LANG_IMAGE_ID;
		break;
	case LAYOUT_RECORD_LANG_IMAGE_ID: // 当前是照片模式→切换到视频模式
		user_data_get()->setting.record_mode = LAYOUT_RECORD_LANG_VIDEO_ID;
		break;
	case LAYOUT_RECORD_LANG_VIDEO_ID: // 当前是视频模式→切换到关闭模式
		user_data_get()->setting.record_mode = LAYOUT_SETTING_LANG_OFF_ID;
		break;
	default: // 异常mode→默认切换到关闭模式
		user_data_get()->setting.record_mode = LAYOUT_SETTING_LANG_OFF_ID;
		break;
	}

	// 更新UI显示
	lv_obj_t *auto_obj = lv_obj_get_child_form_id(lv_scr_act(), SETTING_AUTO_RECORD_BTN_ID + 100);
	if (auto_obj == NULL)
	{
		printf("obj not found\n");
		return;
	}
	lv_obj_set_style_local_value_str(auto_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, setting_auto_record_mode_string_get());
	user_data_save();
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建auto_record设置按钮
** 返回参数说明：
***/
static bool setting_auto_record_btn_create(void)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_auto_record_id_btn_up);
	// static obj_click_data right_click_data = obj_click_data_up_create(setting_auto_record_id_right_btn_up);
	setting_right_btn_base_create(NULL, 127, 84 + (50 * 1), 680, 50,
								  str_get(LAYOUT_RECORD_LANG_AUTO_RECORD_ID),
								  setting_auto_record_mode_string_get(),
								  &click_data,
								  SETTING_AUTO_RECORD_BTN_ID);
	return true;
}

// 按钮点击回调函数
static void setting_motion_detection_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_motion_detection));
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建motion_detection设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_motion_detection_btn_create(void)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_motion_detection_btn_up);
	setting_right_btn_base_create(NULL, 127, 84 + (50 * 2), 680, 50,
								  str_get(LAYOUT_SETTING_LANG_MOTION_DETECTION_ID),
								  user_data_get()->motion.enable ? str_get(LAYOUT_SETTING_LANG_ON_ID) : str_get(LAYOUT_SETTING_LANG_OFF_ID),
								  &click_data,
								  SETTING_MOTION_DETECTION_BTN_ID);
	return true;
}
// 按钮点击回调函数
static void setting_room_id_btn_up(lv_obj_t *obj)
{
	setting_change_flag = true;
	user_data_get()->room_no_flag = true;
	goto_layout(pLAYOUT(intercom));
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建room_id设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_room_id_btn_create(void)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_room_id_btn_up);
	setting_right_btn_base_create(NULL, 127, 84 + (50 * 3), 680, 50,
								  str_get(LAYOUT_SETTING_LANG_ROOM_NO_SETTING_ID),
								  NULL,
								  &click_data,
								  SETTING_ROOM_NO_BTN_ID);
	return true;
}
/**
 * @brief 自定义进度条更新任务
 * @param t LVGL任务对象
 */
static void factory_reset_arc_loader_task(lv_task_t *t)
{

	lv_obj_t *arc = (lv_obj_t *)t->user_data;

	if (arc_loader_update(arc, &start_angle))
	{
		// 如果完成，删除进度条并跳转
		arc_loader_delete(arc);
		lv_task_del(t); // 删除任务
		system("reboot");
	}
}
static void setting_factory_setting_yes_btn_up(lv_obj_t *obj)
{
	setting_change_flag = true;
	setting_msgdialog_msg_bg_delete(LAYOUT_SETTING_FACTORY_RESET_ID);
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(FACTORY_RESET_LOADER_ID, 100, 100);
	// 2. 调用封装函数创建环形进度条
	lv_arc_loader_create(
		cont,
		factory_reset_arc_loader_task,
		120,
		lv_color_hex(0xFFFFFF),
		lv_color_hex(0x007AFF),
		11,
		50,
		0, 0);

	uint8_t lang = user_data_get()->setting.language;
	user_data_reset();
	if (user_data_get()->setting.language != lang)
	{
		/***** 設置字庫 *****/
		extern void lv_ft_font_set_type(int type);
		lv_ft_font_set_type(user_data_get()->setting.language);

		extern void lv_font_afresh_init(void);
		lv_font_afresh_init();
	}
}

static void setting_factory_setting_no_btn_up(lv_obj_t *obj)
{
	printf("no clicked\n");
	LAYOUT_KEY_BINDINGS(setting_key_bindings);
	setting_msgdialog_msg_bg_delete(LAYOUT_SETTING_FACTORY_RESET_ID);
	goto_layout(pLAYOUT(setting));
}
static void setting_factory_setting_btn_up(lv_obj_t *obj)
{
	LAYOUT_KEY_BINDINGS(setting_key_Checkbox_bindings);
	is_from_dialog = true;
	// 保存当前焦点控件的ID
	lv_focus_save(&last_setting_focus_id, NULL);
	lv_group_remove_all_objs(lv_group_get_default());
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(LAYOUT_SETTING_FACTORY_RESET_ID, 460, 156);
	static obj_click_data btn_data = obj_click_data_up_create(setting_factory_setting_yes_btn_up);
	static obj_click_data btn_data1 = obj_click_data_up_create(setting_factory_setting_no_btn_up);
	memory_message_box_create(cont, &btn_data, &btn_data1, LAYOUT_SETTING_FACTORY_RESET_ID);
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建factory_setting设置按钮
** 返回参数说明：
***/
static bool setting_factory_setting_btn_create(void)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_factory_setting_btn_up);
	setting_right_btn_base_create(NULL, 127, 84 + (50 * 4), 680, 50,
								  str_get(LAYOUT_SETTING_LANG_FACTORY_SER_ID),
								  NULL,
								  &click_data,
								  SETTING_FACTORY_RESET_BTN_ID);
	return true;
}

/**
 * @brief SD卡格式化进度条更新任务
 * @param t LVGL任务对象（user_data指向环形进度条arc）
 */
static void format_sd_arc_loader_task(lv_task_t *t)
{
	lv_obj_t *arc = (lv_obj_t *)t->user_data;
	bool UPDATE = arc_loader_update(arc, &start_angle);
	// 检查格式化状态：格式化完成
	if (media_format_sd_state() == false && UPDATE)
	{
		arc_loader_delete(arc);
		lv_task_del(t); // 删除任务
		goto_layout(pLAYOUT(setting));
	}
}
static void setting_formatting_sd_yes_btn_up(lv_obj_t *obj)
{
	setting_change_flag = true;
	printf("yes clicked formatting_sd\n");
	// 删除确认对话框
	setting_msgdialog_msg_bg_delete(LAYOUT_SETTING_FORMAT_SD_ID);

	// 检查SD卡状态
	if (!media_sdcard_insert_check())
	{
		// SD卡未插入，直接返回
		goto_layout(pLAYOUT(setting));
	}

	// 开始格式化
	media_format_sd();
	user_data_get()->new_media_file_flag = false;
	user_data_get()->new_photo_file_flag = false;

	// 创建背景遮罩层
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(SETTING_FORMAT_SD_LOADER_ID, 120, 120);

	// 使用封装函数创建环形进度条
	lv_arc_loader_create(
		cont,
		format_sd_arc_loader_task,
		120,					// 进度条大小
		lv_color_hex(0xFFFFFF), // 背景颜色
		lv_color_hex(0x007AFF), // 指示器颜色
		11,						// 线宽
		50,						// 任务周期(ms)
		0, 0					// 对齐偏移
	);
}
static void setting_formatting_sd_no_btn_up(lv_obj_t *obj)
{
	printf("no clicked formatting_sd\n");
	LAYOUT_KEY_BINDINGS(setting_key_bindings);
	setting_msgdialog_msg_bg_delete(LAYOUT_SETTING_FORMAT_SD_ID);
	goto_layout(pLAYOUT(setting));
}
static void setting_formatting_sd_btn_up(lv_obj_t *obj)
{
	LAYOUT_KEY_BINDINGS(setting_key_Checkbox_bindings);
	is_from_dialog = true;
	// 保存当前焦点控件的ID
	lv_focus_save(&last_setting_focus_id, NULL);
	lv_group_remove_all_objs(lv_group_get_default());
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(LAYOUT_SETTING_FORMAT_SD_ID, 460, 156);
	static obj_click_data btn_data = obj_click_data_up_create(setting_formatting_sd_yes_btn_up);
	static obj_click_data btn_data1 = obj_click_data_up_create(setting_formatting_sd_no_btn_up);
	memory_message_box_create(cont, &btn_data, &btn_data1, LAYOUT_SETTING_FORMAT_SD_ID);
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建formatting_id设置按钮
** 返回参数说明：
***/
static bool setting_formatting_sd_btn_create(void)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_formatting_sd_btn_up);
	setting_right_btn_base_create(NULL, 127, 84 + (50 * 5), 680, 50,
								  str_get(LAYOUT_SETTING_LANG_FORMAT_ID),
								  NULL,
								  &click_data,
								  SETTING_FORMAT_SD_BTN_ID);
	return true;
}

/**
 * @brief 照片备份进度条更新任务
 * @param t LVGL任务对象
 */
static void backup_photos_arc_loader_task(lv_task_t *t)
{

	lv_obj_t *arc = (lv_obj_t *)t->user_data;
	bool UPDATE = arc_loader_update(arc, &start_angle);
	int copied_photo = 0;
	if (media_copy_flash_photo_to_sd_state(&copied_photo) == false && UPDATE)
	{

		arc_loader_delete(arc);
		lv_task_del(t); // 删除任务
		goto_layout(pLAYOUT(setting));
	}

	printf("=====================>>> 已拷贝照片:[%d]\n", copied_photo);
}

/**
 * @brief 照片备份确认按钮回调函数
 */
static void setting_backup_photos_sd_yes_btn_up(lv_obj_t *obj)
{
	setting_change_flag = true;
	// 删除确认对话框
	setting_msgdialog_msg_bg_delete(LAYOUT_SETTING_BACKUP_PHOTOS_SD_ICON_ID);

	// 检查SD卡状态
	if (!media_sdcard_insert_check())
	{
		// SD卡未插入，直接返回
		goto_layout(pLAYOUT(setting));
		printf("SD卡未插入，直接返回\n");
	}

	if (high_speed_media_file_total_get(FILE_TYPE_FLASH_PHOTO) == 0)
	{
		// 没有照片可备份，直接返回
		goto_layout(pLAYOUT(setting));
		printf("没有照片可备份，直接返回\n");
	}

	// 开始备份
	media_copy_flash_photo_to_sd();

	// 创建背景遮罩层
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(SETTING_BACKUP_PHOTOS_LOADER_ID, 120, 120);

	// 使用封装函数创建环形进度条
	lv_arc_loader_create(
		cont,
		backup_photos_arc_loader_task,
		120,					// 进度条大小
		lv_color_hex(0xFFFFFF), // 背景颜色
		lv_color_hex(0x566E7),	// 指示器颜色（绿色表示备份）
		11,						// 线宽
		50,						// 任务周期(ms)
		0, 0					// 对齐偏移
	);
}
static void setting_backup_photos_sd_no_btn_up(lv_obj_t *obj)
{
	printf("no clicked backup_photos_sd\n");
	LAYOUT_KEY_BINDINGS(setting_key_bindings);
	setting_msgdialog_msg_bg_delete(LAYOUT_SETTING_BACKUP_PHOTOS_SD_ICON_ID);
	goto_layout(pLAYOUT(setting));
}
// 按钮点击回调函数
static void setting_backup_photos_sd_btn_up(lv_obj_t *obj)
{
	LAYOUT_KEY_BINDINGS(setting_key_Checkbox_bindings);
	is_from_dialog = true;
	// 保存当前焦点控件的ID
	lv_focus_save(&last_setting_focus_id, NULL);
	lv_group_remove_all_objs(lv_group_get_default());
	// 备份本地照片到SD卡的逻辑实现
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(LAYOUT_SETTING_BACKUP_PHOTOS_SD_ICON_ID, 460, 156);
	static obj_click_data btn_data = obj_click_data_up_create(setting_backup_photos_sd_yes_btn_up);
	static obj_click_data btn_data1 = obj_click_data_up_create(setting_backup_photos_sd_no_btn_up);
	memory_message_box_create(cont, &btn_data, &btn_data1, LAYOUT_SETTING_BACKUP_PHOTOS_SD_ICON_ID);
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建备份本地照片到SD卡的设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_backup_photos_sd_btn_create(void)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_backup_photos_sd_btn_up);
	setting_right_btn_base_create(NULL, 127, 84 + (50 * 6), 680, 50,
								  str_get(LAYOUT_SETTING_BACKUP_PHOTOS_SD_ID),
								  NULL,
								  &click_data,
								  SETTING_BACKUP_PHOTOS_BTN_ID);
	return true;
}

/***
** 日期: 2022-04-28 16:59
** 作者: leo.liu
** 函数作用：消息框按下确认键
** 返回参数说明：
***/
static void setting_always_msgdialog_confirm_btn_up(lv_obj_t *obj)
{
	setting_change_flag = true;

	setting_msgdialog_msg_bg_delete(LAYOUT_SETTING_ADJUST_OBJ_MSG_ID);

	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	user_data_get()->setting.window_display_enable = true;
	goto_layout(pLAYOUT(camera));
}

/***
** 日期: 2022-04-28 16:59
** 作者: leo.liu
** 函数作用：消息框按下返回键
** 返回参数说明：
***/
static void setting_msgdialog_cancel_btn_up(lv_obj_t *obj)
{
	LAYOUT_KEY_BINDINGS(setting_key_bindings);
	setting_adjust_msgdialog_msg_bg_delete(LAYOUT_SETTING_ADJUST_OBJ_MSG_ID);
	goto_layout(pLAYOUT(setting));
}

/***
** 日期: 2022-04-28 14:53
** 作者: leo.liu
** 函数作用：setting_adjust 消息框创建
** 返回参数说明：
***/
static bool setting_adjust_msgdialog_create(void)
{
	lv_obj_t *cont = setting_adjust_msgdialog_msg_bg_create(LAYOUT_SETTING_ADJUST_OBJ_MSG_ID);

	// 创建确认和取消按钮
	setting_msgdialog_msg_confirm_and_cancel_btn_create(cont, setting_msgdialog_cancel_btn_up, setting_always_msgdialog_confirm_btn_up);

	// 创建4个独立选项按钮
	int btn_height = 40;
	int btn_width = lv_obj_get_width(cont) - 120;
	int start_y = 20;

	// 创建按钮点击数据
	static obj_click_data click_data = obj_click_data_up_create(setting_adjust_option_btn_up);

	create_adjust_option_button(cont, btn_width, btn_height, 70, start_y + 0 * (btn_height + 10),
								&click_data, str_get(LAYOUT_HOME_LANG_DOOR1_ID), SETTING_ADJUST_DOOR1_BTN_ID);
	create_adjust_option_button(cont, btn_width, btn_height, 70, start_y + 1 * (btn_height + 10),
								&click_data, str_get(LAYOUT_HOME_LANG_DOOR2_ID), SETTING_ADJUST_DOOR2_BTN_ID);
	create_adjust_option_button(cont, btn_width, btn_height, 70, start_y + 2 * (btn_height + 10),
								&click_data, str_get(LAYOUT_HOME_LANG_CCTV1_ID), SETTING_ADJUST_CCTV1_BTN_ID);
	create_adjust_option_button(cont, btn_width, btn_height, 70, start_y + 3 * (btn_height + 10),
								&click_data, str_get(LAYOUT_HOME_LANG_CCTV2_ID), SETTING_ADJUST_CCTV2_BTN_ID);

	// 设置初始选中状态
	MON_CH ch = monitor_channel_get();
	lv_obj_t *initial_btn = lv_obj_get_child_form_id(cont, SETTING_ADJUST_DOOR1_BTN_ID + ch - 1);
	if (initial_btn)
	{
		update_adjust_button_state(initial_btn, true);
	}

	return true;
}
// 按钮点击回调函数
static void setting_Screen_adjust_btn_up(lv_obj_t *obj)
{
	LAYOUT_KEY_BINDINGS(setting_key_Checkbox_bindings);
	/***** 如果已经创建了，就不再执行 *****/
	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_ADJUST_OBJ_MSG_ID);
	if (parent != NULL)
	{
		return;
	}
	is_from_dialog = true;
	// 保存当前焦点控件的ID
	lv_focus_save(&last_setting_focus_id, NULL);
	lv_group_remove_all_objs(lv_group_get_default());
	setting_adjust_msgdialog_create();
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建Screen_adjust设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_Screen_adjust_btn_create(void)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_Screen_adjust_btn_up);
	setting_right_btn_base_create(NULL, 127, 84 + (50 * 7), 680, 50,
								  str_get(LAYOUT_SETTING_LANG_SCREEN_ADJUST_ID),
								  NULL,
								  &click_data,
								  SETTING_SCREEN_ADJUST_BTN_ID);
	return true;
}

/***
** 日期: 2022-05-04 16:15
** 作者: leo.liu
** 函数作用：版本号显示
** 返回参数说明：
***/
static void setting_version_btn_create(void)
{
	static char sub_string[64] = {0};

	sprintf(sub_string, "%s", SYSTEM_VERSION);
	setting_right_btn_base_create(lv_scr_act(), 127, 84 + (50 * 8), 680, 50, str_get(LAYOUT_ABOUT_LANG_VERSION_ID), sub_string, NULL, 0);
}

static void LAYOUT_ENTER_FUNC(setting)
{
	is_from_dialog = false;
	setting_change_flag = false;
	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(setting_key_bindings);

	lv_obj_t *parent = common_bg_display(lv_scr_act());
	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_btn_create(bottom_parent);						   /*按键ui显示*/

	setting_language_btn_create();
	setting_auto_record_btn_create();
	setting_motion_detection_btn_create();
	setting_room_id_btn_create();
	setting_factory_setting_btn_create();
	setting_formatting_sd_btn_create();
	setting_backup_photos_sd_btn_create();

	setting_Screen_adjust_btn_create();
	setting_version_btn_create();

	// 恢复上次焦点
	lv_focus_restore(parent, last_setting_focus_id, NULL);
	common_btn_triangle_display(lv_group_get_focused(lv_group_get_default()));
}

static void LAYOUT_QUIT_FUNC(setting)
{
	common_obj_null();
	if (setting_change_flag == true)
		user_data_save();
	if (is_from_dialog == false)
		lv_focus_save(&last_setting_focus_id, NULL);
}

CREATE_LAYOUT(setting);
