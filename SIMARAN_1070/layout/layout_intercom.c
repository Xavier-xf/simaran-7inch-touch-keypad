#include "layout_define.h"

#define INTERCOM_LOCAL_ADDR_ID 1

static bool kb_input_room_flag = false; /*在修改房号时区别是输入密码还是输入房号*/
static unsigned int call_num = 0;		// 内线拨打号码
static unsigned int kb_click_num = 0;
static int time_out = 0;
static lv_task_t *call_out_wait_task = NULL;
static lv_task_t *wait_hook_on_task = NULL;
static lv_obj_t *ta = NULL;

static char call_num_save[20] = {0};

static void intercom_wait_hook_on_task_create(void);
static void intercom_call_out_result_task_create(const char *str);
static void intercom_call_out_wait_task_create(void);

static void intercom_key_home(lv_key_t key)
{
	printf("intercom Home key pressed\n");
	if (cur_layout_get() != pLAYOUT(home))
	{
		goto_layout(pLAYOUT(home));
	}
}
static void intercom_key_esc(lv_key_t key)
{
	printf("intercomESC key pressed\n");
	if (user_data_get()->room_no_flag == false)
	{
		goto_layout(pLAYOUT(home));
	}
	else
	{
		goto_layout(pLAYOUT(setting));
	}
}

// 按键绑定表
static const key_binding_t intercom_key_bindings[] = {

	KEY_BIND(LV_KEY_HOME, common_btn_key_down, intercom_key_home),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, intercom_key_esc),
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),

};

// 创建本地地址显示标签
static void intercom_local_addr_label_create(lv_obj_t *parent)
{
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_pos(label, 30, 25);
	lv_obj_set_size(label, 175, 29);
	lv_obj_set_id(label, INTERCOM_LOCAL_ADDR_ID);

	static char str[40] = {0};
	if ((user_data_get()->device_id == GUARD_INTERCOM_NUMBER) || (user_data_get()->device_id == GUARD_2_INTERCOM_NUMBER))
	{
		sprintf(str, "%s : %s", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID),
				(user_data_get()->device_id == GUARD_INTERCOM_NUMBER) ? str_get(LAYOUT_INTERCOM_LANG_GUARD_1_ID) : str_get(LAYOUT_INTERCOM_LANG_GUARD_2_ID));
	}
	else
	{
		sprintf(str, "%s : %03d", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID), user_data_get()->device_id);
	}

	lv_label_set_text(label, str);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
}

static void time_data_display_sw_value_change(lv_obj_t *obj, lv_event_t event)
{
	if (user_data_get()->device_id == GUARD_INTERCOM_NUMBER)
	{
		lv_switch_on(obj, LV_ANIM_ON);
		return;
	}
	if (event == LV_EVENT_VALUE_CHANGED)
		user_data_get()->setting.intercom_receive_enable = lv_switch_get_state(obj);
}

