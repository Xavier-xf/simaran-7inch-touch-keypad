#include "layout_define.h"
#include "media_thumb.h"

#define PHOTO_LIST_IMAGE_BTN_ID 10
#define PHOTO_LIST_PHOTO_PAGE_BTN_ID 6
#define PHOTO_LIST_PHOTO_TATOL_BTN_ID 7
#define PHOTO_LIST_PHOTO_LEFT_BTN_ID 8
#define PHOTO_LIST_PHOTO_RIGHT_BTN_ID 9

static file_type photo_file_type = FILE_TYPE_VIDEO;
static file_type delete_file_type = FILE_TYPE_NONE;

static const file_info *photo_info;
static lv_obj_t *photo_page = NULL;
static char photo_file_path[20] = {0};

static bool delete_button_clicked = false;

static int photo_list_curr_page_num = 1; // 当前页
static int start_angle = 270;			 // 起始角度

static int media_total = 0;
static int exit_time_count = 0;
// static int video_total = 0;

static int photo_index = 0;
static int photo_total;

static void photo_list_key_up_home(lv_key_t key);
static void photo_list_key_up_esc(lv_key_t key);
static void photo_list_key_up_next(lv_key_t key);
static void photo_list_key_up_prev(lv_key_t key);

int photo_index_get(void)
{
	return photo_index;
}
void photo_index_set(int index)
{
	photo_index = index;
}

int photo_total_get(void)
{
	return photo_total;
}
void photo_total_set(int index)
{
	photo_index = index;
}

static int page_num = 0;

typedef enum
{
	PHOTO_LIST_HOME_BTN,
	PHOTO_LIST_IMAGE_BTN,
	PHOTO_LIST_VIDEO_BTN,
	PHOTO_LIST_CALL_BTN,
	PHOTO_LIST_TOTAL_BTN,
} photo_btn_module;

static custom_area photo_list_btn_area[PHOTO_LIST_TOTAL_BTN] =
	{
		{24, 12, 54, 54},
		{28, 410, 157, 170},
		{30, 240, 155, 170},
		{30, 70, 155, 170},
};

// 按键绑定表
static const key_binding_t photo_list_key_bindings[] = {
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, photo_list_key_up_home),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, photo_list_key_up_esc),
	KEY_BIND(LV_KEY_NEXT, common_btn_key_down, photo_list_key_up_next),
	KEY_BIND(LV_KEY_PREV, common_btn_key_down, photo_list_key_up_prev),
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
};
static void photo_list_next_btn_up(lv_obj_t *obj);
static void photo_list_prev_btn_up(lv_obj_t *obj);
static int last_gocusedid = -1;

static void photo_list_next_key_long_down(lv_key_t key)
{
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	int gocusedid = lv_obj_get_id(lv_group_get_focused(lv_group_get_default()));

	printf("===============nowlast_id=%d\n", last_gocusedid);
	if (last_gocusedid == gocusedid)
	{
		photo_list_next_btn_up(NULL);
		lv_photo_list_move_focus(PHOTO_LIST_PHOTO_PAGE_BTN_ID, &gocusedid, photo_total, true);
	}
	last_gocusedid = gocusedid;
}
static void photo_list_next_key_down(lv_key_t key)
{
	common_btn_key_down(key);
	photo_list_next_key_long_down(key);
}
static void photo_list_prev_key_long_down(lv_key_t key)
{
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	int gocusedid = lv_obj_get_id(lv_group_get_focused(lv_group_get_default()));

	if (last_gocusedid == gocusedid)
	{
		photo_list_prev_btn_up(NULL);
		lv_photo_list_move_focus(PHOTO_LIST_PHOTO_PAGE_BTN_ID, &gocusedid, photo_total, false);
	}
	last_gocusedid = gocusedid;
}
static void photo_list_prev_key_down(lv_key_t key)
{
	common_btn_key_down(key);
	photo_list_prev_key_long_down(key);
}

// 按键绑定表
static const key_binding_t photo_list_select_key_bindings[] = {
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, photo_list_key_up_esc),
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, photo_list_key_up_home),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_NEXT, photo_list_next_key_down, photo_list_next_key_long_down),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_PREV, photo_list_prev_key_down, photo_list_prev_key_long_down),

};

