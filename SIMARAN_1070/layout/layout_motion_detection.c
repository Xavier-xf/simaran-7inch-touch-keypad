#include "layout_define.h"

// 先定义枚举类型，确保在函数声明前可见
typedef enum
{
    MOTION_TASK_TIME_DISP,       // 时间显示任务
    MOTION_TASK_NO_SDCARD_DISP,  // SD卡状态任务
    MOTION_TASK_RECORD_IMAGE,    // 拍照结束任务
    MOTION_TASK_RECORD_VIDEO,    // 录像倒计时任务
    MOTION_TASK_MONITOR_COUNT,   // 监控倒计时任务
    MOTION_TASK_PHOTO_COUNTDOWN, // 新增：拍照倒计时任务
    MOTION_TASK_TOTAL
} motion_ticker_type;

typedef enum
{
    CAMERA_MODE_MONITOR = 0,
    CAMERA_MODE_ZOOM
} cam_mode_t;

typedef void (*ticker_func)(void);
typedef struct
{
    bool en;
    const int delay; // 任务间隔(单位:500ms)
    int count;
    ticker_func handler_func;
} ticker_task_t;

// 提前声明所有静态函数
static void motion_ticker_task_restart(motion_ticker_type type);
static void motion_ticker_task_stop(motion_ticker_type type);
static void camera_sdcard_state_display_flush(void);
static void camera_head_monitor_count_label_create(lv_obj_t *parent);
static void camera_head_time_display_flush(void);
static void camera_head_monitor_count_flush(void);

// 全局状态变量
static bool is_recording = false;
static cam_mode_t camera_mode = CAMERA_MODE_MONITOR;
static int camera_record_video_count_down = 0; // 录像倒计时(秒)
static bool is_photo_mode = false;             // 新增：标记是否为拍照模式

// UI组件ID定义
#define CAMERA_HEAD_CH_LABEL_ID 8           // 顶部通道标签
#define CAMERA_HEAD_TIME_LABEL_ID 9         // 顶部时间标签
#define CAMERA_HEAD_SDCADR_ICON_LABEL_ID 10 // 顶部SD卡图标
#define CAMERA_RECORD_ICON_LABEL_ID 11      // 顶部拍照/录制图标
#define CAMERA_MONITOR_COUNT_DOWN_ID 22     // 顶部监控倒计时

// 添加一个辅助函数来获取录像时长
static int get_record_duration_seconds(void)
{
    switch (user_data_get()->motion.record_duration)
    {
    case 0:
        return 10;
    case 1:
        return 20;
    case 2:
        return 30;
    default:
        return 20;
    }
}

// 时间显示刷新
static void camera_head_time_display_flush(void)
{
    lv_obj_t *time_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_TIME_LABEL_ID);
    if (!time_label)
        return;

    struct tm tm = {0};
    user_time_read(&tm);
    struct date temp_date = {tm.tm_year, tm.tm_mon, tm.tm_mday};

    // 根据日历设置显示格式
    if (user_data_get()->setting.calendar == 0)
    {
        temp_date = gregorian2jalali(temp_date);
        lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d:%02d",
                              temp_date.year, temp_date.month, temp_date.day,
                              tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    else
    {
        lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d:%02d",
                              tm.tm_year, tm.tm_mon, tm.tm_mday,
                              tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    lv_obj_set_pos(time_label, 241, 21);
}

// 监控倒计时刷新
static void camera_head_monitor_count_flush(void)
{
    lv_obj_t *countdown_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_MONITOR_COUNT_DOWN_ID);
    if (countdown_label)
    {
        lv_label_set_text_fmt(countdown_label, "%02d", camera_record_video_count_down);
        lv_obj_set_pos(countdown_label, 799, 27);
    }
}

// 时间显示任务
static void camera_head_time_display_task(void)
{
    camera_head_time_display_flush();
}

// 监控倒计时任务
static void camera_monitor_count_dowm_task(void)
{
    camera_head_monitor_count_flush();
}

// 无SD卡显示任务
static void camera_sdcard_display_task(void)
{
    lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_SDCADR_ICON_LABEL_ID);
    if (sdcard_icon_obj && !media_sdcard_insert_check())
    {
        lv_obj_set_hidden(sdcard_icon_obj, !lv_obj_get_hidden(sdcard_icon_obj));
    }
}

