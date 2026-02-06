/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-18 10:48
 * @LastEditTime   : 2023-03-30 17:05
 *******************************************************************/
#include "layout_define.h"
#include "media_thumb.h"

typedef enum
{

	MEMORY_PLAY_BTN_ID,

	MEMORY_TOTAL_BTN,
} video_btn_module;

static custom_area video_btn_area[MEMORY_TOTAL_BTN] =
	{

		{480, 270, 80, 66},

};
#define MEMORY_FUNC_BTN_BG_BLOCK_ID 8
#define MEMORY_HEAD_CH_LABEL_ID 9
#define MEMORY_HEAD_TIME_LABEL_ID 10
#define MEMORY_HEAD_INDEX_LABEL_ID 11
#define MEMORY_HEAD_PLAY_TIME_LABEL_ID 12
#define MEMORY_HEAD_INDEX_NUM_LABEL_ID 13
#define MEMORY_VIDEO_PROGRESS_BAR_ID 14
// #define MEMORY_VIDEO_TIMEOUT_DURATION 60//显示时长 *
static bool memory_video_timeout_kuaijin = false;
static int memory_video_timeout_val = 60;

static bool delete_button_clicked = false;

static int per_time_count = 0;
static int next_time_count = 0;
static lv_task_t *memory_video_per_wait_task = NULL;
static lv_task_t *memory_video_next_wait_task = NULL;

extern int video_index_get(void);
extern void video_index_set(int index);

static int video_total = 0;
static int video_index = 0;

static int media_total = 0;
static int media_index = 0;
// static const file_info *p_media_info = NULL;

extern void photo_index_reset(void);

static void layout_memory_video_load(void);

// 复位显示倒计时
void memory_video_timeout_value_reset(int num)
{
	memory_video_timeout_val = num;
}

static void memory_video_ticker_task(lv_task_t *task_t)
{
	printf(">>>>>>>>>>>>>>>>>[%d]\n", memory_video_timeout_val);
	if (memory_video_timeout_val-- <= 0)
	{
		goto_layout(pLAYOUT(standby));
	}
	if ((video_play_status_get() == VIDEO_PLAY_STATE_PLAY) || (memory_video_timeout_kuaijin == true))
	{
		lv_task_del(task_t);
	}
}

void video_index_reset(void)
{
	video_index = 0;
}

static void memory_video_per_goto_standby_wait_task(lv_task_t *task_t)
{
	if (memory_video_per_wait_task == NULL)
	{
		lv_task_del(task_t);
	}
	per_time_count++;
	printf("++++++++++++++++++++[%d]\n", per_time_count);
	if (per_time_count == 60)
	{
		lv_task_del(task_t);
		goto_layout(pLAYOUT(standby));
	}
}

static void memory_video_next_goto_standby_wait_task(lv_task_t *task_t)
{
	if (memory_video_next_wait_task == NULL)
	{
		lv_task_del(task_t);
	}
	next_time_count++;
	printf("++++++++++++++++++++[%d]\n", next_time_count);
	if (next_time_count == 60)
	{
		// printf
		goto_layout(pLAYOUT(standby));
	}
}

