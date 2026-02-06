// /*******************************************************************
//  * @Descripttion   :
//  * @version        : 1.0.0
//  * @Author         : wxj
//  * @Date           : 2022-11-22 18:12
//  * @LastEditTime   : 2022-11-24 14:29
// *******************************************************************/
// #include "layout_define.h"

// static void bell_back_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(setting));
// }
// static void bell_back_btn_create(lv_obj_t *parent)
// {
//     setting_back_btn_create(parent, bell_back_btn_up);
// }

// extern uint8_t door_x;
// static void bell_door1_ring_btn_up(lv_obj_t *obj)
// {
//     door_x = 1;
//     goto_layout(pLAYOUT(door_ring));
// }

// static void bell_door2_ring_btn_up(lv_obj_t *obj)
// {
//     door_x = 2;
//     goto_layout(pLAYOUT(door_ring));
// }
// //创建bell选择按钮
// static void bell_select_btn_create(lv_obj_t * parent)
// {
//     lv_obj_t *cont = lv_cont_create(parent, NULL);
//     lv_obj_set_size(cont, 452, 244);
//     lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
//     lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
//     lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
//     lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 14);

//     lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_BELL_LANG_BELL_VOL_ID));
//     lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
//     lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -(lv_obj_get_height(cont) / 3));
//     lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     lv_obj_t *line = lv_line_create(cont, NULL);
//     static lv_point_t points[2] = {{0, 80}, {452, 80}};
//     lv_line_set_points(line, points, 2);
//     lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2F2F2F));
//     lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
//     lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

//     line = lv_line_create(cont, line);
//     static lv_point_t points1[2] = {{0, 160}, {452, 160}};
//     lv_line_set_points(line, points1, 2);

//     lv_obj_t *door1_ring_btn = lv_obj_create(cont, NULL);
//     lv_obj_set_id(door1_ring_btn, 1);
//     lv_obj_set_size(door1_ring_btn, 452, 82);
//     lv_obj_set_style_local_bg_opa(door1_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
//     lv_obj_set_style_local_bg_opa(door1_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
//     lv_obj_set_style_local_bg_color(door1_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
//     lv_obj_set_style_local_value_str(door1_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_BELL_LANG_DOOR1_RING_ID));
//     lv_obj_set_style_local_value_color(door1_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_value_align(door1_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
//     lv_obj_set_style_local_value_font(door1_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     lv_obj_align(door1_ring_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 82);
//     static obj_click_data btn_data1 = obj_click_data_up_create(bell_door1_ring_btn_up);
// 	obj_click_event_listen(door1_ring_btn, &btn_data1);

//     lv_obj_t *door2_ring_btn = lv_obj_create(cont, door1_ring_btn);
//     lv_obj_set_id(door2_ring_btn, 2);
//     lv_obj_set_style_local_bg_opa(door2_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
//     lv_obj_set_style_local_pattern_opa(door2_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
//     lv_obj_set_style_local_pattern_opa(door2_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
//     // lv_obj_set_style_local_bg_color(door2_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));

//     static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN2_DOWN_PNG);
//     lv_obj_set_style_local_pattern_image(door2_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
//     lv_obj_set_style_local_value_str(door2_ring_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_BELL_LANG_DOOR2_RING_ID));
//     lv_obj_align(door2_ring_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 164);
//     static obj_click_data btn_data2 = obj_click_data_up_create(bell_door2_ring_btn_up);
// 	obj_click_event_listen(door2_ring_btn, &btn_data2);
// }

// static void LAYOUT_ENTER_FUNC(bell)
// {
// 	lv_obj_t *parent = common_bg_display(lv_scr_act());
//     setting_logo_img_create(parent);
//     bell_back_btn_create(parent);
//     bell_select_btn_create(parent);
// }

// static void LAYOUT_QUIT_FUNC(bell)
// {
// }

// CREATE_LAYOUT(bell);