// 拍照结束任务 - 不再立即退出，而是等待倒计时
static void camera_record_image_end_task(void)
{
    printf("移动侦测：拍照完成，等待倒计时结束\n");
    lv_obj_t *record_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_RECORD_ICON_LABEL_ID);
    lv_obj_set_hidden(record_icon_obj, true);
    // 拍照完成后立即设置标志，但不退出
    user_data_get()->new_photo_file_flag = true;

    // 停止拍照结束任务，启动拍照倒计时任务
    motion_ticker_task_stop(MOTION_TASK_RECORD_IMAGE);
}

// 拍照倒计时任务
static void camera_photo_countdown_task(void)
{
    // 倒计时减1（每500ms执行一次，所以需要2次才减1秒）
    static int tick_count = 0;
    if (++tick_count < 2)
    {
        motion_ticker_task_restart(MOTION_TASK_PHOTO_COUNTDOWN);
        return;
    }
    tick_count = 0;

    if (--camera_record_video_count_down < 0)
    {
        printf("移动侦测：拍照倒计时结束，跳转到待机界面\n");
        is_recording = false;
        motion_ticker_task_stop(MOTION_TASK_PHOTO_COUNTDOWN);
        goto_layout(pLAYOUT(standby));
        return;
    }

    // 更新倒计时显示
    // camera_head_monitor_count_flush();
    motion_ticker_task_restart(MOTION_TASK_PHOTO_COUNTDOWN);
}

// 录像倒计时任务
static void camera_record_video_count_down_task(void)
{
    // 倒计时减1（每500ms执行一次，所以需要2次才减1秒）
    static int tick_count = 0;
    if (++tick_count < 2 && camera_record_video_count_down != 0)
    {
        motion_ticker_task_restart(MOTION_TASK_RECORD_VIDEO);
        return;
    }
    tick_count = 0;

    if (--camera_record_video_count_down < 0 ||
        !media_sdcard_insert_check() ||
        !video_record_status_get() ||
        !video_input_state_get())
    {
        printf("移动侦测：录像结束，剩余时间=%d\n", camera_record_video_count_down);

        // 录像结束处理
        is_recording = false;
        record_video_close();
        if (media_sdcard_insert_check())
        {
            user_data_get()->new_media_file_flag = true;
        }
        motion_ticker_task_stop(MOTION_TASK_RECORD_VIDEO);

        // 跳转到待机界面
        printf("移动侦测：录像完成，跳转到待机界面\n");
        goto_layout(pLAYOUT(standby));
        return;
    }

    // 更新倒计时显示
    camera_head_monitor_count_flush();
    motion_ticker_task_restart(MOTION_TASK_RECORD_VIDEO);
}

// 任务列表定义
ticker_task_t motion_detection_ticker_task[MOTION_TASK_TOTAL] = {
    {false, 2, 2, camera_head_time_display_task},       // 1秒刷新一次时间
    {false, 4, 4, camera_sdcard_display_task},          // 2秒刷新一次SD卡状态
    {false, 2, 2, camera_record_image_end_task},        // 1秒后结束拍照提示
    {false, 1, 1, camera_record_video_count_down_task}, // 500ms刷新一次录像倒计时
    {false, 2, 2, camera_monitor_count_dowm_task},      // 1秒刷新一次监控倒计时
    {false, 1, 1, camera_photo_countdown_task}          // 500ms刷新一次拍照倒计时
};

// 任务控制函数 - 重新启动任务
static void motion_ticker_task_restart(motion_ticker_type type)
{
    if (type < MOTION_TASK_TOTAL)
    {
        motion_detection_ticker_task[type].count = motion_detection_ticker_task[type].delay;
        motion_detection_ticker_task[type].en = true;
    }
}

// 任务控制函数 - 停止任务
static void motion_ticker_task_stop(motion_ticker_type type)
{
    if (type == MOTION_TASK_TOTAL)
    {
        for (int i = 0; i < MOTION_TASK_TOTAL; i++)
        {
            motion_detection_ticker_task[i].en = false;
        }
    }
    else if (type < MOTION_TASK_TOTAL)
    {
        motion_detection_ticker_task[type].en = false;
    }
}

// 刷新区域定义
static void layout_monitor_detection_refresh_1(void)
{
    refresh_area_t area[] = {
        {0, 0, 1024, 70},
        {934, 0, 90, 600}};
    gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}