// 创建intercom选择按钮区域
static void intercom_select_btn_create(lv_obj_t *parent)
{
	if (user_data_get()->room_no_flag)
	{
		// 呼叫号码标签
		lv_obj_t *call_label = lv_label_create(parent, NULL);
		lv_obj_set_pos(call_label, 305, 132);
		lv_obj_set_size(call_label, 147, 29);
		lv_label_set_text(call_label, str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID));
		lv_obj_set_style_local_text_font(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_text_color(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	}
	else
	{
		// 接收对讲开关容器
		lv_obj_t *sw_cont = lv_cont_create(parent, NULL);
		lv_obj_set_pos(sw_cont, 305, 55);
		lv_obj_set_size(sw_cont, 350, 40);
		lv_obj_set_style_local_bg_opa(sw_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

		// 开关标签
		lv_obj_t *label = lv_label_create(sw_cont, NULL);
		lv_label_set_text(label, str_get(LAYOUT_INTERCOM_LANG_INTERCOM_RECEIVE_ID));
		lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
		lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);

		// 开关
		lv_obj_t *sw = lv_switch_create(sw_cont, NULL);
		lv_obj_set_size(sw, 64, 34);
		lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x0A84FF));
		lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xCECECE));
		// -------------------------- 焦点状态样式 --------------------------
		// 为主部件（LV_OBJ_PART_MAIN）设置焦点样式，例如边框
		lv_obj_set_style_local_border_color(sw, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
		lv_obj_set_style_local_border_width(sw, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);

		// *** 为开关的指示器部分设置焦点状态下的颜色 ***
		// 当开关获得焦点且处于打开状态时，改变指示器颜色
		lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_INDIC, LV_STATE_FOCUSED, lv_color_hex(0x00FF00));
		lv_obj_set_style_local_bg_color(sw, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x000000));
		lv_obj_set_ext_click_area(sw, 5, 5, 10, 10);
		lv_obj_align(sw, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
		// 添加到默认组
		lv_group_add_obj(lv_group_get_default(), sw);

		static obj_click_data btn_data = obj_click_data_anything_create(time_data_display_sw_value_change);
		obj_click_event_listen(sw, &btn_data);

		if (user_data_get()->setting.intercom_receive_enable == true)
		{
			lv_switch_on(sw, LV_ANIM_OFF);
		}
		else
		{
			lv_switch_off(sw, LV_ANIM_OFF);
		}

		// 呼叫号码标签
		lv_obj_t *call_label = lv_label_create(parent, NULL);
		lv_obj_set_pos(call_label, 305, 132);
		lv_obj_set_size(call_label, 147, 29);
		lv_label_set_text(call_label, str_get(LAYOUT_INTERCOM_LANG_CALL_NUMBER_ID));
		lv_obj_set_style_local_text_font(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_text_color(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	}
}

static void intercom_ta_up(lv_obj_t *obj)
{
	lv_textarea_set_text(ta, "");
	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
}

static void intercom_number_textarea_create(lv_obj_t *parent)
{
	// 输入框背景
	lv_obj_t *cont = lv_cont_create(parent, NULL);
	lv_obj_set_pos(cont, 461, 115);
	lv_obj_set_size(cont, 258, 64);
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF)); // 白色背景
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
	lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);

	// 输入框
	ta = lv_textarea_create(cont, NULL);
	lv_obj_set_pos(ta, 10, 10);
	lv_obj_set_size(ta, 238, 44);
	lv_textarea_set_text(ta, "");
	lv_textarea_set_cursor_hidden(ta, true);

	lv_textarea_set_one_line(ta, true);
	lv_textarea_set_text_align(ta, LV_LABEL_ALIGN_CENTER);

	lv_textarea_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_local_border_width(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_font(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(30));
	lv_obj_set_style_local_text_color(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色文字

	// 设置value文字颜色为黑色，确保状态信息显示为黑色
	lv_obj_set_style_local_value_color(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_value_font(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(30));
	lv_obj_set_style_local_pad_top(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 1);	 // 上内边距7px
	lv_obj_set_style_local_pad_bottom(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 7); // 下内边距7px

	if (user_data_get()->room_no_flag)
	{
		lv_textarea_set_max_length(ta, 5);
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_INPUT_PASSWORD_ID));
	}
	else
	{
		lv_textarea_set_max_length(ta, 3);
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_INPUT_ROOM_NO_ID));
	}

	static obj_click_data btn_data = obj_click_data_up_create(intercom_ta_up);
	obj_click_event_listen(ta, &btn_data);
}

// 创建单个键盘按钮
static lv_obj_t *create_keyboard_button(lv_obj_t *parent, int x, int y, const char *text, obj_click_data *click_data, bool is_icon_button)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, 130, 61);
	// 添加到默认组
	lv_group_add_obj(lv_group_get_default(), btn);
	// 按钮默认样式
	lv_obj_set_style_local_radius(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_opa(btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);

	// -------------------------- 焦点状态样式 --------------------------
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
	lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_40);
	lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x000000));
	// 按钮按下样式
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, 3);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xFFFFFF));

	if (!is_icon_button && text != NULL)
	{
		// 数字按钮文字
		lv_obj_t *label = lv_label_create(btn, NULL);
		lv_label_set_text(label, text);
		lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
		lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
	}

	if (click_data != NULL)
	{
		obj_click_event_listen(btn, click_data);
	}

	return btn;
}

