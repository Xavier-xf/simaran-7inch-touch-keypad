// /*******************************************************************
//  * @Descripttion   :
//  * @version        : 1.0.0
//  * @Author         : wxj
//  * @Date           : 2022-11-10 15:40
//  * @LastEditTime   : 2023-03-14 15:19
// *******************************************************************/
// #include "layout_define.h"

// #define DOOR_RING_MUTE_ION_ID 1

// //1:door1 2:door2
// uint8_t door_x = 1;

// static int door_ring_index_get(void)
// {
//     if(door_x == 1)
//     {
//         return user_data_get()->setting.door1_tone;
//     }
//     else
//     {
//         return user_data_get()->setting.door2_tone;
//     }
// }
// static void door_ring_index_set(int index)
// {
//     if(door_x == 1)
//     {
//         user_data_get()->setting.door1_tone = index;
//     }
//     else
//     {
//         user_data_get()->setting.door2_tone = index;
//     }
// }

// static int door_ring_volume_get(void)
// {
//     if(door_x == 1)
//     {
//         return user_data_get()->setting.door1_ring_volume;
//     }
//     else
//     {
//         return user_data_get()->setting.door2_ring_volume;
//     }
// }
// static void door_ring_volume_set(int vol)
// {
//     if(door_x == 1)
//     {
//         user_data_get()->setting.door1_ring_volume = vol;
//     }
//     else
//     {
//         user_data_get()->setting.door2_ring_volume = vol;
//     }
// }

// static void door_ring_back_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(bell));
// }
// static void door_ring_back_btn_create(lv_obj_t *parent)
// {
//     setting_back_btn_create(parent, door_ring_back_btn_up);
// }

// static void door_ring_play_start_cb(int index)
// {
//     ring_volume_set(door_ring_volume_get());
// }
// static void door_ring_play_finish_cb(int index)
// {
//     // power_amplifier_enable(false);
// }

// //铃声选择回调
// static void door_ring_select_btn_up(lv_obj_t *obj)
// {
//     int index = door_ring_index_get();
//     if(index >= 6)
//     {
//         index = 0;
//     }
//     index++;
//     door_ring_index_set(index);
//     ring_volume_set(door_ring_volume_get());
//     char str[3] = {0};
//     if(door_ring_volume_get() > 0)
//     {
//         ringplay_play_form_index(index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
//     }
//     sprintf(str, "%d", index);
//     lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str);
// }

// //铃声音量调节回调
// static void door_ring_volume_adj_btn_up(lv_obj_t *obj)
// {
//     lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), DOOR_RING_MUTE_ION_ID);
//     int bell_volume = door_ring_volume_get();
//     if (obj == lv_obj_get_child_form_id(obj->parent, 1))
//     {
// 		if(bell_volume > 0)
//             bell_volume--;
//     }
//     else if(obj == lv_obj_get_child_form_id(obj->parent, 2))
// 	{
// 		if(bell_volume < 5)
//             bell_volume++;
//     }

//     if(bell_volume == 0)
//     {
//         lv_obj_set_hidden(obj1,false);
//     }
//     else
//     {
//         lv_obj_set_hidden(obj1,true);
//     }

//     door_ring_volume_set(bell_volume);
//     lv_slider_set_value((lv_obj_t *)obj->user_data, bell_volume, LV_ANIM_OFF);
//     printf("==============>>> ring volume : [%d]\n", bell_volume);
//     if(bell_volume > 0)
//     {
//         ringplay_play_form_index(door_ring_index_get(), 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
//     }
// }
// //创建 音量设置 和 铃声 选择按钮
// static void door_ring_select_and_volume_shetting_btn_create(lv_obj_t * parent)
// {   //容器
//     lv_obj_t *cont = lv_cont_create(parent, NULL);
//     lv_obj_set_size(cont, 452, 240);
//     lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
//     lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
//     lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
//     lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 14);
//     lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,  door_x == 1 ? str_get(LAYOUT_BELL_LANG_DOOR1_RING_ID) : str_get(LAYOUT_BELL_LANG_DOOR2_RING_ID));
//     lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
//     lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -(lv_obj_get_height(cont) / 3));
//     lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     //分隔线
//     lv_obj_t *line = lv_line_create(cont, NULL);
//     static lv_point_t points[2] = {{0, 70}, {452, 70}};
//     lv_line_set_points(line, points, 2);
//     lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2F2F2F));
//     lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
//     lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

//     line = lv_line_create(cont, line);
//     static lv_point_t points1[2] = {{0, 155}, {452, 155}};
//     lv_line_set_points(line, points1, 2);