static void video_head_label_create(lv_obj_t *parent)
{
	lv_obj_t *label1 = lv_label_create(parent, NULL);
	lv_obj_set_id(label1, MEMORY_HEAD_CH_LABEL_ID);
	lv_obj_set_pos(label1, 34, 24);
	lv_label_set_text(label1, " ");
	lv_obj_set_style_local_text_color(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
	lv_obj_set_style_local_text_font(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));

	lv_obj_t *label2 = lv_label_create(parent, label1);
	lv_obj_set_id(label2, MEMORY_HEAD_TIME_LABEL_ID);
	lv_label_set_text(label2, " ");

	lv_obj_t *label3 = lv_label_create(parent, label1);
	lv_obj_set_id(label3, MEMORY_HEAD_INDEX_LABEL_ID);
	lv_label_set_text(label3, " ");

	lv_obj_t *label4 = lv_label_create(parent, NULL);
	lv_obj_set_id(label4, MEMORY_HEAD_PLAY_TIME_LABEL_ID);
	lv_obj_set_pos(label4, 750, 550);
	lv_obj_set_style_local_text_color(label4, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
	lv_obj_set_style_local_text_font(label4, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_label_set_text(label4, "");

	lv_obj_t *label5 = lv_label_create(parent, label1);
	lv_obj_set_id(label5, MEMORY_HEAD_INDEX_NUM_LABEL_ID);
	lv_label_set_text(label5, " ");

	lv_obj_set_pos(label2, 241, 16);
	lv_obj_set_pos(label3, 640, 16);
	lv_obj_set_pos(label5, 520, 16);
}

/**
 * 功能：创建视频播放进度条
 * 参数：parent - 父对象（建议传当前屏幕 lv_scr_act()，与其他UI同级）
 */
static void video_progress_bar_create(lv_obj_t *parent)
{
	// 1. 创建LVGL进度条对象
	lv_obj_t *progress_bar = lv_bar_create(parent, NULL);
	if (progress_bar == NULL)
		return; // 创建失败直接返回

	// 2. 设置进度条ID
	lv_obj_set_id(progress_bar, MEMORY_VIDEO_PROGRESS_BAR_ID);

	// 3. 设置进度条位置和大小
	lv_obj_set_pos(progress_bar, 50, 550);	// x=20），y=580
	lv_obj_set_size(progress_bar, 652, 22); // 宽984px，高8px

	// 4. 设置进度条样式
	// 4.1 进度条背景样式（深灰色，与设置窗口背景色一致）
	lv_obj_set_style_local_bg_color(progress_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xC7C7C7));
	lv_obj_set_style_local_bg_opa(progress_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_100); // 完全不透明

	// 4.2进度条前景样式（白色，与播放时间文字颜色一致）
	lv_obj_set_style_local_bg_color(progress_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x64CAA));
	lv_obj_set_style_local_bg_opa(progress_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_100);

	// 5. 初始进度设为0（视频未播放时）
	lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF); // LV_ANIM_OFF：无动画，立即生效
}

static void video_head_info_label_display(const file_info *pinfo)
{
	lv_obj_t *label1 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_CH_LABEL_ID);
	MON_CH ch = pinfo->ch;
	lv_label_set_text(label1, ch == MON_CH_DOOR1 ? str_get(LAYOUT_HOME_LANG_DOOR1_ID) : ch == MON_CH_DOOR2 ? str_get(LAYOUT_HOME_LANG_DOOR2_ID)
																					: ch == MON_CH_CCTV1   ? str_get(LAYOUT_HOME_LANG_CCTV1_ID)
																										   : str_get(LAYOUT_HOME_LANG_CCTV2_ID));

	char str[5] = {0};

	memset(str, 0, 5);
	memcpy(str, pinfo->file_name, 2);
	int year = atoi(str) + (atoi(str) < 37 ? 2000 : 1900);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[2]), 2);
	int month = atoi(str);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[4]), 2);
	int day = atoi(str);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[7]), 2);
	int hour = atoi(str);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[9]), 2);
	int min = atoi(str);
	// printf("==============>> [%04d-%02d-%02d   %02d-%02d]\n", year, month, day, hour, min);
	lv_obj_t *label2 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_TIME_LABEL_ID);
	if (user_data_get()->setting.calendar == 0)
	{
		struct date temp_date =
			{
				.year = year,
				.month = month,
				.day = day};
		temp_date = gregorian2jalali(temp_date);
		lv_label_set_text_fmt(label2, "%04d-%02d-%02d %02d:%02d", temp_date.year, temp_date.month, temp_date.day, hour, min);
	}
	else
	{
		lv_label_set_text_fmt(label2, "%04d-%02d-%02d %02d:%02d", year, month, day, hour, min);
	}

	lv_obj_t *label3 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_INDEX_LABEL_ID);
	char str1[30] = {0};
	sprintf(str1, "%04d/%04d", video_index + 1, video_total);
	lv_label_set_text_fmt(label3, str1);

	lv_obj_t *label4 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_INDEX_NUM_LABEL_ID);
	char str2[30] = {0};
	sprintf(str2, "%s :", str_get(LAYOUT_MEMORY_LANG_VIDEO_ID));
	lv_label_set_text_fmt(label4, str2);

	lv_obj_set_pos(label2, 294, 24);
	if (user_data_get()->setting.language == LANG_PERSIAN)
	{
		lv_obj_set_pos(label4, 744, 24);
		lv_obj_set_pos(label3, 576, 24);
	}
	else
	{
		lv_obj_set_pos(label3, 744, 24);
		lv_obj_set_pos(label4, 616, 24);
	}
}
static void video_head_play_time_label_display(int play_time, int play_total)
{
	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_PLAY_TIME_LABEL_ID);
	lv_label_set_text_fmt(label, "%02d:%02d / %02d:%02d", play_time / 60, play_time, play_total / 60, play_total);
}