static void keyboard_button_click(lv_obj_t *obj)
{
	if ((call_out_wait_task != NULL) || (wait_hook_on_task != NULL))
		return;

	const char *txt = lv_label_get_text(lv_obj_get_child(obj, NULL));
	printf("Keyboard button pressed: %s\n", txt);

	lv_textarea_add_char(ta, txt[0]);

	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
}

static void delete_button_click(lv_obj_t *obj)
{
	lv_textarea_del_char(ta);
}

static void intercom_button_click(lv_obj_t *obj)
{
	if (call_out_wait_task != NULL || wait_hook_on_task != NULL)
		return;

	call_num = atoi(lv_textarea_get_text(ta));
	memset(call_num_save, 0, sizeof(call_num_save));
	sprintf(call_num_save, "%s", lv_textarea_get_text(ta));
	printf("Call number: %d\n", call_num);

	if (call_num == user_data_get()->device_id)
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
	}
	else if (call_num > 0 && call_num < 257)
	{
		if (hook_state_get() == true)
		{
			intercom_call_out_wait_task_create();
		}
		else
		{
			intercom_wait_hook_on_task_create();
		}
	}
	else
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
	}
	lv_textarea_set_text(ta, "");
}

static void intercom_ok_button_click(lv_obj_t *obj)
{
	static char str[20] = {0};
	kb_click_num = atoi(lv_textarea_get_text(ta));
	printf("kb_click_num: %d\n", kb_click_num);
	if (kb_click_num == user_data_get()->password)
	{
		printf("==================[%d]=====\n", user_data_get()->password);
		lv_textarea_set_text(ta, "");
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_INPUT_ROOM_NO_ID));
		kb_input_room_flag = true;
		lv_textarea_set_max_length(ta, 3);
		return;
	}
	else if (kb_click_num != user_data_get()->password && !kb_input_room_flag)
	{
		lv_textarea_set_text(ta, "");
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
	}

	if (kb_click_num == user_data_get()->device_id && kb_input_room_flag)
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
	}
	else if (kb_click_num > 0 && kb_click_num < 257 && kb_input_room_flag)
	{
		user_data_get()->device_id = kb_click_num;
		intercom_call_out_result_task_create(str_get(LAYOUT_ROOM_ID_LANG_MODIFY_SUCCESSFUL_ID));
		lv_obj_t *local_addr_obj = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_LOCAL_ADDR_ID);
		sprintf(str, "%s : %03d", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID), user_data_get()->device_id);
		lv_label_set_text(local_addr_obj, str);
		printf("kb_click_num setting sucess,user_data_get()->device_id: %d\n", user_data_get()->device_id);
	}
	else if (kb_input_room_flag)
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
	}
	lv_textarea_set_text(ta, "");
}

static void guard_button_click(lv_obj_t *obj)
{
	if (call_out_wait_task != NULL || wait_hook_on_task != NULL)
		return;

	call_num = GUARD_INTERCOM_NUMBER;
	memset(call_num_save, 0, sizeof(call_num_save));
	sprintf(call_num_save, "%s", str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));
	printf("Call guard: %d\n", call_num);

	if (call_num == user_data_get()->device_id)
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
	}
	else
	{
		if (hook_state_get() == true)
		{
			intercom_call_out_wait_task_create();
		}
		else
		{
			intercom_wait_hook_on_task_create();
		}
	}
	lv_textarea_set_text(ta, "");
}

