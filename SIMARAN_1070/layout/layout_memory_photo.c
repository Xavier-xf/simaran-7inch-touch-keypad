/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-11 18:14
 * @LastEditTime   : 2023-03-30 17:10
 *******************************************************************/
/*
 *当前的文件管理机制，是把sd卡所有的 图片 和 视频 放在media文件
 *但是项目需求是能够单独预览 图片 和 视频，所以需要将media里的文件分类（不改动文件管理机制）
 *方法1：预览前根据文件类型去筛选，翻页时再筛选，直接显示（已实现）
 *方法2：预览前根据文件类型去筛选并将文件索引保存到动态内存，翻页时只需从动态内存中拿出索引即可
 */
#include "layout_define.h"
#include "media_thumb.h"

typedef enum
{
	MEDIA_PHOTO_TYPE = 0,
	MEDIA_VIDEO_TYPE
} memory_media_type;

typedef enum
{
	MEMORY_PLAY_BTN_ID,
	MEMORY_TOTAL_BTN,
} photo_btn_module;

static custom_area photo_btn_area[MEMORY_TOTAL_BTN] =
	{
		{480, 270, 80, 66},
};
#define MEMORY_HEAD_CH_LABEL_ID 9
#define MEMORY_HEAD_TIME_LABEL_ID 10
#define MEMORY_HEAD_INDEX_LABEL_ID 11
#define MEMORY_HEAD_INDEX_NUM_LABEL_ID 12

#define MEMORY_PHOTO_TIMEOUT_DURATION 120 // 显示时长 *
static int memory_photo_timeout_val = MEMORY_PHOTO_TIMEOUT_DURATION;
// 静态变量存储遮罩层，供后续删除（确认/取消时）
static lv_obj_t *dim_mask = NULL;
extern int photo_index_get(void);
extern void photo_index_set(int index);
extern void photo_total_set(int index);

static int photo_total = 0;
// static int photo_index = 0;//1 ～ photo_total

static int media_total = 0;
static int media_index = 0; // 0 ～ media_total-1
// static const file_info *p_media_info = NULL;
static file_type photo_file_type = FILE_TYPE_PHOTO;

static bool delete_button_clicked = false;

static char photo_file_path[20] = {0};

extern void video_index_reset(void);

static void layout_memory_photo_load(void);
static void memory_photo_param_init(void);
static void photo_next_btn_up(void);

// 复位显示倒计时
void memory_photo_timeout_value_reset(void)
{
	memory_photo_timeout_val = MEMORY_PHOTO_TIMEOUT_DURATION;
}

static int photo_index_new = 0;

void photo_index_reset(void)
{
	photo_index_new = 0;
}

static void photo_head_label_create(lv_obj_t *parent)
{
	lv_obj_t *label1 = lv_label_create(parent, NULL);
	lv_obj_set_id(label1, MEMORY_HEAD_CH_LABEL_ID);
	lv_obj_set_pos(label1, 34, 24);
	lv_label_set_text(label1, " ");
	lv_obj_set_style_local_text_color(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
	lv_obj_set_style_local_text_font(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_t *label2 = lv_label_create(parent, label1);
	lv_obj_set_id(label2, MEMORY_HEAD_TIME_LABEL_ID);
	lv_label_set_text(label2, " ");

	lv_obj_t *label3 = lv_label_create(parent, label1);
	lv_obj_set_id(label3, MEMORY_HEAD_INDEX_LABEL_ID);
	lv_label_set_text(label3, " ");

	lv_obj_t *label4 = lv_label_create(parent, label1);
	lv_obj_set_id(label4, MEMORY_HEAD_INDEX_NUM_LABEL_ID);
	lv_label_set_text(label4, " ");

	lv_obj_set_pos(label2, 294, 24);
	lv_obj_set_pos(label3, 640, 16);
	lv_obj_set_pos(label4, 520, 16);
}
static void photo_head_label_display(const file_info *pinfo)
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
	sprintf(str1, "%04d/%04d", photo_index_new + 1, photo_total);
	lv_label_set_text(label3, str1);

	lv_obj_t *label4 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_INDEX_NUM_LABEL_ID);
	char str2[30] = {0};
	sprintf(str2, "%s :", str_get(LAYOUT_MEMORY_LANG_IMAGE_ID));
	lv_label_set_text(label4, str2);

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

static void photo_prev_btn_up(void)
{
	if (photo_total <= 1)
		return;

	if (photo_index_new >= 0)
	{
		photo_index_new--;
	}
	if (photo_index_new < 0)
	{
		photo_index_new = photo_total - 1;
	}
	photo_index_set(photo_index_get() + 1);
	if (photo_index_get() >= photo_total)
	{
		photo_index_set(0);
	}
	printf("==============>>>>>>>photo_index_get()[%d]\n", photo_index_get());
	layout_memory_photo_load();
}

static lv_task_t *photo_play_task_t = NULL;

static void memory_photo_ticker_task(lv_task_t *task_t)
{
	if (memory_photo_timeout_val-- <= 0)
	{
		if (dim_mask != NULL)
		{
			lv_obj_del(dim_mask);
			dim_mask = NULL;
		}
		goto_layout(pLAYOUT(standby));
	}
	if (photo_play_task_t != NULL)
	{
		lv_task_del(task_t);
	}
}

static void photo_play_task(lv_task_t *task_t)
{
	photo_next_btn_up();
}

static void photo_play_btn_up(lv_obj_t *obj)
{
	if (photo_total <= 1)
		return;
	if (photo_play_task_t == NULL)
	{
		standby_timer_close();
		photo_play_task_t = lv_layout_task_create(photo_play_task, 3000, LV_TASK_PRIO_LOW, NULL);
		lv_obj_del(bottom_select_btn);
		common_bottom_view_btn_create(bottom_parent);
		memory_photo_timeout_value_reset();
	}
	else
	{
		lv_task_del(photo_play_task_t);
		photo_play_task_t = NULL;
		lv_obj_del(bottom_select_btn);
		common_bottom_view_select_btn_create(bottom_parent);
		memory_photo_timeout_value_reset();
		lv_layout_task_create(memory_photo_ticker_task, 500, LV_TASK_PRIO_HIGH, NULL);
	}
}
// 创建play按钮
static void photo_play_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_play_btn_up);
	camera_img_btn_create(parent, photo_btn_area[MEMORY_PLAY_BTN_ID], NULL, &btn_data, NULL);
}