//     //音量减按键
//     lv_obj_t *vol_sub_btn = lv_obj_create(cont, NULL);
//     lv_obj_set_id(vol_sub_btn, 1);
//     lv_obj_set_size(vol_sub_btn, 40, 40);
//     lv_obj_set_style_local_bg_opa(vol_sub_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
//     static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_VOL_SUB_PNG);
//     lv_obj_set_style_local_pattern_image(vol_sub_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
//     lv_obj_align(vol_sub_btn, NULL, LV_ALIGN_IN_LEFT_MID, 37, 0);
//     static obj_click_data btn_data1 = obj_click_data_up_create(door_ring_volume_adj_btn_up);
//     obj_click_event_listen(vol_sub_btn, &btn_data1);
//     lv_obj_set_ext_click_area(vol_sub_btn, 10, 10, 10, 10);
//     // 音量加按键
//     lv_obj_t *vol_add_btn = lv_obj_create(cont, vol_sub_btn);
//     lv_obj_set_id(vol_add_btn, 2);
//     lv_obj_set_size(vol_add_btn, 40, 40);
//     static rom_bin_info info2 = rom_bin_info_get(ROM_UI_SETTING_VOL_ADD_PNG);
//     lv_obj_set_style_local_pattern_image(vol_add_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
//     lv_obj_align(vol_add_btn, NULL, LV_ALIGN_IN_RIGHT_MID, -37, 0);
//     static obj_click_data btn_data2 = obj_click_data_up_create(door_ring_volume_adj_btn_up);
//     obj_click_event_listen(vol_add_btn, &btn_data2);

//     //当前音量值 滑块
//     lv_obj_t *slider = lv_slider_create(cont, NULL);
// 	lv_obj_set_click(slider, false);
// 	lv_obj_set_size(slider, 220, 4);
//     lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
//     lv_slider_set_range(slider, 0, 5);
//     lv_slider_set_value(slider, door_ring_volume_get(), LV_ANIM_OFF);
//     lv_obj_set_adv_hittest(slider, false);
//     lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xD9D9D9));
//     lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
//     lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_pad_all(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 3);
// 	vol_sub_btn->user_data = slider;
// 	vol_add_btn->user_data = slider;

//     //铃声选择按键
//     lv_obj_t *ring_select_btn = lv_obj_create(cont, NULL);
//     lv_obj_set_click(ring_select_btn, false);
//     lv_obj_set_size(ring_select_btn, 452, 85);
//     lv_obj_set_style_local_bg_opa(ring_select_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
//     lv_obj_set_style_local_value_str(ring_select_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_BELL_LANG_RING_ID));
//     lv_obj_set_style_local_value_align(ring_select_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
//     lv_obj_set_style_local_value_ofs_x(ring_select_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 18);
//     lv_obj_set_style_local_value_font(ring_select_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     lv_obj_align(ring_select_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 155);
//     // static obj_click_data btn_data3 = obj_click_data_up_create(door_ring_select_btn_up);
// 	// obj_click_event_listen(ring_select_btn, &btn_data3);

//     //铃声索引显示
//     lv_obj_t *ring_index_obj = lv_obj_create(ring_select_btn, NULL);
//     // lv_obj_set_click(ring_index_obj, false);
//     lv_obj_set_size(ring_index_obj, 30, 20);
//     lv_obj_set_ext_click_area(ring_index_obj, 30, 30, 30, 30);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_BELL_NEXT_PNG);
//     lv_obj_set_style_local_pattern_image(ring_index_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
//     lv_obj_set_style_local_pattern_align(ring_index_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);

//     static obj_click_data btn_data3 = obj_click_data_up_create(door_ring_select_btn_up);
// 	obj_click_event_listen(ring_index_obj, &btn_data3);

//     static char str[3];
//     sprintf(str, "%d", door_ring_index_get());
//     lv_obj_set_style_local_value_str(ring_index_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str);
//     lv_obj_set_style_local_value_ofs_x(ring_index_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -40);
//     // lv_obj_set_style_local_value_align(ring_index_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
//     lv_obj_set_style_local_value_font(ring_index_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     lv_obj_align(ring_index_obj, NULL, LV_ALIGN_IN_RIGHT_MID, -37, 0);
//     // ring_select_btn->user_data = ring_index_obj;
// }

// //静音ion显示
// lv_obj_t *door_ring_mute_img_create(lv_obj_t *parent)
// {
//     lv_obj_t * obj = lv_obj_create(parent,NULL);
//     lv_obj_set_id(obj,DOOR_RING_MUTE_ION_ID);
// 	lv_obj_set_pos(obj, 495, 280);
// 	lv_obj_set_size(obj, 34, 34);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_MUTE_PNG);
// 	lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
//     if(door_ring_volume_get() == 0)
//     {
//         lv_obj_set_hidden(obj,false);
//     }
//     else
//     {
//         lv_obj_set_hidden(obj,true);
//     }

// 	return obj;
// }

// static void LAYOUT_ENTER_FUNC(door_ring)
// {
// 	lv_obj_t *parent = common_bg_display(lv_scr_act());
//     setting_logo_img_create(parent);
//     door_ring_back_btn_create(parent);
//     door_ring_select_and_volume_shetting_btn_create(parent);
//     door_ring_mute_img_create(parent);
// }

// static void LAYOUT_QUIT_FUNC(door_ring)
// {
//     ringplay_play_stop();
//     user_data_save();
// }

// CREATE_LAYOUT(door_ring);