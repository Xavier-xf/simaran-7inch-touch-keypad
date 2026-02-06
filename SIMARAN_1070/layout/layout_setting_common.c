#include "layout_define.h"
#include "layout_setting_common.h"
/**
 * @brief 封装的环形进度条创建函数实现
 */
lv_obj_t *lv_arc_loader_create(lv_obj_t *parent,
							   lv_task_cb_t task_cb,
							   uint16_t arc_size,
							   lv_color_t bg_color,
							   lv_color_t indic_color,
							   uint8_t line_width,
							   uint32_t task_period,
							   int32_t align_x,
							   int32_t align_y)
{
	/************************ 1. 校验参数合法性 ************************/
	if (parent == NULL || task_cb == NULL)
	{ // 父对象和任务回调不可为空
		LV_LOG_WARN("lv_arc_loader_create: parent or task_cb is NULL");
		return NULL;
	}

	/************************ 2. 创建环形进度条对象 ************************/
	lv_obj_t *arc = lv_arc_create(parent, NULL);
	if (arc == NULL)
	{ // 异常处理：创建失败返回NULL
		LV_LOG_ERROR("lv_arc_loader_create: arc create failed");
		return NULL;
	}

	/************************ 3. 初始化背景弧样式 ************************/
	static lv_style_t style_bg; // static避免栈溢出，且全局复用（仅初始化一次）
	static bool style_bg_inited = false;
	if (!style_bg_inited)
	{
		lv_style_init(&style_bg);
		style_bg_inited = true;
	}
	// 设置背景弧样式：线宽、颜色
	lv_style_set_line_width(&style_bg, LV_STATE_DEFAULT, line_width);
	lv_style_set_line_color(&style_bg, LV_STATE_DEFAULT, bg_color);
	lv_obj_add_style(arc, LV_ARC_PART_BG, &style_bg); // 应用背景样式
	lv_arc_set_bg_angles(arc, 0, 360);				  // 背景弧：360度完整圆环

	/************************ 4. 初始化指示器弧样式 ************************/
	static lv_style_t style_indic; // static全局复用
	static bool style_indic_inited = false;
	if (!style_indic_inited)
	{
		lv_style_init(&style_indic);
		style_indic_inited = true;
	}
	// 设置指示器弧样式：线宽、颜色
	lv_style_set_line_width(&style_indic, LV_STATE_DEFAULT, line_width);
	lv_style_set_line_color(&style_indic, LV_STATE_DEFAULT, indic_color);
	lv_obj_add_style(arc, LV_ARC_PART_INDIC, &style_indic); // 应用指示器样式
	lv_arc_set_angles(arc, 270, 270 + 30);

	/************************ 5. 设置进度条位置和大小 ************************/
	lv_obj_set_size(arc, arc_size, arc_size); // 设置进度条大小（宽=高）
	// 对齐方式：默认屏幕中心，支持自定义偏移（align_x/align_y）
	lv_obj_align(arc, parent, LV_ALIGN_CENTER, align_x, align_y);

	/************************ 6. 创建进度条更新任务 ************************/
	// 任务周期：默认20ms，回调函数：用户传入的task_cb，用户数据：当前arc对象
	lv_task_t *task = lv_layout_task_create(task_cb, task_period, LV_TASK_PRIO_LOWEST, arc);
	if (task == NULL)
	{ // 任务创建失败：销毁arc，避免内存泄漏
		LV_LOG_ERROR("lv_arc_loader_create: task create failed");
		lv_obj_del(arc);
		return NULL;
	}

	return arc; // 返回创建的环形进度条对象
}

/**
 * @brief 更新环形进度条旋转角度
 * @param arc 进度条对象
 * @return bool 返回true表示已完成，需要销毁进度条
 */
bool arc_loader_update(lv_obj_t *arc, int *start_angle_ptr)
{
	// 计算结束角度（固定30度弧段）
	int16_t end_angle = 40 + *start_angle_ptr;

	// 设置弧角度
	lv_arc_set_angles(arc, *start_angle_ptr, end_angle);

	*start_angle_ptr = (*start_angle_ptr + 10) % 360;

	// 检测是否完成
	if (*start_angle_ptr >= 280)
	{
		return true; // 返回true表示已完成
	}

	return false; // 返回false表示还在旋转中
}
void arc_loader_delete(lv_obj_t *arc)
{
	if (arc == NULL)
		return;

	lv_obj_t *mask_cont = lv_obj_get_parent(arc); // 获取遮罩层容器

	if (arc)
	{
		lv_obj_del(arc); // 销毁进度条
	}

	if (mask_cont)
	{
		lv_obj_del(mask_cont); // 销毁遮罩层
	}
}

/***
** 日期: 2022-04-26 17:08
** 作者: leo.liu
** 函数作用：setting btn创建并返回到主界面
** 返回参数说明：
***/
static void setting_cancel_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(home));
}

/***
** 日期: 2022-04-26 17:09
** 作者: leo.liu
** 函数作用：创建设置页面的返回按钮
** 返回参数说明：
***/
bool setting_cancel_btn_create(void (*obj_click_up_callback)(lv_obj_t *))
{
	lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
	if (obj == NULL)
	{
		printf("create cancel btn failed \n");
		return false;
	}

	lv_obj_set_size(obj, 36, 36);
	lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_LEFT, 34, 24);
	lv_obj_set_ext_click_area(obj, 34, 34, 16, 22); // 扩展点击区域

	static rom_bin_info image = rom_bin_info_get(ROM_UI_SETTING_PREV_PNG);
	lv_obj_set_style_local_pattern_image(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &image);
	lv_obj_set_style_local_pattern_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

	lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

	static obj_click_data click_data = obj_click_data_up_create(NULL);
	if (obj_click_up_callback == NULL)
	{
		click_data.up = setting_cancel_btn_up;
	}
	else
	{
		click_data.up = obj_click_up_callback;
	}
	obj_click_event_listen(obj, &click_data);

	return true;
}

