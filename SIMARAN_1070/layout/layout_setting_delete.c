#include "layout_define.h"

#define SETTING_DELETE_BACE_BTN_ID 1
#define DELETE_PICTURE_BTN_ID 2
#define DELETE_VIDEO_BTN_ID 3

static file_type delete_file_type = FILE_TYPE_NONE;
static int exit_time_count = 0;
static int delete_media_total = 0;

extern void setting_string_format_switch(char *dst_str, const char *src_str, int num);

static void delete_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}
static void delete_back_btn_create(lv_obj_t *parent)
{
    lv_obj_t *obj = setting_back_btn_create(parent, delete_back_btn_up);
    lv_obj_set_id(obj, SETTING_DELETE_BACE_BTN_ID);
}

// 创建 消息框
lv_obj_t *message_box_create(lv_obj_t *parent, const char *title, obj_click_data *yes_btn_data, obj_click_data *no_btn_data)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, 452, 134);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);

    lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, title);
    lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -(lv_obj_get_height(cont) / 4));
    lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

    lv_obj_t *line = lv_line_create(cont, NULL);
    static lv_point_t points[2] = {{0, 67}, {452, 67}};
    lv_line_set_points(line, points, 2);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2F2F2F));
    lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

    line = lv_line_create(cont, line);
    static lv_point_t points1[2] = {{0, 67}, {0, 134}};
    lv_line_set_points(line, points1, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 1);

    lv_obj_t *yes_btn = lv_obj_create(cont, NULL);
    lv_obj_set_id(yes_btn, 1);
    lv_obj_set_size(yes_btn, 226, 67);
    lv_obj_set_style_local_bg_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_pattern_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pattern_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_LEFT_PNG);
    lv_obj_set_style_local_pattern_image(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_value_str(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(COMMON_LANG_YES_ID));
    lv_obj_set_style_local_value_color(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF2B00));
    lv_obj_set_style_local_value_align(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_font(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(yes_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    if (yes_btn_data != NULL)
        obj_click_event_listen(yes_btn, yes_btn_data);

    lv_obj_t *no_btn = lv_obj_create(cont, yes_btn);
    lv_obj_set_id(no_btn, 2);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_RIGHT_PNG);
    lv_obj_set_style_local_pattern_image(no_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
    lv_obj_set_style_local_value_str(no_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(COMMON_LANG_NO_ID));
    lv_obj_set_style_local_value_color(no_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
    lv_obj_align(no_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    if (no_btn_data != NULL)
        obj_click_event_listen(no_btn, no_btn_data);

    return cont;
}

// 创建memory界面的消息框
lv_obj_t *memory_message_box_create(lv_obj_t *parent, obj_click_data *yes_btn_data, obj_click_data *no_btn_data, layout_lang_id id)
{
    // 1. 整体消息框容器 (460*156) - 核心：禁用点击，仅按钮可选中
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, 460, 156);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF)); // 白底
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCCCCCC));
    lv_obj_set_click(cont, false); // 关键：禁用整体容器点击，避免干扰按钮

    // 2. 提示语区域 (上2/3，460*96) - 禁用点击
    lv_obj_t *msg_area = lv_cont_create(cont, NULL);
    lv_obj_set_size(msg_area, 460, 96);
    lv_obj_align(msg_area, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_style_local_bg_opa(msg_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pad_all(msg_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 15);
    lv_obj_set_click(msg_area, false); // 关键：禁用提示语区域点击，仅按钮可交互

    // 提示语文本
    lv_obj_t *msg_label = lv_label_create(msg_area, NULL);
    lv_label_set_text(msg_label, str_get(id)); // 删除提示语
    lv_obj_set_style_local_text_color(msg_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x333333));
    lv_obj_set_style_local_text_font(msg_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_label_set_align(msg_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(msg_label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_click(msg_label, false); // 禁用文本点击

    // 3. 分隔线1：提示语区域 ↓ 按钮区域（横向黑线，460*1）
    lv_obj_t *line1 = lv_obj_create(cont, NULL);
    lv_obj_set_size(line1, 460, 1);                                                                     // 宽=消息框宽，高=1（细线）
    lv_obj_align(line1, NULL, LV_ALIGN_IN_TOP_MID, 0, 96);                                              // 位于提示语区域底部（y=96，与msg_area高一致）
    lv_obj_set_style_local_bg_color(line1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色
    lv_obj_set_style_local_bg_opa(line1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);               // 完全不透明
    lv_obj_set_click(line1, false);                                                                     // 禁用分隔线点击

    // 4. 按钮区域容器 (下1/3，460*60)
    lv_obj_t *btn_area = lv_cont_create(cont, NULL);
    lv_obj_set_size(btn_area, 460, 59);
    lv_obj_align(btn_area, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_local_bg_opa(btn_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pad_all(btn_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_click(btn_area, false); // 禁用按钮区域容器点击，仅单个按钮可交互

    // 5. 确认按钮 (230*60) - 唯一可点击的区域之一
    lv_obj_t *yes_btn = lv_obj_create(btn_area, NULL);
    lv_group_add_obj(lv_group_get_default(), yes_btn);
    lv_obj_set_size(yes_btn, 230, 59);
    lv_obj_set_pos(yes_btn, 0, 0);
    lv_obj_set_style_local_radius(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_color(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEBEBEB)); // lanse默认
    lv_obj_set_style_local_bg_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    lv_obj_set_style_local_border_width(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_bg_color(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xDC0000));
    lv_obj_set_style_local_bg_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_100);
    // -------------------------- 焦点状态样式 --------------------------
    // lv_obj_set_style_local_border_color(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
    // lv_obj_set_style_local_border_width(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
    lv_obj_set_style_local_bg_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_100);
    lv_obj_set_style_local_bg_color(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));

    // 确认按钮图片 (20*20)
    lv_obj_t *yes_img = lv_obj_create(yes_btn, NULL);
    lv_obj_set_size(yes_img, 20, 20);
    static rom_bin_info yes_info = rom_bin_info_get(ROM_UI_SETTING_YES_PNG);
    lv_obj_set_style_local_pattern_image(yes_img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &yes_info);
    lv_obj_align(yes_img, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_click(yes_img, false); // 禁用图片点击，点击事件由按钮父对象响应

    if (yes_btn_data != NULL)
        obj_click_event_listen(yes_btn, yes_btn_data); // 绑定确认按钮点击回调

    // 6. 分隔线2：确认按钮 ←→ 取消按钮（纵向黑线，1*60）
    lv_obj_t *line2 = lv_obj_create(btn_area, NULL);
    lv_obj_set_size(line2, 1, 59); // 宽=1（细线），高=按钮高
    lv_obj_set_pos(line2, 230, 0);
    lv_obj_set_style_local_bg_color(line2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000)); // 黑色
    lv_obj_set_style_local_bg_opa(line2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);               // 完全不透明
    lv_obj_set_click(line2, false);                                                                     // 禁用分隔线点击

    // 7. 取消按钮 (230*60) - 唯一可点击的区域之二
    lv_obj_t *no_btn = lv_obj_create(btn_area, NULL);
    lv_group_add_obj(lv_group_get_default(), no_btn);
    lv_obj_set_size(no_btn, 229, 59);
    lv_obj_set_pos(no_btn, 231, 0);
    lv_obj_set_style_local_radius(no_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_color(no_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xEBEBEB)); // 灰色默认
    lv_obj_set_style_local_bg_opa(no_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    lv_obj_set_style_local_border_width(no_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_bg_color(no_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xDC0000));
    lv_obj_set_style_local_bg_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_100);

    // -------------------------- 焦点状态样式 --------------------------
    // lv_obj_set_style_local_border_color(no_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));
    // lv_obj_set_style_local_border_width(no_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 2);
    lv_obj_set_style_local_bg_opa(no_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_100);
    lv_obj_set_style_local_bg_color(no_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x0081DC));

    // 取消按钮图片 (20*20)
    lv_obj_t *no_img = lv_obj_create(no_btn, NULL);
    lv_obj_set_size(no_img, 20, 20);
    static rom_bin_info no_info = rom_bin_info_get(ROM_UI_SETTING_NO_PNG);
    lv_obj_set_style_local_pattern_image(no_img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &no_info);
    lv_obj_align(no_img, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_click(no_img, false); // 禁用图片点击，点击事件由按钮父对象响应

    if (no_btn_data != NULL)
        obj_click_event_listen(no_btn, no_btn_data); // 绑定取消按钮点击回调

    return cont;
}

static void delete_file_state_display_task(lv_task_t *task_t)
{
    if (delete_file_type != FILE_TYPE_NONE)
    {
        char str[40] = {0};
        if (!media_file_delete_all_state())
        {
            setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 100);
            lv_label_set_text(task_t->user_data, str);
            delete_file_type = FILE_TYPE_NONE;
            exit_time_count = 1;
        }
        else
        {
            int cur_total = high_speed_media_file_total_get(delete_file_type);
            setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), (int)((delete_media_total - cur_total) * 100 / delete_media_total));
            lv_label_set_text(task_t->user_data, str);
        }
    }
    else
    {
        if (--exit_time_count <= 0)
        {
            goto_layout(pLAYOUT(setting));
        }
    }
}

static void delete_file_state_display_task_create(const char *state_info)
{
    standby_timer_close();
    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, state_info);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
    exit_time_count = 1;
    lv_layout_task_create(delete_file_state_display_task, 500, LV_TASK_PRIO_LOWEST, label);

    if (delete_file_type != FILE_TYPE_NONE)
    {
        media_file_delete_all_start(delete_file_type);
    }
}

