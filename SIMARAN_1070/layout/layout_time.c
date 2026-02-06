#include "layout_define.h"

typedef enum 
{
	TIME_YEAR_BTN_ID,
	TIME_MONTH_BTN_ID,
	TIME_DAY_BTN_ID,
	TIME_HOUR_BTN_ID,
	TIME_MIN_BTN_ID,
	TIME_TOTAL_BTN,
}time_btn_module;

static custom_area time_btn_area[TIME_TOTAL_BTN] = 
{
	{520, 87, 331, 48},
	{520, 161, 331, 48},
	{520, 235, 331, 48},
	{520, 309, 331, 48},
	{520, 383, 331, 48},
};


#define TIME_CALENDAR_PERSIAN_BTN_ID 0
#define TIME_CALENDAR_WEATERN_BTN_ID 1


static struct tm temp_tm;
static struct date temp_date;
static bool time_change_flag = false;

static char str_year[5] = {0};
static char str_month[5] = {0};
static char str_day[5] = {0};
static char str_hour[5] = {0};
static char str_min[5] = {0};




#define wxj 1



static void time_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}
static void time_back_btn_create(lv_obj_t *parent)
{
    setting_back_btn_create(parent, time_back_btn_up);
}


static void time_data_display_sw_value_change(lv_obj_t *obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
        user_data_get()->setting.time_display_enable = lv_switch_get_state(obj);
}

static void time_data_display_sw_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
#if wxj
    lv_obj_set_pos(cont, 85, 150);
#else
    lv_obj_set_pos(cont, 85, 194);
#endif
    lv_obj_set_size(cont, 415, 57);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);
    
    lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_CALENDAR_LANG_TIME_DISPLAY_ID));
    lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));

    lv_obj_t *sw = lv_switch_create(cont, NULL);
    lv_obj_set_size(sw, 64, 34);
    lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x69CC00));
    lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xCECECE));
    lv_obj_set_ext_click_area(sw, 5, 5, 10, 10);
    static obj_click_data btn_data = obj_click_data_anything_create(time_data_display_sw_value_change);
	obj_click_event_listen(sw, &btn_data);
    
    if(user_data_get()->setting.time_display_enable == true)
    {
        lv_switch_on(sw, LV_ANIM_OFF);
    }
    else
    {
        lv_switch_off(sw, LV_ANIM_OFF);
    }

    // if(user_data_get()->setting.language == LANG_ENGLISH)
    {
        lv_obj_align(sw, NULL, LV_ALIGN_IN_RIGHT_MID, -7, 0);
        lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
        lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 13);
    }
    // else
    // {
    //     lv_obj_align(sw, NULL, LV_ALIGN_IN_LEFT_MID, 7, 0);
    //     lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
    //     lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -13);
    // }
}




#if wxj

// static void time_clock_style_select_select_btn_up(lv_obj_t *obj)
// {
//     lv_obj_t *white_btn = lv_obj_get_child_form_id(obj->parent, 1);
//     lv_obj_t *black_btn = lv_obj_get_child_form_id(obj->parent, 2);
//     if(white_btn == obj)
//     {
//         user_data_get()->setting.clock_style = false;
//         lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
//         lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
//         lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
//         lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
//     }
//     else
//     {
//         user_data_get()->setting.clock_style = true;
//         lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
//         lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
//         lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
//         lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
//     }
// }



static void time_standby_clock_display_sw_value_change(lv_obj_t *obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
        user_data_get()->setting.clock_style = lv_switch_get_state(obj);
}


