/*******************************************************************
 * @Descripttion   : 
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-10 15:29
 * @LastEditTime   : 2022-11-28 08:34
*******************************************************************/
#include "layout_define.h"


static void reset_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}
static void reset_back_btn_create(lv_obj_t *parent)
{
    setting_back_btn_create(parent, reset_back_btn_up);
}


static void reset_no_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}
static void reset_yes_btn_up(lv_obj_t *obj)
{
    uint8_t lang = user_data_get()->setting.language;
    user_data_reset();
    if(user_data_get()->setting.language != lang)
    {
        /***** 設置字庫 *****/
        extern void lv_ft_font_set_type(int type);
        lv_ft_font_set_type(user_data_get()->setting.language);
        
        extern void lv_font_afresh_init(void);
        lv_font_afresh_init();
    }
    goto_layout(pLAYOUT(setting));
}
//创建reset选择按钮
static void reset_select_btn_create(lv_obj_t * parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(reset_no_btn_up);
    static obj_click_data btn_data1 = obj_click_data_up_create(reset_yes_btn_up);
    message_box_create(lv_scr_act(), str_get(LAYOUT_RESET_LANG_FACTORY_ID), &btn_data1, &btn_data);
}

static void LAYOUT_ENTER_FUNC(setting_reset)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    setting_logo_img_create(parent);
    reset_back_btn_create(parent);
    reset_select_btn_create(parent);
}

static void LAYOUT_QUIT_FUNC(setting_reset)
{
   
}

CREATE_LAYOUT(setting_reset);