#include "layout_define.h"
typedef enum
{
    security_scr_act_obj_id_cont_security,
    security_scr_act_obj_id_cont_password,
    security_scr_act_obj_id_msgdialog,
} security_scr_act_obj_id;

typedef enum
{
    security_cont_security_obj_id_auto_btn,
    security_cont_security_obj_id_sensor_1_cont,
    security_cont_security_obj_id_sensor_2_cont,
    security_cont_security_obj_id_work_txt,
    security_cont_security_obj_id_apple_btn
} security_cont_security_obj_id;

typedef enum
{
    security_sensor_cont_obj_id_checkbox,
} security_sensor_cont_obj_id;

typedef enum
{
    security_cont_password_cont_obj_id_enter_pwd_cont,
    security_cont_password_cont_obj_id_enter_pwd_cont_to_label,
} security_cont_password_cont_obj_id;

/***
**   日期:2022-05-26 14:02:27
**   作者: leo.liu
**   函数作用：创建设置报警的容器
**   参数说明:
***/
static lv_obj_t *security_security_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(cont, security_scr_act_obj_id_cont_security);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_size(cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    return cont;
}

static void security_password_cancel_btn_up(void)
{
    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_security);
    lv_obj_set_hidden(parent, false);
    parent = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_password);
    lv_obj_set_hidden(parent, true);
    setting_msgdialog_msg_bg_delete(security_scr_act_obj_id_msgdialog);
}

static void security_cancel_btn_up(lv_obj_t *obj)
{
    lv_obj_t *pwd_cont = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_password);

    if (lv_obj_get_hidden(pwd_cont))
    {
        if (security_enter_flag == 1)
        {
            printf("goto etc \n");
            goto_layout(pLAYOUT(setting));
        }
        else
        {
            printf("back home \n");
            goto_layout(pLAYOUT(home));
        }
    }
    else
    {
        printf("back \n");
        security_password_cancel_btn_up();
    }
}

/***
**   日期:2022-05-26 14:03:48
**   作者: leo.liu
**   函数作用：创建头部文本显示
**   参数说明:
***/
static void security_head_txt_obj_create(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent, NULL);
    if (obj == NULL)
    {
        printf("create security failed \n");
        return;
    }

    // lv_obj_set_pos(obj, 543, 23);
    lv_obj_set_size(obj, 118, 47);
    lv_obj_align(obj, parent, LV_ALIGN_IN_TOP_MID, 0, 24);

    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SECURITY_LANGUAGE_ID_SECURITY));
    lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
}

static void security_alarm_list_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(alarm_list));
}
/***
**   日期:2022-05-26 14:04:12
**   作者: leo.liu
**   函数作用：创建头部alarmlist 按钮
**   参数说明:
***/
static void security_alarm_list_btn_create(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent, NULL);
    lv_obj_set_size(obj, 58, 58);
    lv_obj_align(obj, parent, LV_ALIGN_IN_TOP_RIGHT, -16, 20);
    static rom_bin_info img = rom_bin_info_get(ROM_UI_SECURITY_LIST_PNG);
    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img);
    static obj_click_data click_data = obj_click_data_up_create(security_alarm_list_btn_up);
    obj_click_event_listen(obj, &click_data);
}

static void security_auto_record_btn_up(lv_obj_t *obj)
{
    bool at = user_data_get()->alarm.auto_record;
    at = at == false ? true : false;

    lv_state_t state = at == true ? LV_STATE_CHECKED : LV_STATE_DEFAULT;
    lv_obj_set_state(obj, state);

    user_data_get()->alarm.auto_record = at;
    user_data_save();
}
/***
**   日期:2022-05-26 14:04:47
**   作者: leo.liu
**   函数作用：创建自动记录按钮
**   参数说明:
***/
static void security_auto_record_btn_create(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent, NULL);
    lv_obj_set_id(obj, security_cont_security_obj_id_auto_btn);
    // lv_obj_set_pos(obj, 872, 81);
    lv_obj_set_size(obj, 150, 47);
    lv_obj_align(obj, parent, LV_ALIGN_IN_TOP_RIGHT, -92, 110);
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3a3f47));
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, lv_color_hex(0xff6b64));
    lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 29);
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SECURITY_LANGUAGE_ID_AUTO_REC));
    lv_state_t state = user_data_get()->alarm.auto_record == true ? LV_STATE_CHECKED : LV_STATE_DEFAULT;
    lv_obj_set_state(obj, state);

    static obj_click_data click_data = obj_click_data_up_create(security_auto_record_btn_up);
    obj_click_event_listen(obj, &click_data);

    if ((user_data_get()->alarm.alarm_1_enable == true) || (user_data_get()->alarm.alarm_2_enable == true))
    {
        lv_obj_set_click(obj, false);
    }
}

