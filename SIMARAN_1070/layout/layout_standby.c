/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-19 11:15
 * @LastEditTime   : 2022-11-24 16:36
 *******************************************************************/
#include "layout_define.h"

static lv_task_t *motion_move_check_task_t = NULL;
static lv_task_t *motion_timing_check_task_t = NULL;
static lv_task_t *motion_delay_start_task_t = NULL;
/***移动侦测相关函数***/
static void standby_resource_release_sfunc(bool motion)
{
	if (motion)
	{
		if (motion_move_check_task_t != NULL)
		{
			lv_task_del(motion_move_check_task_t);
			motion_move_check_task_t = NULL;
		}
		if (motion_timing_check_task_t != NULL)
		{
			lv_task_del(motion_timing_check_task_t);
			motion_timing_check_task_t = NULL;
		}
	}
}
// static void motion_sel_dvr(void)
// {
// 	standby_resource_release_sfunc(true);
// 	if (user_data_get()->motion.select_camera == 0)
// 	{
// 		monitor_channel_set(MON_CH_DOOR1);
// 		monitor_valid_channel_set(MON_CH_DOOR1, true);
// 	}
// 	else if (user_data_get()->motion.select_camera == 1)
// 	{
// 		monitor_channel_set(MON_CH_DOOR2);
// 		monitor_valid_channel_set(MON_CH_DOOR2, true);
// 	}
// 	goto_layout(pLAYOUT(camera));
// }
static void layout_motion_monitor_open(void)
{
	if (user_data_get()->motion.select_camera == 0)
	{
		monitor_channel_set(MON_CH_DOOR1);
	}
	else if (user_data_get()->motion.select_camera == 1)
	{
		monitor_channel_set(MON_CH_DOOR2);
	}
	else if (user_data_get()->motion.select_camera == 2)
	{
		monitor_channel_set(MON_CH_CCTV1);
	}
	else if (user_data_get()->motion.select_camera == 3)
	{
		monitor_channel_set(MON_CH_CCTV2);
	}
	else
	{
		printf("####### motion_dvr_ch error: %d ############################\n", user_data_get()->motion.select_camera);
	}
}

// 移动侦测检查任务
static void motion_move_check_task(lv_task_t *task)
{

	standby_resource_release_sfunc(true);

	// 移动侦测触发后的处理
	printf("Motion detected! Taking action...\n");
	if (user_data_get()->motion.select_camera == 0)
	{
		monitor_channel_set(MON_CH_DOOR1);
		monitor_valid_channel_set(MON_CH_DOOR1, true);
	}
	else if (user_data_get()->motion.select_camera == 1)
	{
		monitor_channel_set(MON_CH_DOOR2);
		monitor_valid_channel_set(MON_CH_DOOR2, true);
	}
	else if (user_data_get()->motion.select_camera == 2)
	{
		monitor_channel_set(MON_CH_CCTV1);
		monitor_valid_channel_set(MON_CH_CCTV1, true);
	}
	else if (user_data_get()->motion.select_camera == 3)
	{
		monitor_channel_set(MON_CH_CCTV2);
		monitor_valid_channel_set(MON_CH_CCTV2, true);
	}
	monitor_enter_mask_set(MON_ENTER_NONE);
	// door_audio_select_pin_ctrl();
	lv_task_del(task);
	task = NULL;
	printf("--------------------&&&&&&&&&&&&&&&&>1");
	goto_layout(pLAYOUT(motion_detection));
}

// 初始化移动侦测参数
static void motion_init(void)
{
	// 根据灵敏度设置参数
	switch (user_data_get()->motion.sensivity)
	{
	case 0: // 低灵敏度
		motion_detection_sensivity(40, 128);
		break;
	case 1: // 中灵敏度
		motion_detection_sensivity(30, 96);
		break;
	case 2: // 高灵敏度
		motion_detection_sensivity(20, 64);
		break;
	default:
		motion_detection_sensivity(30, 96);
		break;
	}
}