static void time_clock_style_select_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_pos(cont, 85, 230);
    lv_obj_set_size(cont, 415, 57);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);

    lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_CALENDAR_LANG_CLOCK_STYLE_ID));
    lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
    lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
    lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 13);


    lv_obj_t *sw = lv_switch_create(cont, NULL);
    lv_obj_set_size(sw, 64, 34);
    lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xCECECE));
    lv_obj_align(sw, NULL, LV_ALIGN_IN_RIGHT_MID, -7, 0);
    lv_obj_set_ext_click_area(sw, 5, 5, 10, 10);
    static obj_click_data btn_data = obj_click_data_anything_create(time_standby_clock_display_sw_value_change);
	obj_click_event_listen(sw, &btn_data);


    if(user_data_get()->setting.clock_style == true)
    {
        lv_switch_on(sw, LV_ANIM_OFF);
    }
    else
    {
        lv_switch_off(sw, LV_ANIM_OFF);
    }






    // lv_obj_t *white_btn = lv_obj_create(cont, NULL);
    // lv_obj_set_id(white_btn, 1);
    // lv_obj_set_size(white_btn, 60, 40);
    // lv_obj_set_style_local_bg_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    // lv_obj_set_style_local_bg_color(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xD5DCE0));
    // lv_obj_set_style_local_outline_color(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
    // lv_obj_set_style_local_outline_color(white_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x46CC00));
    // lv_obj_set_style_local_outline_width(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
    // static obj_click_data btn_data1 = obj_click_data_up_create(time_clock_style_select_select_btn_up);
	// obj_click_event_listen(white_btn, &btn_data1);

    // lv_obj_t *black_btn = lv_obj_create(cont, white_btn);
    // lv_obj_set_id(black_btn, 2);
    // lv_obj_set_size(black_btn, 60, 40);
    // lv_obj_set_style_local_bg_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    // lv_obj_set_style_local_bg_color(black_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x151515));
    // static obj_click_data btn_data2 = obj_click_data_up_create(time_clock_style_select_select_btn_up);
	// obj_click_event_listen(black_btn, &btn_data2);

    // if(user_data_get()->setting.clock_style)
    // {
    //     lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    //     lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    //     lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    //     lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    // }
    // else
    // {
    //     lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    //     lv_obj_set_style_local_outline_opa(white_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    //     lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    //     lv_obj_set_style_local_outline_opa(black_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    // }

    // if(user_data_get()->setting.language == LANG_ENGLISH)
    // {
    //     lv_obj_align(white_btn, NULL, LV_ALIGN_IN_RIGHT_MID, -90, 0);
    //     lv_obj_align(black_btn, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);
    //     lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
    //     lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 13);
    // }
    // else
    // {
    //     lv_obj_align(white_btn, NULL, LV_ALIGN_IN_LEFT_MID, 10, 0);
    //     lv_obj_align(black_btn, NULL, LV_ALIGN_IN_LEFT_MID, 90, 0);
    //     lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
    //     lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -13);
    // }

}

#endif






static void time_calendar_persian_btn_up(lv_obj_t *obj)
{
    if(user_data_get()->setting.calendar == 0) return;
    lv_obj_t *weatern_btn = lv_obj_get_child_form_id(obj->parent, TIME_CALENDAR_WEATERN_BTN_ID);

    user_data_get()->setting.calendar = 0;
    lv_obj_set_style_local_value_color(weatern_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
    lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));

    temp_date = gregorian2jalali(temp_date);
    sprintf(str_year, "%04d", temp_date.year);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_YEAR_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_MONTH_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_DAY_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}

static void time_calendar_weatern_btn_up(lv_obj_t *obj)
{
    if(user_data_get()->setting.calendar == 1) return;
    lv_obj_t *persian_btn = lv_obj_get_child_form_id(obj->parent, TIME_CALENDAR_PERSIAN_BTN_ID);

    user_data_get()->setting.calendar = 1;
    lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
    lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));

    temp_date = jalali2gregorian(temp_date);
    sprintf(str_year, "%04d", temp_date.year);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_YEAR_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_MONTH_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_DAY_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}