/***
**   日期:2022-05-26 15:57:46
**   作者: leo.liu
**   函数作用：sensor1容器显示
**   参数说明:
***/
static void security_sensor_1_cont_display(lv_obj_t *cont)
{
    bool en = user_data_get()->alarm.alarm_1_enable;
    if (en == true)
    {
        lv_obj_set_state(cont, LV_STATE_CHECKED);
        lv_obj_t *checkbox = lv_obj_get_child_form_id(cont, security_sensor_cont_obj_id_checkbox);
        lv_checkbox_set_checked(checkbox, true);
        lv_obj_set_click(cont, false);
    }
    else
    {
        en = user_data_get()->alarm.alarm_2_enable;
        if (en == true)
        {
            lv_obj_set_click(cont, false);
        }
        lv_obj_set_state(cont, LV_STATE_DEFAULT);
    }
}

static void layout_security_msg_confirm_btn_up(lv_obj_t *obj)
{
    setting_msgdialog_msg_bg_delete(security_scr_act_obj_id_msgdialog);
}

/***
**   日期:2022-05-26 16:57:48
**   作者: leo.liu
**   函数作用：创建消息框
**   参数说明:
***/
static bool security_msg_dialog_create(const char *string)
{
    lv_obj_t *dialog = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_msgdialog);
    if (dialog != NULL)
    {
        return false;
    }
    dialog = setting_msgdialog_msg_bg_create(security_scr_act_obj_id_msgdialog, 460, 200);
    setting_msgdialog_msg_confirm_btn_create(dialog, layout_security_msg_confirm_btn_up);
    setting_msgdialog_msg_create(dialog, string);
    return true;
}

static lv_obj_t *security_sensor_bg_create(lv_obj_t *obj)
{
    lv_obj_t *cont = lv_cont_create(obj, NULL);
    lv_obj_set_size(cont, 880, 469);
    lv_obj_align(cont, obj, LV_ALIGN_CENTER, 0, 25);

    lv_obj_set_style_local_radius(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 16);
    lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1C1C1C));

    return cont;
}

static void security_sensor_cont_up(lv_obj_t *obj)
{
    lv_obj_t *checkbox = lv_obj_get_child_form_id(obj, security_sensor_cont_obj_id_checkbox);
    if (lv_checkbox_is_checked(checkbox) == true)
    {
        lv_checkbox_set_checked(checkbox, false);
        lv_obj_set_state(obj, LV_STATE_DEFAULT);
        return;
    }
    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_security);
    lv_obj_t *sensor1_cont = lv_obj_get_child_form_id(parent, security_cont_security_obj_id_sensor_1_cont);
    lv_obj_t *sensor2_cont = lv_obj_get_child_form_id(parent, security_cont_security_obj_id_sensor_2_cont);

    if (obj == sensor1_cont)
    {
        if (sercurity_sensor1_normal_check() == true)
        {
            lv_checkbox_set_checked(checkbox, true);
            lv_obj_set_state(obj, LV_STATE_CHECKED);
        }
        else
        {
            security_msg_dialog_create(str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_1_ERROR));
        }
    }
    else if (obj == sensor2_cont)
    {
        if (sercurity_sensor2_normal_check() == true)
        {
            lv_checkbox_set_checked(checkbox, true);
            lv_obj_set_state(obj, LV_STATE_CHECKED);
        }
        else
        {
            security_msg_dialog_create(str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_2_ERROR));
        }
    }
}
/***
**   日期:2022-05-26 14:05:12
**   作者: leo.liu
**   函数作用：创建sensor 1容器
**   参数说明:
***/
static lv_obj_t *security_sensor_1_cont_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_id(cont, security_cont_security_obj_id_sensor_1_cont);
    lv_obj_set_size(cont, 281, 264);
    lv_obj_align(cont, parent, LV_ALIGN_CENTER, -162, 0);
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 16);
    lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3a3f47));
    // lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_CHECKED, lv_color_hex(0x0094ff));

    static rom_bin_info img_online = rom_bin_info_get(ROM_UI_SECURITY_ALARM_ONLINE_PNG);
    static rom_bin_info img_offline = rom_bin_info_get(ROM_UI_SECURITY_ALARM_OFFLINE_PNG);
    static rom_bin_info img_select = rom_bin_info_get(ROM_UI_SECURITY_ALARM_SELECT_PNG);

    lv_obj_set_style_local_pattern_image(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, sercurity_sensor1_normal_check() ? &img_online : &img_offline);
    lv_obj_set_style_local_pattern_image(cont, LV_CONT_PART_MAIN, LV_STATE_CHECKED, &img_select);

    lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_1));
    lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_BOTTOM_MID);
    lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -10);
    static obj_click_data click_data = obj_click_data_up_create(security_sensor_cont_up);
    obj_click_event_listen(cont, &click_data);
    return cont;
}