// 创建通道标签
static void camera_head_channel_label_create(lv_obj_t *parent)
{
    lv_obj_t *ch_obj = lv_obj_create(parent, NULL);
    lv_obj_set_id(ch_obj, CAMERA_HEAD_CH_LABEL_ID);
    lv_obj_set_pos(ch_obj, 34, 24);

    MON_CH ch = monitor_channel_get();
    const char *ch_name = "";
    if (ch == MON_CH_DOOR1)
        ch_name = str_get(LAYOUT_HOME_LANG_DOOR1_ID);
    else if (ch == MON_CH_DOOR2)
        ch_name = str_get(LAYOUT_HOME_LANG_DOOR2_ID);
    else if (ch == MON_CH_CCTV1)
        ch_name = str_get(LAYOUT_HOME_LANG_CCTV1_ID);
    else if (ch == MON_CH_CCTV2)
        ch_name = str_get(LAYOUT_HOME_LANG_CCTV2_ID);

    lv_obj_set_style_local_value_str(ch_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, ch_name);
    lv_obj_set_style_local_value_color(ch_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0XDDFF00));
    lv_obj_set_style_local_value_font(ch_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
}

// 创建时间标签
static void camera_head_time_label_create(lv_obj_t *parent)
{
    lv_obj_t *time_label = lv_label_create(parent, NULL);
    lv_obj_set_id(time_label, CAMERA_HEAD_TIME_LABEL_ID);
    lv_obj_set_style_local_text_color(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
    lv_obj_set_style_local_text_font(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    camera_head_time_display_flush();
    motion_ticker_task_restart(MOTION_TASK_TIME_DISP);
}
// 更新拍照/录制图标显示
static void motion_detection_record_state_display_flush(void)
{
    lv_obj_t *record_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_RECORD_ICON_LABEL_ID);
    if (!record_icon_obj)
        return;

    static rom_bin_info info_photo = rom_bin_info_get(ROM_UI_CAMERA_MOTION_DETECTION_PHOTO_PNG);
    static rom_bin_info info_record = rom_bin_info_get(ROM_UI_CAMERA_MOTION_DETECTIONG_RECORD_PNG);

    if (user_data_get()->motion.saving_fmt == 0)
    { // 拍照模式
        lv_obj_set_style_local_pattern_image(record_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info_photo);
    }
    else
    { // 录像模式
        lv_obj_set_style_local_pattern_image(record_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info_record);
    }
    lv_obj_set_hidden(record_icon_obj, false);
}
// 创建拍照/录制图标
static void motion_detection_record_icon_create(lv_obj_t *parent)
{
    lv_obj_t *sdcard_icon_obj = lv_obj_create(parent, NULL);
    lv_obj_set_id(sdcard_icon_obj, CAMERA_RECORD_ICON_LABEL_ID);
    lv_obj_set_pos(sdcard_icon_obj, 789, 16);
    lv_obj_set_size(sdcard_icon_obj, 48, 48);
    motion_detection_record_state_display_flush();
}

// 创建SD卡图标
static void camera_sdcard_icon_create(lv_obj_t *parent)
{
    lv_obj_t *sdcard_icon_obj = lv_obj_create(parent, NULL);
    lv_obj_set_id(sdcard_icon_obj, CAMERA_HEAD_SDCADR_ICON_LABEL_ID);
    lv_obj_set_pos(sdcard_icon_obj, 863, 21);
    lv_obj_set_size(sdcard_icon_obj, 38, 38);
    camera_sdcard_state_display_flush();
}
// 创建移动侦测图标
static void motion_detection_motion_icon_create(lv_obj_t *parent)
{
    lv_obj_t *motion_icon_obj = lv_obj_create(parent, NULL);
    lv_obj_set_pos(motion_icon_obj, 715, 16);
    lv_obj_set_size(motion_icon_obj, 48, 48);

    static rom_bin_info info_motion = rom_bin_info_get(ROM_UI_CAMERA_MOTION_DETECTION_PNG);
    lv_obj_set_style_local_pattern_image(motion_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info_motion);
    lv_obj_set_hidden(motion_icon_obj, false);
}
// 创建监控倒计时标签
static void camera_head_monitor_count_label_create(lv_obj_t *parent)
{
    lv_obj_t *countdown_label = lv_label_create(parent, NULL);
    lv_obj_set_id(countdown_label, CAMERA_MONITOR_COUNT_DOWN_ID);
    lv_obj_set_style_local_text_color(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDDFF00));
    lv_obj_set_style_local_text_font(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(16));
    camera_head_monitor_count_flush();
}