static void delete_file_state_display_task(lv_task_t *task_t)
{
	lv_obj_t *arc = (lv_obj_t *)task_t->user_data;
	arc_loader_update(arc, &start_angle);
	if (delete_file_type != FILE_TYPE_NONE)
	{

		if (!media_file_delete_all_state())
		{
			delete_file_type = FILE_TYPE_NONE;
			exit_time_count = 1;
		}
	}
	else
	{
		if (--exit_time_count <= 0)
		{
			arc_loader_delete(arc);
			goto_layout(pLAYOUT(photo_list));
		}
	}
}
static void photo_list_delete_yes_btn_up(lv_obj_t *obj)
{
	setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(0, 100, 100);
	// 2. 调用封装函数创建环形进度条
	lv_arc_loader_create(
		cont,
		delete_file_state_display_task,
		120,
		lv_color_hex(0xFFFFFF),
		lv_color_hex(0x007AFF),
		11,
		50,
		0, 0);
	if (media_sdcard_insert_check() == true)
	{
		delete_file_type = FILE_TYPE_PHOTO;
	}
	else
	{
		delete_file_type = FILE_TYPE_FLASH_PHOTO;
	}
	int total;
	media_file_total_get(delete_file_type, &total, NULL);

	if (total == 0)
	{
		delete_file_type = FILE_TYPE_NONE;
		exit_time_count = 1;
	}
	else
	{
		exit_time_count = 1;
		media_file_delete_all_start(delete_file_type);
	}
	user_data_get()->new_media_file_flag = 0;
}

static void photo_list_delete_no_btn_up(lv_obj_t *obj)
{
	printf("no clicked\n");
	LAYOUT_KEY_BINDINGS(photo_list_key_bindings);
	setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
	goto_layout(pLAYOUT(photo_list));
}

static void photo_list_key_up_home(lv_key_t key)
{
	if (photo_total == 0)
		return;
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	lv_group_set_wrap(lv_group_get_default(), true);
	// 标记为已点击
	delete_button_clicked = true;
	LAYOUT_KEY_BINDINGS(photo_list_select_key_bindings);
	lv_group_focus_freeze(lv_group_get_default(), false);
	lv_group_remove_all_objs(lv_group_get_default());
	lv_obj_t *cont = setting_msgdialog_msg_bg_create(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID, 460, 156);
	static obj_click_data btn_data = obj_click_data_up_create(photo_list_delete_yes_btn_up);
	static obj_click_data btn_data1 = obj_click_data_up_create(photo_list_delete_no_btn_up);
	memory_message_box_create(cont, &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
}
static void photo_list_key_up_esc(lv_key_t key)
{
	if (delete_button_clicked)
	{
		goto_layout(pLAYOUT(photo_list));
		return;
	}
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), PHOTO_LIST_IMAGE_BTN_ID);
	if (obj->group_p == NULL)
	{
		LAYOUT_KEY_BINDINGS(photo_list_key_bindings);
		lv_group_add_obj(lv_group_get_default(), obj);
		lv_group_focus_obj(obj);
		lv_group_focus_freeze(lv_group_get_default(), true);
	}
	else
	{
		goto_layout(pLAYOUT(home));
	}
}

static void photo_list_key_up_next(lv_key_t key)
{
	goto_layout(pLAYOUT(call_list));
}
static void photo_list_key_up_prev(lv_key_t key)
{
	goto_layout(pLAYOUT(video_list));
}

void photo_play_parameter_init(void)
{

	if (media_sdcard_insert_check() == true)
	{
		photo_file_type = FILE_TYPE_PHOTO;
		sprintf(photo_file_path, "%s", SD_PHOTO_PATH);
		sd_media_all_file_total_get(&media_total, NULL);
		printf("====================>> sd卡媒体文件总数:[%d]\n", media_total);
	}
	else
	{
		photo_file_type = FILE_TYPE_FLASH_PHOTO;
		sprintf(photo_file_path, "%s", FLASH_PHOTO_PATH);
		media_file_total_get(photo_file_type, &media_total, NULL);
		printf("====================>> flash媒体文件总数:[%d]\n", media_total);
	}

	media_file_total_get(photo_file_type, &photo_total, NULL);
	printf("====================>> 图片文件总数:[%d]\n", photo_total);
}

// 创建call按钮
static void photo_list_call_btn_create(lv_obj_t *parent)
{
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_CALL_PNG);
	photo_and_video_btn_create(parent, photo_list_btn_area[PHOTO_LIST_CALL_BTN], str_get(LAYOUT_MEMORY_LANG_CALL_ID), NULL, &info, true, false);
}