static void delete_no_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting_delete));
}
static void delete_picture_yes_btn_up(lv_obj_t *obj)
{
    if (media_file_delete_all_state())
    {
        return;
    }
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 1), false);                           // yes按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 2), false);                           // no按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), SETTING_DELETE_BACE_BTN_ID), false); // 退出按键禁止点击

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
    delete_media_total = total;
    char str[40] = {0};
    if (total == 0)
    {
        delete_file_type = FILE_TYPE_NONE;
        setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 100);
        delete_file_state_display_task_create(str);
    }
    else
    {
        setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 0);
        delete_file_state_display_task_create(str);
    }
    user_data_get()->new_photo_file_flag = 0;
}
static void delete_picture_msg_box_create(void)
{
    static obj_click_data btn_data = obj_click_data_up_create(delete_no_btn_up);
    static obj_click_data btn_data1 = obj_click_data_up_create(delete_picture_yes_btn_up);
    message_box_create(lv_scr_act(), str_get(LAYOUT_DELETE_LANG_ALL_PICTURES_ID), &btn_data1, &btn_data);
}

static void delete_video_yes_btn_up(lv_obj_t *obj)
{
    if (media_file_delete_all_state())
    {
        return;
    }
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 1), false);                           // yes按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 2), false);                           // no按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), SETTING_DELETE_BACE_BTN_ID), false); // 退出按键禁止点击

    if (media_sdcard_insert_check() == true)
    {
        delete_file_type = FILE_TYPE_VIDEO;
        int total;
        media_file_total_get(delete_file_type, &total, NULL);
        delete_media_total = total;
        char str[40] = {0};
        if (total == 0)
        {
            delete_file_type = FILE_TYPE_NONE;
            setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 100);
            delete_file_state_display_task_create(str);
        }
        else
        {
            setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 0);
            delete_file_state_display_task_create(str);
        }
    }
    else
    {
        delete_file_type = FILE_TYPE_NONE;
        delete_file_state_display_task_create(str_get(COMMON_LANG_NO_SD_ID));
    }
    user_data_get()->new_media_file_flag = 0;
}
static void delete_video_msg_box_create(void)
{
    static obj_click_data btn_data = obj_click_data_up_create(delete_no_btn_up);
    static obj_click_data btn_data1 = obj_click_data_up_create(delete_video_yes_btn_up);
    message_box_create(lv_scr_act(), str_get(LAYOUT_DELETE_LANG_ALL_VIDEOS_ID), &btn_data1, &btn_data);
}

