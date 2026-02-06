/*******************************************************************
 * @Descripttion   : 
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-22 18:12
 * @LastEditTime   : 2022-11-24 14:29
*******************************************************************/
#include "layout_define.h"



void dev_touch_func(lv_obj_t *obj)
{
    lv_indev_t * indev = lv_indev_get_act();
    lv_point_t p;

    if(lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER)
    {
        lv_indev_get_point(indev, &p);
        // LOG_B_WHITE("(%d, %d)\n", p.x, p.y);

        lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
        lv_obj_set_size(cont, 1, 1);
        lv_obj_set_pos(cont, p.x, p.y);
        lv_obj_set_click(cont, false);
        lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
        lv_obj_set_style_local_bg_color(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

        lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(obj, 20, 1);
        lv_obj_align(obj, cont, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_click(obj, false);
        lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
        lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));

        obj = lv_obj_create(lv_scr_act(), obj);
        lv_obj_set_size(obj, 1, 20);
        lv_obj_set_click(obj, false);
        lv_obj_align(obj, cont, LV_ALIGN_CENTER, 0, 0);

    }
    
}







extern void obj_screen_click_event_register(void (*callback)(lv_obj_t*));


static void LAYOUT_ENTER_FUNC(click)
{
    // click_btn_creant();
    obj_screen_click_event_register(dev_touch_func) ;

}

static void LAYOUT_QUIT_FUNC(click)
{
}

CREATE_LAYOUT(click);