lv_obj_t *setting_back_btn_create(lv_obj_t *parent, void (*back_btn_up)(lv_obj_t *))
{
	static obj_click_data btn_data;
	btn_data.down = NULL;
	btn_data.up = back_btn_up;
	btn_data.anything_func = NULL;
	static custom_area setting_back_btn_area = {876, 442, 110, 110};

	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_pos(btn_obj, setting_back_btn_area.x, setting_back_btn_area.y);
	lv_obj_set_size(btn_obj, setting_back_btn_area.w, setting_back_btn_area.h);
	if (cur_layout_get() == pLAYOUT(time))
	{
		static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_HOME_PNG);
		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_style_local_value_str(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(COMMON_LANG_HOME_ID));
	}
	else
	{
		static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_BACK_PNG);
		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
		lv_obj_set_style_local_value_str(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(COMMON_LANG_RETURN_ID));
	}

	obj_click_event_listen(btn_obj, &btn_data);
	lv_obj_set_ext_click_area(btn_obj, 15, 15, 15, 15);

	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

	lv_obj_set_style_local_value_align(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_value_font(btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_set_style_local_value_blend_mode(btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);

	return btn_obj;
}

lv_obj_t *setting_logo_img_create(lv_obj_t *parent)
{
	lv_obj_t *logo = lv_obj_create(parent, NULL);
	lv_obj_set_pos(logo, 43, 18);
	lv_obj_set_size(logo, 110, 110);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_ENGLISH_LOGO_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_PERSIAN_LOGO_PNG);

	if (user_data_get()->setting.language == LANG_ENGLISH)
	{
		lv_obj_set_style_local_pattern_image(logo, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	}
	else
	{
		lv_obj_set_style_local_pattern_image(logo, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
	}

	return logo;
}
/***
** 日期: 2022-04-28 10:40
** 作者: leo.liu
** 函数作用：创建setting的顶部文本
** 返回参数说明：
***/
lv_obj_t *setting_head_label_create(const char *string)
{
	lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
	if (obj == NULL)
	{
		printf("create head label failed \n");
		return false;
	}

	lv_obj_set_size(obj, LV_HOR_RES_MAX, 75);
	lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 8);

	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_10);
	// lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
	lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
	lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	return obj;
}
/***
** 日期: 2022-04-28 10:40
** 作者: leo.liu
** 函数作用：创建带双向箭头的字符串按钮
** 返回参数说明：
***/
static lv_obj_t *setting_btn_sub_string_with_double_arrows_create(lv_obj_t *parent, int x, int y, int w, int h,
																  const char *main_string, const char *sub_string,
																  unsigned int sub_obj_id,
																  obj_click_data *left_arrow_click_data,
																  obj_click_data *right_arrow_click_data)
{
	lv_obj_t *btn = lv_btn_create(parent == NULL ? lv_scr_act() : parent, NULL);
	if (btn == NULL)
	{
		printf("create setting right btn failed \n");
		return NULL;
	}
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	// -------------------------- 焦点状态样式 --------------------------
	// lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
	// lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
	// lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_40);
	// lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));

	// 添加到默认组
	lv_group_add_obj(lv_group_get_default(), btn);
	if (sub_obj_id != 0)
	{
		lv_obj_set_id(btn, sub_obj_id);
	}
	if (main_string != NULL)
	{
		lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, main_string);
		lv_obj_set_style_local_value_font(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_value_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
		lv_obj_set_style_local_value_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
		lv_obj_set_style_local_value_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x66, 0x66, 0x66));
		lv_obj_set_style_local_value_color(btn, LV_SWITCH_PART_BG, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));
	}

	// 创建子字符串显示区域
	if (sub_string != NULL)
	{
		lv_obj_t *sub_obj = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
		if (sub_obj == NULL)
		{
			printf("create setting sub string fail \n");
			return btn;
		}
		lv_obj_set_pos(sub_obj, x + w - 175, y); // 留出空间给两个箭头
		lv_obj_set_size(sub_obj, 120, h);		 // 固定宽度显示子字符串
		lv_obj_set_click(sub_obj, false);

		lv_obj_set_style_local_value_font(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_value_align(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
		lv_obj_set_style_local_value_color(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x97, 0x97, 0x97));
		lv_obj_set_style_local_value_color(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x33, 0x33, 0x33));
		lv_obj_set_style_local_value_color(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));

		lv_obj_set_style_local_value_str(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, sub_string);

		if (sub_obj_id != 0)
		{
			lv_obj_set_id(sub_obj, sub_obj_id + 100);
		}
	}

	// 创建左箭头按钮
	if (left_arrow_click_data != NULL)
	{
		static rom_bin_info left_arrow_img = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
		lv_obj_t *left_arrow_btn = lv_btn_create(parent == NULL ? lv_scr_act() : parent, NULL);
		lv_obj_set_pos(left_arrow_btn, x + w - 240, y + (h - 28) / 2); // 左侧箭头位置
		lv_obj_set_size(left_arrow_btn, 35, 30);
		lv_obj_set_style_local_bg_opa(left_arrow_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_pattern_image(left_arrow_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &left_arrow_img);
		lv_obj_set_style_local_pattern_align(left_arrow_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

		obj_click_event_listen(left_arrow_btn, left_arrow_click_data);
	}

	// 创建右箭头按钮
	if (right_arrow_click_data != NULL)
	{
		static rom_bin_info right_arrow_img = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
		lv_obj_t *right_arrow_btn = lv_btn_create(parent == NULL ? lv_scr_act() : parent, NULL);
		lv_obj_set_pos(right_arrow_btn, x + w - 26, y + (h - 28) / 2); // 右侧箭头位置
		lv_obj_set_size(right_arrow_btn, 35, 30);
		lv_obj_set_style_local_bg_opa(right_arrow_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_pattern_image(right_arrow_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &right_arrow_img);
		lv_obj_set_style_local_pattern_align(right_arrow_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

		obj_click_event_listen(right_arrow_btn, right_arrow_click_data);
	}

	/***** 设置下底边框 *****/
	lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
	lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);

	return btn;
}
/***
** 日期: 2022-05-05 14:12
** 作者: leo.liu
** 函数作用：设置右边字符串的按钮
** 返回参数说明：
***/
static lv_obj_t *setting_btn_sub_string_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, const char *sub_string, unsigned int sub_obj_id, bool right_icon)
{
	lv_obj_t *btn = lv_btn_create(parent == NULL ? lv_scr_act() : parent, NULL);
	if (btn == NULL)
	{
		printf("create setting right btn failed \n");
		return NULL;
	}
	if (sub_obj_id != 0)
	{
		lv_obj_set_id(btn, sub_obj_id);
	}
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	// -------------------------- 焦点状态样式 --------------------------
	// lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
	// lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
	// lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_40);
	// lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));
	if (main_string != NULL)
	{
		lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, main_string);
		lv_obj_set_style_local_value_font(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_value_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
		lv_obj_set_style_local_value_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
		lv_obj_set_style_local_value_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x66, 0x66, 0x66));
		lv_obj_set_style_local_value_color(btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));
	}

	if (sub_string != NULL)
	{
		lv_obj_t *sub_obj = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
		if (sub_obj == NULL)
		{
			printf("create setting sub string fail \n");
			return btn;
		}
		lv_obj_set_pos(sub_obj, x, y);
		lv_obj_set_size(sub_obj, w, h);
		lv_obj_set_click(sub_obj, false);

		lv_obj_set_style_local_value_font(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_value_align(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
		if (right_icon)
		{

			lv_obj_set_style_local_value_ofs_x(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -40);
		}
		else
		{
			lv_obj_set_style_local_value_ofs_x(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -22);
		}

		lv_obj_set_style_local_value_color(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x97, 0x97, 0x97));
		lv_obj_set_style_local_value_color(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x0566E7));
		lv_obj_set_style_local_value_color(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));

		lv_obj_set_style_local_value_str(sub_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, sub_string);

		if (sub_obj_id != 0)
		{
			lv_obj_set_id(sub_obj, sub_obj_id + 100);
		}
	}

	if (right_icon)
	{
		// 添加到默认组
		lv_group_add_obj(lv_group_get_default(), btn);
		static rom_bin_info img = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img);
		lv_obj_set_style_local_pattern_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
	}

	/***** 设置下底边框 *****/
	lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
	lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	return btn;
}