static void photo_next_btn_up(void)
{
	if (photo_total <= 1)
		return;

	if (photo_index_new <= (photo_total - 1))
	{
		photo_index_new++;
	}
	if (photo_index_new > (photo_total - 1))
	{
		photo_index_new = 0;
	}
	photo_index_set(photo_index_get() - 1);
	if (photo_index_get() < 0)
	{
		photo_index_set(photo_total - 1);
	}

	printf("==============>>>>>>>photo_index_get_next()[%d]\n", photo_index_get());
	layout_memory_photo_load();
}

static void photo_delete_yes_btn_up(lv_obj_t *obj)
{
	if (dim_mask != NULL)
	{
		lv_obj_del(dim_mask);
		dim_mask = NULL;
	}
	media_file_delete(photo_file_type, photo_index_new);
	photo_index_set(photo_index_get() - 1);
	goto_layout(pLAYOUT(memory_photo));
}
static void photo_delete_no_btn_up(lv_obj_t *obj)
{
	if (dim_mask != NULL)
	{
		lv_obj_del(dim_mask);
		dim_mask = NULL;
	}
	lv_obj_t *btn_area = obj->parent;
	lv_obj_t *msg_box_cont = btn_area->parent;
	lv_obj_del(msg_box_cont);
	goto_layout(pLAYOUT(memory_photo));
}
static void create_dim_mask()
{
	// 先删除旧遮罩（防止重复创建）
	if (dim_mask != NULL)
	{
		lv_obj_del(dim_mask);
		dim_mask = NULL;
	}

	// 创建遮罩层，父对象为当前屏幕
	dim_mask = lv_obj_create(lv_scr_act(), NULL);
	// 尺寸覆盖全屏
	lv_obj_set_size(dim_mask, 1024, 600);
	// 对齐方式：全屏覆盖
	lv_obj_align(dim_mask, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	// 背景色：黑色，不透明度 50%（半透明，实现变暗效果）
	lv_obj_set_style_local_bg_color(dim_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(dim_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50); // 50%不透明
	// 禁用遮罩层点击（避免干扰弹窗操作）
	lv_obj_set_click(dim_mask, false);
}
static void memory_photo_delete_btn_up(lv_key_t key)
{
	if (photo_total == 0)
		return;
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	lv_group_remove_all_objs(lv_group_get_default());
	delete_button_clicked = true;
	if (photo_play_task_t != NULL)
	{
		lv_task_del(photo_play_task_t);
		photo_play_task_t = NULL;
		lv_obj_del(bottom_select_btn);
		common_bottom_view_select_btn_create(bottom_parent);
		memory_photo_timeout_value_reset();
		lv_layout_task_create(memory_photo_ticker_task, 500, LV_TASK_PRIO_HIGH, NULL);
	}
	// 创建全屏遮罩层（屏幕变暗）
	create_dim_mask();
	static obj_click_data btn_data = obj_click_data_up_create(photo_delete_yes_btn_up);
	static obj_click_data btn_data1 = obj_click_data_up_create(photo_delete_no_btn_up);
	memory_message_box_create(lv_scr_act(), &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_PICTURE_ID);
}

static void layout_memory_photo_load(void)
{
	thumb_media_buffer_clear();
	const file_info *pinfo = media_file_info_get(photo_file_type, photo_index_new);
	char file[128] = {0};
	strcpy(file, photo_file_path);
	strcat(file, pinfo->file_name);

	printf("=================>> 图片索引:[%d]   索引:[%d]   文件路径:[%s] \n", photo_index_new, media_index, file);

	thumb_media_load(0, 0, 1024, 600, file);
	// printf("=============[][][%d]\n",pinfo->is_new);
	if (pinfo->is_new == true)
	{
		// printf("11111111111111111111111\n");
		media_file_new_clear(pinfo->type, photo_index_new);
	}
	photo_head_label_display(pinfo);
	lv_obj_invalidate(lv_scr_act());
}

static void memory_photo_param_init(void)
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

	if (photo_total == 0) // 没有图片
	{
		goto_layout(pLAYOUT(photo_list));
		return;
	}
	else
	{
		lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	}

	// printf("===========photo_index=[%d]\n",photo_index_get());
	if (photo_index_get() < 0)
	{
		photo_index_set(0);
		photo_index_new = photo_total - 1;
	}
	else
	{
		photo_index_new = photo_total - (photo_index_get()) - 1;
	}
	if (photo_index_new < 0)
	{
		photo_index_new = photo_total - 1;
	}
	printf("===========photo_index=[%d]\n", photo_index_get());
	thumb_media_open();
	layout_memory_photo_load();
}

static void memory_photo_sdcard_state_change_event_cb(void)
{
	photo_index_reset();
	video_index_reset();
	photo_index_set(0);
	goto_layout(pLAYOUT(memory_photo));
}

static void memory_photo_next_key_long_down(lv_key_t key)
{
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	photo_next_btn_up();
}
static void memory_photo_next_key_down(lv_key_t key)
{
	common_btn_key_down(key);
	memory_photo_next_key_long_down(key);
}
static void memory_photo_prev_key_long_down(lv_key_t key)
{
	if (delete_button_clicked)
	{
		printf("Delete button already clicked, ignoring\n");
		return;
	}
	photo_prev_btn_up();
}
static void memory_photo_prev_key_down(lv_key_t key)
{
	common_btn_key_down(key);
	memory_photo_prev_key_long_down(key);
}
static void memory_photo_key_up_esc(lv_key_t key)
{
	if (delete_button_clicked)
	{
		goto_layout(pLAYOUT(memory_photo));
	}
	else
	{
		goto_layout(pLAYOUT(photo_list));
	}
}
// 按键绑定表
static const key_binding_t memory_photo_key_bindings[] = {
	KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
	KEY_BIND(LV_KEY_ESC, common_btn_key_down, memory_photo_key_up_esc),
	KEY_BIND(LV_KEY_HOME, common_btn_key_down, memory_photo_delete_btn_up),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_NEXT, memory_photo_next_key_down, memory_photo_next_key_long_down),
	KEY_BIND_PRESS_LONG_PRESS(LV_KEY_PREV, memory_photo_prev_key_down, memory_photo_prev_key_long_down),
};

static void LAYOUT_ENTER_FUNC(memory_photo)
{
	printf("come in memory_photo\n");

	delete_button_clicked = false;
	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(memory_photo_key_bindings);
	lv_obj_t *parent = lv_scr_act();

	photo_head_label_create(parent);

	photo_play_btn_create(parent); /*播放按钮*/

	bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
	common_bottom_view_select_btn_create(bottom_parent);			   /*按键ui显示*/

	lyaout_sd_state_callback_register(memory_photo_sdcard_state_change_event_cb);

	memory_photo_param_init(); /*初始化*/
}
static void LAYOUT_QUIT_FUNC(memory_photo)
{
	common_obj_null();
	if (photo_play_task_t != NULL)
	{
		lv_task_del(photo_play_task_t);
		photo_play_task_t = NULL;
	}
	lv_obj_set_style_local_value_str(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
	standby_timer_restart(true);

	if (cur_layout_get() != pLAYOUT(memory_photo) && cur_layout_get() != pLAYOUT(photo_list))
	{
		printf("===================1215465465\n");
		photo_index_reset();
		video_index_reset();

		photo_index_set(0);
	}
	thumb_media_close();
}
CREATE_LAYOUT(memory_photo);