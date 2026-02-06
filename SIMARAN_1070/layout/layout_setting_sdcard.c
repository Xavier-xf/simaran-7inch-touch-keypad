/*******************************************************************
 * @Descripttion   : 
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-10 14:46
 * @LastEditTime   : 2023-03-04 11:30
*******************************************************************/
#include "layout_define.h"


#define SETTING_SDCARD_BACE_BTN_ID 1
#define SETTING_SDCARD_FORMAT_SD_LABEL_ID 2

static int exit_time_count = 0;
static int delete_media_total = 0;

lv_obj_t *label_format = NULL;

static void sdcard_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}
static void sdcard_back_btn_create(lv_obj_t *parent)
{
    lv_obj_t *obj = setting_back_btn_create(parent, sdcard_back_btn_up);
    lv_obj_set_id(obj, SETTING_SDCARD_BACE_BTN_ID);
}


static void sdcard_format_sd_label_create(lv_obj_t *parent)
{
    label_format = lv_label_create(parent, NULL);
    lv_obj_set_id(label_format, SETTING_SDCARD_FORMAT_SD_LABEL_ID);
    // if(media_sdcard_insert_check())
    // {
    //     lv_label_set_text(label, str_get(LAYOUT_SD_LANG_FORMATING_ID));
    //     media_format_sd();
    // }
    // else
    // {
    //     lv_label_set_text(label, str_get(COMMON_LANG_NO_SD_ID));
    // }
    lv_label_set_text(label_format,"");
    lv_obj_set_style_local_text_font(label_format, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    if(user_data_get()->setting.language == LANG_PERSIAN)
    {
        lv_obj_align(label_format, NULL, LV_ALIGN_IN_TOP_MID, -100, 160);
    }
    else
    {
        lv_obj_align(label_format, NULL, LV_ALIGN_IN_TOP_MID, 60, 160);
    }

}





void setting_string_format_switch(char *dst_str,const char *src_str, int num)
{
    if(user_data_get()->setting.language == LANG_PERSIAN)
        sprintf(dst_str, "%s %% %d", src_str, num);
    else
        sprintf(dst_str, "%s %d %%", src_str, num);
}




static void sdcard_copy_state_display_task(lv_task_t *task_t)
{
    char str[40] = {0};
    int copied_photo = 0;
    if (media_copy_flash_photo_to_sd_state(&copied_photo) == false)
    {
        if(media_sdcard_insert_check())
        {
            setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), 100);
            lv_label_set_text(task_t->user_data, str);
            lv_obj_align(task_t->user_data, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
        }
        if (--exit_time_count <= 0)
        {
            goto_layout(pLAYOUT(setting));
        }
    }
    else
    {
        setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), (int)(copied_photo * 100 / delete_media_total));
        printf("=====================>>> 已拷贝照片:[%d]\n", copied_photo);
        lv_label_set_text(task_t->user_data, str);
        lv_obj_align(task_t->user_data, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
    }
}






static void sdcard_format_state_display_task(lv_task_t *task_t)
{
    if(media_format_sd_state() == false)
    {
        if(media_sdcard_insert_check() && exit_time_count > 3)
        {
            lv_label_set_text(label_format,"");
            lv_label_set_text(task_t->user_data, str_get(LAYOUT_SD_LANG_FORMAT_OK_ID));
            lv_obj_align(task_t->user_data, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
            
            exit_time_count = 3;
        }

        if (--exit_time_count <= 0)
        {
            goto_layout(pLAYOUT(setting));
        }
    }
    else
    {
        static char str[10][10] = {".\0", "..\0", "...\0"};
        lv_label_set_text(task_t->user_data, str_get(LAYOUT_SD_LANG_FORMATING_ID));
        lv_label_set_text(label_format,str[exit_time_count++ % 3]);
        // lv_label_set_text_fmt(task_t->user_data, "%s%s", str_get(LAYOUT_SD_LANG_FORMATING_ID), str[exit_time_count++ % 3]);
        // lv_obj_align(task_t->user_data, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);
    }
    // printf("===================>>>>> exit_time_count : [%d]\n", exit_time_count);
}

static void sdcard_no_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting_sdcard));
}
static void sdcard_copy_yes_btn_up(lv_obj_t *obj)
{
    if (media_copy_flash_photo_to_sd_state(NULL))
    {
        return;
    }
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 1), false);//yes按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 2), false);//no按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SDCARD_BACE_BTN_ID), false);//退出按键禁止点击

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    if(media_sdcard_insert_check())
    {
        delete_media_total = high_speed_media_file_total_get(FILE_TYPE_FLASH_PHOTO);
        char str[40] = {0};
        if (delete_media_total == 0)
        {
            setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), 100);
            lv_label_set_text(label, str);
        }
        else
        {
            setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), 0);
            lv_label_set_text(label, str);
            media_copy_flash_photo_to_sd();
        }
    }
    else
    {
        lv_label_set_text(label, str_get(COMMON_LANG_NO_SD_ID));
    }
    
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
    exit_time_count = 3;
    lv_layout_task_create(sdcard_copy_state_display_task, 200, LV_TASK_PRIO_LOWEST, label);
}
static void sdcard_copy_msg_box_create(void)
{
    static obj_click_data btn_data = obj_click_data_up_create(sdcard_no_btn_up);
    static obj_click_data btn_data1 = obj_click_data_up_create(sdcard_copy_yes_btn_up);
    message_box_create(lv_scr_act(), str_get(LAYOUT_SD_LANG_COPY_ID), &btn_data1, &btn_data);
}








