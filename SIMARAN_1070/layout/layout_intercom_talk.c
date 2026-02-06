#include "layout_define.h"

#define INTERCOM_TALK_TEXT_LABEL_ID 0
#define INTERCOM_TALK_RING_CONT_ID 1
#define INTERCOM_TALK_RING_BAR_ID 2
#define INTERCOM_TALK_VOL_SLIDER_ID 4
#define INTERCOM_TALK_VOL_LABEL_ID 5
#define INTERCOM_TALK_TIME_COUNT_LABEL_ID 6
#define INTERCOM_TALK_COUNTDOWN_RING_ID 7
#define INTERCOM_TALK_LEFT_ARROW_ID 8
#define INTERCOM_TALK_RIGHT_ARROW_ID 9
#define INTERCOM_TALK_CALL_FROM_LABEL_ID 10
#define INTERCOM_TALK_ROOM_NUMBER_LABEL_ID 11
#define INTERCOM_TALK_VOL_SLIDER_BG_ID 12
#define INTERCOM_TALK_VOL_SLIDER_INDICATOR_ID 13
#define INTERCOM_TALK_SOUND_BTN_OBJ_ID 14

static lv_task_t *hung_up_task = NULL;
static lv_task_t *talking_task = NULL;

static int count = 180; // 3分钟倒计时（180秒）

static void intercom_talk_hung_up_task_create(void);

static void intercom_talk_hung_up_btn_up(lv_obj_t *obj)
{
	if (hung_up_task == NULL)
	{
		intercom_state_set(INTERCOM_STATE_HUNG_UP);
		intercom_talk_hung_up_task_create();
	}
	if (talking_task != NULL)
	{
		lv_task_del(talking_task);
		talking_task = NULL;
	}
}

// 创建中心倒计时环和标签
static void intercom_talk_countdown_create(lv_obj_t *parent)
{
	// 创建倒计时环
	lv_obj_t *countdown_ring = lv_obj_create(parent, NULL);
	lv_obj_set_id(countdown_ring, INTERCOM_TALK_COUNTDOWN_RING_ID);
	lv_obj_set_size(countdown_ring, 270, 270);
	lv_obj_set_pos(countdown_ring, 376, 151);
	static rom_bin_info ring_info = rom_bin_info_get(ROM_UI_INTERCOM_COUNTDOWN_RING_PNG);
	lv_obj_set_style_local_pattern_image(countdown_ring, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &ring_info);

	// 创建倒计时标签 - 使用分秒格式，字体大小与旧UI相同
	lv_obj_t *countdown_label = lv_label_create(parent, NULL);
	lv_obj_set_id(countdown_label, INTERCOM_TALK_TIME_COUNT_LABEL_ID);
	lv_obj_set_size(countdown_label, 200, 77);
	lv_obj_set_pos(countdown_label, 465, 260);

	// 设置字体样式 - 使用与旧通话界面相同的字体大小
	lv_obj_set_style_local_text_font(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(40));
	lv_obj_set_style_local_text_line_space(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_letter_space(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);

	// 初始设置为00:00
	lv_label_set_text_fmt(countdown_label, "00:00");
	lv_label_set_align(countdown_label, LV_LABEL_ALIGN_CENTER);
}

// 创建左右箭头和标签
static void intercom_talk_arrows_create(lv_obj_t *parent)
{
	// 左侧箭头
	lv_obj_t *left_arrow = lv_obj_create(parent, NULL);
	lv_obj_set_id(left_arrow, INTERCOM_TALK_LEFT_ARROW_ID);
	lv_obj_set_size(left_arrow, 52, 32);
	lv_obj_set_pos(left_arrow, 296, 270);
	static rom_bin_info left_arrow_info = rom_bin_info_get(ROM_UI_INTERCOM_POINT_PNG);
	lv_obj_set_style_local_pattern_image(left_arrow, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &left_arrow_info);

	// 左侧提示语 - 显示"正在与"
	lv_obj_t *call_from_label = lv_label_create(parent, NULL);
	lv_obj_set_id(call_from_label, INTERCOM_TALK_CALL_FROM_LABEL_ID);
	lv_obj_set_size(call_from_label, 181, 39);
	lv_obj_set_pos(call_from_label, 101, 266);
	lv_obj_set_style_local_text_font(call_from_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));
	lv_label_set_text(call_from_label, str_get(LAYOUT_INTERCOM_LANG_TALKING_WITH_ID));

	// 右侧箭头
	lv_obj_t *right_arrow = lv_obj_create(parent, NULL);
	lv_obj_set_id(right_arrow, INTERCOM_TALK_RIGHT_ARROW_ID);
	lv_obj_set_size(right_arrow, 52, 32);
	lv_obj_set_pos(right_arrow, 674, 271);
	static rom_bin_info right_arrow_info = rom_bin_info_get(ROM_UI_INTERCOM_POINT_PNG);
	lv_obj_set_style_local_pattern_image(right_arrow, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &right_arrow_info);

	// 右侧房号或名称
	lv_obj_t *room_label = lv_label_create(parent, NULL);
	lv_obj_set_id(room_label, INTERCOM_TALK_ROOM_NUMBER_LABEL_ID);
	lv_obj_set_size(room_label, 150, 39);
	lv_obj_set_pos(room_label, 750, 266);
	lv_obj_set_style_local_text_font(room_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));

	int talking_number = intercom_number_get();
	if ((talking_number == GUARD_INTERCOM_NUMBER) || (talking_number == GUARD_2_INTERCOM_NUMBER))
	{
		lv_label_set_text(room_label, talking_number == GUARD_INTERCOM_NUMBER ? str_get(LAYOUT_INTERCOM_LANG_GUARD_1_ID) : str_get(LAYOUT_INTERCOM_LANG_GUARD_2_ID));
	}
	else
	{
		lv_label_set_text_fmt(room_label, "%03d", talking_number);
	}
}

