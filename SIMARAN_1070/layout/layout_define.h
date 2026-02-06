#ifndef _LAYOUT_DEFINE_H_
#define _LAYOUT_DEFINE_H_
#define TUYA_PID "nhi8jlgj989p9l5b" // 7.0 "wrw4vog3vosbq4yd" // 10.1  "iet0idk0yw3fgren"
#include <stdbool.h>
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "lvgl/lv_img_decoder.h"
#include "lvgl/lv_group.h"
#include "rom.h"
#include "lv_msg_event.h"
#include "language.h"
#include "user_data.h"
#include "user_common.h"
#include "video_decode.h"
#include "user_standby.h"
#include "user_monitor.h"
#include "user_time.h"
#include "user_file.h"
#include "user_call.h"
#include "audio_input.h"
#include "layout_common.h"
#include "user_gpio.h"
#include "layout_setting_common.h"
#include "video_decode.h"
#include "user_monitor.h"
#include "tp9950.h"
#include "ringplay.h"
#include "tuya/tuya_api.h"
#include "video_encode.h"
#include "user_record.h"
#include "video_record.h"
#include "video_input.h"
#include "user_gpio.h"
#include "video_play.h"
#include "user_intercom.h"
#include "ConvertCalendar.h"
#include "user_alarm_list.h"
#include "motion_detection.h"

#define SYSTEM_VERSION "Ver.2025_1120_194922"

DEFINE_LAYOUT(logo);
DEFINE_LAYOUT(standby);
DEFINE_LAYOUT(home);
DEFINE_LAYOUT(camera);
DEFINE_LAYOUT(memory_photo);
DEFINE_LAYOUT(memory_video);
DEFINE_LAYOUT(security);
DEFINE_LAYOUT(alarm_list);
DEFINE_LAYOUT(setting_motion_detection);
DEFINE_LAYOUT(setting_time_period_control);
DEFINE_LAYOUT(setting);
DEFINE_LAYOUT(setting_password);
DEFINE_LAYOUT(setting_time);
DEFINE_LAYOUT(setting_sound);
DEFINE_LAYOUT(language);
DEFINE_LAYOUT(time);
DEFINE_LAYOUT(setting_delete);
DEFINE_LAYOUT(setting_sdcard);
DEFINE_LAYOUT(setting_record);
DEFINE_LAYOUT(setting_reset);
DEFINE_LAYOUT(setting_about);
DEFINE_LAYOUT(bell);
DEFINE_LAYOUT(setting_room_id);
DEFINE_LAYOUT(door_ring);
DEFINE_LAYOUT(intercom);
DEFINE_LAYOUT(intercom_out);
DEFINE_LAYOUT(intercom_in);
DEFINE_LAYOUT(intercom_talk);
DEFINE_LAYOUT(photo_list);
DEFINE_LAYOUT(video_list);
DEFINE_LAYOUT(call_list);
DEFINE_LAYOUT(click);
DEFINE_LAYOUT(motion_detection);

extern lv_group_t *lv_group_get_default(void);
#endif