/***
**   日期:2022-05-26 14:05:56
**   作者: leo.liu
**   函数作用：创建sensor1 checkbox
**   参数说明:
***/
static void security_sensor1_checkbox_creaet(lv_obj_t *parent)
{
    lv_obj_t *checkbox = lv_checkbox_create(parent, NULL);
    // lv_obj_set_hidden(checkbox, true);
    lv_obj_set_id(checkbox, security_sensor_cont_obj_id_checkbox);
    lv_obj_set_pos(checkbox, 180, 9);
    lv_obj_set_size(checkbox, 38, 38);
    lv_obj_set_click(checkbox, false);
    lv_checkbox_set_text(checkbox, "");
}

/***
**   日期:2022-05-26 15:57:46
**   作者: leo.liu
**   函数作用：sensor2容器显示
**   参数说明:
***/
static void security_sensor_2_cont_display(lv_obj_t *cont)
{
    bool en = user_data_get()->alarm.alarm_2_enable;
    if (en == true)
    {
        lv_obj_set_state(cont, LV_STATE_CHECKED);
        lv_obj_t *checkbox = lv_obj_get_child_form_id(cont, security_sensor_cont_obj_id_checkbox);
        lv_checkbox_set_checked(checkbox, true);
        lv_obj_set_click(cont, false);
    }
    else
    {
        en = user_data_get()->alarm.alarm_1_enable;
        if (en == true)
        {
            lv_obj_set_click(cont, false);
        }
        lv_obj_set_state(cont, LV_STATE_DEFAULT);
    }
}
/***
**   日期:2022-05-26 14:06:27
**   作者: leo.liu
**   函数作用：创建sensor2 容器
**   参数说明:
***/
static lv_obj_t *security_sensor_2_cont_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_id(cont, security_cont_security_obj_id_sensor_2_cont);
    lv_obj_set_size(cont, 281, 264);
    lv_obj_align(cont, parent, LV_ALIGN_CENTER, 162, 0);
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 16);
    lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3a3f47));
    // lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_CHECKED, lv_color_hex(0x0094ff));

    static rom_bin_info img_online = rom_bin_info_get(ROM_UI_SECURITY_ALARM_ONLINE_PNG);
    static rom_bin_info img_offline = rom_bin_info_get(ROM_UI_SECURITY_ALARM_OFFLINE_PNG);
    static rom_bin_info img_select = rom_bin_info_get(ROM_UI_SECURITY_ALARM_SELECT_PNG);

    lv_obj_set_style_local_pattern_image(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, sercurity_sensor2_normal_check() ? &img_online : &img_offline);
    lv_obj_set_style_local_pattern_image(cont, LV_CONT_PART_MAIN, LV_STATE_CHECKED, &img_select);

    lv_obj_set_style_local_value_str(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_2));
    lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_BOTTOM_MID);
    lv_obj_set_style_local_value_ofs_y(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -10);
    static obj_click_data click_data = obj_click_data_up_create(security_sensor_cont_up);
    obj_click_event_listen(cont, &click_data);
    return cont;
}
/***
**   日期:2022-05-26 14:05:56
**   作者: leo.liu
**   函数作用：创建sensor2 checkbox
**   参数说明:
***/
static void security_sensor2_checkbox_creaet(lv_obj_t *parent)
{
    lv_obj_t *checkbox = lv_checkbox_create(parent, NULL);
    lv_obj_set_id(checkbox, security_sensor_cont_obj_id_checkbox);
    lv_obj_set_pos(checkbox, 180, 9);
    lv_obj_set_size(checkbox, 38, 38);
    lv_obj_set_click(checkbox, false);
    lv_checkbox_set_text(checkbox, "");
}
/***
**   日期:2022-05-26 14:07:11
**   作者: leo.liu
**   函数作用：创建提示工作中
**   参数说明:
***/
static void security_work_txt_create(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent, NULL);
    lv_obj_set_id(obj, security_cont_security_obj_id_work_txt);
    // lv_obj_set_pos(obj, 370, 458);
    lv_obj_set_size(obj, 355, 47);
    lv_obj_align(obj, parent, LV_ALIGN_IN_TOP_MID, 0, 110);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_WORKING));
    lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xff6666));
    lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    if ((user_data_get()->alarm.alarm_1_enable == true) || (user_data_get()->alarm.alarm_2_enable == true))
    {
        lv_obj_set_hidden(obj, false);
    }
    else
    {
        lv_obj_set_hidden(obj, true);
    }
}