static void intercom_number_kb_create(lv_obj_t *parent)
{
	// 键盘容器
	lv_obj_t *kb_cont = lv_cont_create(parent, NULL);
	lv_obj_set_pos(kb_cont, 305, 191);
	lv_obj_set_size(kb_cont, 414, 353);
	lv_obj_set_style_local_bg_opa(kb_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

	// 键盘按钮点击数据
	static obj_click_data kb_click_data = obj_click_data_up_create(keyboard_button_click);
	static obj_click_data delete_click_data = obj_click_data_up_create(delete_button_click);
	static obj_click_data intercom_click_data = obj_click_data_up_create(intercom_button_click);
	static obj_click_data intercom_ok_btn_up = obj_click_data_up_create(intercom_ok_button_click);
	static obj_click_data guard_click_data = obj_click_data_up_create(guard_button_click);

	// 第一行
	create_keyboard_button(kb_cont, 0, 0, "1", &kb_click_data, false);
	create_keyboard_button(kb_cont, 142, 0, "2", &kb_click_data, false);
	create_keyboard_button(kb_cont, 284, 0, "3", &kb_click_data, false);

	// 第二行
	create_keyboard_button(kb_cont, 0, 73, "4", &kb_click_data, false);
	create_keyboard_button(kb_cont, 142, 73, "5", &kb_click_data, false);
	create_keyboard_button(kb_cont, 284, 73, "6", &kb_click_data, false);

	// 第三行
	create_keyboard_button(kb_cont, 0, 146, "7", &kb_click_data, false);
	create_keyboard_button(kb_cont, 142, 146, "8", &kb_click_data, false);
	create_keyboard_button(kb_cont, 284, 146, "9", &kb_click_data, false);

	// 第四行 - 对讲按钮
	if (!user_data_get()->room_no_flag)
	{
		lv_obj_t *intercom_btn = create_keyboard_button(kb_cont, 0, 219, "", &intercom_click_data, true);
		static rom_bin_info intercom_info = rom_bin_info_get(ROM_UI_INTERCOM_CALL_PNG);
		lv_obj_set_style_local_pattern_image(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &intercom_info);
		lv_obj_set_style_local_pattern_recolor(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0DFF00));
		lv_obj_set_style_local_pattern_recolor_opa(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	}
	else
	{
		lv_obj_t *intercom_btn = create_keyboard_button(kb_cont, 0, 219, "", &intercom_ok_btn_up, true);
		static rom_bin_info intercom_info = rom_bin_info_get(ROM_UI_INTERCOM_OK_PNG);
		lv_obj_set_style_local_pattern_image(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &intercom_info);
		lv_obj_set_style_local_pattern_recolor(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0DFF00));
		lv_obj_set_style_local_pattern_recolor_opa(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	}

	// 0按钮
	create_keyboard_button(kb_cont, 142, 219, "0", &kb_click_data, false);

	// 删除按钮
	lv_obj_t *delete_btn = create_keyboard_button(kb_cont, 284, 219, "", &delete_click_data, true);
	static rom_bin_info delete_info = rom_bin_info_get(ROM_UI_INTERCOM_DELETE_PNG);
	lv_obj_set_style_local_pattern_image(delete_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &delete_info);
	lv_obj_set_style_local_pattern_recolor(delete_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
	lv_obj_set_style_local_pattern_recolor_opa(delete_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);

	if (user_data_get()->room_no_flag == false)
	{
		// 呼叫管理员按钮（最下面单独一行）
		lv_obj_t *guard_btn = lv_btn_create(kb_cont, NULL);
		lv_obj_set_pos(guard_btn, 0, 292);
		lv_obj_set_size(guard_btn, 414, 61);
		// 添加到默认组
		lv_group_add_obj(lv_group_get_default(), guard_btn);
		lv_group_focus_obj(guard_btn);
		// 按钮默认样式
		lv_obj_set_style_local_radius(guard_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
		lv_obj_set_style_local_border_width(guard_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
		lv_obj_set_style_local_border_color(guard_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xF0F0F0));

		// -------------------------- 焦点状态样式 --------------------------
		lv_obj_set_style_local_border_color(guard_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
		lv_obj_set_style_local_border_width(guard_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
		lv_obj_set_style_local_bg_opa(guard_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_40);
		lv_obj_set_style_local_bg_color(guard_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x000000));

		// 按钮按下样式
		lv_obj_set_style_local_border_width(guard_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, 3);
		lv_obj_set_style_local_border_color(guard_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xFFFFFF));

		obj_click_event_listen(guard_btn, &guard_click_data);

		// 管理员按钮图标和文字
		static rom_bin_info guard_info = rom_bin_info_get(ROM_UI_INTERCOM_GUARD_PNG);

		// 创建容器来管理图标和文字的布局
		lv_obj_t *guard_container = lv_cont_create(guard_btn, NULL);
		lv_obj_set_size(guard_container, 414, 61);
		lv_obj_align(guard_container, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_style_local_bg_opa(guard_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_click(guard_container, false);
		// 创建文字标签
		lv_obj_t *guard_label = lv_label_create(guard_container, NULL);
		lv_label_set_text(guard_label, str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));
		lv_obj_set_style_local_text_font(guard_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
		lv_obj_set_style_local_text_color(guard_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_align(guard_label, NULL, LV_ALIGN_IN_BOTTOM_MID, -10, 0);

		// 创建图标
		lv_obj_t *guard_icon = lv_img_create(guard_container, NULL);
		lv_obj_set_size(guard_icon, 48, 48);
		lv_obj_align(guard_icon, NULL, LV_ALIGN_IN_BOTTOM_MID, 80, 0);

		// 设置图标源
		lv_img_set_src(guard_icon, &guard_info);

		// 设置图标颜色
		lv_obj_set_style_local_image_recolor(guard_icon, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0A84FF));
		lv_obj_set_style_local_image_recolor_opa(guard_icon, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	}
}

static void intercom_call_out_result_task_create(const char *str)
{
	lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str);
	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

static void intercom_call_out_wait_task(lv_task_t *task_t)
{
	if (--time_out < 20)
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_NO_RESPONSE_ID));
		lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
		intercom_state_set(INTERCOM_STATE_IDLE);
	}
	if (--time_out < 0)
	{
		lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, "");
		lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
		lv_task_del(task_t);
		call_out_wait_task = NULL;
	}

	if (intercom_unit_busy_state_get())
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_UNIT_BUSY_ID));
		lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
		lv_task_del(task_t);
		call_out_wait_task = NULL;
		intercom_state_set(INTERCOM_STATE_IDLE);
	}

	if (intercom_line_busy_state_get())
	{
		intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_BUS_BUSY_ID));
		lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
		lv_task_del(task_t);
		call_out_wait_task = NULL;
		intercom_state_set(INTERCOM_STATE_IDLE);
	}
}

