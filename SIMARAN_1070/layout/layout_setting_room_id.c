/*******************************************************************
 * @Descripttion   : 
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-22 16:53
 * @LastEditTime   : 2023-03-15 15:24
*******************************************************************/
#include "layout_define.h"


#define SETTING_ROOM_LOCAL_ID_LABEL_ID 0

// static lv_task_t *room_id_setting_result_task_t = NULL;

// static void room_id_back_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(setting));
// }
// static void room_id_back_btn_create(lv_obj_t *parent)
// {
//     setting_back_btn_create(parent, room_id_back_btn_up);
// }



static const char *kb_map[] =
	{"1", "2", "3", "\n",
	 "4", "5", "6", "\n",
	 "7", "8", "9", "\n",
	 "*", "0", "#", ""};


static lv_obj_t * ta = NULL;
static void room_id_number_textarea_create(lv_obj_t *parent)
{
	ta = lv_textarea_create(parent, NULL);
	lv_obj_set_pos(ta, 254, 69);
	lv_obj_set_size(ta, 170, 28);
	lv_textarea_set_text(ta, "");
	static char str[40] = {0};
	if(user_data_get()->device_id == GUARD_INTERCOM_NUMBER)
	{
		sprintf(str, "%s:%s", str_get(LAYOUT_INTERCOM_LANG_LOCAL_ADDR_ID), str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));
	}
	else
	{
		sprintf(str, "%s:%03d", str_get(LAYOUT_INTERCOM_LANG_LOCAL_ADDR_ID), user_data_get()->device_id);
	}
	lv_textarea_set_placeholder_text(ta, str);
	lv_textarea_set_cursor_hidden(ta, true);
	lv_textarea_set_max_length(ta, 3);
	lv_textarea_set_one_line(ta, true);
	lv_textarea_set_text_align(ta, LV_LABEL_ALIGN_RIGHT);
	lv_textarea_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
	// lv_obj_set_style_local_border_color(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	// lv_obj_set_style_local_border_color(ta, LV_TEXTAREA_PART_BG, LV_STATE_PRESSED, lv_color_hex(0xFFFFFF));
	// lv_obj_set_style_local_border_color(ta, LV_TEXTAREA_PART_BG, LV_STATE_FOCUSED, lv_color_hex(0xFFFFFF));
	// lv_obj_set_style_local_border_width(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 2);
	// lv_obj_set_style_local_border_side(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_text_font(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(14));
	lv_obj_set_style_local_text_color(ta, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, lv_color_hex(0xB9B9B9));
	lv_obj_set_style_local_value_align(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
}

static void room_id_number_kb_up(lv_obj_t * obj)
{
	const char * txt = lv_btnmatrix_get_active_btn_text(obj);
	lv_textarea_add_char(ta, txt[0]);
	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
}