/**
 * 功能：更新视频播放进度条
 * 参数：
 *   play_time - 当前播放时间（单位：秒）
 *   play_total - 视频总时长（单位：秒）
 */
static void video_progress_bar_update(int play_time, int play_total)
{

	if (delete_button_clicked)
	{
		return;
	}

	// 查找进度条对象
	lv_obj_t *progress_bar = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_VIDEO_PROGRESS_BAR_ID);
	if (progress_bar == NULL)
	{
		printf("进度条未找到：MEMORY_VIDEO_PROGRESS_BAR_ID\n");
		return;
	}

	// 处理异常情况
	if (play_total <= 0 || play_time < 0)
	{
		lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF); // 进度设为0
		return;
	}
	if (play_time >= play_total)
	{
		lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF); // 进度设为100%
		return;
	}

	// 3. 计算进度百分比（play_time / play_total × 100）
	int progress = (play_time * 100) / play_total;

	// 4. 更新进度条
	lv_bar_set_value(progress_bar, progress, LV_ANIM_OFF); // LV_ANIM_ON：启用进度变化动画
}

static void video_prev_btn_up(void)
{
	if (video_total <= 1)
		return;

	if (video_index >= 0)
	{
		video_index--;
	}
	if (video_index < 0)
	{
		video_index = video_total - 1;
	}

	if (video_index_get() < 0)
	{
		video_index_set(0);
	}
	if (video_index_get() >= (video_total - 1))
	{
		video_index_set(-1);
	}
	video_index_set(video_index_get() + 1);

	printf("--------------per_video_index_get[%d]\n", video_index_get());
	video_play_stop();
	layout_memory_video_load();
	memory_video_timeout_kuaijin = true;
	memory_video_next_wait_task = NULL;
	if (memory_video_per_wait_task == NULL)
	{
		per_time_count = 0;
		memory_video_per_wait_task = lv_layout_task_create(memory_video_per_goto_standby_wait_task, 1000, LV_TASK_PRIO_LOW, NULL);
	}
}

static void video_play_btn_up(lv_obj_t *obj)
{

	if (video_total <= 0)
		return;
	VIDEO_PLAY_STATUS status = video_play_status_get();
	memory_video_per_wait_task = NULL;
	memory_video_next_wait_task = NULL;
	memory_video_timeout_kuaijin = false;
	if (status == VIDEO_PLAY_STATE_IDLE)
	{
		const file_info *pinfo = media_file_info_get(FILE_TYPE_VIDEO, video_index);
		char file[128] = {0};
		strcpy(file, SD_VIDEO_PATH);
		strcat(file, pinfo->file_name);
		printf("============video_play_index=[%s]\n", file);
		video_play_start(file);
	}
	else
	{

		video_play_pause();
	}

	if (video_play_status_get() == VIDEO_PLAY_STATE_PLAY)
	{
		standby_timer_close();
		lv_obj_del(bottom_select_btn);
		common_bottom_view_btn_create(bottom_parent);
	}
	else
	{
		lv_obj_del(bottom_select_btn);
		common_bottom_view_select_btn_create(bottom_parent);
	}
}
// 创建play按钮
static void video_play_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(video_play_btn_up);
	lv_obj_t *btn = camera_img_btn_create(parent, video_btn_area[MEMORY_PLAY_BTN_ID], NULL, &btn_data, NULL);
	lv_obj_set_id(btn, MEMORY_PLAY_BTN_ID);
}

