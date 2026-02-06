#include "layout_define.h"

static void language_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}
static void language_back_btn_create(lv_obj_t *parent)
{
    setting_back_btn_create(parent, language_back_btn_up);
}

/***** 設置字庫 *****/
extern void lv_ft_font_set_type(int type);
/***** 重新初始化字庫 *****/
extern void lv_font_afresh_init(void);
static void language_english_btn_up(lv_obj_t *obj)
{
    if(user_data_get()->setting.language == LANG_ENGLISH) return;
    // lv_obj_t *persian_btn = lv_obj_get_child_form_id(obj->parent, 2);
    if(user_data_get()->setting.language == LANG_PERSIAN)
    {
        user_data_get()->setting.language = LANG_ENGLISH;
        // lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
        // lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	    lv_ft_font_set_type(user_data_get()->setting.language);
        lv_font_afresh_init();
        goto_layout(pLAYOUT(language));
    }
}

static void language_persian_btn_up(lv_obj_t *obj)
{
    if(user_data_get()->setting.language == LANG_PERSIAN) return;
    // lv_obj_t *english_btn = lv_obj_get_child_form_id(obj->parent, 1);    
    if(user_data_get()->setting.language == LANG_ENGLISH)
    {
        user_data_get()->setting.language = LANG_PERSIAN;
        // lv_obj_set_style_local_value_color(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
        // lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
        lv_ft_font_set_type(user_data_get()->setting.language);
        lv_font_afresh_init();
        goto_layout(pLAYOUT(language));
    }
}
//创建Language选择按钮
static void language_select_btn_create(lv_obj_t * parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, 452, 244);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 14);

    lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_LANGUAGE_LANG_SELECT_ID));
    lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -(lv_obj_get_height(cont) / 3));
    lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    
    lv_obj_t *line = lv_line_create(cont, NULL);
    static lv_point_t points[2] = {{0, 80}, {452, 80}};
    lv_line_set_points(line, points, 2);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2F2F2F));
    lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
    
    line = lv_line_create(cont, line);
    static lv_point_t points1[2] = {{0, 160}, {452, 160}};
    lv_line_set_points(line, points1, 2);

    lv_obj_t *english_btn = lv_obj_create(cont, NULL);
    lv_obj_set_id(english_btn, 1);
    lv_obj_set_size(english_btn, 452, 80);
    lv_obj_set_style_local_bg_opa(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(english_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
    lv_obj_set_style_local_value_str(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_LANGUAGE_LANG_ENGLISH_ID));
    lv_obj_set_style_local_value_align(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_font(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(english_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 82);
    static obj_click_data btn_data1 = obj_click_data_up_create(language_english_btn_up);
	obj_click_event_listen(english_btn, &btn_data1);

    lv_obj_t *persian_btn = lv_obj_create(cont, english_btn);
    lv_obj_set_id(persian_btn, 2);
    lv_obj_set_size(persian_btn, 452, 80);
    lv_obj_set_style_local_bg_opa(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    lv_obj_set_style_local_pattern_opa(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pattern_opa(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    // lv_obj_set_style_local_bg_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));

    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN2_DOWN_PNG);
    lv_obj_set_style_local_pattern_image(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_value_str(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_LANGUAGE_LANG_PERSIAN_ID));
    lv_obj_align(persian_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 164);
    static obj_click_data btn_data2 = obj_click_data_up_create(language_persian_btn_up);
	obj_click_event_listen(persian_btn, &btn_data2);

    if(user_data_get()->setting.language == LANG_ENGLISH)
    {
        lv_obj_set_style_local_value_color(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
        lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
    }
    else if(user_data_get()->setting.language == LANG_PERSIAN)
    {
        lv_obj_set_style_local_value_color(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
        lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
    }    
}


    



static void LAYOUT_ENTER_FUNC(language)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    setting_logo_img_create(parent);
    language_back_btn_create(parent);
    language_select_btn_create(parent);
}

static void LAYOUT_QUIT_FUNC(language)
{
    user_data_save();
}

CREATE_LAYOUT(language);