/***
** 日期: 2022-05-05 14:12
** 作者: leo.liu
** 函数作用：设置左边为checkbox图标
** 返回参数说明：
***/
static lv_obj_t *setting_btn_sub_checkbox_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, bool focus, char sub_type_show)
{
	lv_obj_t *btn = lv_checkbox_create(parent == NULL ? lv_scr_act() : parent, NULL);
	if (btn == NULL)
	{
		printf("create setting right btn failed \n");
		return NULL;
	}
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);

	lv_obj_set_ext_click_area(btn, 0, w - 38, (h - 38) / 2, (h - 38) / 2);
	lv_obj_set_style_local_bg_opa(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
	if (sub_type_show == 2)
	{
		static rom_bin_info image_off_d = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_RED_PNG);
		static rom_bin_info image_on_d = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_RED_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, &image_off_d);
		lv_obj_set_style_local_pattern_image(btn, LV_CHECKBOX_PART_BG, LV_STATE_CHECKED, &image_on_d);
	}
	else if (sub_type_show == 4)
	{
		static rom_bin_info image_off_d = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_RED_PNG);
		static rom_bin_info image_on_d = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_RED_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, &image_off_d);
		lv_obj_set_style_local_pattern_image(btn, LV_CHECKBOX_PART_BG, LV_STATE_CHECKED, &image_on_d);
	}

	lv_obj_set_style_local_pattern_align(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);

	lv_checkbox_set_text(btn, "");
	if (main_string != NULL)
	{
		lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, main_string);
		lv_obj_set_style_local_value_font(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(31));
		lv_obj_set_style_local_value_align(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
		lv_obj_set_style_local_value_color(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
		lv_obj_set_style_local_value_color(btn, LV_CHECKBOX_PART_BG, LV_STATE_PRESSED, lv_color_make(0x66, 0x66, 0x66));
		lv_obj_set_style_local_value_ofs_x(btn, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, 70);
	}

	lv_checkbox_set_checked(btn, focus);

	lv_obj_t *obj = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
	lv_obj_set_pos(obj, x, y + h - 2);
	lv_obj_set_size(obj, w, 2);
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
	lv_obj_set_click(obj, false);
	return btn;
}

/***
** 日期: 2022-05-05 14:31
** 作者: leo.liu
** 函数作用：创建开关的按钮
** 返回参数说明：
***/
static lv_obj_t *setting_btn_sub_switch_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, bool focus)
{
	lv_obj_t *obj = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
	lv_obj_set_pos(obj, x - 10, y);
	lv_obj_set_size(obj, w + 10, h - 10);
	lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 28);
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));		   /* 灰黑 */
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27)); /* 灰黑 */

	lv_obj_t *lv_switch = lv_switch_create(parent == NULL ? lv_scr_act() : parent, NULL);
	if (lv_switch == NULL)
	{
		printf("create setting right lv_switch failed \n");
		return NULL;
	}
	lv_obj_set_pos(lv_switch, x + w - 95, y + (h - 54) / 2);
	lv_obj_set_size(lv_switch, 80, 40);
	// 添加到焦点组并设置点击事件
	lv_group_add_obj(lv_group_get_default(), lv_switch);
	lv_obj_set_ext_click_area(lv_switch, w - 95, 0, (h - 54) / 2, (h - 54) / 2);
	lv_obj_set_style_local_bg_opa(lv_switch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_set_style_local_bg_color(lv_switch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x808080));
	lv_obj_set_style_local_bg_color(lv_switch, LV_SWITCH_PART_INDIC, LV_STATE_CHECKED, lv_color_hex(0X0566E7));

	// -------------------------- 焦点状态样式 --------------------------
	lv_obj_set_style_local_border_color(lv_switch, LV_SWITCH_PART_BG, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
	lv_obj_set_style_local_border_width(lv_switch, LV_SWITCH_PART_BG, LV_STATE_FOCUSED, 2);
	// lv_obj_set_style_local_bg_opa(lv_switch, LV_SWITCH_PART_BG, LV_STATE_FOCUSED, LV_OPA_40);
	// lv_obj_set_style_local_bg_color(lv_switch, LV_SWITCH_PART_BG, LV_STATE_FOCUSED, lv_color_hex(0x000000));

	lv_obj_set_style_local_value_color(lv_switch, LV_SWITCH_PART_BG, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));
	if (main_string != NULL)
	{
		lv_obj_set_style_local_value_str(lv_switch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, main_string);
		lv_obj_set_style_local_value_font(lv_switch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_value_align(lv_switch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
		lv_obj_set_style_local_value_ofs_x(lv_switch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, -w + 95);
		lv_obj_set_style_local_value_color(lv_switch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF)); /* 白色 */
		lv_obj_set_style_local_value_color(lv_switch, LV_SWITCH_PART_BG, LV_STATE_PRESSED, lv_color_make(0x66, 0x66, 0x66)); /* 灰色 */
	}

	if (focus)
	{
		lv_switch_on(lv_switch, false);
	}
	else
	{
		lv_switch_off(lv_switch, false);
	}

	return lv_switch;
}