static void delete_selete_btn_up(lv_obj_t *obj)
{
    if (obj == lv_obj_get_child_form_id(obj->parent, DELETE_PICTURE_BTN_ID))
    {
        delete_picture_msg_box_create();
    }
    else if (obj == lv_obj_get_child_form_id(obj->parent, DELETE_VIDEO_BTN_ID))
    {
        delete_video_msg_box_create();
    }
    lv_obj_del(obj->parent);
}
// 创建delete选择按钮
static void delete_select_btn_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, 452, 134);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);

    lv_obj_t *line = lv_line_create(cont, NULL);
    static lv_point_t points[2] = {{0, 67}, {452, 67}};
    lv_line_set_points(line, points, 2);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2F2F2F));
    lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

    lv_obj_t *del_picture_btn = lv_obj_create(cont, NULL);
    lv_obj_set_id(del_picture_btn, DELETE_PICTURE_BTN_ID);
    lv_obj_set_size(del_picture_btn, 452, 67);
    lv_obj_set_style_local_bg_opa(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_pattern_opa(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pattern_opa(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_UP_PNG);
    lv_obj_set_style_local_pattern_image(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_value_str(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_DELETE_LANG_ALL_PICTURES_ID));
    lv_obj_set_style_local_value_color(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_align(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_font(del_picture_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(del_picture_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    static obj_click_data btn_data1 = obj_click_data_up_create(delete_selete_btn_up);
    obj_click_event_listen(del_picture_btn, &btn_data1);

    lv_obj_t *del_video_btn = lv_obj_create(cont, del_picture_btn);
    lv_obj_set_id(del_video_btn, DELETE_VIDEO_BTN_ID);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_DOWN_PNG);
    lv_obj_set_style_local_pattern_image(del_video_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
    lv_obj_set_style_local_value_str(del_video_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_DELETE_LANG_ALL_VIDEOS_ID));
    lv_obj_align(del_video_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    static obj_click_data btn_data2 = obj_click_data_up_create(delete_selete_btn_up);
    obj_click_event_listen(del_video_btn, &btn_data2);
}

static void LAYOUT_ENTER_FUNC(setting_delete)
{
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    setting_logo_img_create(parent);
    delete_back_btn_create(parent);
    delete_select_btn_create(parent);
}

static void LAYOUT_QUIT_FUNC(setting_delete)
{
    standby_timer_restart(true);
}

CREATE_LAYOUT(setting_delete);