static void intercom_talk_text_icon_create(lv_obj_t *parent)
{
	// 挂断按钮
	lv_obj_t *hung_up_btn = lv_obj_create(parent, NULL);
	lv_obj_set_size(hung_up_btn, 170, 76);
	lv_obj_set_pos(hung_up_btn, 427, 490);
	lv_obj_set_ext_click_area(hung_up_btn, 20, 20, 20, 20);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_INTERCOM_HUNG_UP_PNG);
	lv_obj_set_style_local_pattern_image(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_style_local_pattern_recolor(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
	lv_obj_set_style_local_pattern_recolor_opa(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
	static obj_click_data btn_data = obj_click_data_up_create(intercom_talk_hung_up_btn_up);
	obj_click_event_listen(hung_up_btn, &btn_data);
	lv_group_add_obj(lv_group_get_default(), hung_up_btn);
	lv_obj_set_hidden(hung_up_btn, true);
}

extern bool intercom_call_in_flag;
static void intercom_talk_hung_up_task(lv_task_t *task_t)
{
	if (intercom_call_in_flag == true)
	{
		goto_layout(pLAYOUT(standby));
	}
	else
	{
		goto_layout(pLAYOUT(intercom));
	}
}

static void intercom_talk_hung_up_task_create(void)
{
	lv_obj_t *countdown_label = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_TIME_COUNT_LABEL_ID);
	if (countdown_label != NULL)
	{
		lv_label_set_text(countdown_label, "00:00");
	}

	hung_up_task = lv_layout_task_create(intercom_talk_hung_up_task, 2000, LV_TASK_PRIO_LOW, NULL);
}

static void intercom_talk_talking_task(lv_task_t *task_t)
{
	if (--count < 0 || intercom_state_get() == INTERCOM_STATE_IDLE)
	{
		if (talking_task != NULL)
		{
			lv_task_del(talking_task);
			talking_task = NULL;
		}
		intercom_state_set(INTERCOM_STATE_IDLE);
		intercom_talk_hung_up_task_create();
		return;
	}

	// 使用分秒格式显示时间
	lv_label_set_text_fmt(task_t->user_data, "%02d:%02d", (180 - count) / 60, (180 - count) % 60);
}

static void intercom_talk_talking_task_create(void)
{
	if (talking_task != NULL)
	{
		lv_task_del(talking_task);
		talking_task = NULL;
	}

	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_TIME_COUNT_LABEL_ID);
	lv_label_set_text(label, "00:00"); // 初始显示3分钟

	count = 180; // 重置为180秒（3分钟）
	talking_task = lv_layout_task_create(intercom_talk_talking_task, 1000, LV_TASK_PRIO_LOW, label);

	// 启动音频通话
	if ((intercom_number_get() == GUARD_INTERCOM_NUMBER || OwnID == GUARD_INTERCOM_NUMBER) ||
		(intercom_number_get() == GUARD_2_INTERCOM_NUMBER || OwnID == GUARD_2_INTERCOM_NUMBER))
	{
		door_audio_talk(AUDIO_CH_GUARD);
	}
	else
	{
		door_audio_talk(AUDIO_CH_INTER);
	}
}

static void intercom_talk_key_home(lv_key_t key)
{
	if (cur_layout_get() != pLAYOUT(home))
	{
		goto_layout(pLAYOUT(home));
	}
}
static void intercom_talk_back_btn_up(lv_key_t key)
{
	if (hung_up_task == NULL)
	{
		intercom_state_set(INTERCOM_STATE_HUNG_UP);
		intercom_talk_hung_up_task_create();
	}
	if (talking_task != NULL)
	{
		lv_task_del(talking_task);
		talking_task = NULL;
	}
}

// 按键绑定表
static const key_binding_t intercom_talk_key_bindings[] = {

	KEY_BIND(LV_KEY_HOME, common_btn_key_down, intercom_talk_key_home),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, intercom_talk_back_btn_up),
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
	KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),
};
static void LAYOUT_ENTER_FUNC(intercom_talk)
{
	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(intercom_talk_key_bindings);
	power_amplifier_enable(true);
	standby_timer_close();
	backlight_enable(true);
	lv_obj_t *parent = common_bg_display(lv_scr_act());

	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_btn_create(bottom_parent);						   /*按键ui显示*/

	intercom_talk_text_icon_create(parent); // 挂断按钮
	intercom_talk_countdown_create(parent); // 倒计时环和标签
	intercom_talk_arrows_create(parent);	// 左右箭头和标签

	intercom_talk_talking_task_create();
	intercom_state_set(INTERCOM_STATE_TALKING);
}

static void LAYOUT_QUIT_FUNC(intercom_talk)
{
	common_obj_null();
	hung_up_task = NULL;
	talking_task = NULL;
	standby_timer_restart(true);
	door_audio_talk(AUDIO_CH_CLOSE);
	user_data_save();
}

CREATE_LAYOUT(intercom_talk);