static void photo_list_image_btn_up(lv_obj_t *obj)
{
	if (photo_total <= 0)
		return;
	LAYOUT_KEY_BINDINGS(photo_list_select_key_bindings);
	lv_group_focus_freeze(lv_group_get_default(), false);
	lv_group_remove_obj(lv_obj_get_child_form_id(lv_scr_act(), PHOTO_LIST_IMAGE_BTN_ID));
	lv_obj_t *first_file_btn = NULL;
	// 先找到列表容器
	lv_obj_t *image_page = lv_obj_get_child_form_id(lv_scr_act(), PHOTO_LIST_PHOTO_PAGE_BTN_ID);
	if (image_page != NULL)
	{
		// 从 image_page 中找 第一个文件控件
		first_file_btn = lv_obj_get_child_form_id(image_page, last_gocusedid);
	}

	// 设置焦点
	if (first_file_btn != NULL)
	{
		lv_group_focus_obj(first_file_btn);
		printf("焦点已移动到第一个文件按钮\n");
	}
	else
	{
		first_file_btn = lv_obj_get_child_form_id(image_page, 0);
		last_gocusedid = 0;
		lv_group_focus_obj(first_file_btn);
	}
}
// 创建image按钮
static void photo_list_image_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_list_image_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_RED_PNG);
	lv_obj_t *btn = photo_and_video_btn_create(parent, photo_list_btn_area[PHOTO_LIST_IMAGE_BTN], str_get(LAYOUT_MEMORY_LANG_IMAGE_LISE_ID), &btn_data, &info, false, true);
	lv_obj_set_id(btn, PHOTO_LIST_IMAGE_BTN_ID);
}

// 创建video按钮
static void photo_list_video_btn_create(lv_obj_t *parent)
{

	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_VIDEO_PNG);
	photo_and_video_btn_create(parent, photo_list_btn_area[PHOTO_LIST_VIDEO_BTN], str_get(LAYOUT_MEMORY_LANG_VIDEO_LISE_ID), NULL, &info, true, false);
}

static void photo_list_btn_up(lv_obj_t *obj)
{
	photo_index = obj->obj_id;
	printf("==========@@@@@@@@==[%d]======\n", photo_index);
	if (photo_total == 0)
	{
		photo_index = -1;
	}

	goto_layout(pLAYOUT(memory_photo));
}

static lv_obj_t *photo_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int i)
{
	printf("#########################111\n");

	lv_obj_t *btn = lv_obj_create(parent, NULL);
	lv_obj_set_id(btn, i);
	lv_group_add_obj(lv_group_get_default(), btn);
	printf("id=%d\n", i);
	lv_obj_set_drag_parent(btn, true);
	// lv_btn_set_layout(btn, LV_FIT_NONE);
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);
	// set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3BD741));
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x4f4f4f));

	// -------------------------- 焦点状态样式 --------------------------
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
	lv_obj_set_style_local_border_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_20);
	lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_20);
	lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));
	// 为按钮添加点击事件

	static obj_click_data click_data = obj_click_data_up_create(photo_list_btn_up);
	obj_click_event_listen(btn, &click_data);

	photo_info = media_file_info_get(photo_file_type, photo_total - i - 1);
	printf("#########################333\n");

	lv_color_t new_file_color_def = lv_color_hex(0xC52323); // 新文件-默认字体色
	lv_color_t normal_color_def = lv_color_hex(0xFFFFFF);	// 非新文件-默认字体色

	// 根据“是否为新文件”选择对应的颜色组
	lv_color_t curr_color_def = (photo_info->is_new) ? new_file_color_def : normal_color_def;

	char year[32] = {0};
	char month[32] = {0};
	char day[32] = {0};
	char hour[32] = {0};
	char min[32] = {0};
	char second[32] = {0};
	static char file_year[64] = {0};
	// static char file_time[64] = {0};
	strncpy(year, photo_info->file_name, 2);
	strncpy(month, photo_info->file_name + 2, 2);
	strncpy(day, photo_info->file_name + 4, 2);
	strncpy(hour, photo_info->file_name + 7, 2);
	strncpy(min, photo_info->file_name + 9, 2);
	strncpy(second, photo_info->file_name + 11, 2);

	printf("------year[%s]\n", photo_info->file_name);
	printf("------year[%s]\n", year);
	printf("------month[%s]\n", month);
	printf("------day[%s]\n", day);
	printf("------hour[%s]\n", hour);
	printf("------min[%s]\n", min);
	printf("------second[%s]\n", second);

	struct date temp_date =
		{
			.year = atoi(year) + 2000,
			.month = atoi(month),
			.day = atoi(day)};
	temp_date = gregorian2jalali(temp_date);

	if (user_data_get()->setting.calendar == 1)
	{
		sprintf(file_year, "20%s-%s-%s %s:%s", year, month, day, hour, min);
	}
	else
	{
		sprintf(file_year, "%d-%02d-%02d %s:%s",
				temp_date.year, temp_date.month, temp_date.day,
				hour, min);
	}
	// sprintf(file_time, "%s:%s:%s", hour, min, second);
	printf("---------------------file_year[%s]\n", file_year);
	// printf("---------------------file_time[%s]\n", file_time);

	lv_obj_set_style_local_value_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
	lv_obj_set_style_local_value_ofs_x(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 73);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	lv_obj_t *obj = lv_obj_create(btn, NULL);
	if (photo_info->ch == MON_CH_DOOR1)
	{
		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR1_ID));
	}
	else if (photo_info->ch == MON_CH_DOOR2)
	{
		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR2_ID));
	}