//创建calendar选择按钮
static void time_calendar_select_btn_create(lv_obj_t * parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_pos(cont, 85, 309);
    lv_obj_set_size(cont, 415, 124);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);

    lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(COMMON_LANG_CALENDAR_ID));
    lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -(lv_obj_get_height(cont) / 4));
    lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
    
    
    lv_obj_t *line = lv_line_create(cont, NULL);
    static lv_point_t points[2];
    points[0].x = 0;
    points[0].y = lv_obj_get_height(cont) / 2;
    points[1].x = lv_obj_get_width(cont);
    points[1].y = lv_obj_get_height(cont) / 2;
    lv_line_set_points(line, points, 2);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2F2F2F));
    lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

    
    line = lv_line_create(cont, line);
    static lv_point_t points1[2];
    points1[0].x = 0;
    points1[0].y = lv_obj_get_height(cont) / 2 + 1;
    points1[1].x = 0;
    points1[1].y = lv_obj_get_height(cont);
    lv_line_set_points(line, points1, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    /* 右边按键*/
    lv_obj_t *persian_btn = lv_obj_create(cont, NULL);
    lv_obj_set_id(persian_btn, TIME_CALENDAR_PERSIAN_BTN_ID);
    lv_obj_set_size(persian_btn, lv_obj_get_width(cont) / 2, lv_obj_get_height(cont) / 2 - 1);
    lv_obj_set_ext_click_area(persian_btn, 10, 10, 10, 10);
    lv_obj_set_style_local_bg_opa(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pattern_opa(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pattern_opa(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN2_LEFT_PNG);
    lv_obj_set_style_local_pattern_image(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_value_str(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_CALENDAR_LANG_PERSIAN_ID));
    lv_obj_set_style_local_value_align(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_font(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
    lv_obj_align(persian_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    static obj_click_data btn_data1 = obj_click_data_up_create(time_calendar_persian_btn_up);
	obj_click_event_listen(persian_btn, &btn_data1);

    /* 左边按键*/
    lv_obj_t *weatern_btn = lv_obj_create(cont, persian_btn);
    lv_obj_set_id(weatern_btn, TIME_CALENDAR_WEATERN_BTN_ID);
    lv_obj_set_ext_click_area(weatern_btn, 10, 10, 10, 10);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_SELECT_BTN2_RIGHT_PNG);
    lv_obj_set_style_local_pattern_image(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
    lv_obj_set_style_local_value_str(weatern_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_CALENDAR_LANG_WEATERN_ID));
    lv_obj_align(weatern_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    static obj_click_data btn_data2 = obj_click_data_up_create(time_calendar_weatern_btn_up);
	obj_click_event_listen(weatern_btn, &btn_data2);


    if(user_data_get()->setting.calendar == 0)
    {
        lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
        lv_obj_set_style_local_value_color(weatern_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
    }
    else if(user_data_get()->setting.calendar == 1)
    {
        lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
        lv_obj_set_style_local_value_color(weatern_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
    }    
}



static void time_setting_btn_create(lv_obj_t *parent, custom_area btn_area, const char *str, char *value, obj_click_data *prev_btn_data, obj_click_data *next_btn_data, unsigned int id)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_id(cont, id);
    lv_obj_set_pos(cont, btn_area.x, btn_area.y);
    lv_obj_set_size(cont, btn_area.w, btn_area.h);

    if(value != NULL)
        lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, value);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_value_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_ofs_x(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 70);
    lv_obj_set_style_local_value_font(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));

    lv_obj_t *prev_btn = lv_obj_create(cont, NULL);
    lv_obj_set_size(prev_btn, 18, 28);
    lv_obj_align(prev_btn, NULL, LV_ALIGN_IN_RIGHT_MID, -170, 0);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_SETTING_PREV_PNG);
	lv_obj_set_style_local_pattern_image(prev_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
    lv_obj_set_ext_click_area(prev_btn, 30, 20, 15, 15);
    if(prev_btn_data != NULL)
        obj_click_event_listen(prev_btn, prev_btn_data);
    cont->user_data = prev_btn;

    lv_obj_t *next_btn = lv_obj_create(cont, prev_btn);
    lv_obj_align(next_btn, NULL, LV_ALIGN_IN_RIGHT_MID, -13, 0);
    static rom_bin_info info2 = rom_bin_info_get(ROM_UI_SETTING_NEXT_PNG);
	lv_obj_set_style_local_pattern_image(next_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
    lv_obj_set_ext_click_area(next_btn, 20, 30, 15, 15);
    if(next_btn_data != NULL)
        obj_click_event_listen(next_btn, next_btn_data);
    prev_btn->user_data = next_btn;

    if(str != NULL)
    {
        lv_obj_t *label = lv_label_create(cont, NULL);
        lv_label_set_text(label, str);
        lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
        lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
        // lv_obj_align(label, NULL, LV_ALIGN_IN_RIGHT_MID, -230, 0);
        lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 40, 0);
    }
}


static void time_year_setting_btn_up(lv_obj_t *obj)
{
    lv_obj_t *prev_btn = (obj->parent)->user_data;
    lv_obj_t *next_btn = prev_btn->user_data;
    if(prev_btn == obj)
    {
        if(user_data_get()->setting.calendar == 1)
        {
            if(--temp_date.year < 2022)
                temp_date.year = 2037;
        }
        else
        {
            if(--temp_date.year < 1400)
                temp_date.year = 1415;
        }
    }
    else if(next_btn == obj)
    {
        if(user_data_get()->setting.calendar == 1)
        {
            if(++temp_date.year > 2037)
                temp_date.year = 2022;
        }
        else
        {
            if(++temp_date.year > 1415)
                temp_date.year = 1400;
        }
    }
    time_change_flag = true;
    sprintf(str_year, "%4d", temp_date.year);
    lv_obj_set_style_local_value_str(obj->parent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);

    int mon_late_day = user_data_get()->setting.calendar == 1 ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if(temp_date.day > mon_late_day)
    {
        temp_date.day = mon_late_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_DAY_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}
static void time_month_setting_btn_up(lv_obj_t *obj)
{
    lv_obj_t *prev_btn = (obj->parent)->user_data;
    lv_obj_t *next_btn = prev_btn->user_data;
    if(prev_btn == obj)
    {
        if(--temp_date.month < 1)
            temp_date.month = 12;
    }
    else if(next_btn == obj)
    {
        if(++temp_date.month > 12)
            temp_date.month = 1;
    }
    time_change_flag = true;
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(obj->parent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);

    int mon_late_day = user_data_get()->setting.calendar == 1 ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if(temp_date.day > mon_late_day)
    {
        temp_date.day = mon_late_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), TIME_DAY_BTN_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}
static void time_day_setting_btn_up(lv_obj_t *obj)
{
    lv_obj_t *prev_btn = (obj->parent)->user_data;
    lv_obj_t *next_btn = prev_btn->user_data;
    if(prev_btn == obj)
    {
        if(--temp_date.day < 1)
            temp_date.day = user_data_get()->setting.calendar == 1 ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    }
    else if(next_btn == obj)
    {
        if(++temp_date.day > (user_data_get()->setting.calendar == 1 ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month)))
            temp_date.day = 1;
    }
    time_change_flag = true;
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(obj->parent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}
static void time_hour_setting_btn_up(lv_obj_t *obj)
{
    lv_obj_t *prev_btn = (obj->parent)->user_data;
    lv_obj_t *next_btn = prev_btn->user_data;
    if(prev_btn == obj)
    {
        if(--temp_tm.tm_hour < 0)
            temp_tm.tm_hour = 23;
    }
    else if(next_btn == obj)
    {
        if(++temp_tm.tm_hour > 23)
            temp_tm.tm_hour = 0;
    }
    time_change_flag = true;
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    lv_obj_set_style_local_value_str(obj->parent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_hour);
}
static void time_min_setting_btn_up(lv_obj_t *obj)
{
    lv_obj_t *prev_btn = (obj->parent)->user_data;
    lv_obj_t *next_btn = prev_btn->user_data;
    if(prev_btn == obj)
    {
        if(--temp_tm.tm_min < 0)
            temp_tm.tm_min = 59;
    }
    else if(next_btn == obj)
    {
        if(++temp_tm.tm_min > 59)
            temp_tm.tm_min = 0;
    }
    time_change_flag = true;
    sprintf(str_min, "%02d", temp_tm.tm_min);
    lv_obj_set_style_local_value_str(obj->parent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_min);
}
static void time_setting_btn_display(void)
{
    user_time_read(&temp_tm);
    
    temp_date.year = temp_tm.tm_year;
    temp_date.month = temp_tm.tm_mon;
    temp_date.day = temp_tm.tm_mday;

    if(temp_date.year < 2022)
    {
        temp_date.year = 2022;
        temp_date.month = 1;
        temp_date.day = 1;
    }
    if(user_data_get()->setting.calendar == 0)
    {
        temp_date = gregorian2jalali(temp_date);
    }

    
    sprintf(str_year, "%04d", temp_date.year);
    static obj_click_data btn_data1 = obj_click_data_up_create(time_year_setting_btn_up);
    time_setting_btn_create(lv_scr_act(), time_btn_area[TIME_YEAR_BTN_ID], str_get(LAYOUT_CALENDAR_LANG_YEAR_ID), str_year, &btn_data1, &btn_data1, TIME_YEAR_BTN_ID);


    sprintf(str_month, "%02d", temp_date.month);
    static obj_click_data btn_data2 = obj_click_data_up_create(time_month_setting_btn_up);
    time_setting_btn_create(lv_scr_act(), time_btn_area[TIME_MONTH_BTN_ID], str_get(LAYOUT_CALENDAR_LANG_MONTH_ID), str_month, &btn_data2, &btn_data2, TIME_MONTH_BTN_ID);


    sprintf(str_day, "%02d",temp_date.day);
    static obj_click_data btn_data3 = obj_click_data_up_create(time_day_setting_btn_up);
    time_setting_btn_create(lv_scr_act(), time_btn_area[TIME_DAY_BTN_ID], str_get(LAYOUT_CALENDAR_LANG_DAY_ID), str_day, &btn_data3, &btn_data3, TIME_DAY_BTN_ID);


    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    static obj_click_data btn_data4 = obj_click_data_up_create(time_hour_setting_btn_up);
    time_setting_btn_create(lv_scr_act(), time_btn_area[TIME_HOUR_BTN_ID], str_get(LAYOUT_CALENDAR_LANG_HOUR_ID), str_hour, &btn_data4, &btn_data4, TIME_HOUR_BTN_ID);


    sprintf(str_min, "%02d", temp_tm.tm_min);
    static obj_click_data btn_data5 = obj_click_data_up_create(time_min_setting_btn_up);
    time_setting_btn_create(lv_scr_act(), time_btn_area[TIME_MIN_BTN_ID], str_get(LAYOUT_CALENDAR_LANG_MIN_ID), str_min, &btn_data5, &btn_data5, TIME_MIN_BTN_ID);
}


static void LAYOUT_ENTER_FUNC(time)
{
    time_change_flag = false;
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    setting_logo_img_create(parent);
    time_back_btn_create(parent);
    time_data_display_sw_create(parent);
#if wxj
    time_clock_style_select_create(parent);
#endif
    time_calendar_select_btn_create(parent);
    time_setting_btn_display();
}
static void LAYOUT_QUIT_FUNC(time)
{
    if(time_change_flag == true)
    {
        if(user_data_get()->setting.calendar == 0)
        {
            temp_date = jalali2gregorian(temp_date);
        }
        temp_tm.tm_year = temp_date.year;
        temp_tm.tm_mon = temp_date.month;
        temp_tm.tm_mday = temp_date.day;
        temp_tm.tm_sec = 0;
        standby_timer_close();
        user_time_set(&temp_tm);

        user_time_read(&temp_tm);
        if(temp_tm.tm_year < 2022)
        {
            temp_tm.tm_year = 2022;
            user_time_set(&temp_tm);
        }

        standby_timer_restart(true);
    }    
    user_data_save();
    extern unsigned long long calibrate_rtc_timestamp;
    calibrate_rtc_timestamp = user_timestamp_get();

}

CREATE_LAYOUT(time);