static void security_apple_btn_display(void)
{
    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_security);
    lv_obj_t *obj = lv_obj_get_child_form_id(parent, security_cont_security_obj_id_apple_btn);
    if ((user_data_get()->alarm.alarm_1_enable == true) || (user_data_get()->alarm.alarm_2_enable == true))
    {
        lv_obj_set_state(obj, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_set_state(obj, LV_STATE_DEFAULT);
    }
}
static void security_apple_btn_up(lv_obj_t *obj)
{
    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_security);
    lv_obj_t *cont_1 = lv_obj_get_child_form_id(parent, security_cont_security_obj_id_sensor_1_cont);
    lv_obj_t *chekbox1 = lv_obj_get_child_form_id(cont_1, security_sensor_cont_obj_id_checkbox);
    lv_obj_t *cont_2 = lv_obj_get_child_form_id(parent, security_cont_security_obj_id_sensor_2_cont);
    lv_obj_t *chekbox2 = lv_obj_get_child_form_id(cont_2, security_sensor_cont_obj_id_checkbox);

    if ((user_data_get()->alarm.alarm_1_enable == true) || (user_data_get()->alarm.alarm_2_enable == true))
    {
        /*****  进入输入密码界面 *****/
        lv_obj_set_hidden(parent, true);
        parent = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_password);
        lv_obj_set_hidden(parent, false);
        return;
    }
    char error = 0;
    if ((lv_checkbox_is_checked(chekbox1) == true) && (sercurity_sensor1_normal_check() == false))
    {
        error = 0x01;
    }

    if ((lv_checkbox_is_checked(chekbox2) == true) && (sercurity_sensor2_normal_check() == false))
    {
        error |= 0x02;
    }

    if (error)
    {
        if (error == 0x01)
        {
            security_msg_dialog_create(str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_1_ERROR));
        }
        else if (error == 0x02)
        {
            security_msg_dialog_create(str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_2_ERROR));
        }
        else
        {
            security_msg_dialog_create(str_get(LAYOUT_SECURITY_LANGUAGE_ID_SENSOR_1_AND_2_ERROR));
        }
    }
    else
    {
        if ((lv_checkbox_is_checked(chekbox1) == false) && (lv_checkbox_is_checked(chekbox2) == false))
        {
            msg_toast(str_get(SENSOR_NOT_SELECTED), 1, 0, 0);
            return;
        }

        if (lv_checkbox_is_checked(chekbox1) == true)
        {
            user_data_get()->alarm.alarm_1_enable = true;
        }
        if (lv_checkbox_is_checked(chekbox2) == true)
        {
            user_data_get()->alarm.alarm_2_enable = true;
        }

        user_data_save();
        security_sensor_1_cont_display(cont_1);
        security_sensor_2_cont_display(cont_2);
        security_apple_btn_display();
        lv_obj_t *obj = lv_obj_get_child_form_id(parent, security_cont_security_obj_id_work_txt);
        lv_obj_set_hidden(obj, false);

        obj = lv_obj_get_child_form_id(parent, security_cont_security_obj_id_auto_btn);
        lv_obj_set_click(obj, false);
    }
}
/***
**   日期:2022-05-26 15:50:13
**   作者: leo.liu
**   函数作用：创建应用按钮
**   参数说明:
***/
static void security_apply_btn_create(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent, NULL);
    lv_obj_set_id(obj, security_cont_security_obj_id_apple_btn);
    lv_obj_set_size(obj, LV_VER_RES_MAX, 54);
    lv_obj_align(obj, parent, LV_ALIGN_IN_BOTTOM_MID, 0, -66);
    lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_SECURITY_LANGUAGE_ID_EXE));
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0081DC));
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, str_get(LAYOUT_SECURITY_LANGUAGE_ID_END));
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_CHECKED, lv_color_hex(0x47494a));
    lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

    static obj_click_data click_data = obj_click_data_up_create(security_apple_btn_up);
    obj_click_event_listen(obj, &click_data);

    security_apple_btn_display();
}