/***
** 日期: 2022-05-05 13:53
** 作者: leo.liu
** 函数作用：setting btn 基础函数
** 返回参数说明：
	sub_type_show:
	0,不显示，
	1:显示字符串，并且根据click_data显示right icon
	2:显示checkbox(单选)
	3:显示switch
	4:多选
***/
static lv_obj_t *setting_btn_base_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, obj_click_data *click_data,
										 char sub_type_show, unsigned int sub_obj_id, const char *sub_string, bool fouces, obj_click_data *click_data2)
{
	lv_obj_t *btn = NULL;
	if (sub_type_show == 1)
	{
		btn = setting_btn_sub_string_create(parent, x, y, w, h, main_string, sub_string, sub_obj_id, click_data ? true : false);
	}
	else if ((sub_type_show == 2) || (sub_type_show == 4))
	{
		btn = setting_btn_sub_checkbox_create(parent, x, y, w, h, main_string, fouces, sub_type_show);
	}
	else if (sub_type_show == 3)
	{
		btn = setting_btn_sub_switch_create(parent, x, y, w, h, main_string, fouces);
	}
	else if (sub_type_show == 5) // 新增双向箭头类型
	{

		btn = setting_btn_sub_string_with_double_arrows_create(parent, x, y, w, h, main_string, sub_string, sub_obj_id, click_data, click_data2);
	}
	if (btn == NULL)
	{
		printf("create setting_btn_base_create failed \n");
		return NULL;
	}

	if (click_data != NULL && sub_type_show != 5)
	{
		obj_click_event_listen(btn, click_data);
	}
	else
	{
		lv_obj_set_click(btn, false);
	}

	return btn;
}
/***
** 日期: 2022-04-28 11:59
** 作者: leo.liu
** 函数作用：创建带双向箭头的设置按钮
** 返回参数说明：
***/
lv_obj_t *setting_double_arrow_btn_create(lv_obj_t *parent, int x, int y, int w, int h,
										  const char *main_string, const char *sub_string,
										  obj_click_data *left_arrow_click_data,
										  obj_click_data *right_arrow_click_data, unsigned int sub_obj_id)
{
	return setting_btn_base_create(parent, x, y, w, h, main_string, right_arrow_click_data, 5, sub_obj_id, sub_string, false, left_arrow_click_data);
}
/***
** 日期: 2022-04-28 11:59
** 作者: leo.liu
** 函数作用：创建右边按钮基础函数
** 返回参数说明：
***/
lv_obj_t *setting_right_btn_base_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, const char *sub_string, obj_click_data *click_data, unsigned int sub_obj_id)
{
	return setting_btn_base_create(parent, x, y, w, h, main_string, click_data, 1, sub_obj_id, sub_string, 0, NULL);
}
/***
**   日期:2022-07-04 17:00:03
**   作者: leo.liu
**   函数作用：销毁消息框
**   参数说明:
***/
bool setting_adjust_msgdialog_msg_bg_delete(int id)
{
	lv_obj_t *dialog = lv_obj_get_child_form_id(lv_scr_act(), id);
	if (dialog == NULL)
	{
		return false;
	}
	lv_obj_del(dialog);

	dialog = lv_obj_get_child_form_id(lv_scr_act(), id + 1);
	if (dialog == NULL)
	{
		return false;
	}
	lv_obj_del(dialog);

	return true;
}
/***
**   日期:2022-07-04 17:00:03
**   作者: leo.liu
**   函数作用：销毁消息框
**   参数说明:
***/
bool setting_msgdialog_msg_bg_delete(int id)
{

	lv_obj_t *bg_container = lv_obj_get_child_form_id(lv_scr_act(), id + 1);
	if (bg_container == NULL)
	{
		printf("未找到背景容器 (ID: %d)\n", id + 1);
		return false;
	}

	lv_obj_t *msg_container = lv_obj_get_child_form_id(bg_container, id);
	if (msg_container == NULL)
	{
		printf("未找到消息框容器 (ID: %d)\n", id);
		// 但仍然删除背景容器
		lv_obj_del(bg_container);
		return true;
	}

	lv_obj_del(bg_container);

	return true;
}
/***
** 日期: 2022-04-29 15:56
** 作者: leo.liu
** 函数作用：消息框的背景创建
** 返回参数说明：
***/
lv_obj_t *setting_adjust_msgdialog_msg_bg_create(int id)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	if (cont == NULL)
	{
		printf("create auto record cont failed \n");
		return NULL;
	}
	if (id > 0)
	{
		lv_obj_set_id(cont, id + 1);
	}
	lv_obj_set_click(cont, true);
	lv_obj_set_pos(cont, 0, 0);
	lv_obj_set_size(cont, LV_HOR_RES_MAX, LV_VER_RES_MAX); // 一整个屏幕大小
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

	lv_obj_t *msgcont = lv_cont_create(lv_scr_act(), NULL);
	if (id > 0)
	{
		lv_obj_set_id(msgcont, id);
	}
	// lv_obj_set_pos(cont, 352, 224);
	lv_obj_set_size(msgcont, LV_HOR_RES_MAX / 2, LV_VER_RES_MAX / 2);
	lv_obj_align(msgcont, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(msgcont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x303030));
	lv_obj_set_style_local_bg_opa(msgcont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	/* 边框 */
	lv_obj_set_style_local_border_width(msgcont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(msgcont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5E5E5E));
	lv_obj_set_style_local_border_side(msgcont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);

	return msgcont;
}

/***
** 日期: 2022-04-29 15:56
** 作者: leo.liu
** 函数作用：消息框的背景创建（带亮度变暗效果）
** 返回参数说明：
***/
lv_obj_t *setting_msgdialog_msg_bg_create(int id, int x, int y)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	if (cont == NULL)
	{
		printf("create auto record cont failed \n");
		return NULL;
	}
	if (id > 0)
	{
		lv_obj_set_id(cont, id + 1);
	}
	lv_obj_set_click(cont, true);
	lv_obj_set_pos(cont, 0, 0);
	lv_obj_set_size(cont, LV_HOR_RES_MAX, LV_VER_RES_MAX); // 一整个屏幕大小

	// 修改：将背景设置为半透明黑色（实现亮度变暗一半的效果）
	// LV_OPA_50 表示50%不透明度，即半透明
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色背景
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);				// 50%透明度

	lv_obj_t *msgcont = lv_cont_create(cont, NULL); // 修改：将msgcont创建在cont内部，确保在遮罩层上方
	if (id > 0)
	{
		lv_obj_set_id(msgcont, id);
	}
	lv_obj_set_size(msgcont, x, y);
	lv_obj_align(msgcont, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(msgcont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x303030));
	lv_obj_set_style_local_bg_opa(msgcont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

	return msgcont;
}

