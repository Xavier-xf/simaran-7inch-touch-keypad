#include "layout_define.h"
extern lv_key_t lv_get_button_last_key(void);
typedef enum
{
    LAYOUT_SETTING_TIME_PERIOD_OBJ_CONT,
    LAYOUT_SETTING_TIME_PERIOD_OBJ_START_HOUR,
    LAYOUT_SETTING_TIME_PERIOD_OBJ_START_MIN,
    LAYOUT_SETTING_TIME_PERIOD_OBJ_START_SEC,
    LAYOUT_SETTING_TIME_PERIOD_OBJ_END_HOUR,
    LAYOUT_SETTING_TIME_PERIOD_OBJ_END_MIN,
    LAYOUT_SETTING_TIME_PERIOD_OBJ_END_SEC,

    LAYOUT_SETTING_TIME_PERIOD_TOTAL, // 总个数标识，用于遍历/参数校验
} layout_setting_time_period_module;

// 全局变量保存容器指针
static lv_obj_t *g_time_period_cont = NULL;
static int PerTime_DetKey_Flag = 0;

// 移动侦测设置界面 - Home键抬起：返回主界面
static void time_period_setting_key_up_home(lv_key_t key)
{
    printf("motion_detect Home key pressed\n");
    if (cur_layout_get() != pLAYOUT(home))
    {
        goto_layout(pLAYOUT(home));
    }
}

// 移动侦测设置界面 - ESC键抬起
static void time_period_setting_key_up_esc(lv_key_t key)
{
    printf("motion_detect ESC key pressed\n");
    if (PerTime_DetKey_Flag == 0)
    {
        goto_layout(pLAYOUT(setting_motion_detection));
    }
    else
    {
        PerTime_DetKey_Flag = 0;
        lv_group_focus_freeze(lv_group_get_default(), false);
        lv_group_set_editing(lv_group_get_default(), false);
    }
}

// 按键绑定表
static const key_binding_t time_period_setting_key_bindings[] = {
    KEY_BIND(LV_KEY_HOME, common_btn_key_down, time_period_setting_key_up_home),
    KEY_BIND(LV_KEY_ESC, common_btn_key_down, time_period_setting_key_up_esc),
    KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),
};

// 核心功能：根据timer_en状态设置按钮和滚筒容器
static void setting_time_period_enable_btn_up(lv_obj_t *obj)
{
    bool state = (lv_checkbox_get_state(obj) & LV_STATE_CHECKED) ? true : false;

    user_data_get()->motion.timer_en = state;
    user_data_save();

    if (g_time_period_cont == NULL)
    {
        printf("g_time_period_cont is NULL\n");
        return;
    }
    lv_obj_set_hidden(g_time_period_cont, !state);
    update_motion_buttons_group_state(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_TOTAL, state);
    printf("设置容器隐藏状态: %s\n", !state ? "隐藏" : "显示");
}

/***
** 函数作用：创建时间段开关按钮
***/
static void setting_time_period_enable_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_time_period_enable_btn_up);
    setting_sub_btn_base_create(NULL, 127, 84 + (50 * 0), 680, 65,
                                str_get(LAYOUT_SET_DETECTIONG_OPEN_DETECTION_ID),
                                &click_data,
                                user_data_get()->motion.timer_en, 3);
}

