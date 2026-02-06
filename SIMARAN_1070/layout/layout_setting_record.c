// #include "layout_define.h"

// #define RECORD_INSERT_SD_CARD_TIP_LABEL_ID 1 // 提示插卡标签的id
// #define RECORD_IMAGE_MODE_BTN_ID 2
// #define RECORD_VIDEO_MODE_BTN_ID 3

// static void record_back_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(setting));
// }
// static void record_back_btn_create(lv_obj_t *parent)
// {
//     setting_back_btn_create(parent, record_back_btn_up);
// }

// static void record_auto_record_sw_value_change(lv_obj_t *obj, lv_event_t event)
// {
//     if (event == LV_EVENT_VALUE_CHANGED)
//         user_data_get()->setting.auto_record_enable = lv_switch_get_state(obj);
// }
// static void record_auto_record_sw_create(lv_obj_t *parent)
// {
//     lv_obj_t *cont = lv_cont_create(parent, NULL);
//     lv_obj_set_pos(cont, 285, 146);
//     lv_obj_set_size(cont, 452, 64);
//     // lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 30);
//     lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
//     lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
//     lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);

//     lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_RECORD_LANG_AUTO_RECORD_ID));
//     lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     lv_obj_t *sw = lv_switch_create(cont, NULL);
//     lv_obj_set_size(sw, 64, 34);
//     lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x69CC00));
//     lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xCECECE));
//     lv_obj_align(sw, NULL, LV_ALIGN_IN_RIGHT_MID, -33, 0);
//     lv_obj_set_ext_click_area(sw, 5, 5, 10, 10);
//     static obj_click_data btn_data = obj_click_data_anything_create(record_auto_record_sw_value_change);
//     obj_click_event_listen(sw, &btn_data);

//     if (user_data_get()->setting.auto_record_enable == true)
//     {
//         lv_switch_on(sw, LV_ANIM_OFF);
//     }
//     else
//     {
//         lv_switch_off(sw, LV_ANIM_OFF);
//     }

//     // if(user_data_get()->setting.language == LANG_ENGLISH)
//     {
//         lv_obj_align(sw, NULL, LV_ALIGN_IN_RIGHT_MID, -33, 0);
//         lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
//         lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 25);
//     }
//     // else
//     // {
//     //     lv_obj_align(sw, NULL, LV_ALIGN_IN_LEFT_MID, 33, 0);
//     //     lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
//     //     lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -25);
//     // }
// }

// static void record_image_btn_up(lv_obj_t *obj)
// {
//     if (user_data_get()->setting.record_mode == 0)
//         return;
//     lv_obj_t *video_btn = lv_obj_get_child_form_id(obj->parent, RECORD_VIDEO_MODE_BTN_ID);
//     if (user_data_get()->setting.record_mode == 1)
//     {
//         user_data_get()->setting.record_mode = 0;
//         lv_obj_set_style_local_value_color(video_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
//         lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
//     }
// }

// // sd卡提示任務
// static lv_task_t *sd_card_tip_task_t = NULL;
// static void record_video_btn_tip_task(lv_task_t *task_t)
// {
//     lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), RECORD_INSERT_SD_CARD_TIP_LABEL_ID);

//     if (media_sdcard_insert_check())
//     {
//         if (sd_card_tip_task_t != NULL)
//         {
//             lv_task_del(sd_card_tip_task_t);
//             sd_card_tip_task_t = NULL;
//         }
//         lv_obj_set_hidden(label, true);
//         return;
//     }
//     lv_obj_set_hidden(label, !lv_obj_get_hidden(label));
// }
// static void record_video_btn_up(lv_obj_t *obj)
// {
//     if (user_data_get()->setting.record_mode == 1)
//         return;
//     lv_obj_t *image_btn = lv_obj_get_child_form_id(obj->parent, RECORD_IMAGE_MODE_BTN_ID);
//     if (user_data_get()->setting.record_mode == 0)
//     {
//         if (media_sdcard_insert_check() == false)
//         {
//             if (sd_card_tip_task_t != NULL)
//             {
//                 lv_task_del(sd_card_tip_task_t);
//                 sd_card_tip_task_t = NULL;
//             }
//             lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), RECORD_INSERT_SD_CARD_TIP_LABEL_ID), false);
//             sd_card_tip_task_t = lv_layout_task_create(record_video_btn_tip_task, 2000, LV_TASK_PRIO_LOWEST, NULL);
//             return;
//         }
//         user_data_get()->setting.record_mode = 1;
//         lv_obj_set_style_local_value_color(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
//         lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
//     }
// }
// // 创建record模式选择按钮
// static void record_mode_select_btn_create(lv_obj_t *parent)
// {
//     lv_obj_t *cont = lv_cont_create(parent, NULL);
//     lv_obj_set_pos(cont, 287, 260);
//     lv_obj_set_size(cont, 452, 134);
//     // lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 94);
//     lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
//     lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
//     lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);

