/*******************************************************************
 * @Descripttion   : UI铃声设置功能
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2024-01-20 10:00
 * @LastEditTime   : 2024-01-20 11:30
 *******************************************************************/
#include "layout_define.h"

#define SETTING_SOUND_SYSTEM_KEYTONE_ID 0x05
#define SETTING_SOUND_RING_VOLUME_SET_ID 0x06
#define SETTING_SOUND_TIME_ID 0x07

#define SETTING_SOUND_RING_VOLUME_ID 0x08
#define SETTING_SOUND_RING_2_VOLUME_ID 0x9
#define SETTING_SOUND_SONG_ID 0x10
#define SETTING_SOUND_SONG_2_ID 0x11

static char volume_str[3] = {0};
static char volume2_str[3] = {0};
static char ring_index_str[3] = {0};
static char ring2_index_str[3] = {0};
static bool sound_change_flag = false; // 修改标记（退出时判断是否保存）
static int Sound_DetKey_Flag = 0;

static void setting_sound_key_up_home(lv_key_t key);
static void setting_sound_key_up_esc(lv_key_t key);
static void setting_sound_next_key_down(lv_key_t key);
static void setting_sound_prev_key_down(lv_key_t key);
static void setting_sound_next_key_long_down(lv_key_t key);
static void setting_sound_prev_key_long_down(lv_key_t key);

// 按键绑定表
static const key_binding_t setting_sound_key_bindings[] = {
    KEY_BIND(LV_KEY_HOME, common_btn_key_down, setting_sound_key_up_home),
    KEY_BIND(LV_KEY_ESC, common_btn_key_down, setting_sound_key_up_esc),
    KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),
};

// 设置SOUND界面 - Home键抬起：返回主界面
static void setting_sound_key_up_home(lv_key_t key)
{
    if (cur_layout_get() != pLAYOUT(home))
    {
        goto_layout(pLAYOUT(home));
    }
}

// 设置SOUND界面 - ESC键抬起
static void setting_sound_key_up_esc(lv_key_t key)
{
    if (Sound_DetKey_Flag == 0)
    {
        goto_layout(pLAYOUT(home));
    }
    else
    {
        Sound_DetKey_Flag = 0;
        common_btn_triangle_hidden();
        lv_group_focus_freeze(lv_group_get_default(), false);
        LAYOUT_KEY_BINDINGS(setting_sound_key_bindings);
    }
}

// 设置sound界面 - ENTER键抬起
static void motion_detect_setting_key_up_enter(lv_key_t key)
{
    if (Sound_DetKey_Flag)
    {
        Sound_DetKey_Flag = 0;
        common_btn_triangle_hidden();
        lv_group_focus_freeze(lv_group_get_default(), false);
        LAYOUT_KEY_BINDINGS(setting_sound_key_bindings);
    }
    else
    {
        Sound_DetKey_Flag = lv_obj_get_id(lv_group_get_focused(lv_group_get_default()));
    }
}

// 按键绑定表
static const key_binding_t setting_sound_advanced_key_bindings[] = {
    KEY_BIND(LV_KEY_HOME, common_btn_key_down, setting_sound_key_up_home),
    KEY_BIND(LV_KEY_ESC, common_btn_key_down, setting_sound_key_up_esc),
    KEY_BIND(LV_KEY_ENTER, common_btn_key_down, motion_detect_setting_key_up_enter),
    KEY_BIND_PRESS_LONG_PRESS(LV_KEY_NEXT, setting_sound_next_key_down, setting_sound_next_key_long_down),
    KEY_BIND_PRESS_LONG_PRESS(LV_KEY_PREV, setting_sound_prev_key_down, setting_sound_prev_key_long_down),
};

// 获取指定门口机的铃声索引
static int door_ring_index_get(int door_index)
{
    if (MON_CH_DOOR1 == door_index)
    {
        return user_data_get()->setting.door1_tone;
    }
    else
    {
        return user_data_get()->setting.door2_tone;
    }
}