/***
** 日期: 2022-04-28 16:26
** 作者: leo.liu
** 函数作用：创建消息框的关闭和确认函数创建基础函数
** 返回参数说明：
***/
static bool setting_msgdialog_confirm_cancel_btn_create(lv_obj_t *parent, int x, int y, int w, int h, obj_click_data *click_data, rom_bin_info *image, lv_color_t bg_color, lv_color_t press_color, lv_align_t align)
{
	lv_obj_t *obj = lv_btn_create(parent, NULL);
	if (obj == NULL)
	{
		printf("create msgdialog btn failed \n");
		return false;
	}

	lv_obj_set_pos(obj, x, y);
	lv_obj_set_size(obj, w, h);
	lv_obj_align(obj, parent, align, 0, 0);

	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bg_color);
	lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_PRESSED, press_color);

	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_100);
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));

	if (image != NULL)
	{
		lv_obj_set_style_local_pattern_image(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, image);
		lv_obj_set_style_local_pattern_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	}

	if (click_data != NULL)
	{ // 添加到默认组
		lv_group_add_obj(lv_group_get_default(), obj);
		obj_click_event_listen(obj, click_data);
	}

	return true;
}
/***
** 日期: 2022-04-28 17:20
** 作者: leo.liu
** 函数作用：消息框按键矩阵按下回调函数
** 返回参数说明：
***/
static void setting_msgdialog_btnmatrix_up(lv_obj_t *obj)
{
	uint16_t btn_id = lv_btnmatrix_get_active_btn(obj);
	if (btn_id == LV_BTNMATRIX_BTN_NONE)
		return;

	if (lv_btnmatrix_get_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_DISABLED))
		return;

	lv_btnmatrix_clear_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECK_STATE);
	lv_btnmatrix_set_btn_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
}
/***
 ** 日期: 2022-04-28 15:32
 ** 作者: leo.liu
 ** 函数作用：创建按键矩阵
 ** 返回参数说明：
 ***/
lv_obj_t *setting_msgdialog_btnmatrix_create(lv_obj_t *parent, int obj_id, const char *string_map[])
{
	lv_obj_t *btnmatrix = lv_btnmatrix_create(parent, NULL);
	if (btnmatrix == NULL)
	{
		printf("auto recrod create btnmatrix failed \n");
		return NULL;
	}
	// 将 btnmatrix 添加到默认焦点组
	lv_group_add_obj(lv_group_get_default(), btnmatrix);
	lv_group_set_editing(lv_group_get_default(), true);
	lv_obj_set_id(btnmatrix, obj_id);
	lv_obj_set_size(btnmatrix, lv_obj_get_width(parent) - 72, lv_obj_get_height(parent) - 72);
	lv_obj_align(btnmatrix, parent, LV_ALIGN_IN_TOP_MID, 0, 0);
	lv_obj_set_style_local_text_font(btnmatrix, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));

	static rom_bin_info image_no = rom_bin_info_get(ROM_UI_SETTING_CIRCLE_OPTION_NO_PNG);
	lv_obj_set_style_local_pattern_image(btnmatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &image_no);
	lv_obj_set_style_local_pattern_align(btnmatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);

	static rom_bin_info image_ck = rom_bin_info_get(ROM_UI_SETTING_CIRCLE_OPTION_CK_PNG);
	lv_obj_set_style_local_pattern_image(btnmatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, &image_ck);
	lv_obj_set_style_local_pattern_align(btnmatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_ALIGN_IN_LEFT_MID);

	lv_obj_set_style_local_bg_opa(btnmatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_FOCUSED, LV_OPA_100);
	lv_obj_set_style_local_bg_color(btnmatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));

	lv_btnmatrix_set_map(btnmatrix, string_map);
	lv_btnmatrix_set_align(btnmatrix, LV_LABEL_ALIGN_LEFT, 58, 0);

	static obj_click_data click_data = obj_click_data_up_create(setting_msgdialog_btnmatrix_up);
	obj_click_event_listen(btnmatrix, &click_data);

	lv_btnmatrix_clear_btn_ctrl_all(btnmatrix, LV_BTNMATRIX_CTRL_CHECK_STATE);

	return btnmatrix;
}
/***
** 日期: 2022-05-04 09:10
** 作者: leo.liu
** 函数作用：消息框的确认按钮创建
** 返回参数说明：
***/
void setting_msgdialog_msg_confirm_and_cancel_btn_create(lv_obj_t *parent, void (*cancel_func)(lv_obj_t *obj), void (*confirm_func)(lv_obj_t *obj))
{

	/* 确认按钮 */
	static rom_bin_info image_confirm = rom_bin_info_get(ROM_UI_SECURITY_02_BTN_BTN_BOTTOM_CONFIRM_PNG);
	static obj_click_data click_data_confirm = {NULL, NULL, NULL};
	click_data_confirm.up = confirm_func;
	setting_msgdialog_confirm_cancel_btn_create(parent, 0, 274, lv_obj_get_width(parent) / 2, 72, &click_data_confirm, &image_confirm, lv_color_make(0x40, 0x40, 0x40), lv_color_make(0x00, 0x6E, 0xBE), LV_ALIGN_IN_BOTTOM_LEFT);

	/* 取消按钮 */
	static rom_bin_info image_cancel = rom_bin_info_get(ROM_UI_SECURITY_02_BTN_BTN_BOTTOM_CANCEL_PNG);
	static obj_click_data click_data_cancel = {NULL, NULL, NULL};
	click_data_cancel.up = cancel_func;
	setting_msgdialog_confirm_cancel_btn_create(parent, 288, 274, lv_obj_get_width(parent) / 2, 72, &click_data_cancel, &image_cancel, lv_color_make(0x40, 0x40, 0x40), lv_color_make(0x47, 0x49, 0x4a), LV_ALIGN_IN_BOTTOM_RIGHT);
}
/***
** 日期: 2022-05-04 09:10
** 作者: leo.liu
** 函数作用：消息框的确认按钮创建
** 返回参数说明：
***/
void setting_msgdialog_msg_confirm_btn_create(lv_obj_t *parent, void (*confirm_func)(lv_obj_t *obj))
{
	static rom_bin_info image_confirm = rom_bin_info_get(ROM_UI_SECURITY_02_BTN_BTN_BOTTOM_CONFIRM_PNG);
	static obj_click_data click_data_confirm = {NULL, NULL, NULL};
	click_data_confirm.up = confirm_func;
	setting_msgdialog_confirm_cancel_btn_create(parent, 0, 274, lv_obj_get_width(parent), 72, &click_data_confirm, &image_confirm, lv_color_hex(0X0081DC), lv_color_make(0xB8, 0x8D, 0x56), LV_ALIGN_IN_BOTTOM_MID);
}