//     lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_RECORD_LANG_RECORD_MODE_ID));
//     lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
//     lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -(lv_obj_get_height(cont) / 4));
//     lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     lv_obj_t *line = lv_line_create(cont, NULL);
//     static lv_point_t points[2] = {{0, 67}, {452, 67}};
//     lv_line_set_points(line, points, 2);
//     lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2F2F2F));
//     lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
//     lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

//     line = lv_line_create(cont, line);
//     static lv_point_t points1[2] = {{0, 67}, {0, 134}};
//     lv_line_set_points(line, points1, 2);
//     lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

//     lv_obj_t *image_btn = lv_obj_create(cont, NULL);
//     lv_obj_set_id(image_btn, RECORD_IMAGE_MODE_BTN_ID);
//     lv_obj_set_size(image_btn, 226, 67);
//     lv_obj_set_style_local_bg_opa(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
//     lv_obj_set_style_local_pattern_opa(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
//     lv_obj_set_style_local_pattern_opa(image_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_LEFT_PNG);
//     lv_obj_set_style_local_pattern_image(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
//     lv_obj_set_style_local_value_str(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_RECORD_LANG_IMAGE_ID));
//     lv_obj_set_style_local_value_align(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
//     lv_obj_set_style_local_value_font(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     lv_obj_align(image_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
//     static obj_click_data btn_data1 = obj_click_data_up_create(record_image_btn_up);
//     obj_click_event_listen(image_btn, &btn_data1);

//     lv_obj_t *video_btn = lv_obj_create(cont, image_btn);
//     lv_obj_set_id(video_btn, RECORD_VIDEO_MODE_BTN_ID);
//     static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN1_RIGHT_PNG);
//     lv_obj_set_style_local_pattern_image(video_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
//     lv_obj_set_style_local_value_str(video_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_RECORD_LANG_VIDEO_ID));
//     lv_obj_align(video_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
//     static obj_click_data btn_data2 = obj_click_data_up_create(record_video_btn_up);
//     obj_click_event_listen(video_btn, &btn_data2);

//     if (media_sdcard_insert_check() == false)
//     {
//         user_data_get()->setting.record_mode = 0;
//     }

//     if (user_data_get()->setting.record_mode == 0)
//     {
//         lv_obj_set_style_local_value_color(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
//         lv_obj_set_style_local_value_color(video_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
//     }
//     else if (user_data_get()->setting.record_mode == 1)
//     {
//         lv_obj_set_style_local_value_color(image_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
//         lv_obj_set_style_local_value_color(video_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
//     }
// }

// static void record_insert_sdcard_tip_label_create(lv_obj_t *parent)
// {
//     lv_obj_t *label = lv_label_create(parent, NULL);
//     lv_label_set_text(label, str_get(LAYOUT_RECORD_LANG_PLEASE_INSERT_SD));
//     lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -156);
//     lv_obj_set_hidden(label, true);
//     lv_obj_set_id(label, RECORD_INSERT_SD_CARD_TIP_LABEL_ID);
// }

// static void LAYOUT_ENTER_FUNC(setting_record)
// {
//     lv_obj_t *parent = common_bg_display(lv_scr_act());
//     setting_logo_img_create(parent);
//     record_back_btn_create(parent);
//     record_auto_record_sw_create(parent);
//     record_mode_select_btn_create(parent);
//     record_insert_sdcard_tip_label_create(parent);
// }
// static void LAYOUT_QUIT_FUNC(setting_record)
// {
//     if (sd_card_tip_task_t != NULL)
//     {
//         lv_task_del(sd_card_tip_task_t);
//         sd_card_tip_task_t = NULL;
//     }
//     user_data_save();
// }

// CREATE_LAYOUT(setting_record);