// 设置指定门口机的铃声索引
static void door_ring_index_set(int door_index, int index)
{
    if (MON_CH_DOOR1 == door_index)
    {
        user_data_get()->setting.door1_tone = index;
    }
    else
    {
        user_data_get()->setting.door2_tone = index;
    }
}

// 获取指定门口机的铃声音量
static int door_ring_volume_get(int door_index)
{
    if (MON_CH_DOOR1 == door_index)
    {
        return user_data_get()->setting.door1_ring_volume;
    }
    else
    {
        return user_data_get()->setting.door2_ring_volume;
    }
}

// 设置指定门口机的铃声音量
static void door_ring_volume_set(int door_index, int vol)
{
    if (door_index == MON_CH_DOOR1)
    {
        user_data_get()->setting.door1_ring_volume = vol;
    }
    else
    {
        user_data_get()->setting.door2_ring_volume = vol;
    }
}

// 门口1铃声播放开始回调
static void door_ring_play_start_cb(int index)
{
    ring_volume_set(door_ring_volume_get(MON_CH_DOOR1));
    // 播放开始处理
}

// 门口2铃声播放开始回调
static void door2_ring_play_start_cb(int index)
{
    ring_volume_set(door_ring_volume_get(MON_CH_DOOR2));
    // 播放开始处理
}
// 铃声播放结束回调
static void door_ring_play_finish_cb(int index)
{
    // 播放完成处理
}

// 系统按键音开关回调
static void setting_sound_system_keytone_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    // 切换系统按键音开关状态
    bool current_state = lv_switch_get_state(obj);

    // 保存系统按键音设置
    user_data_get()->setting.key_tone_enable = current_state;
    user_data_save();
    ringplay_touchsound_mute_set(!current_state);
}

static void setting_volume_ring_time_refresh(void)
{
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_TIME_ID + 100);
    if (obj == NULL)
    {
        printf("obj not found\n");
        return;
    }

    static char buf[5] = {0};
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%ds", user_data_get()->setting.ring_time);

    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, (const char *)buf);
}

// 时间设置回调（右箭头）
static void setting_sound_time_set_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    // 时间设置功能 - 增加时间
    printf("Time setting - increase\n");
    user_data_get()->setting.ring_time += 5;
    if (user_data_get()->setting.ring_time > 60)
    {
        user_data_get()->setting.ring_time = 0;
    }
    user_data_save();

    setting_volume_ring_time_refresh();
}

// 时间设置回调（左箭头）
static void setting_sound_time_set_left_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    // 时间设置功能 - 减少时间
    printf("Time setting - decrease\n");
    user_data_get()->setting.ring_time -= 5;
    if (user_data_get()->setting.ring_time < 0)
    {
        user_data_get()->setting.ring_time = 60;
    }
    user_data_save();

    setting_volume_ring_time_refresh();
}