static void intercom_call_out_wait_task_create(void)
{
	lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str_get(LAYOUT_INTERCOM_LANG_CALLING_ID));
	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
	time_out = 100;

	extern void intercom_busy_state_reset(void);
	intercom_busy_state_reset();

	call_out_wait_task = lv_layout_task_create(intercom_call_out_wait_task, 100, LV_TASK_PRIO_LOW, NULL);
	intercom_number_set(call_num);
	intercom_state_set(INTERCOM_STATE_CALL);
}

static void intercom_wait_hook_on_task(lv_task_t *task_t)
{
	if (--time_out < 0)
	{
		lv_task_del(task_t);
		wait_hook_on_task = NULL;
		lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
		return;
	}
	if (hook_state_get() == true)
	{
		lv_task_del(task_t);
		wait_hook_on_task = NULL;
		intercom_call_out_wait_task_create();
		return;
	}
	static bool hidden_flag = false;
	hidden_flag = !hidden_flag;
	printf("Waiting for handset: %s\n", lv_textarea_get_text(ta));
	lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, hidden_flag ? call_num_save : str_get(LAYOUT_INTERCOM_LANG_TAKE_HANDSET_ID));
	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

static void intercom_wait_hook_on_task_create(void)
{
	time_out = 60;
	lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str_get(LAYOUT_INTERCOM_LANG_TAKE_HANDSET_ID));
	lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
	wait_hook_on_task = lv_layout_task_create(intercom_wait_hook_on_task, 500, LV_TASK_PRIO_LOW, NULL);
}

static void LAYOUT_ENTER_FUNC(intercom)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());

	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_btn_create(bottom_parent);						   /*按键ui显示*/

	intercom_local_addr_label_create(parent); // 本地地址显示
	intercom_select_btn_create(parent);		  // 开关和标签
	intercom_number_kb_create(parent);		  // 键盘
	intercom_number_textarea_create(parent);  // 号码输入框

	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(intercom_key_bindings);
}

static void LAYOUT_QUIT_FUNC(intercom)
{
	common_obj_null();
	user_data_get()->room_no_flag = false;
	user_data_save();
	if (call_out_wait_task != NULL &&
		cur_layout_get() != pLAYOUT(intercom_in) &&
		cur_layout_get() != pLAYOUT(intercom_out) &&
		cur_layout_get() != pLAYOUT(intercom_talk))
		intercom_state_set(INTERCOM_STATE_HUNG_UP);

	call_out_wait_task = NULL;
	wait_hook_on_task = NULL;

	kb_input_room_flag = false;
}

CREATE_LAYOUT(intercom);