// 更新SD卡状态显示
static void camera_sdcard_state_display_flush(void)
{
    lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_SDCADR_ICON_LABEL_ID);
    if (!sdcard_icon_obj)
        return;

    static rom_bin_info info_insert = rom_bin_info_get(ROM_UI_CAMERA_SD_YES_PNG);
    static rom_bin_info info_error = rom_bin_info_get(ROM_UI_CAMERA_SDCARD_ERROR_PNG);
    static rom_bin_info info_no = rom_bin_info_get(ROM_UI_CAMERA_NO_SDCARD_PNG);

    if (media_sdcard_insert_check())
    {
        lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,
                                             user_data_get()->sd_card_pattion ? &info_error : &info_insert);
        lv_obj_set_hidden(sdcard_icon_obj, false);
        motion_ticker_task_stop(MOTION_TASK_NO_SDCARD_DISP);
    }
    else
    {
        lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info_no);
        lv_obj_set_hidden(sdcard_icon_obj, false);
        motion_ticker_task_restart(MOTION_TASK_NO_SDCARD_DISP);
    }
}

// SD卡状态变化回调
static void camera_sdcard_state_change_func(void)
{
    camera_sdcard_state_display_flush();
    if (!media_sdcard_insert_check())
    {
        user_data_get()->new_media_file_flag = false;
        user_data_get()->new_photo_file_flag = false;
    }
}

// 进入监控模式
static void camera_goto_monitor_mode(lv_obj_t *parent)
{
    camera_mode = CAMERA_MODE_MONITOR;
    video_input_display_zoom_set(100);
    video_input_display_offset_set(0, 0);
    lv_obj_clean(parent);

    camera_record_video_count_down = get_record_duration_seconds();

    bottom_parent = bottom_main(parent, &commom_time_key_btn_area[0]); /*按键底框显示*/
    common_bottom_btn_create(bottom_parent);                           /*按键ui显示*/

    // 创建顶部UI元素

    camera_head_channel_label_create(parent);
    camera_head_time_label_create(parent);
    motion_detection_motion_icon_create(parent); // 新增：移动侦测图标
    motion_detection_record_icon_create(parent); // 新增：拍照/录制图标
    camera_sdcard_icon_create(parent);
    if (user_data_get()->motion.saving_fmt != 0)
        camera_head_monitor_count_label_create(parent);

    layout_monitor_detection_refresh_1();
    lyaout_sd_state_callback_register(camera_sdcard_state_change_func);
}

// 自动拍照/录像处理
static void camera_auto_record(void)
{
    printf("移动侦测：开始自动录制检查\n");

    if (is_recording)
    {
        printf("移动侦测：正在录制中，跳过\n");
        return;
    }
    if (!media_sdcard_insert_check())
    {
        printf("移动侦测：SD卡未插入，跳过\n");
        return;
    }
    if (!video_input_state_get())
    {
        printf("移动侦测：视频输入状态无效，跳过\n");
        return;
    }

    printf("移动侦测：准备自动录制，保存格式=%d\n", user_data_get()->motion.saving_fmt);

    // 直接使用user_data中的motion配置
    if (user_data_get()->motion.saving_fmt == 0)
    { // 拍照模式
        printf("移动侦测：尝试自动拍照...\n");
        if (record_jpeg_start(REC_MODE_MOTION))
        {
            printf("移动侦测：自动拍照启动成功\n");
            is_recording = true;
            is_photo_mode = true;

            // 设置倒计时时间（使用配置的录像时长）
            camera_record_video_count_down = get_record_duration_seconds();
            printf("移动侦测：设置拍照倒计时时长=%d秒\n", camera_record_video_count_down);

            // 启动拍照结束任务和拍照倒计时任务
            motion_ticker_task_restart(MOTION_TASK_RECORD_IMAGE);
            motion_ticker_task_restart(MOTION_TASK_PHOTO_COUNTDOWN);
            // camera_head_monitor_count_flush(); // 立即刷新倒计时显示
        }
        else
        {
            printf("移动侦测：自动拍照启动失败\n");
        }
    }
    else
    { // 录像模式
        printf("移动侦测：尝试自动录像...\n");
        if (record_video_start(REC_MODE_MOTION))
        {
            printf("移动侦测：自动录像启动成功\n");
            is_recording = true;
            is_photo_mode = false;

            // 使用配置的录像时长
            camera_record_video_count_down = get_record_duration_seconds();
            printf("移动侦测：设置录像时长=%d秒\n", camera_record_video_count_down);
            motion_ticker_task_restart(MOTION_TASK_RECORD_VIDEO);
            camera_head_monitor_count_flush(); // 立即刷新倒计时显示
        }
        else
        {
            printf("移动侦测：自动录像启动失败\n");
        }
    }
}