#if 1
	else if (photo_info->ch == MON_CH_CCTV1)
	{
		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV1_ID));
	}
	else if (photo_info->ch == MON_CH_CCTV2)
	{
		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV2_ID));
	}
#endif
	lv_obj_set_style_local_value_color(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态
	// 其他样式
	lv_obj_set_style_local_value_font(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	if (user_data_get()->setting.language == LANG_ENGLISH)
	{
		lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
	}
	else
	{
		lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
	}
	// 位置和点击属性
	lv_obj_align(obj, btn, LV_ALIGN_IN_LEFT_MID, 310, 5);
	lv_obj_set_click(obj, false);

	lv_obj_t *label2 = lv_label_create(btn, NULL);
	lv_label_set_text(label2, file_year);			 // 设置日期文本
	lv_label_set_align(label2, LV_LABEL_ALIGN_LEFT); // 文本左对齐
	// 【核心样式】设置“默认”和“按下”状态的字体颜色
	lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态
																									 // 位置
	lv_obj_align(label2, btn, LV_ALIGN_IN_LEFT_MID, 30, 0);
	// 其他样式
	lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_t *label3 = lv_label_create(btn, NULL);

	// 根据mode值设置录制模式文本（手动/自动/运动检测）
	const char *mode_text = NULL;
	int mode_align = 495;
	switch (photo_info->mode)
	{
	case REC_MODE_MANUAL:
		mode_text = str_get(LAYOUT_MEMORY_LANG_MANUAL_ID); // 手动录制
		break;
	case REC_MODE_AUTO:
		mode_text = str_get(LAYOUT_MEMORY_LANG_AUTO_ID); // 自动录制
		if (user_data_get()->setting.language == LANG_PERSIAN)
		{
			mode_align = 475;
		}
		else
		{
			mode_align = 515;
		}
		break;
	case REC_MODE_MOTION:
		mode_text = str_get(LAYOUT_MEMORY_LANG_MOTION); // 运动检测录制
		break;
	default:
		mode_text = str_get(LAYOUT_MEMORY_LANG_MANUAL_ID); // 手动录制
		break;
	}
	lv_label_set_text(label3, mode_text); // 标签文本设置
	// 其他样式
	lv_label_set_align(label3, LV_LABEL_ALIGN_LEFT);
	// 位置
	lv_obj_align(label3, btn, LV_ALIGN_IN_LEFT_MID, mode_align, 0);
	lv_obj_set_style_local_text_font(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_set_style_local_text_color(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态

	lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
	lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);

	return btn;
}

static void photo_list_cont_create(lv_obj_t *parent)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(cont, 207, 76);
	lv_obj_set_size(cont, 699, 497);
	lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 21);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x292929));
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
}