/***
** 函数作用：创建时间段设置容器（专门用于放置滚筒和图标）
***/
static lv_obj_t *setting_time_period_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    if (cont == NULL)
    {
        printf("time period setting cont create failed \n");
        return NULL;
    }
    lv_obj_set_id(cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_CONT);
    lv_obj_set_pos(cont, 127, 84 + (50 * 1));
    lv_obj_set_size(cont, 680, 450);

    // 保存全局指针
    g_time_period_cont = cont;

    /***** 判定滚筒是否修改 *****/
    static bool modidy = false;
    modidy = false;
    cont->user_data = &modidy;

    lv_obj_set_hidden(cont, !user_data_get()->motion.timer_en);
    printf("容器初始隐藏状态: %s\n", !user_data_get()->motion.timer_en ? "隐藏" : "显示");
    return cont;
}
static void setting_time_roller_base_change(lv_obj_t *obj, lv_event_t ev)
{
    lv_key_t current_key = lv_get_button_last_key();
    if (PerTime_DetKey_Flag == 0)
    {
        if (ev == LV_EVENT_RELEASED)
        {
            printf("setting_time_roller_base_change\n");

            if (current_key == LV_KEY_ENTER)
            {
                PerTime_DetKey_Flag = 1;
                lv_group_focus_freeze(lv_group_get_default(), true);
                lv_group_set_editing(lv_group_get_default(), true);
                // lv_obj_add_state(obj, LV_STATE_EDITED);
            }
        }
    }
    else if (PerTime_DetKey_Flag == 1)
    {
        if (ev == LV_EVENT_RELEASED)
        {
            if (current_key == LV_KEY_ENTER)
            {
                PerTime_DetKey_Flag = 0;
                lv_group_focus_freeze(lv_group_get_default(), false);
                lv_group_set_editing(lv_group_get_default(), false);
                // lv_obj_clear_state(obj, LV_STATE_EDITED);
            }
        }
    }
    if (ev == LV_EVENT_RELEASED || ev == LV_EVENT_LONG_PRESSED_REPEAT)
    {

        printf("滚筒按键事件: key=%d, PerTime_DetKey_Flag=%d\n", current_key, PerTime_DetKey_Flag);

        if (PerTime_DetKey_Flag == 1) // 编辑模式下
        {
            if (current_key == LV_KEY_NEXT)
            {
                printf("编辑模式下向下滚动\n");
                // 手动向下滚动
                int16_t current = lv_roller_get_selected(obj);
                int16_t option_cnt = lv_roller_get_option_cnt(obj);
                lv_roller_set_selected(obj, (current + 1) % option_cnt, LV_ANIM_ON);

                // 手动触发值改变事件
                if (obj->user_data != NULL)
                {
                    bool *pmodiy = (bool *)(obj->user_data);
                    if ((*pmodiy) == false)
                    {
                        *pmodiy = true;
                    }
                }
            }
            else if (current_key == LV_KEY_PREV)
            {
                printf("编辑模式下向上滚动\n");
                // 手动向上滚动
                int16_t current = lv_roller_get_selected(obj);
                int16_t option_cnt = lv_roller_get_option_cnt(obj);
                lv_roller_set_selected(obj, (current - 1 + option_cnt) % option_cnt, LV_ANIM_ON);

                // 手动触发值改变事件
                if (obj->user_data != NULL)
                {
                    bool *pmodiy = (bool *)(obj->user_data);
                    if ((*pmodiy) == false)
                    {
                        *pmodiy = true;
                    }
                }
            }
        }
    }

    if (ev == LV_EVENT_VALUE_CHANGED)
    {
        if (obj->user_data != NULL)
        {
            bool *pmodiy = (bool *)(obj->user_data);
            if ((*pmodiy) == false)
            {
                *pmodiy = true;
            }
        }
    }
}

/***
** 函数作用：创建时间滚筒（按照相对位置设置）
***/
static bool setting_time_period_hour_rooler_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_anything_create(setting_time_roller_base_change);
    // 开始时间滚筒 - 相对位置
    setting_time_roller_base(parent, 289, 40, 66, 122, 0, 23, user_data_get()->motion.start.tm_hour, &click_data, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_HOUR);
    setting_time_roller_base(parent, 377, 40, 66, 122, 0, 59, user_data_get()->motion.start.tm_min, &click_data, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_MIN);
    setting_time_roller_base(parent, 465, 40, 66, 122, 0, 59, user_data_get()->motion.start.tm_sec, &click_data, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_SEC);

    // 结束时间滚筒 - 相对位置
    setting_time_roller_base(parent, 289, 253, 66, 122, 0, 23, user_data_get()->motion.end.tm_hour, &click_data, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_HOUR);
    setting_time_roller_base(parent, 377, 253, 66, 122, 0, 59, user_data_get()->motion.end.tm_min, &click_data, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_MIN);
    setting_time_roller_base(parent, 465, 253, 66, 122, 0, 59, user_data_get()->motion.end.tm_sec, &click_data, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_SEC);

    return true;
}

/*************************************************************************
 * @brief  创建图标和分隔符（按照相对位置设置）
 **************************************************************************/