// 门口机1铃声选择回调（右箭头 - 下一首）
static void setting_sound_song_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int index = door_ring_index_get(MON_CH_DOOR1);
    if (index >= 6)
    {
        index = 1; // 循环到第一首
    }
    else
    {
        index++;
    }
    door_ring_index_set(MON_CH_DOOR1, index);

    // 更新显示
    sprintf(ring_index_str, "%02d", index);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_SONG_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, ring_index_str);

    // 如果音量大于0，播放铃声预览
    if (door_ring_volume_get(MON_CH_DOOR1) > 0)
    {
        // ring_volume_set(door_ring_volume_get(MON_CH_DOOR1));
        ringplay_play_form_index(index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机1铃声选择回调（左箭头 - 上一首）
static void setting_sound_song_btn_left_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int index = door_ring_index_get(MON_CH_DOOR1);
    if (index <= 1)
    {
        index = 6; // 循环到最后一首
    }
    else
    {
        index--;
    }
    door_ring_index_set(MON_CH_DOOR1, index);

    // 更新显示
    sprintf(ring_index_str, "%02d", index);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_SONG_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, ring_index_str);

    // 如果音量大于0，播放铃声预览
    if (door_ring_volume_get(MON_CH_DOOR1) > 0)
    {
        // ring_volume_set(door_ring_volume_get(MON_CH_DOOR1));
        ringplay_play_form_index(index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机2铃声选择回调（右箭头 - 下一首）
static void setting_sound_song_2_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int index = door_ring_index_get(MON_CH_DOOR2);
    if (index >= 6)
    {
        index = 1; // 循环到第一首
    }
    else
    {
        index++;
    }
    door_ring_index_set(MON_CH_DOOR2, index);

    // 更新显示
    sprintf(ring2_index_str, "%02d", index);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_SONG_2_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, ring2_index_str);

    // 如果音量大于0，播放铃声预览
    if (door_ring_volume_get(MON_CH_DOOR2) > 0)
    {
        // ring_volume_set(door_ring_volume_get(MON_CH_DOOR2));
        ringplay_play_form_index(index, 100, door2_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机2铃声选择回调（左箭头 - 上一首）
static void setting_sound_song_2_btn_left_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int index = door_ring_index_get(MON_CH_DOOR2);
    if (index <= 1)
    {
        index = 6; // 循环到最后一首
    }
    else
    {
        index--;
    }
    door_ring_index_set(MON_CH_DOOR2, index);

    // 更新显示
    sprintf(ring2_index_str, "%02d", index);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_SONG_2_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, ring2_index_str);

    // 如果音量大于0，播放铃声预览
    if (door_ring_volume_get(MON_CH_DOOR2) > 0)
    {
        // ring_volume_set(door_ring_volume_get(MON_CH_DOOR2));
        ringplay_play_form_index(index, 100, door2_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机1铃声音量调节回调（右箭头 - 音量加）
static void setting_sound_ring_volume_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int bell_volume = door_ring_volume_get(MON_CH_DOOR1);

    bell_volume++;
    if (bell_volume > 4)
    {
        bell_volume = 0;
    }
    door_ring_volume_set(MON_CH_DOOR1, bell_volume);

    // 更新显示
    sprintf(volume_str, "%d", bell_volume);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_RING_VOLUME_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, volume_str);

    printf("Door1 Ring volume : [%d]\n", bell_volume);

    // 如果音量大于0，播放铃声预览
    if (bell_volume > 0)
    {
        ringplay_play_form_index(door_ring_index_get(MON_CH_DOOR1), 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机1铃声音量调节回调（左箭头 - 音量减）
static void setting_sound_ring_volume_btn_left_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int bell_volume = door_ring_volume_get(MON_CH_DOOR1);

    bell_volume--;
    if (bell_volume < 0)
    {
        bell_volume = 4;
    }

    door_ring_volume_set(MON_CH_DOOR1, bell_volume);

    // 更新显示
    sprintf(volume_str, "%d", bell_volume);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_RING_VOLUME_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, volume_str);

    printf("Door1 Ring volume : [%d]\n", bell_volume);

    // 如果音量大于0，播放铃声预览
    if (bell_volume > 0)
    {
        ringplay_play_form_index(door_ring_index_get(MON_CH_DOOR1), 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机2铃声音量调节回调（右箭头 - 音量加）
static void setting_sound_ring_2_volume_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int bell_volume = door_ring_volume_get(MON_CH_DOOR2);
    bell_volume++;
    if (bell_volume > 4)
    {
        bell_volume = 0;
    }
    door_ring_volume_set(MON_CH_DOOR2, bell_volume);

    // 更新显示
    sprintf(volume2_str, "%d", bell_volume);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_RING_2_VOLUME_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, volume2_str);

    printf("Door2 Ring volume : [%d]\n", bell_volume);

    // 如果音量大于0，播放铃声预览
    if (bell_volume > 0)
    {
        ringplay_play_form_index(door_ring_index_get(MON_CH_DOOR2), 100, door2_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机2铃声音量调节回调（左箭头 - 音量减）
static void setting_sound_ring_2_volume_btn_left_btn_up(lv_obj_t *obj)
{
    sound_change_flag = true;
    int bell_volume = door_ring_volume_get(MON_CH_DOOR2);
    bell_volume--;
    if (bell_volume < 0)
    {
        bell_volume = 4;
    }

    door_ring_volume_set(MON_CH_DOOR2, bell_volume);

    // 更新显示
    sprintf(volume2_str, "%d", bell_volume);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SOUND_RING_2_VOLUME_ID + 100), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, volume2_str);

    printf("Door2 Ring volume : [%d]\n", bell_volume);
    // 如果音量大于0，播放铃声预览
    if (bell_volume > 0)
    {
        ringplay_play_form_index(door_ring_index_get(MON_CH_DOOR2), 100, door2_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

static void setting_sound_selected_btn(lv_obj_t *obj)
{
    if (Sound_DetKey_Flag == 0)
    {
        common_btn_triangle_display(lv_group_get_focused(lv_group_get_default()));
    }
    lv_group_focus_freeze(lv_group_get_default(), true);
    // 绑定当前页面的按键
    LAYOUT_KEY_BINDINGS(setting_sound_advanced_key_bindings);
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建System keytone设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_sound_system_keytone_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_sound_system_keytone_btn_up);
    bool keytone_state = user_data_get()->setting.key_tone_enable;
    setting_sub_btn_base_create(NULL, 193, 108 + (66 * 0), 548, 66,
                                str_get(LAYOUT_SETTING_LANG_SYSTEM_KEYTONE_ID),
                                &click_data, keytone_state, 3);
    return true;
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建time_set设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_sound_time_set_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_sound_time_set_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_sound_time_set_left_btn_up);
    lv_obj_t *btn = setting_double_arrow_btn_create(NULL, 193, 175 + (66 * 0), 548, 66,
                                                    str_get(LAYOUT_SETTING_LANG_TIME_ID),
                                                    "10s",
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_SOUND_TIME_ID);
    setting_volume_ring_time_refresh();
    static obj_click_data btn_data = obj_click_data_up_create(setting_sound_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建Ring volume 设置按钮（铃声音量调节）
** 返回参数说明：成功创建返回true
***/
static bool setting_sound_ring_volume_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_sound_ring_volume_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_sound_ring_volume_btn_left_btn_up);

    int ring_volume = door_ring_volume_get(MON_CH_DOOR1);
    sprintf(volume_str, "%d", ring_volume);
    lv_obj_t *btn = setting_double_arrow_btn_create(NULL, 193, 175 + (66), 548, 66,
                                                    str_get(LAYOUT_INTERCOM_LANG_RING_VOL_ID),
                                                    volume_str,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_SOUND_RING_VOLUME_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_sound_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建Ring 2 volume 设置按钮（铃声音量调节）
** 返回参数说明：成功创建返回true
***/
static bool setting_sound_ring_2_volume_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_sound_ring_2_volume_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_sound_ring_2_volume_btn_left_btn_up);

    int ring_volume = door_ring_volume_get(MON_CH_DOOR2);
    sprintf(volume2_str, "%d", ring_volume);
    lv_obj_t *btn = setting_double_arrow_btn_create(NULL, 193, 175 + (66 * 2), 548, 66,
                                                    str_get(LAYOUT_INTERCOM_LANG_RING2_VOL_ID),
                                                    volume2_str,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_SOUND_RING_2_VOLUME_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_sound_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建song设置按钮（铃声选择）
** 返回参数说明：成功创建返回true
***/
static bool setting_sound_song_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_sound_song_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_sound_song_btn_left_btn_up);

    int song_index = door_ring_index_get(MON_CH_DOOR1);
    printf("Door1 song_index: [%d]\n", song_index);
    sprintf(ring_index_str, "%02d", song_index);
    printf("ring_index_str: [%s], 长度: %d\n", ring_index_str, strlen(ring_index_str));
    lv_obj_t *btn = setting_double_arrow_btn_create(NULL, 193, 175 + (66 * 3), 548, 66,
                                                    str_get(LAYOUT_SETTING_LANG_SONG_ID),
                                                    ring_index_str,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_SOUND_SONG_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_sound_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建song2设置按钮（铃声选择）
** 返回参数说明：成功创建返回true
***/
static bool setting_sound_song_2_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_sound_song_2_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_sound_song_2_btn_left_btn_up);

    int song_index = door_ring_index_get(MON_CH_DOOR2);
    printf("Door2 song_index: [%d]\n", song_index);
    sprintf(ring2_index_str, "%02d", song_index);
    printf("ring2_index_str: [%s], 长度: %d\n", ring2_index_str, strlen(ring2_index_str));
    lv_obj_t *btn = setting_double_arrow_btn_create(NULL, 193, 175 + (66 * 4), 548, 66,
                                                    str_get(LAYOUT_SETTING_LANG_SONG2_ID),
                                                    ring2_index_str,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_SOUND_SONG_2_ID);
    static obj_click_data btn_data = obj_click_data_up_create(setting_sound_selected_btn);
    obj_click_event_listen(btn, &btn_data);
    return true;
}

static void setting_sound_next_key_long_down(lv_key_t key)
{
    switch (Sound_DetKey_Flag)
    {
    case SETTING_SOUND_TIME_ID:
        setting_sound_time_set_btn_up(NULL);
        break;

    case SETTING_SOUND_RING_VOLUME_ID:
        setting_sound_ring_volume_btn_up(NULL);
        break;

    case SETTING_SOUND_RING_2_VOLUME_ID:
        setting_sound_ring_2_volume_btn_up(NULL);
        break;

    case SETTING_SOUND_SONG_ID:
        setting_sound_song_btn_up(NULL);
        break;

    case SETTING_SOUND_SONG_2_ID:
        setting_sound_song_2_btn_up(NULL);
        break;

    default:
        break;
    }
}

static void setting_sound_next_key_down(lv_key_t key)
{
    common_btn_key_down(key);
    setting_sound_next_key_long_down(key);
}

static void setting_sound_prev_key_long_down(lv_key_t key)
{
    switch (Sound_DetKey_Flag)
    {
    case SETTING_SOUND_TIME_ID:
        setting_sound_time_set_left_btn_up(NULL);
        break;

    case SETTING_SOUND_RING_VOLUME_ID:
        setting_sound_ring_volume_btn_left_btn_up(NULL);
        break;

    case SETTING_SOUND_RING_2_VOLUME_ID:
        setting_sound_ring_2_volume_btn_left_btn_up(NULL);
        break;

    case SETTING_SOUND_SONG_ID:
        setting_sound_song_btn_left_btn_up(NULL);
        break;

    case SETTING_SOUND_SONG_2_ID:
        setting_sound_song_2_btn_left_btn_up(NULL);
        break;

    default:
        break;
    }
}

static void setting_sound_prev_key_down(lv_key_t key)
{
    common_btn_key_down(key);
    setting_sound_prev_key_long_down(key);
}

static void LAYOUT_ENTER_FUNC(setting_sound)
{
    LAYOUT_KEY_BINDINGS(setting_sound_key_bindings);
    printf("============================enter_ setting_sound\n");
    sound_change_flag = false;
    Sound_DetKey_Flag = 0;
    lv_obj_t *parent = common_bg_display(lv_scr_act());

    top_time_date_text_create(parent);                                 // 时间显示
    bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
    common_bottom_btn_create(bottom_parent);                           /*按键ui显示*/

    setting_sound_system_keytone_btn_create();
    setting_sound_time_set_btn_create();
    setting_sound_ring_volume_btn_create();
    setting_sound_ring_2_volume_btn_create();
    setting_sound_song_btn_create();
    setting_sound_song_2_btn_create();
}

static void LAYOUT_QUIT_FUNC(setting_sound)
{
    common_obj_null();
    if (sound_change_flag == true)
    {
        // 退出时停止铃声播放
        ringplay_play_stop();
        // 保存用户数据
        user_data_save();
    }
}

CREATE_LAYOUT(setting_sound);