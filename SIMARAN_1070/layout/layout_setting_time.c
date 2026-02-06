#include "layout_define.h"

// 全局临时变量
static struct tm temp_tm;             // 临时时间结构体（时/分存储）
static struct date temp_date;         // 临时日期结构体（年/月/日存储）
static bool time_change_flag = false; // 时间修改标记（退出时判断是否保存）
// 时间显示字符串缓冲区
static char str_year[5] = {0};
static char str_month[5] = {0};
static char str_day[5] = {0};
static char str_hour[5] = {0};
static char str_min[5] = {0};

static int Time_DetKey_Flag = 0;

#define SETTING_TIME_YEAR_ID 0x05
#define SETTING_TIME_MONTH_ID 0x06
#define SETTING_TIME_DAY_ID 0x07
#define SETTING_TIME_HOUR_ID 0x08
#define SETTING_TIME_MINUTE_ID 0x09
// 日历切换开关ID
#define SETTING_TIME_CALENDAR_SW_ID 0x10

static void setting_time_key_up_home(lv_key_t key);
static void setting_time_key_up_esc(lv_key_t key);

static void setting_time_next_key_down(lv_key_t key);
static void setting_time_prev_key_down(lv_key_t key);
static void setting_time_next_key_long_down(lv_key_t key);
static void setting_time_prev_key_long_down(lv_key_t key);

// 按键绑定表
static const key_binding_t setting_time_key_bindings[] = {
    KEY_BIND(LV_KEY_HOME, common_btn_key_down, setting_time_key_up_home),
    KEY_BIND(LV_KEY_ESC, common_btn_key_down, setting_time_key_up_esc),
    KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),
};

// 设置time界面 - Home键抬起：返回主界面
static void setting_time_key_up_home(lv_key_t key)
{
    printf("motion_detect Home key pressed\n");
    if (cur_layout_get() != pLAYOUT(home))
    {
        goto_layout(pLAYOUT(home));
    }
}

// 设置time界面 - ESC键抬起
static void setting_time_key_up_esc(lv_key_t key)
{
    printf("motion_detect ESC key pressed\n");
    if (Time_DetKey_Flag == 0)
    {
        goto_layout(pLAYOUT(home));
    }
    else
    {
        Time_DetKey_Flag = 0;
        common_btn_triangle_hidden();
        lv_group_focus_freeze(lv_group_get_default(), false);
        LAYOUT_KEY_BINDINGS(setting_time_key_bindings);
    }
}
// 设置time界面 - ENTER键抬起
static void motion_detect_setting_key_up_enter(lv_key_t key)
{

    if (Time_DetKey_Flag)
    {
        Time_DetKey_Flag = 0;
        common_btn_triangle_hidden();
        lv_group_focus_freeze(lv_group_get_default(), false);
        LAYOUT_KEY_BINDINGS(setting_time_key_bindings);
    }
    else
    {
        Time_DetKey_Flag = lv_obj_get_id(lv_group_get_focused(lv_group_get_default()));
    }
}
// 按键绑定表
static const key_binding_t setting_time_advanced_key_bindings[] = {
    KEY_BIND(LV_KEY_HOME, common_btn_key_down, setting_time_key_up_home),
    KEY_BIND(LV_KEY_ESC, common_btn_key_down, setting_time_key_up_esc),
    KEY_BIND(LV_KEY_ENTER, common_btn_key_down, motion_detect_setting_key_up_enter),
    KEY_BIND_PRESS_LONG_PRESS(LV_KEY_NEXT, setting_time_next_key_down, setting_time_next_key_long_down),
    KEY_BIND_PRESS_LONG_PRESS(LV_KEY_PREV, setting_time_prev_key_down, setting_time_prev_key_long_down),
};

// 开关回调：处理公历/波斯历切换逻辑（状态变化时触发）
static void setting_time_calendar_sw_value_change(lv_obj_t *obj)
{

    // 获取开关当前状态（on=公历，off=波斯历）
    bool is_gregorian = lv_switch_get_state(obj);
    // 更新用户数据中的日历模式
    user_data_get()->setting.calendar = is_gregorian ? 1 : 0;
    time_change_flag = true; // 标记时间已修改

    // 保存当前日期，避免重复转换导致数据错误
    struct date old_date = temp_date;
    // 日历转换：根据开关状态切换公历/波斯历
    if (is_gregorian)
    {
        // 波斯历 → 公历
        temp_date = jalali2gregorian(old_date);
    }
    else
    {
        // 公历 → 波斯历
        temp_date = gregorian2jalali(old_date);
    }

    //  刷新年/月/日显示（确保切换后文本同步）
    // 年显示更新
    sprintf(str_year, "%04d", temp_date.year);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_YEAR_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);

    // 月显示更新
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MONTH_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);

    // 日显示更新（需校验当月天数，避免转换后日期超出范围）
    int mon_last_day = is_gregorian ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day; // 日期修正为当月最后一天
    }
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}