static void setting_time_period_icon_display(lv_obj_t *parent)
{
    /* 开始的图标 - 相对位置 */
    lv_obj_t *obj = lv_obj_create(parent, NULL);
    lv_obj_set_pos(obj, 161, 71);
    lv_obj_set_size(obj, 62, 62);
    static rom_bin_info img_start = rom_bin_info_get(ROM_UI_SETTING_TIME_START_PNG);
    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_start);

    /* 结束的图标 - 相对位置 */
    obj = lv_obj_create(parent, obj);
    lv_obj_set_pos(obj, 163, 285);
    static rom_bin_info img_end = rom_bin_info_get(ROM_UI_SETTING_TIME_END_PNG);
    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_end);

    /* 时间中间的冒号 - 开始时间 - 相对位置 */
    obj = lv_obj_create(parent, NULL);
    static char string_2[] = {":"};
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string_2);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
    lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4A90E2)); // 蓝色

    // 开始时间的第一个冒号（时:分）- 相对位置
    lv_obj_set_pos(obj, 348, 86);
    lv_obj_set_size(obj, 22, 27);

    // 开始时间的第二个冒号（分:秒）- 相对位置
    obj = lv_obj_create(parent, obj);
    lv_obj_set_pos(obj, 435, 86);
    lv_obj_set_size(obj, 22, 27);

    /* 时间中间的冒号 - 结束时间 - 相对位置 */
    obj = lv_obj_create(parent, obj);
    // 结束时间的第一个冒号（时:分）- 相对位置
    lv_obj_set_pos(obj, 348, 302);
    lv_obj_set_size(obj, 22, 27);

    // 结束时间的第二个冒号（分:秒）- 相对位置
    obj = lv_obj_create(parent, obj);
    lv_obj_set_pos(obj, 435, 302);
    lv_obj_set_size(obj, 22, 27);

    /* 创建分割线 - 相对位置 */
    obj = lv_obj_create(parent, NULL);
    lv_obj_set_pos(obj, 22, 208);
    lv_obj_set_size(obj, 658, 1);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x444466));
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
}

static void LAYOUT_ENTER_FUNC(setting_time_period_control)
{
    printf("============================enter_setting_time_prtiod\n");
    // 绑定当前页面的按键
    LAYOUT_KEY_BINDINGS(time_period_setting_key_bindings);
    lv_obj_t *parent = common_bg_display(lv_scr_act());

    bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
    common_bottom_btn_create(bottom_parent);                           /*按键ui显示*/

    // 创建时间段开关按钮
    setting_time_period_enable_btn_create();

    // 创建时间段设置容器（包含滚筒和图标）
    lv_obj_t *cont = setting_time_period_cont_create();

    // 创建时间滚筒和图标
    setting_time_period_hour_rooler_create(cont);
    setting_time_period_icon_display(cont);

    update_motion_buttons_group_state(cont, LAYOUT_SETTING_TIME_PERIOD_TOTAL, user_data_get()->motion.timer_en);
}

static void LAYOUT_QUIT_FUNC(setting_time_period_control)
{
    common_obj_null();
    if (g_time_period_cont == NULL)
    {
        printf("退出时容器为空，尝试查找...\n");
        g_time_period_cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_TIME_PERIOD_OBJ_CONT);
    }
    bool modify = *((bool *)g_time_period_cont->user_data);
    printf("退出时修改标志: %s\n", modify ? "已修改" : "未修改");
    if (modify == true)
    {

        if (g_time_period_cont != NULL)
        {
            char buffer[8] = {0};
            lv_obj_t *obj = NULL;

            /***** 保存开始时间 *****/
            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_HOUR);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int hour;
                sscanf(buffer, "%d", &hour);
                user_data_get()->motion.start.tm_hour = hour;
                printf("保存开始小时: %d\n", hour);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_MIN);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int min;
                sscanf(buffer, "%d", &min);
                user_data_get()->motion.start.tm_min = min;
                printf("保存开始分钟: %d\n", min);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_SEC);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int sec;
                sscanf(buffer, "%d", &sec);
                user_data_get()->motion.start.tm_sec = sec;
                printf("保存开始秒钟: %d\n", sec);
            }

            /***** 保存结束时间 *****/
            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_HOUR);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int hour;
                sscanf(buffer, "%d", &hour);
                user_data_get()->motion.end.tm_hour = hour;
                printf("保存结束小时: %d\n", hour);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_MIN);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int min;
                sscanf(buffer, "%d", &min);
                user_data_get()->motion.end.tm_min = min;
                printf("保存结束分钟: %d\n", min);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_SEC);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int sec;
                sscanf(buffer, "%d", &sec);
                user_data_get()->motion.end.tm_sec = sec;
                printf("保存结束秒钟: %d\n", sec);
            }

            user_data_save();
            printf("用户数据已保存\n");
        }
        else
        {
            printf("退出时未找到容器\n");
        }
    }

    // 重置全局变量
    g_time_period_cont = NULL;
}

CREATE_LAYOUT(setting_time_period_control);