static void video_next_btn_up(void)
{
	if (video_total <= 1)
		return;

	if (video_index <= (video_total - 1))
	{
		video_index++;
	}
	if (video_index > (video_total - 1))
	{
		video_index = 0;
	}

	video_index_set(video_index_get() - 1);

	if (video_index_get() < 0)
	{
		video_index_set(video_total - 1);
	}
	printf("--------------next_video_index_get[%d]\n", video_index_get());
	video_play_stop();
	layout_memory_video_load();
	memory_video_timeout_kuaijin = true;
	memory_video_per_wait_task = NULL;
	if (memory_video_next_wait_task == NULL)
	{
		next_time_count = 0;
		memory_video_next_wait_task = lv_layout_task_create(memory_video_next_goto_standby_wait_task, 1000, LV_TASK_PRIO_LOW, NULL);
	}
}

static void video_delete_yes_btn_up(lv_obj_t *obj)
{
	media_file_delete(FILE_TYPE_VIDEO, video_index);
	printf("====================[%d]\n", video_index);
	if (video_index == 0)
	{
		video_index_set(0);
	}
	video_index_set(video_index_get() - 1);
	goto_layout(pLAYOUT(memory_video));
}

static void video_delete_no_btn_up(lv_obj_t *obj)
{
	lv_group_remove_all_objs(lv_group_get_default());
	lv_obj_t *msg_box_cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_MEMORY_LANG_DELETE_VIDEO_ID);
	lv_obj_del(msg_box_cont);
	delete_button_clicked = false;
	lv_group_add_obj(lv_group_get_default(), lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PLAY_BTN_ID));
}
// 创建delete按钮
static void memory_video_delete_btn_up(lv_key_t key)
{
	if (video_total == 0)
		return;
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	lv_group_remove_all_objs(lv_group_get_default());
	delete_button_clicked = true;
	VIDEO_PLAY_STATUS state = video_play_status_get();
	if (state == VIDEO_PLAY_STATE_PLAY)
	{
		video_play_pause();
	}
	memory_video_per_wait_task = NULL;
	memory_video_next_wait_task = NULL;
	memory_video_timeout_kuaijin = false;

	static obj_click_data btn_data = obj_click_data_up_create(video_delete_yes_btn_up);
	static obj_click_data btn_data1 = obj_click_data_up_create(video_delete_no_btn_up);
	lv_obj_t *btn = memory_message_box_create(lv_scr_act(), &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_VIDEO_ID);
	lv_obj_set_id(btn, LAYOUT_MEMORY_LANG_DELETE_VIDEO_ID);
}

static void layout_play_state_task(lv_task_t *task_t)
{
	int cur = -1, total = -1;
	static VIDEO_PLAY_STATUS prev_statu = VIDEO_PLAY_STATE_IDLE;
	VIDEO_PLAY_STATUS statu = video_play_status_get();

	video_play_duration_get(&cur, &total); // 获取播放时长,idle狀態無法獲取
	if (cur != total)
	{
		video_head_play_time_label_display(cur / 1000, total / 1000);
		video_progress_bar_update(cur, total);
	}
	else if (total != -1)
	{
		video_head_play_time_label_display(0, total / 1000);
		video_progress_bar_update(0, 0);
	}

	if (prev_statu != statu)
	{
		prev_statu = statu;
		// if(cur == 0)
		// 	return;
		if (prev_statu == VIDEO_PLAY_STATE_PLAY)
		{
		}
		else
		{
			lv_obj_del(bottom_select_btn);
			common_bottom_view_select_btn_create(bottom_parent);
			// video_progress_bar_update(0, 0);
			memory_video_timeout_value_reset(60);
			lv_layout_task_create(memory_video_ticker_task, 1000, LV_TASK_PRIO_MID, NULL);
		}
	}
}

static void layout_memory_video_load(void)
{
	thumb_media_buffer_clear();
	const file_info *pinfo = media_file_info_get(FILE_TYPE_VIDEO, video_index);
	printf("=================>> 视频索引:[%d]   索引:[%d]   文件名:[%s] \n", video_index, media_index, pinfo->file_name);
	char file[128] = {0};
	strcpy(file, SD_VIDEO_PATH);
	strcat(file, pinfo->file_name);
	thumb_media_load(0, 0, 1024, 600, file);
	if (pinfo->is_new == true)
	{
		media_file_new_clear(pinfo->type, video_index);
	}
	int total_duration_ms = video_get_duration_without_play(file);
	video_head_info_label_display(pinfo);
	video_head_play_time_label_display(0, total_duration_ms / 1000);
	video_progress_bar_update(0, 0);
	lv_obj_invalidate(lv_scr_act());
}