static void room_id_number_kb_create(lv_obj_t *parent)
{
	lv_obj_t * num_kb = lv_btnmatrix_create(parent, NULL);
	lv_obj_set_pos(num_kb, 56, 34);
	// lv_obj_set_size(num_kb, 184, 222);
	lv_obj_set_size(num_kb, 164, 222);
	lv_btnmatrix_set_map(num_kb, kb_map);
	lv_btnmatrix_set_one_check(num_kb, true);
	
	// lv_obj_set_style_local_bg_color(num_kb, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xAA9191));
	// lv_obj_set_style_local_bg_opa(num_kb, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
	lv_obj_set_style_local_pad_all(num_kb, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
	// lv_obj_set_style_local_pad_ver(num_kb, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 10);

	lv_obj_set_style_local_pad_inner(num_kb, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 10);

	lv_obj_set_style_local_size(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 48);

	lv_obj_set_style_local_border_color(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_border_color(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_color(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0x919191));
	lv_obj_set_style_local_bg_opa(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_bg_opa(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_OPA_50);
	lv_obj_set_style_local_border_width(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_radius(num_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

	lv_obj_set_style_local_text_font(num_kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, FONT_SIZE(32));

	static obj_click_data btn_data = obj_click_data_up_create(room_id_number_kb_up);
	obj_click_event_listen(num_kb, &btn_data);
}


static void room_id_delete_btn_up(lv_obj_t * obj)
{
	lv_textarea_del_char(ta);
}
static void room_id_delete_btn_create(lv_obj_t *parent)
{
	lv_obj_t *delete_btn = lv_obj_create(parent, NULL);
	lv_obj_set_size(delete_btn, 26, 16);
	lv_obj_align(delete_btn, ta, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_INTERCOM_DELETE_PNG);
	lv_obj_set_style_local_pattern_image(delete_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_ext_click_area(delete_btn, 10, 10, 10, 10);

	static obj_click_data btn_data = obj_click_data_up_create(room_id_delete_btn_up);
	obj_click_event_listen(delete_btn, &btn_data);

	lv_obj_t *line = lv_line_create(parent, NULL);
	static lv_point_t points[2];
	points[0].x = 0;
	points[0].y = 0;
	points[1].x = lv_obj_get_width(ta) + lv_obj_get_width(delete_btn);
	points[1].y = 0;
	lv_line_set_points(line, points, 2);
	lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_align(line, ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
}



// static void room_id_setting_result_task(lv_task_t *task_t)
// {
// 	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
// 	lv_task_del(task_t);
// 	room_id_setting_result_task_t = NULL;
// }
static void room_id_setting_result_task_create(const char *str)
{
	lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str);
	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
	// if(room_id_setting_result_task_t != NULL)
	// {
	// 	lv_task_del(room_id_setting_result_task_t);
	// 	room_id_setting_result_task_t = NULL;
	// }
	// room_id_setting_result_task_t = lv_layout_task_create(room_id_setting_result_task, 1000, LV_TASK_PRIO_LOW, NULL);
}



static void room_id_inter_btn_up(lv_obj_t *obj)
{
	int call_num = atoi(lv_textarea_get_text(ta));
	char str1[40] = {0};
	char str2[40] = {0};
	if(call_num > 0 && call_num < 65)
	{
		user_data_get()->device_id = call_num;

		sprintf(str1, "%s:%03d", str_get(LAYOUT_ROOM_ID_LANG_ROOM_ID_ID), user_data_get()->device_id);
		lv_label_set_text(lv_obj_get_child_form_id(lv_scr_act(), SETTING_ROOM_LOCAL_ID_LABEL_ID), str1);
		lv_obj_align(lv_obj_get_child_form_id(lv_scr_act(), SETTING_ROOM_LOCAL_ID_LABEL_ID), NULL, LV_ALIGN_IN_TOP_RIGHT, -19, 12);
		
		sprintf(str2, "%s:%03d", str_get(LAYOUT_INTERCOM_LANG_LOCAL_ADDR_ID), user_data_get()->device_id);
		lv_textarea_set_placeholder_text(ta, str2);

		room_id_setting_result_task_create(str_get(LAYOUT_ROOM_ID_LANG_MODIFY_SUCCESSFUL_ID));
	}
	else if(strcmp(lv_textarea_get_text(ta), "###") == 0)
	{
		user_data_get()->device_id = GUARD_INTERCOM_NUMBER;

		sprintf(str1, "%s:%s", str_get(LAYOUT_ROOM_ID_LANG_ROOM_ID_ID), str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));
		lv_label_set_text(lv_obj_get_child_form_id(lv_scr_act(), SETTING_ROOM_LOCAL_ID_LABEL_ID), str1);
		lv_obj_align(lv_obj_get_child_form_id(lv_scr_act(), SETTING_ROOM_LOCAL_ID_LABEL_ID), NULL, LV_ALIGN_IN_TOP_RIGHT, -19, 12);
		
		sprintf(str2, "%s:%s", str_get(LAYOUT_INTERCOM_LANG_LOCAL_ADDR_ID), str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));
		lv_textarea_set_placeholder_text(ta, str2);

		room_id_setting_result_task_create(str_get(LAYOUT_ROOM_ID_LANG_MODIFY_SUCCESSFUL_ID));
	}
	else
	{
		room_id_setting_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
	}
	lv_textarea_set_text(ta, "");
}
//创建comfirm按钮
static void room_id_comfirm_btn_create(lv_obj_t * parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(room_id_inter_btn_up);
	lv_obj_t *btn = lv_obj_create(parent, NULL);
	lv_obj_set_pos(btn, 273, 130);
	lv_obj_set_size(btn, 179, 40);
	lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x69CC00));
	lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x336600));
	lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 6);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(COMMON_LANG_CONFIRM_ID));
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(16));
	obj_click_event_listen(btn, &btn_data);
}




static void room_id_local_id_label_create(lv_obj_t * parent)
{
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_id(label, SETTING_ROOM_LOCAL_ID_LABEL_ID);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(14));

	char str[40] = {0};
	if(user_data_get()->device_id == GUARD_INTERCOM_NUMBER)
	{
		sprintf(str, "%s:%s", str_get(LAYOUT_ROOM_ID_LANG_ROOM_ID_ID), str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));
	}
	else
	{
		sprintf(str, "%s:%03d", str_get(LAYOUT_ROOM_ID_LANG_ROOM_ID_ID), user_data_get()->device_id);
	}
	
	lv_label_set_text(label, str);
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_RIGHT, -19, 12);
}




static void LAYOUT_ENTER_FUNC(setting_room_id)
{
	standby_timer_close();
	lv_obj_t *parent = common_bg_display(lv_scr_act());
	// setting_logo_img_create(parent);
    // room_id_back_btn_create(parent);
	room_id_number_textarea_create(parent);
	room_id_number_kb_create(parent);
	room_id_delete_btn_create(parent);
	room_id_comfirm_btn_create(parent);
	room_id_local_id_label_create(parent);
	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_btn_create(bottom_parent); /*按键ui显示*/
}
static void LAYOUT_QUIT_FUNC(setting_room_id)
{
	common_obj_null();
	// room_id_setting_result_task_t = NULL;
	user_data_save();
}
CREATE_LAYOUT(setting_room_id);