static void photo_list_head_label_create(lv_obj_t *parent)
{
	lv_obj_t *set_page = lv_cont_create(parent, NULL);
	lv_obj_set_pos(set_page, 230, 90);
	lv_obj_set_size(set_page, 638, 45);
	// set_location(photo_page, 180, 89, 748, 426);
	lv_obj_set_style_local_value_color(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
	lv_obj_set_style_local_value_align(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
	lv_obj_set_style_local_value_ofs_x(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
	lv_obj_set_style_local_value_font(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_t *label = lv_label_create(set_page, NULL);
	lv_obj_set_pos(label, 50, 0);
	lv_obj_set_size(label, 176, 29);
	// lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 9, 44);
	// if(user_data_get()->media_disp_mode == 0)
	// {
	lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_RECORD_TIME_ID));
	// }
	// else
	// {
	// 	lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_VIDEO_LISE_ID));
	// }

	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_t *label2 = lv_label_create(set_page, NULL);
	lv_obj_set_pos(label2, 310, 0);
	lv_obj_set_size(label2, 171, 29);

	lv_label_set_text(label2, str_get(LAYOUT_MEMORY_LANG_DEVICE_ID));

	lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_t *label3 = lv_label_create(set_page, NULL);
	lv_obj_set_pos(label3, 460, 0);
	lv_obj_set_size(label3, 167, 29);

	lv_label_set_text(label3, str_get(LAYOUT_MEMORY_LANG_RECORD_MODE_ID));

	lv_obj_set_style_local_text_color(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_set_style_local_border_width(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_border_color(set_page, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
	lv_obj_set_style_local_border_side(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
}

/***
**   日期:2022-05-23 16:28:24
**   作者: leo.liu
**   函数作用：页面显示
**   参数说明:
***/
static void playback_total_label_display(void)
{
	static char str[20];
	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), PHOTO_LIST_PHOTO_TATOL_BTN_ID);
	int total_page_num = (photo_total == 0) ? 0 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0));
	if (photo_total == 0)
	{
		photo_list_curr_page_num = 0;
	}
	sprintf(str, "%d/%d", photo_list_curr_page_num, total_page_num);
	lv_label_set_text(label, str);
	lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
}
/***
**   日期:2022-05-23 16:25:07
**   作者: leo.liu
**   函数作用：显示页面创建
**   参数说明:
***/
static void playback_total_label_create(void)
{
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_id(label, PHOTO_LIST_PHOTO_TATOL_BTN_ID);
	lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(label, 453, 525);
	lv_obj_set_size(label, 222, 34);
	// lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_MID, 36, -71);
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	playback_total_label_display();
}

static void photo_list_prev_btn_up(lv_obj_t *obj)
{
	if (--photo_list_curr_page_num <= 0)
	{
		photo_list_curr_page_num = (photo_total == 0) ? 1 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0));
		if (photo_list_curr_page_num == 1)
			return;
	}
	playback_total_label_display();
	lv_obj_clean(photo_page);
	for (int i = 0; ((page_num - photo_list_curr_page_num) * 6 + i) < photo_total; i++)
	{
		// 先判断是否超过6个
		if (i >= 6)
			break;

		// 计算文件索引，判断是否超过总文件数
		int file_idx = (page_num - photo_list_curr_page_num) * 6 + i;
		if (file_idx >= photo_total)
			break;

		// 创建控件
		if (photo_list_page_btn_create(photo_page, 0, i * 62, 638, 62, file_idx) == NULL)
		{
			i--;
		}
	}
}

static void photo_list_next_btn_up(lv_obj_t *obj)
{
	if (++photo_list_curr_page_num > ((photo_total == 0) ? 0 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0))))
	{
		photo_list_curr_page_num = 1;
		if (((photo_total == 0) ? 1 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0))) == 1)
			return;
	}
	playback_total_label_display();
	lv_obj_clean(photo_page);
	for (int i = 0; ((page_num - photo_list_curr_page_num) * 6 + i) < photo_total; i++)
	{
		// 先判断是否超过6个
		if (i >= 6)
			break;

		// 计算文件索引，判断是否超过总文件数
		int file_idx = (page_num - photo_list_curr_page_num) * 6 + i;
		if (file_idx >= photo_total)
			break;

		// 创建控件
		if (photo_list_page_btn_create(photo_page, 0, i * 62, 638, 62, file_idx) == NULL)
		{
			i--;
		}
	}
}

