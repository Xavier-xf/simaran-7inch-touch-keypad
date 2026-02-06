#include "layout_define.h"

#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT 0X01
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT 0X02
#define SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_NUM 0X03

#define SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT 0X04

#define SETTING_PASSWORD_OBJ_ID_MSG_CONT 0X05
#define SETTING_PASSWORD_OBJ_ID_APPLE_BTN 0X06

typedef enum
{
    SETTING_PASSWORD_HOME_BTN_ID,
    SETTING_PASSWORD_TOTAL_BTN,
} setting_btn_module;

static custom_area setting_btn_area[SETTING_PASSWORD_TOTAL_BTN] =
    {
        {24, 12, 54, 54},
};

static void setting_password_home_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}
// 创建home按钮
static void setting_password_home_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_password_home_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
    common_img_btn_create(parent, setting_btn_area[SETTING_PASSWORD_HOME_BTN_ID], NULL, &btn_data, &info, SETTING_PASSWORD_HOME_BTN_ID,
                          setting_btn_area[SETTING_PASSWORD_HOME_BTN_ID], setting_btn_area[SETTING_PASSWORD_HOME_BTN_ID]);
}

/***
** 日期: 2022-05-09 11:54
** 作者: leo.liu
** 函数作用：复位输入框
** 返回参数说明：
***/
static void setting_password_not_match_task(lv_task_t *task_t)
{
    lv_obj_t *label = (lv_obj_t *)task_t->user_data;
    lv_label_set_text(label, str_get(SETTING_PASSWORD_LANG_ID_ENTER_PWD));
    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT);
    setting_password_input_reset(parent, 4);

    lv_task_del(task_t);
}

/***
** 日期: 2022-05-09 15:19
** 作者: leo.liu
** 函数作用：检测密码是否很简单
** 返回参数说明：
***/
static int setting_password_check_passowrd_easy(lv_obj_t *parent)
{
    char easy_password[3][5] = {{"0000"}, {"1234"}, {"4321"}};
    char buffer[10] = {0};
    memset(buffer, 0, sizeof(buffer));
    setting_password_get_string(parent, buffer);

    char first_num[5] = {0};
    char second_num[5] = {0};
    strncpy(first_num, buffer, 4);
    strncpy(second_num, buffer + 4, 4);

    for (int i = 0; i < 3; i++)
    {
        if (strncmp(easy_password[i], buffer, 4) == 0)
        {
            return 1;
        }
    }

    if (first_num == second_num)
    {
        return 2;
    }

    return 0;
}

/***
** 日期: 2022-05-09 14:16
** 作者: leo.liu
** 函数作用：输入新密码按键事件
** 返回参数说明：
***/
static void setting_password_keyborad_new_password_btn_up(lv_obj_t *obj, lv_event_t ev)
{
    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);
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
        setting_password_del_string(parent, 8);
        if (setting_password_edit_index_get(parent) == 7)
        {
            lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
            lv_obj_set_state(btn, LV_STATE_DISABLED);
        }
    }
    else if (setting_password_input_string(parent, txt, 8) == true)
    {
        if ((setting_password_edit_index_get(parent) == 8))
        {
            int pwd_type = setting_password_check_passowrd_easy(parent);
            if (pwd_type == 0)
            {
                lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
                lv_obj_set_state(btn, LV_STATE_DEFAULT);
            }
            else if (pwd_type == 1)
            {
                msg_toast(str_get(THE_PWD_TOO_SIMPLE), 1, 0, 0);
            }
            else
            {
                msg_toast(str_get(THE_PWD_NOT_MATCH), 2, 0, 0);
            }
        }
    }
}

/***
** 日期: 2022-05-09 11:55
** 作者: leo.liu
** 函数作用：输入键盘事件
** 返回参数说明：
***/
static void setting_password_keyboard_enter_password_btn_up(lv_obj_t *obj, lv_event_t ev)
{
    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT);
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
            printf("input:%s, password:%c%c%c%c \n", buffer, user_data_get()->etc.password[0], user_data_get()->etc.password[1], user_data_get()->etc.password[2], user_data_get()->etc.password[3]);

            if (strncmp(buffer, user_data_get()->etc.password, 4) == 0)
            {
                printf("input password success ! \n");
                lv_obj_t *new_parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);
                lv_obj_set_hidden(parent, true);
                lv_obj_set_hidden(new_parent, false);

                obj_click_data *click_data = (obj_click_data *)obj->clict_data;
                click_data->anything_func = setting_password_keyborad_new_password_btn_up;
                setting_password_input_reset(new_parent, 8);

                lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
                lv_obj_set_state(btn, LV_STATE_DISABLED);
                lv_obj_set_hidden(btn, false);
            }
            else
            {
                printf("input password failed ! \n");
                msg_toast(str_get(THE_PWD_NOT_MATCH), 2, 0, 0);
                lv_obj_t *label = lv_obj_get_child_form_id(parent, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT);
                lv_label_set_text(label, str_get(THE_PWD_NOT_MATCH));
                lv_layout_task_create(setting_password_not_match_task, 1000, LV_TASK_PRIO_MID, label);
            }
        }
    }
}
/***
** 日期: 2022-05-07 14:05
** 作者: leo.liu
** 函数作用：数字键盘创建
** 返回参数说明：
***/
static void setting_password_keyboard_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_anything_create(setting_password_keyboard_enter_password_btn_up);

    click_data.anything_func = setting_password_keyboard_enter_password_btn_up;
    setting_passowrd_num_keyboard_create(NULL, 152, 80, 302, 420, &click_data);
}

