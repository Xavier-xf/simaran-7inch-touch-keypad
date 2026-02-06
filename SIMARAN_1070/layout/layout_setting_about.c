/*******************************************************************
 * @Descripttion   : 
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-10 15:29
 * @LastEditTime   : 2023-03-09 15:37
*******************************************************************/
#include "layout_define.h"


static void about_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}
static void about_back_btn_create(lv_obj_t *parent)
{
    setting_back_btn_create(parent, about_back_btn_up);
}

static void about_version_info_obj_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, 452, 134);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x434242));
    lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_radius(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 20);

    lv_obj_t *model_label = lv_label_create(cont, NULL);
    lv_label_set_text_fmt(model_label, "%s : %s TVD-1070P", str_get(LAYOUT_ABOUT_LANG_MODEL_ID), str_get(LAYOUT_ABOUT_LANG_TABA_ID)); //TVD-1043i
    lv_obj_set_style_local_text_font(model_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(model_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    lv_obj_t *version_label = lv_label_create(cont, model_label);
    int day = 0, month = 0, year = 0;
	get_curr_relesse_date(&day, &month, &year);
	lv_label_set_text_fmt(version_label, "%s : %02d.%d.%d", str_get(LAYOUT_ABOUT_LANG_VERSION_ID), year % 100, month, day);
    if(user_data_get()->setting.language == LANG_ENGLISH)
    {
        lv_obj_align(version_label, model_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }
    else
    {
        lv_obj_align(version_label, model_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 12);
    }



    // static char str[64] = {0};
    // static char str1[64] = {0};
    // static char str2[64] = {0};
    // lv_obj_t *model_obj = lv_obj_create(cont, NULL);

    // sprintf(str,"%s : %s TVD-1070P", str_get(LAYOUT_ABOUT_LANG_MODEL_ID), str_get(LAYOUT_ABOUT_LANG_TABA_ID));
    // lv_obj_set_style_local_value_str(model_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,str);
    // lv_obj_set_style_local_value_font(model_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    // lv_obj_align(model_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
    
    // lv_obj_t *version_obj = lv_obj_create(cont, model_obj);
    // int day = 0, month = 0, year = 0;
	// get_curr_relesse_date(&day, &month, &year);
    // sprintf(str1,"%02d.%d.%d", year % 100, month, day);
    // lv_obj_set_style_local_value_str(version_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,str1);
    // lv_obj_align(version_obj, model_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    // lv_obj_t *version_label = lv_obj_create(cont, model_obj);
    // sprintf(str2,"%s :  ",str_get(LAYOUT_ABOUT_LANG_VERSION_ID));
    // lv_obj_set_style_local_value_str(version_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,str2);
    // lv_obj_align(version_label, model_obj, LV_ALIGN_OUT_BOTTOM_LEFT, -90, 12);

    // if(user_data_get()->setting.language == LANG_ENGLISH)
    // {
    //     lv_obj_align(version_label, model_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    // }
    // else
    // {
    //     lv_obj_align(version_label, model_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 12);
    // }

}

static void LAYOUT_ENTER_FUNC(setting_about)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    setting_logo_img_create(parent);
    about_back_btn_create(parent);
    about_version_info_obj_create(parent);
}

static void LAYOUT_QUIT_FUNC(setting_about)
{
   
}

CREATE_LAYOUT(setting_about);