// 创建日历切换开关
static bool setting_time_calendar_sw_create(void)
{
    //  绑定开关回调
    static obj_click_data sw_click_data = obj_click_data_up_create(setting_time_calendar_sw_value_change);

    // 确定开关初始状态（根据当前日历模式：公历=on，波斯历=off）
    bool init_is_gregorian = (user_data_get()->setting.calendar == 1);

    // 调用已封装的开关创建函数（type=3触发setting_btn_sub_switch_create）
    lv_obj_t *sw_btn = setting_sub_btn_base_create(
        NULL,                             // 父对象：NULL→默认屏幕
        193,                              // x坐标：与时间按钮统一
        127 + (66 * 0),                   // y坐标：第一位
        548,                              // 宽度：与时间按钮一致
        66,                               // 高度：与时间按钮一致
        str_get(COMMON_LANG_CALENDAR_ID), // 开关文本："日历"
        &sw_click_data,                   // 回调函数：处理切换逻辑
        init_is_gregorian,                // 初始状态：公历=开，波斯历=关
        3                                 // type=3：对应你封装的开关类型
    );

    // 设置开关ID
    if (sw_btn != NULL)
    {
        lv_obj_set_id(sw_btn, SETTING_TIME_CALENDAR_SW_ID);
    }
    return (sw_btn != NULL);
}

static void setting_time_selected_btn(lv_obj_t *obj)
{
    if (Time_DetKey_Flag == 0)
    {
        common_btn_triangle_display(lv_group_get_focused(lv_group_get_default()));
    }
    lv_group_focus_freeze(lv_group_get_default(), true);

    // 绑定当前页面的按键
    LAYOUT_KEY_BINDINGS(setting_time_advanced_key_bindings);
}

// 年/月/日/时/分 双箭头控制逻辑（左减右加）
// ------------------------------ 年按钮逻辑 ------------------------------
// 左箭头：年份减1（公历2022-2037，波斯历1400-1415循环）
static void setting_time_year_left_btn_up(lv_obj_t *obj)
{
    if (user_data_get()->setting.calendar == 1) // 公历
    {
        if (--temp_date.year < 2022)
            temp_date.year = 2037;
    }
    else // 波斯历
    {
        if (--temp_date.year < 1400)
            temp_date.year = 1415;
    }
    time_change_flag = true; // 标记时间已修改
    // 更新显示文本
    sprintf(str_year, "%04d", temp_date.year);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_YEAR_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    // 校验日期合法性（年变化可能影响当月天数）
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 右箭头：年份加1（同左箭头范围）
static void setting_time_year_btn_up(lv_obj_t *obj)
{
    if (user_data_get()->setting.calendar == 1) // 公历
    {
        if (++temp_date.year > 2037)
            temp_date.year = 2022;
    }
    else // 波斯历
    {
        if (++temp_date.year > 1415)
            temp_date.year = 1400;
    }
    time_change_flag = true;
    sprintf(str_year, "%04d", temp_date.year);

    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_YEAR_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    // 校验日期合法性
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 创建年按钮（传入当前年份文本）
static bool setting_time_year_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_year_btn_up);     // 右箭头（加）
    static obj_click_data left_data = obj_click_data_up_create(setting_time_year_left_btn_up); // 左箭头（减）
    lv_obj_t *btn = setting_double_arrow_btn_create(
        NULL,
        193, 127 + (66 * 1), 548, 66,
        str_get(LAYOUT_CALENDAR_LANG_YEAR_ID),
        str_year, // 初始显示当前年份
        &right_data,
        &left_data,
        SETTING_TIME_YEAR_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

// ------------------------------ 月按钮逻辑 ------------------------------
// 左箭头：月份减1（1-12循环）
static void setting_time_month_letf_btn_up(lv_obj_t *obj)
{
    if (--temp_date.month < 1)
        temp_date.month = 12;
    time_change_flag = true;
    // 更新显示
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MONTH_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
    // 校验日期合法性（月变化影响当月天数）
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 右箭头：月份加1（1-12循环）
static void setting_time_month_btn_up(lv_obj_t *obj)
{
    if (++temp_date.month > 12)
        temp_date.month = 1;
    time_change_flag = true;
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MONTH_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
    // 校验日期合法性
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 创建月按钮（传入当前月份文本）
static bool setting_time_month_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_month_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_month_letf_btn_up);
    lv_obj_t *btn = setting_double_arrow_btn_create(
        NULL,
        193, 127 + (66 * 2), 548, 66,
        str_get(LAYOUT_CALENDAR_LANG_MONTH_ID),
        str_month, // 初始显示当前月份
        &right_data,
        &left_data,
        SETTING_TIME_MONTH_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

// ------------------------------ 日按钮逻辑 ------------------------------
// 左箭头：日期减1（1-当月最后一天循环）
static void setting_time_day_left_btn_up(lv_obj_t *obj)
{
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);

    if (--temp_date.day < 1)
        temp_date.day = mon_last_day;

    time_change_flag = true;
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}

// 右箭头：日期加1（1-当月最后一天循环）
static void setting_time_day_btn_up(lv_obj_t *obj)
{
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);

    if (++temp_date.day > mon_last_day)
        temp_date.day = 1;

    time_change_flag = true;
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}

// 创建日按钮（传入当前日期文本）
static bool setting_time_day_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_day_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_day_left_btn_up);
    lv_obj_t *btn = setting_double_arrow_btn_create(
        NULL,
        193, 127 + (66 * 3), 548, 66,
        str_get(LAYOUT_CALENDAR_LANG_DAY_ID),
        str_day, // 初始显示当前日期
        &right_data,
        &left_data,
        SETTING_TIME_DAY_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

// ------------------------------ 时按钮逻辑 ------------------------------
// 左箭头：小时减1（0-23循环）
static void setting_time_hour_left_btn_up(lv_obj_t *obj)
{
    if (--temp_tm.tm_hour < 0)
        temp_tm.tm_hour = 23;
    time_change_flag = true;
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_HOUR_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_hour);
}

// 右箭头：小时加1（0-23循环）
static void setting_time_hour_btn_up(lv_obj_t *obj)
{
    if (++temp_tm.tm_hour > 23)
        temp_tm.tm_hour = 0;
    time_change_flag = true;
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_HOUR_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_hour);
}