// 延迟自动录制函数
static void camera_auto_record_delayed(lv_task_t *task)
{
    if (motion_detection_check() == false)
    {
        return;
    }
    // 根据LCD使能控制屏幕
    backlight_enable(user_data_get()->motion.lcd_en);
    motion_detection_stop();
    printf("移动侦测：延迟自动录制检查\n");
    camera_auto_record();
    if (task)
    {
        lv_task_del(task);
    }
}

// 主任务处理
static void camera_ticker_task(lv_task_t *task_t)
{
    // 处理所有使能的任务
    for (int i = 0; i < MOTION_TASK_TOTAL; i++)
    {
        if (motion_detection_ticker_task[i].en)
        {
            motion_detection_ticker_task[i].count--;
            if (motion_detection_ticker_task[i].count <= 0)
            {
                motion_detection_ticker_task[i].handler_func();
            }
        }
    }
}

// 创建主任务
static void camera_ticker_task_create(void)
{
    lv_layout_task_create(camera_ticker_task, 500, LV_TASK_PRIO_HIGH, NULL);
}
static bool lcd_just_opened = false;
static void standby_bg_click_event_cb(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}

static void motion_detect_key_up_home(lv_key_t key)
{
    if (cur_layout_get() != pLAYOUT(home))
    {
        goto_layout(pLAYOUT(home));
    }
}
static void motion_detect_key_up_esc(lv_key_t key)
{
    goto_layout(pLAYOUT(standby));
}

// 按键绑定表
static const key_binding_t motion_detect_key_bindings[] = {
    KEY_BIND(LV_KEY_HOME, common_btn_key_down, motion_detect_key_up_home),
    KEY_BIND(LV_KEY_ESC, common_btn_key_down, motion_detect_key_up_esc),
    KEY_BIND_PRESS_ONLY(LV_KEY_ENTER, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_NEXT, common_btn_key_down),
    KEY_BIND_PRESS_ONLY(LV_KEY_PREV, common_btn_key_down),
};
// 进入移动侦测界面
static void LAYOUT_ENTER_FUNC(motion_detection)
{
    printf("移动侦测：进入移动侦测界面\n");
    layout_monitor_detection_refresh_1();

    // 绑定当前页面的按键
    LAYOUT_KEY_BINDINGS(motion_detect_key_bindings);

    lcd_just_opened = false; // 重置状态
    backlight_enable(false);
    // 创建背景点击事件
    static obj_click_data bg_btn_data = obj_click_data_up_create(standby_bg_click_event_cb);
    obj_click_event_listen(lv_scr_act(), &bg_btn_data);
    lv_group_add_obj(lv_group_get_default(), lv_scr_act());
    // 初始化监控
    monitor_open(true, 0x03);
    audio_input_capture_enable(true);
    jpg_encode_capture_enable(true);
    standby_timer_close();
    is_recording = false;
    is_photo_mode = false;

    // 创建UI
    lv_obj_t *parent = lv_scr_act();
    lv_obj_set_style_local_pattern_image(parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);
    camera_goto_monitor_mode(parent);
    fb_gui_layer_rect_fill(0x00, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    // 启动自动录制
    printf("移动侦测：等待设备初始化...\n");
    lv_layout_task_create(camera_auto_record_delayed, 500, LV_TASK_PRIO_MID, NULL);

    // 启动任务
    camera_ticker_task_create();
}

// 退出移动侦测界面
static void LAYOUT_QUIT_FUNC(motion_detection)
{
    printf("移动侦测：退出移动侦测界面\n");
    // 停止所有任务和录制
    ringplay_play_stop();
    motion_ticker_task_stop(MOTION_TASK_TOTAL);
    audio_input_capture_enable(false);
    lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
    motion_detection_destory();
    obj_click_event_listen(lv_scr_act(), NULL);
    monitor_close();
    record_video_close();
    record_jpeg_close();
    user_data_save();
    standby_timer_restart(true);
}

CREATE_LAYOUT(motion_detection);