// 定时检查任务
static void motion_timing_check_task(lv_task_t *task)
{
	// 如果启用了时间段控制
	if (user_data_get()->motion.timer_en)
	{
		// 获取当前时间
		time_t now = time(NULL);
		struct tm *now_tm = localtime(&now);
		int current_minutes = now_tm->tm_hour * 60 + now_tm->tm_min;

		// 计算开始和结束时间的分钟数
		int start_minutes = user_data_get()->motion.start.tm_hour * 60 + user_data_get()->motion.start.tm_min;
		int end_minutes = user_data_get()->motion.end.tm_hour * 60 + user_data_get()->motion.end.tm_min;

		// 检查当前时间是否在设定的时间段内
		bool in_time_range = false;
		if (start_minutes <= end_minutes)
		{
			// 时间段在同一天内
			in_time_range = (current_minutes >= start_minutes && current_minutes <= end_minutes);
		}
		else
		{
			// 时间段跨越午夜
			in_time_range = (current_minutes >= start_minutes || current_minutes <= end_minutes);
		}

		if (in_time_range)
		{
			if (motion_move_check_task_t == NULL)
			{
				motion_init();
				motion_detection_init();
				motion_detection_start();
				printf("启动移动侦测任务\n");
				motion_move_check_task_t = lv_layout_task_create(motion_move_check_task, 100, LV_TASK_PRIO_HIGH, NULL);
			}
		}
		else
		{
			if (motion_move_check_task_t)
			{
				lv_task_del(motion_move_check_task_t);
				motion_move_check_task_t = NULL;
				motion_detection_stop();
			}
		}
	}
	else
	{
		lv_task_del(task);
		motion_timing_check_task_t = NULL;
		// 没有启用时间段控制，直接启动移动侦测
		printf("启动移动侦测任务\n");
		motion_init();
		motion_detection_init();
		motion_detection_start();
		motion_move_check_task_t = lv_layout_task_create(motion_move_check_task, 100, LV_TASK_PRIO_HIGH, NULL);
	}
}

// 延时启动移动侦测任务
static void motion_delay_start_task(lv_task_t *task)
{

	if (motion_delay_start_task_t != NULL)
	{
		lv_task_del(motion_delay_start_task_t);
		motion_delay_start_task_t = NULL;
	}
	if (user_data_get()->motion.enable)
	{

		refresh_area_t area = {0, 0, 1024, 600};
		gui_refresh_area(&area, 1);
		layout_motion_monitor_open();

		// 启动定时检查任务
		motion_timing_check_task_t = lv_layout_task_create(motion_timing_check_task, 100, LV_TASK_PRIO_HIGH, NULL);
	}
}

static void standby_bg_click_event_cb(lv_obj_t *obj)
{
	backlight_enable(true);
	goto_layout(pLAYOUT(home));
}

static void standby_key_up_home(lv_key_t key)
{
	printf("standby Home key pressed\n");
	if (cur_layout_get() != pLAYOUT(home))
	{
		goto_layout(pLAYOUT(home));
	}
}
static const key_binding_t standby_key_bindings[] = {
	KEY_BIND_RELEASE_ONLY(LV_KEY_HOME, standby_key_up_home),
};
static void LAYOUT_ENTER_FUNC(standby)
{
	standby_timer_close();
	backlight_enable(false);

	// 绑定当前页面的按键
	LAYOUT_KEY_BINDINGS(standby_key_bindings);
	// 创建背景点击事件
	static obj_click_data bg_btn_data = obj_click_data_up_create(standby_bg_click_event_cb);
	lv_group_add_obj(lv_group_get_default(), lv_scr_act());
	obj_click_event_listen(lv_scr_act(), &bg_btn_data);

	// 如果移动侦测启用，延时启动
	if (user_data_get()->motion.enable)
	{
		printf("Motion detection enabled.\n");
		motion_delay_start_task_t = lv_layout_task_create(motion_delay_start_task, 2000, LV_TASK_PRIO_HIGH, NULL);
	}
}

static void LAYOUT_QUIT_FUNC(standby)
{
	// 清理移动侦测资源
	standby_resource_release_sfunc(true);

	obj_click_event_listen(lv_scr_act(), NULL);
	standby_timer_restart(true);
	lv_task_clean();
}

CREATE_LAYOUT(standby);