/*************************************************************************
 * @brief  文本域
 * @date   2022-10-21 09:45
 * @author xiaoele
 **************************************************************************/
void setting_msgdialog_msg_create(lv_obj_t *parent, const char *msg_string)
{
	lv_obj_t *text = lv_textarea_create(parent, NULL);
	lv_obj_set_width(text, lv_obj_get_width(parent) - 16);

	if (text == NULL)
	{
		printf("obj is null!\n");
		return;
	}

	if (msg_string == NULL)
	{
		printf("str is null\n");
		return;
	}

	lv_textarea_set_text(text, msg_string);
	lv_textarea_set_cursor_hidden(text, true);
	lv_textarea_set_text_align(text, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(text, parent, LV_ALIGN_CENTER, 0, -75 / 2);

	// lv_obj_set_style_local_value_font(text, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(40));
	lv_obj_set_style_local_text_font(text, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
}
/***
** 日期: 2022-04-28 11:59
** 作者: leo.liu
** 函数作用：创建设置图标
** 返回参数说明：focus
	0,不显示，
	1:显示字符串，并且根据click_data显示right icon
	2:显示checkbox
	3:显示switch
	4:多选框
***/
lv_obj_t *setting_sub_btn_base_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, obj_click_data *click_data, bool focus, int type)
{
	return setting_btn_base_create(parent, x, y, w, h, main_string, click_data, type, 0, NULL, focus, NULL);
}

// static void setting_time_roller_base_change(lv_obj_t *obj, lv_event_t ev)
// {
// 	printf("setting_time_roller_base_change\n");
// 	// lv_group_focus_freeze(lv_group_get_default(), true);
// 	// lv_group_set_editing(lv_group_get_default(), true);
// 	if (ev == LV_EVENT_VALUE_CHANGED)
// 	{
// 		if (obj->user_data != NULL)
// 		{
// 			bool *pmodiy = (bool *)(obj->user_data);
// 			if ((*pmodiy) == false)
// 			{
// 				*pmodiy = true;
// 			}
// 		}
// 	}
// }

/***
** 日期: 2022-05-06 10:22
** 作者: leo.liu
** 函数作用：滚动基础函数
** 返回参数说明：
***/
lv_obj_t *setting_time_roller_base(lv_obj_t *parent, int x, int y, int w, int h, int min, int max, int cur, obj_click_data *click_data, int obj_id)
{
	/* 图标 */
	lv_obj_t *obj = lv_obj_create(parent, NULL);
	lv_obj_set_click(obj, false);
	lv_obj_set_pos(obj, x, y);
	lv_obj_set_size(obj, w, h);

	static rom_bin_info img = rom_bin_info_get(ROM_UI_SETTING_ROLLER_ICON_PNG);
	lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img);

	/* 滚筒 */
	lv_obj_t *roller = lv_roller_create(parent, NULL);
	lv_group_add_obj(lv_group_get_default(), roller);
	lv_obj_set_id(roller, obj_id);
	lv_obj_set_style_local_text_line_space(roller, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 30);
	// lv_obj_set_style_local_text_color(roller, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x80, 0x80, 0x80));

	// lv_obj_set_style_local_bg_opa(roller, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_set_style_local_bg_opa(roller, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_bg_color(roller, LV_ROLLER_PART_SELECTED, LV_STATE_FOCUSED, lv_color_hex(0X0566E7));
	lv_obj_set_style_local_text_color(roller, LV_ROLLER_PART_SELECTED, LV_STATE_FOCUSED, lv_color_hex(0x0566E7));
	// lv_obj_set_style_local_bg_color(roller, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x80, 0x80, 0x80));
	// lv_obj_set_style_local_bg_color(roller, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, lv_color_make(0x50, 0x50, 0x99));
	// lv_obj_set_style_local_text_color(roller, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, lv_color_make(0x00, 0x96, 0xFF));

	char opt[512] = {0};
	for (int i = max; i >= min; i--)
	{
		char buf[8] = {0};
		sprintf(buf, "%02d%s", i, i == min ? "" : "\n");
		strcat(opt, buf);
	}

	lv_roller_set_options(roller, opt, LV_ROLLER_MODE_INFINITE);
	lv_roller_set_visible_row_count(roller, 1);
	lv_obj_set_width(roller, w + 24);
	lv_obj_align(roller, obj, LV_ALIGN_CENTER, 0, 0);

	lv_obj_set_ext_click_area(roller, 0, 0, 54, 54);

	lv_roller_set_selected(roller, max - cur, false);
	// static obj_click_data click_data = obj_click_data_anything_create(setting_time_roller_base_change);
	roller->user_data = parent->user_data;
	obj_click_event_listen(roller, click_data);

	return roller;
}