/***
** 日期: 2022-05-07 16:04
** 作者: leo.liu
** 函数作用：创建输入密码容器
** 返回参数说明：
***/
static void setting_password_enter_pwd_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(cont, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_CONT);

    lv_obj_set_size(cont, 312, 167);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_RIGHT_MID, -80, 0);

    lv_obj_t *obj = lv_label_create(cont, NULL);
    lv_obj_set_id(obj, SETTING_PASSWORD_OBJ_ID_ENTER_PASSWORD_TXT);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_CROP);
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 312, 56);
    lv_label_set_align(obj, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_label_set_text(obj, str_get(SETTING_PASSWORD_LANG_ID_ENTER_PWD));
    setting_password_input_label_create(cont, 0, 91, 1);
}

/***
** 日期: 2022-05-09 11:59
** 作者: leo.liu
** 函数作用：输入新密码
** 返回参数说明：
***/
static void setting_password_input_new_pwd_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(cont, SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);

    lv_obj_set_size(cont, 400, 388);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_RIGHT_MID, -80, -20);
    lv_obj_set_hidden(cont, true);

    lv_obj_t *obj = lv_obj_create(cont, NULL);
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 400, 56);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_TOP_LEFT);
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(SETTING_PASSWORD_LANG_ID_PWD_RESET));

    obj = lv_obj_create(cont, obj);
    lv_obj_set_x(obj, 80);
    lv_obj_set_y(obj, 44);
    lv_obj_set_height(obj, 40);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(SETTING_PASSWORD_LANG_ID_NEW_PWD));

    obj = lv_obj_create(cont, obj);
    lv_obj_set_x(obj, 80);
    lv_obj_set_y(obj, 254);
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(SETTING_PASSWORD_LANG_ID_CONFIRM_PWD));

    setting_password_input_label_create(cont, 0, 142, 2);
}

/***
** 日期: 2022-05-09 14:29
** 作者: leo.liu
** 函数作用：创建一个满屏容器
** 返回参数说明：
***/
static void setting_password_msg_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_size(cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

    lv_obj_set_id(cont, SETTING_PASSWORD_OBJ_ID_MSG_CONT);
    lv_obj_set_hidden(cont, true);

    lv_obj_t *obj = lv_obj_create(cont, NULL);
    lv_obj_set_pos(obj, 366, 321);
    lv_obj_set_size(obj, 550, 160);
    lv_obj_align(obj, cont, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_local_radius(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(THE_PWD_NOT_MATCH));
}

static void lv_setting_password_msg_task(lv_task_t *task_t)
{
    lv_obj_t *cont = (lv_obj_t *)task_t->user_data;
    lv_obj_set_hidden(cont, true);
    lv_task_del(task_t);
}

static void setting_password_apple_btn_up(lv_obj_t *obj)
{
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_MSG_CONT);
    if (lv_obj_get_hidden(cont) == false)
    {
        return;
    }

    lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), SETTING_PASSWORD_OBJ_ID_INPUT_NEW_PASSWORD_CONT);
    if (parent == NULL)
    {
        return;
    }

    if (setting_password_edit_index_get(parent) == 8)
    {
        char buffer[10] = {0};
        setting_password_get_string(parent, buffer);
        if (strncmp(&buffer[0], &buffer[4], 4) == 0)
        {
            strncpy(user_data_get()->etc.password, buffer, 4);
            user_data_save();
            goto_layout(pLAYOUT(setting));
        }
        else
        {
            lv_obj_set_hidden(cont, false);
            lv_layout_task_create(lv_setting_password_msg_task, 1000, LV_TASK_PRIO_MID, cont);
        }
    }
}
/***
** 日期: 2022-05-09 14:42
** 作者: leo.liu
** 函数作用：创建应用按钮
** 返回参数说明：
***/
static void setting_password_apply_btn_create(void)
{
    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_id(btn, SETTING_PASSWORD_OBJ_ID_APPLE_BTN);
    lv_obj_set_size(btn, LV_HOR_RES_MAX, 75);
    lv_obj_align(btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_hidden(btn, true);

    static obj_click_data click_data = obj_click_data_up_create(setting_password_apple_btn_up);
    obj_click_event_listen(btn, &click_data);

    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1C315B));
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x584C36));
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xEBD1A1));

    lv_obj_set_style_local_value_font(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
    lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x888888));
    lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(SETTING_PASSWORD_LANG_ID_APPLE));

    lv_obj_set_state(btn, LV_STATE_DISABLED);
}

static void LAYOUT_ENTER_FUNC(setting_password)
{
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    setting_password_home_btn_create(parent); /*home显示*/

    setting_password_keyboard_btn_create();
    setting_password_enter_pwd_cont_create();
    setting_password_input_new_pwd_cont_create();
    setting_password_apply_btn_create();
    setting_password_msg_cont_create();
}

static void LAYOUT_QUIT_FUNC(setting_password)
{
}

CREATE_LAYOUT(setting_password);