static void sdcard_format_yes_btn_up(lv_obj_t *obj)
{
    if(media_format_sd_state())
    {
        return;
    }
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 1), false);//yes按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 2), false);//no按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SDCARD_BACE_BTN_ID), false);//退出按键禁止点击

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    
    if(media_sdcard_insert_check())
    {
        lv_label_set_text(label, str_get(LAYOUT_SD_LANG_FORMATING_ID));
        media_format_sd();
        user_data_get()->new_media_file_flag = false;
        user_data_get()->new_photo_file_flag = false;
    }
    else
    {
        lv_label_set_text(label, str_get(COMMON_LANG_NO_SD_ID));
    }
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
    exit_time_count = 0;
    lv_layout_task_create(sdcard_format_state_display_task, 200, LV_TASK_PRIO_LOWEST, label);
}
static void sdcard_format_msg_box_create(void)
{
    static obj_click_data btn_data = obj_click_data_up_create(sdcard_no_btn_up);
    static obj_click_data btn_data1 = obj_click_data_up_create(sdcard_format_yes_btn_up);
    message_box_create(lv_scr_act(), str_get(LAYOUT_SD_LANG_FORMAT_ID), &btn_data1, &btn_data);
}






static void sdcard_selete_btn_up(lv_obj_t *obj)
{
    if(obj == lv_obj_get_child_form_id(obj->parent, 1))
    {
        sdcard_format_msg_box_create();
    }
    else if(obj == lv_obj_get_child_form_id(obj->parent, 2))
    {
        sdcard_copy_msg_box_create();
    }
    lv_obj_del(obj->parent);
}
//创建sdcard选择按钮
static void sdcard_select_btn_create(lv_obj_t * parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    // lv_obj_set_pos(cont,286,233);
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


    lv_obj_t *format_btn = lv_obj_create(cont, NULL);
    lv_obj_set_id(format_btn, 1);
    lv_obj_set_size(format_btn, 452, 67);
    lv_obj_set_style_local_bg_opa(format_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_pattern_opa(format_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pattern_opa(format_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_UP_PNG);
    lv_obj_set_style_local_pattern_image(format_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_value_str(format_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SD_LANG_FORMAT_ID));
    lv_obj_set_style_local_value_color(format_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_align(format_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_font(format_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(format_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    static obj_click_data btn_data1 = obj_click_data_up_create(sdcard_selete_btn_up);
	obj_click_event_listen(format_btn, &btn_data1);

    lv_obj_t *copy_btn = lv_obj_create(cont, format_btn);
    lv_obj_set_id(copy_btn, 2);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_DOWN_PNG);
    lv_obj_set_style_local_pattern_image(copy_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
    lv_obj_set_style_local_value_str(copy_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SD_LANG_COPY_ID));
    lv_obj_align(copy_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    static obj_click_data btn_data2 = obj_click_data_up_create(sdcard_selete_btn_up);
	obj_click_event_listen(copy_btn, &btn_data2);
}


    



static void LAYOUT_ENTER_FUNC(setting_sdcard)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    setting_logo_img_create(parent);
    sdcard_back_btn_create(parent);
    sdcard_select_btn_create(parent);
    sdcard_format_sd_label_create(parent);
}

static void LAYOUT_QUIT_FUNC(setting_sdcard)
{
}

CREATE_LAYOUT(setting_sdcard);