// 创建时按钮（传入当前小时文本）
static bool setting_time_hour_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_hour_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_hour_left_btn_up);
    lv_obj_t *btn = setting_double_arrow_btn_create(
        NULL,
        193, 127 + (66 * 4), 548, 66,
        str_get(LAYOUT_CALENDAR_LANG_HOUR_ID),
        str_hour, // 初始显示当前小时
        &right_data,
        &left_data,
        SETTING_TIME_HOUR_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

// ------------------------------ 分按钮逻辑 ------------------------------
// 左箭头：分钟减1（0-59循环）
static void setting_time_minute_left_btn_up(lv_obj_t *obj)
{
    if (--temp_tm.tm_min < 0)
        temp_tm.tm_min = 59;
    time_change_flag = true;
    sprintf(str_min, "%02d", temp_tm.tm_min);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MINUTE_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_min);
}

// 右箭头：分钟加1（0-59循环）
static void setting_time_minute_btn_up(lv_obj_t *obj)
{
    if (++temp_tm.tm_min > 59)
        temp_tm.tm_min = 0;
    time_change_flag = true;
    sprintf(str_min, "%02d", temp_tm.tm_min);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MINUTE_ID + 100), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_min);
}

// 创建分按钮（传入当前分钟文本）
static bool setting_time_minute_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_minute_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_minute_left_btn_up);
    lv_obj_t *btn = setting_double_arrow_btn_create(
        NULL,
        193, 127 + (66 * 5), 548, 66,
        str_get(LAYOUT_CALENDAR_LANG_MIN_ID),
        str_min, // 初始显示当前分钟
        &right_data,
        &left_data,
        SETTING_TIME_MINUTE_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

static void setting_time_next_key_down(lv_key_t key)
{
    common_btn_key_down(key);
    if (Time_DetKey_Flag == SETTING_TIME_YEAR_ID)
    {
        setting_time_year_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MONTH_ID)
    {
        setting_time_month_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_DAY_ID)
    {
        setting_time_day_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_HOUR_ID)
    {
        setting_time_hour_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MINUTE_ID)
    {
        setting_time_minute_btn_up(NULL);
    }
}
static void setting_time_prev_key_down(lv_key_t key)
{
    common_btn_key_down(key);
    if (Time_DetKey_Flag == SETTING_TIME_YEAR_ID)
    {
        setting_time_year_left_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MONTH_ID)
    {
        setting_time_month_letf_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_DAY_ID)
    {
        setting_time_day_left_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_HOUR_ID)
    {
        setting_time_hour_left_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MINUTE_ID)
    {
        setting_time_minute_left_btn_up(NULL);
    }
}

static void setting_time_next_key_long_down(lv_key_t key)
{

    if (Time_DetKey_Flag == SETTING_TIME_YEAR_ID)
    {
        setting_time_year_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MONTH_ID)
    {
        setting_time_month_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_DAY_ID)
    {
        setting_time_day_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_HOUR_ID)
    {
        setting_time_hour_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MINUTE_ID)
    {
        setting_time_minute_btn_up(NULL);
    }
}
static void setting_time_prev_key_long_down(lv_key_t key)
{

    if (Time_DetKey_Flag == SETTING_TIME_YEAR_ID)
    {
        setting_time_year_left_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MONTH_ID)
    {
        setting_time_month_letf_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_DAY_ID)
    {
        setting_time_day_left_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_HOUR_ID)
    {
        setting_time_hour_left_btn_up(NULL);
    }
    else if (Time_DetKey_Flag == SETTING_TIME_MINUTE_ID)
    {
        setting_time_minute_left_btn_up(NULL);
    }
}

// 布局进入：初始化时间数据+创建所有按钮
static void LAYOUT_ENTER_FUNC(setting_time)
{
    LAYOUT_KEY_BINDINGS(setting_time_key_bindings);
    printf("============================enter_setting_time\n");
    lv_obj_t *parent = common_bg_display(lv_scr_act());

    top_time_date_text_create(parent);                                 // 时间显示
    bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
    common_bottom_btn_create(bottom_parent);                           /*按键ui显示*/

    // 初始化：读取当前系统时间，填充临时变量与显示字符串
    Time_DetKey_Flag = 0;
    time_change_flag = false; // 重置修改标记
    user_time_read(&temp_tm); // 读取当前时间到temp_tm
    // 同步日期到temp_date
    temp_date.year = temp_tm.tm_year;
    temp_date.month = temp_tm.tm_mon;
    temp_date.day = temp_tm.tm_mday;
    // 时间合法性校验（确保年份不小于2022）
    if (temp_date.year < 2022)
    {
        temp_date.year = 2022;
        temp_date.month = 1;
        temp_date.day = 1;
        temp_tm.tm_year = 2022;
        temp_tm.tm_mon = 1;
        temp_tm.tm_mday = 1;
    }
    // 若当前是波斯历，转换为波斯历日期
    if (user_data_get()->setting.calendar == 0)
    {
        temp_date = gregorian2jalali(temp_date);
    }
    // 填充显示字符串
    sprintf(str_year, "%04d", temp_date.year);
    sprintf(str_month, "%02d", temp_date.month);
    sprintf(str_day, "%02d", temp_date.day);
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    sprintf(str_min, "%02d", temp_tm.tm_min);

    // 创建时间设置按钮（传入初始化后的显示文本）
    setting_time_calendar_sw_create();
    setting_time_year_btn_create();
    setting_time_month_btn_create();
    setting_time_day_btn_create();
    setting_time_hour_btn_create();
    setting_time_minute_btn_create();
}

// 布局退出：保存修改后的时间
static void LAYOUT_QUIT_FUNC(setting_time)
{
    common_obj_null();
    if (time_change_flag == true) // 仅当时间被修改时执行保存
    {
        // 若当前是波斯历，先转换为公历再保存
        if (user_data_get()->setting.calendar == 0)
        {
            temp_date = jalali2gregorian(temp_date);
        }
        // 同步临时日期到temp_tm
        temp_tm.tm_year = temp_date.year;
        temp_tm.tm_mon = temp_date.month;
        temp_tm.tm_mday = temp_date.day;
        temp_tm.tm_sec = 0; // 秒重置为0
        // 关闭待机定时器->设置新时间->重启定时器
        standby_timer_close();
        user_time_set(&temp_tm);
        // 二次校验时间（防止异常）
        user_time_read(&temp_tm);
        if (temp_tm.tm_year < 2022)
        {
            temp_tm.tm_year = 2022;
            user_time_set(&temp_tm);
        }
        standby_timer_restart(true);
        // 保存用户数据（日历/时间设置）
        user_data_save();
        // 更新RTC校准时间戳
        extern unsigned long long calibrate_rtc_timestamp;
        calibrate_rtc_timestamp = user_timestamp_get();
    }
}

CREATE_LAYOUT(setting_time);