static void photo_list_page_display(lv_obj_t *parent)
{

	photo_page = lv_cont_create(parent, NULL);
	lv_obj_set_id(photo_page, PHOTO_LIST_PHOTO_PAGE_BTN_ID);
	lv_obj_set_pos(photo_page, 230, 131);
	lv_obj_set_size(photo_page, 638, 373);
	// set_location(photo_page, 180, 89, 748, 426);
	lv_obj_set_style_local_value_color(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
	lv_obj_set_style_local_value_align(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
	lv_obj_set_style_local_value_ofs_x(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
	lv_obj_set_style_local_value_font(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	if (photo_index_get() < 0)
	{
		photo_index_set(0);
	}
	int select_index = photo_index_get();
	page_num = (photo_total / 6 + ((photo_total % 6) ? 1 : 0));
	photo_list_curr_page_num = ((select_index <= 6) ? page_num : (page_num - (select_index / 6)));
	printf("+++++++++++++++++>>>>>>>>>111111[%d]====[%d]\n", photo_list_curr_page_num, select_index);
	for (int i = 0; ((page_num - photo_list_curr_page_num) * 6 + i) < photo_total; i++)
	{
		// 先判断是否超过6个
		if (i >= 6)
			break;

		// 计算文件索引，判断是否超过总文件数
		int file_idx = (page_num - photo_list_curr_page_num) * 6 + i;
		if (file_idx >= photo_total)
			break;

		// 创建控件
		if (photo_list_page_btn_create(photo_page, 0, i * 62, 638, 62, file_idx) == NULL)
		{
			i--;
		}
	}

	lv_obj_t *prev_btn = lv_obj_create(lv_scr_act(), NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
	lv_obj_set_pos(prev_btn, 404, 528);
	lv_obj_set_size(prev_btn, 72, 72);
	// lv_obj_set_size(prev_btn, 72, 72);
	// lv_obj_align(prev_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, -158, 0);
	lv_obj_set_style_local_pattern_image(prev_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_hidden(prev_btn, true);

	static obj_click_data btn_data1 = obj_click_data_up_create(photo_list_prev_btn_up);
	obj_click_event_listen(prev_btn, &btn_data1);

	lv_obj_t *next_btn = lv_obj_create(lv_scr_act(), NULL);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
	lv_obj_set_pos(next_btn, 670, 528);
	lv_obj_set_size(next_btn, 72, 72);
	// lv_obj_set_size(next_btn, 72, 72);
	// lv_obj_align(next_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 158, 0);
	lv_obj_set_style_local_pattern_image(next_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info1);
	lv_obj_set_hidden(next_btn, true);

	static obj_click_data btn_data2 = obj_click_data_up_create(photo_list_next_btn_up);
	obj_click_event_listen(next_btn, &btn_data2);
}

// SD卡拔插状态
static void photo_list_sdcard_state_change_func(void)
{
	if (media_sdcard_insert_check() == false)
	{
		photo_index = 0;
		goto_layout(pLAYOUT(photo_list));
	}
	else
	{
		photo_index = 0;
		goto_layout(pLAYOUT(photo_list));
	}
}

static void LAYOUT_ENTER_FUNC(photo_list)
{
	// standby_timer_close();
	delete_button_clicked = false;
	printf("============================enter_photo_list\n");
	user_data_get()->new_photo_file_flag = false;
	photo_play_parameter_init();
	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(photo_list_key_bindings);

	lv_obj_t *parent = common_bg_display(lv_scr_act());

	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_view_select_btn_create(bottom_parent);			   /*按键ui显示*/

	photo_list_image_btn_create(parent);

	photo_list_video_btn_create(parent);

	photo_list_call_btn_create(parent);

	lv_group_focus_freeze(lv_group_get_default(), true);
	lv_group_set_wrap(lv_group_get_default(), false);

	photo_list_cont_create(parent);

	photo_list_head_label_create(parent);

	photo_list_page_display(parent); // 在当前活跃的屏幕上创建容器

	playback_total_label_create();

	lyaout_sd_state_callback_register(photo_list_sdcard_state_change_func);
}
static void LAYOUT_QUIT_FUNC(photo_list)
{
	common_obj_null();
	lv_group_set_wrap(lv_group_get_default(), true);
	const layout *cur_layout = cur_layout_get();
	if ((cur_layout != pLAYOUT(memory_photo)))
	{
		thumb_media_close();
	}

	lyaout_sd_state_callback_register(NULL);
	if ((cur_layout != pLAYOUT(memory_photo)) && (cur_layout != pLAYOUT(photo_list)))
		photo_index_set(0);
}
CREATE_LAYOUT(photo_list);