/***
** 日期: 2022-05-09 11:31
** 作者: leo.liu
** 函数作用：复位输入框
** 返回参数说明：
***/
bool setting_password_input_reset(lv_obj_t *parent, int edit_max)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}
	(*((int *)label->user_data)) = 0;

	for (int i = 0; i < edit_max; i++)
	{
		lv_obj_t *obj = lv_obj_get_child_form_id(parent, 10 + i);
		if (obj != NULL)
		{
			lv_obj_set_hidden(obj, true);
		}
		obj = lv_obj_get_child_form_id(parent, 20 + i);
		if (obj != NULL)
		{
			lv_obj_set_hidden(obj, true);
		}

		obj = lv_obj_get_child_form_id(parent, 30 + i);
		if (obj != NULL)
		{
			lv_obj_set_state(obj, LV_STATE_DEFAULT);
		}
	}
	return true;
}
/***
** 日期: 2022-05-07 13:52
** 作者: leo.liu
** 函数作用：数字密码输入键盘
** 返回参数说明：
***/
lv_obj_t *setting_passowrd_num_keyboard_create(lv_obj_t *parent, int x, int y, int w, int h, obj_click_data *click_data)
{
	lv_obj_t *keyboard = lv_keyboard_create(parent == NULL ? lv_scr_act() : parent, NULL);
	if (keyboard == NULL)
	{
		printf("create keyboard failed \n");
		return NULL;
	}
	lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUM);
	lv_obj_set_pos(keyboard, x, y);
	lv_obj_set_size(keyboard, w, h);
	// lv_obj_align(keyboard, NULL, LV_ALIGN_IN_LEFT_MID, x, 0);

	lv_obj_set_style_local_text_font(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, FONT_SIZE(40));
	lv_obj_set_style_local_bg_color(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_color(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED, lv_color_hex(0x0096ff));

	lv_obj_set_style_local_border_opa(keyboard, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_bg_opa(keyboard, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_bg_opa(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, LV_OPA_10);

	lv_obj_set_style_local_radius(keyboard, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, 60);
	lv_obj_set_style_local_pad_inner(keyboard, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 24);

	/* 删除键 [del] */
	static rom_bin_info img = rom_bin_info_get(ROM_UI_SETTING_BTN_EXIT_PNG);
	lv_btnmatrix_set_pattern_image(keyboard, 10, &img);

	obj_click_event_listen(keyboard, click_data);

	return keyboard;
}

/***
** 日期: 2022-05-09 10:34
** 作者: leo.liu
** 函数作用：延时隐藏密码
** 返回参数说明：
***/
static void setting_password_input_string_task(lv_task_t *task_t)
{
	lv_obj_t *edit_label = (lv_obj_t *)(task_t->user_data);
	if (edit_label == NULL)
	{
		return;
	}

	int edit_index = *((int *)edit_label->user_data);
	int cur_index = (edit_label->obj_id) % 10;
	if (cur_index >= edit_index)
	{
		return;
	}

	lv_obj_set_hidden(edit_label, true);

	int id = 20 + cur_index;
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_obj_get_parent(edit_label), id);
	if (obj == NULL)
	{
		return;
	}
	lv_obj_set_hidden(obj, false);

	lv_task_del(task_t);
}
/***
** 日期: 2022-05-09 10:53
** 作者: leo.liu
** 函数作用：设置下直线的选中线条
** 返回参数说明：
***/
static void setting_password_line_set_cheked(lv_obj_t *parent, int cur_id, int max_edit)
{
	for (int i = 0; i < max_edit; i++)
	{
		int id = 30 + i;
		lv_obj_t *obj = lv_obj_get_child_form_id(parent, id);
		lv_obj_set_state(obj, id == cur_id ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
	}
}
/***
** 日期: 2022-05-09 10:26
** 作者: leo.liu
** 函数作用：输入编辑的数值
** 返回参数说明：
***/
bool setting_password_input_string(lv_obj_t *parent, const char *string, int max_edit)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}

	int edit_index = *((int *)label->user_data);
	if (edit_index >= max_edit)
	{
		return false;
	}
	int id = 10 + edit_index;
	lv_obj_t *edit_label = lv_obj_get_child_form_id(parent, id);
	if (edit_label == NULL)
	{
		printf("dind id:%d failed \n", id);
		return false;
	}

	setting_password_line_set_cheked(parent, 30 + edit_index, max_edit);

	lv_obj_set_hidden(edit_label, false);
	lv_label_set_text(edit_label, string);
	(*((int *)label->user_data))++;

	lv_layout_task_create(setting_password_input_string_task, 500, LV_TASK_PRIO_MID, edit_label);
	return true;
}
/***
** 日期: 2022-05-09 10:57
** 作者: leo.liu
** 函数作用：删除一个字符
** 返回参数说明：
***/
bool setting_password_del_string(lv_obj_t *parent, int max_edit)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}

	int edit_index = *((int *)label->user_data);
	if (edit_index == 0)
	{
		return false;
	}
	edit_index -= 1;

	int id = 10 + edit_index;
	lv_obj_t *obj = lv_obj_get_child_form_id(parent, id);
	if (obj == NULL)
	{
		printf("dind id:%d failed \n", id);
		return false;
	}
	lv_obj_set_hidden(obj, true);

	id = 20 + edit_index;
	obj = lv_obj_get_child_form_id(parent, id);
	if (obj == NULL)
	{
		return false;
	}
	lv_obj_set_hidden(obj, true);

	setting_password_line_set_cheked(parent, 30 + edit_index - 1, max_edit);

	(*((int *)label->user_data))--;
	return true;
}

/***
** 日期: 2022-05-09 11:22
** 作者: leo.liu
** 函数作用：获取输入字符串
** 返回参数说明：
***/
bool setting_password_get_string(lv_obj_t *parent, char *buffer)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}
	int edit_index = *((int *)label->user_data);
	if (edit_index == 0)
	{
		return false;
	}

	for (int i = 0; i < edit_index; i++)
	{
		label = lv_obj_get_child_form_id(parent, 10 + i);
		if (label == NULL)
		{
			printf("find obj failed %d\n", i);
		}
		strcat(buffer, lv_label_get_text(label));
	}

	return true;
}
/***
** 日期: 2022-05-09 11:27
** 作者: leo.liu
** 函数作用：获取当前输入的索引
** 返回参数说明：
***/
int setting_password_edit_index_get(lv_obj_t *parent)
{
	lv_obj_t *label = lv_obj_get_child_form_id(parent, 11);
	if (label == NULL)
	{
		return false;
	}
	int edit_index = *((int *)label->user_data);
	return edit_index;
}
#define TOAST_BG_COLOR (0XB0B0B0)
#define TOAST_TEXT_COLOR (0X0A0A0A)
/*************************************************************************
 * @brief  toast动画结束后执行的函数 删除控件
 * @date   2022-09-03 11:41
 * @author xiaoele
 **************************************************************************/
static void lv_toast_obj_del(lv_anim_t *a)
{
	lv_obj_del(a->var);
}
/*************************************************************************
 * @brief  消息弹窗 0>success 1>warning 2>error 3>notice
 * @date   2022-09-03 10:55
 * @author xiaoele
 * @param  str
 * @param  type
 * @param  repeat  0->不重复， n->重复次数 LV_ANIM_REPEAT_INFINITE->无限循环
 * @param  offset  [0]不偏移 [1]向下偏移 [-1]向上偏移
 **************************************************************************/