/***
**   日期:2022-05-26 14:07:45
**   作者: leo.liu
**   函数作用：创建密码输入容器
**   参数说明:
***/
static lv_obj_t *security_password_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(cont, security_scr_act_obj_id_cont_password);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_size(cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_hidden(cont, true);

    lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);

    setting_cancel_btn_create(security_cancel_btn_up);
    return cont;
}

/***
**   日期:2022-05-26 17:32:11
**   作者: leo.liu
**   函数作用：数字键盘
**   参数说明:
***/
static void security_password_keyboard_password_btn_up(lv_obj_t *obj, lv_state_t ev)
{
    lv_obj_t *dialog = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_msgdialog);
    if (dialog != NULL)
    {
        return;
    }

    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), security_scr_act_obj_id_cont_password);
    parent = lv_obj_get_child_form_id(parent, security_cont_password_cont_obj_id_enter_pwd_cont);
    if (parent == NULL)
    {
        return;
    }

    if (ev != LV_EVENT_VALUE_CHANGED)
    {
        return;
    }

    const char *txt = lv_btnmatrix_get_active_btn_text(obj);
    if (txt[0] == ' ')
    {
        setting_password_del_string(parent, 4);
    }
    else if (setting_password_input_string(parent, txt, 4) == true)
    {
        if (setting_password_edit_index_get(parent) == 4)
        {
            char buffer[5] = {0};
            setting_password_get_string(parent, buffer);
            printf("password:%c%c%c%c \n", user_data_get()->etc.password[0], user_data_get()->etc.password[1], user_data_get()->etc.password[2], user_data_get()->etc.password[3]);
            if (strncmp(buffer, user_data_get()->etc.password, 4) == 0)
            {
                printf("input password success ! \n");
                user_data_get()->alarm.alarm_1_enable = false;
                user_data_get()->alarm.alarm_2_enable = false;
                user_data_save();
                goto_layout(pLAYOUT(security));
            }
            else
            {
                printf("input password failed ! \n");
                security_msg_dialog_create(str_get(THE_PWD_NOT_MATCH));
            }
        }
    }
}

/***
**   日期:2022-05-26 14:08:07
**   作者: leo.liu
**   函数作用：创建数字键盘
**   参数说明:
***/
static void security_password_keyboard_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_anything_create(security_password_keyboard_password_btn_up);
    setting_passowrd_num_keyboard_create(parent, 152, 100, 302, 420, &click_data);
}
/***
** 日期: 2022-05-07 16:04
** 作者: leo.liu
** 函数作用：创建输入密码容器
** 返回参数说明：
***/
static void security_password_enter_pwd_cont_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_id(cont, security_cont_password_cont_obj_id_enter_pwd_cont);

    lv_obj_set_size(cont, 312, 167);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_RIGHT_MID, -80, 0);

    lv_obj_t *obj = lv_label_create(cont, NULL);
    lv_obj_set_id(obj, security_cont_password_cont_obj_id_enter_pwd_cont_to_label);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_CROP);
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 312, 56);
    lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_label_set_text(obj, str_get(SETTING_PASSWORD_LANG_ID_ENTER_PWD));
    setting_password_input_label_create(cont, 0, 91, 1);
}

static void LAYOUT_ENTER_FUNC(security)
{
    printf("enter\n");
    lv_obj_t *cont = security_security_cont_create();
    security_sensor_bg_create(cont);
    setting_cancel_btn_create(security_cancel_btn_up);

    security_head_txt_obj_create(cont);
    security_alarm_list_btn_create(cont);
    security_auto_record_btn_create(cont);

    lv_obj_t *sensor_cont = security_sensor_1_cont_create(cont);
    security_sensor1_checkbox_creaet(sensor_cont);
    security_sensor_1_cont_display(sensor_cont);

    sensor_cont = security_sensor_2_cont_create(cont);
    security_sensor2_checkbox_creaet(sensor_cont);
    security_sensor_2_cont_display(sensor_cont);

    security_work_txt_create(cont);
    security_apply_btn_create(cont);

    cont = security_password_cont_create();
    security_password_keyboard_create(cont);
    security_password_enter_pwd_cont_create(cont);

    if (!sercurity_sensor1_normal_check() && !sercurity_sensor2_normal_check())
    {
        msg_toast(str_get(PLEASE_CHECK_SENSOR_STATE), 2, 0, 0);
    }
}

static void LAYOUT_QUIT_FUNC(security)
{
}

CREATE_LAYOUT(security);