static void memory_video_param_init(void)
{
	if (media_sdcard_insert_check() == true)
	{
		sd_media_all_file_total_get(&media_total, NULL);
		printf("====================>> sd卡媒体文件总数:[%d]\n", media_total);
		media_file_total_get(FILE_TYPE_VIDEO, &video_total, NULL);
		printf("====================>> 视频文件总数:[%d]\n", video_total);
	}
	else
	{
		media_total = 0;
		video_total = 0;
	}

	if (video_total == 0)
	{
		goto_layout(pLAYOUT(video_list));
		return;
	}
	else
	{
		lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
		// if(video_index == 0 || video_index > video_total)
		// {
		// 	// printf("===============>>> video_total:[%d]\n", video_total);
		// 	video_index = video_total;
		// }
	}
	printf("--------------[%d]\n", video_index_get());
	if (video_index_get() < 0)
	{
		// video_index_set(0);
		video_index = video_total - 1;
	}
	else
	{
		video_index = video_total - video_index_get() - 1;
	}
	printf("----------video_index----[%d]\n", video_index_get());
	// video_index = video_index_get();
	thumb_media_open();
	layout_memory_video_load();

	lv_layout_task_create(layout_play_state_task, 100, LV_TASK_PRIO_LOW, NULL);
}

static void memory_video_sdcard_state_change_event_cb(void)
{
	photo_index_reset();
	video_index_reset();
	if (media_sdcard_insert_check() == true)
	{
		goto_layout(pLAYOUT(memory_video));
	}
	else
	{
		goto_layout(pLAYOUT(home));
	}
}
static void memory_video_next_key_long_down(lv_key_t key)
{
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	video_next_btn_up();
}
static void memory_video_next_key_down(lv_key_t key)
{
	common_btn_key_down(key);
	memory_video_next_key_long_down(key);
}
static void memory_video_prev_key_long_down(lv_key_t key)
{
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	video_prev_btn_up();
}
static void memory_video_prev_key_down(lv_key_t key)
{
	common_btn_key_down(key);
	memory_video_prev_key_long_down(key);
}
static void memory_video_key_up_esc(lv_key_t key)
{
	if (delete_button_clicked)
	{
		video_delete_no_btn_up(NULL);
	}
	else
	{
		goto_layout(pLAYOUT(video_list));
	}
}
// 按键绑定表
static const key_binding_t memory_video_key_bindings[] = {
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, memory_video_key_up_esc),
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, memory_video_delete_btn_up),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_NEXT, memory_video_next_key_down, memory_video_next_key_long_down),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_PREV, memory_video_prev_key_down, memory_video_prev_key_long_down),
};
static void LAYOUT_ENTER_FUNC(memory_video)
{
	delete_button_clicked = false;
	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(memory_video_key_bindings);
	lv_obj_click_down_callback_register(NULL);
	printf("come in memory_video\n");

	lv_obj_t *parent = lv_scr_act();

	video_head_label_create(parent);
	video_progress_bar_create(parent);

	video_play_btn_create(parent);

	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_view_select_btn_create(bottom_parent);			   /*按键ui显示*/

	lyaout_sd_state_callback_register(memory_video_sdcard_state_change_event_cb);

	memory_video_param_init();
}
static void LAYOUT_QUIT_FUNC(memory_video)
{
	common_obj_null();
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
	video_play_stop();
	/*视频停止播放的时候，关闭功放，按键音会开功放，会听到打开功放的声音*/
	power_amplifier_enable(true);
	if (cur_layout_get() == pLAYOUT(camera))
	{
		backlight_enable(false);
	}

	lv_obj_set_style_local_value_str(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
	standby_timer_restart(true);

	if (cur_layout_get() != pLAYOUT(video_list) && cur_layout_get() != pLAYOUT(memory_video))
	{
		photo_index_reset();
		video_index_reset();

		video_index_set(0);
	}
	thumb_media_close();
}
CREATE_LAYOUT(memory_video);