void msg_toast(const char *str, short int type, int repeat, int offset)
{
	static rom_bin_info imgarr[] = {rom_bin_info_get(ROM_UI_SETTING_SUCCESS_PNG), rom_bin_info_get(ROM_UI_SETTING_WARNING_PNG), rom_bin_info_get(ROM_UI_SETTING_ERROR_PNG), rom_bin_info_get(ROM_UI_SETTING_NOTICE_PNG)};
	lv_obj_t *toast_cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_t *label = lv_label_create(toast_cont, NULL);
	lv_obj_t *icon = lv_obj_create(toast_cont, NULL);

	lv_obj_set_click(label, false);
	lv_obj_set_click(icon, false);

	lv_obj_set_size(icon, 24, 24);

	lv_label_set_text_fmt(label, "%s", str);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);

	lv_obj_set_style_local_pattern_image(icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &imgarr[type]);

	lv_obj_set_style_local_pad_top(toast_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 12);
	lv_obj_set_style_local_pad_left(toast_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 24);
	lv_obj_set_style_local_pad_right(toast_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 24);
	lv_obj_set_style_local_pad_bottom(toast_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 12);

	lv_obj_set_style_local_radius(toast_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 8);
	lv_obj_set_style_local_bg_opa(toast_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	lv_obj_set_style_local_bg_color(toast_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(TOAST_BG_COLOR));

	lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(TOAST_TEXT_COLOR));
	lv_obj_set_style_local_text_font(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	lv_obj_set_auto_realign(toast_cont, true);
	lv_obj_set_auto_realign(icon, true);
	lv_obj_set_auto_realign(label, true);

	lv_obj_align(toast_cont, NULL, LV_ALIGN_CENTER, 0, LV_VER_RES_MAX * (0.2 + offset * 0.1));
	lv_obj_align(icon, toast_cont, LV_ALIGN_IN_LEFT_MID, 24, 0);
	lv_obj_align(label, icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

	lv_cont_set_fit(toast_cont, LV_FIT_TIGHT);

	/* 动画 自动删除 */
	lv_anim_t toast_anim;
	lv_anim_init(&toast_anim);
	lv_anim_set_time(&toast_anim, 300);
	lv_anim_set_delay(&toast_anim, 2000);
	lv_anim_set_var(&toast_anim, toast_cont);
	lv_anim_set_repeat_count(&toast_anim, repeat);
	lv_anim_set_exec_cb(&toast_anim, (lv_anim_exec_xcb_t)lv_obj_set_height);
	lv_anim_set_values(&toast_anim, lv_obj_get_height(toast_cont), 0);
	lv_anim_set_ready_cb(&toast_anim, lv_toast_obj_del);
	lv_anim_start(&toast_anim);
}

/***
** 日期: 2022-05-07 16:58
** 作者: leo.liu
** 函数作用：设置密码输入框
** 返回参数说明
** 3x4个控件
** LABEL ：显示文本
** OBJ ：显示圆点
** OBJ :显示下划线
** 要获取控件:可以通过矩阵id获取，如第一个lable:11
***/
bool setting_password_input_label_create(lv_obj_t *parent, int x, int y, int row)
{
	static int edit_index = 0;
	edit_index = 0;
	/***** 行号减1 *****/
	if (row < 1)
	{
		return false;
	}

	for (int i = 0; i < row; i++)
	{
		lv_obj_t *label = lv_label_create(parent == NULL ? lv_scr_act() : parent, NULL);
		lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
		lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
		lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_FONT_SIZE_31);
		lv_label_set_text(label, "");
		lv_obj_set_size(label, 60, 60);
		label->user_data = &edit_index;

		lv_obj_set_pos(label, x, y + 14 + i * 170);
		lv_obj_set_id(label, 10 + i * 4);

		label = lv_label_create(parent == NULL ? lv_scr_act() : parent, label);
		lv_obj_set_x(label, x + (60 + 24) * 1);
		lv_obj_set_id(label, 11 + i * 4);

		label = lv_label_create(parent == NULL ? lv_scr_act() : parent, label);
		lv_obj_set_x(label, x + (60 + 24) * 2);
		lv_obj_set_id(label, 12 + i * 4);

		label = lv_label_create(parent == NULL ? lv_scr_act() : parent, label);
		lv_obj_set_x(label, x + (60 + 24) * 3);
		lv_obj_set_id(label, 13 + i * 4);

		lv_obj_t *rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
		rot->user_data = &edit_index;
		lv_obj_set_size(rot, 16, 16);
		lv_obj_set_hidden(rot, true);

		lv_obj_set_style_local_bg_opa(rot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
		lv_obj_set_style_local_bg_color(rot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_radius(rot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 60);

		lv_obj_set_pos(rot, x + 22, y + 29 + i * 170);
		lv_obj_set_id(rot, 20 + i * 4);

		rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, rot);
		lv_obj_set_x(rot, x + 22 + (60 + 24) * 1);
		lv_obj_set_id(rot, 21 + i * 4);

		rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, rot);
		lv_obj_set_x(rot, x + 22 + (60 + 24) * 2);
		lv_obj_set_id(rot, 22 + i * 4);

		rot = lv_obj_create(parent == NULL ? lv_scr_act() : parent, rot);
		lv_obj_set_x(rot, x + 22 + (60 + 24) * 3);
		lv_obj_set_id(rot, 23 + i * 4);

		lv_obj_t *line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, NULL);
		line->user_data = &edit_index;
		lv_obj_set_size(line, 60, 4);
		lv_obj_set_style_local_bg_opa(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
		lv_obj_set_style_local_bg_color(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_bg_color(line, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, lv_color_hex(0x0096ff));
		lv_obj_set_style_local_radius(line, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 60);
		lv_obj_set_id(line, 30 + i * 4);
		lv_obj_set_pos(line, x, y + 74 + i * 170);

		line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, line);
		lv_obj_set_x(line, x + (60 + 24) * 1);
		lv_obj_set_id(line, 31 + i * 4);

		line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, line);
		lv_obj_set_x(line, x + (60 + 24) * 2);
		lv_obj_set_id(line, 32 + i * 4);

		line = lv_obj_create(parent == NULL ? lv_scr_act() : parent, line);
		lv_obj_set_x(line, x + (60 + 24) * 3);
		lv_obj_set_id(line, 33 + i * 4);
	}
	return true;
}

void update_motion_buttons_group_state(lv_obj_t *parent, int obj_id, bool add_to_group)
{

	for (int i = 1; i < obj_id; i++)
	{
		if (add_to_group)
		{
			lv_group_add_obj(lv_group_get_default(), lv_obj_get_child_form_id(parent, i));
		}
		else
		{
			lv_group_remove_obj(lv_obj_get_child_form_id(parent, i));
		}
	}
}
