#include "user_gpio.h"
#include <stdio.h>
#include "user_time.h"
#include <pthread.h>
#include "lv_msg_event.h"
#include "user_data.h"
#include "user_monitor.h"
#include "user_intercom.h"
#include "user_common.h"

#define SENSOR1_DET_PIN 94
#define SENSOR2_DET_PIN 95

static bool hook_state = false;

/***
** 日期: 2022-04-28 10:08
** 作者: leo.liu
** 函数作用：背光控制
** 返回参数说明：
***/
#define BL_CTRL_PIN 82
bool backlight_enable(bool en)
{
	return gpio_level_set(BL_CTRL_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-05-12 11:07
** 作者: leo.liu
** 函数作用：tp9950复位
** 返回参数说明：
***/
#define TP_RESET_GPIO 72
void tp9950_pin_reset(void)
{
	gpio_level_set(TP_RESET_GPIO, GPIO_LEVEL_HIGH);
	usleep(1000 * 10);
	gpio_level_set(TP_RESET_GPIO, GPIO_LEVEL_LOW);
	usleep(1000 * 10);
	gpio_level_set(TP_RESET_GPIO, GPIO_LEVEL_HIGH);
	usleep(1000 * 10);
}
void tp9950_power_off(void)
{
	gpio_level_set(TP_RESET_GPIO, GPIO_LEVEL_LOW);
}
void tp9950_power_on(void)
{
	gpio_level_set(TP_RESET_GPIO, GPIO_LEVEL_HIGH);
}
bool tp9950_power_get(void)
{
	GPIO_LEVEL level;
	gpio_level_read(TP_RESET_GPIO, &level);
	return level == GPIO_LEVEL_LOW ? false : true;
}

/***
** 日期: 2022-05-12 09:43
** 作者: leo.liu
** 函数作用：IO电平变化检测
** 返回参数说明：
***/
#define HOOK_STATE_PIN 0 // 听筒状态检测脚
#define DOOR1_CALL_PIN 1 // door1 call机检测脚
#define DOOR2_CALL_PIN 2 // door2 call机检测脚
// #define KEY_LOCK_PIN 74		 // 室內机开锁检测脚
#define DOOR_CALL_DELAY 3000 // （ms）连续call机延时等待
void *gpio_det_task(void *arg)
{
	int gpio_group_pin[] = {HOOK_STATE_PIN, DOOR1_CALL_PIN, DOOR2_CALL_PIN};
	GPIO_LEVEL gpio_group_level[] = {GPIO_LEVEL_LOW, GPIO_LEVEL_LOW, GPIO_LEVEL_LOW};
	for (int i = 0; i < sizeof(gpio_group_pin) / sizeof(int); i++)
	{
		if (gpio_level_read(gpio_group_pin[i], &gpio_group_level[i]) == false)
		{
			printf("read gpio%d level failed \n", gpio_group_pin[i]);
		}
	}

	hook_state = gpio_group_level[0] == GPIO_LEVEL_HIGH ? 1 : 0;

	unsigned long long door1_call_ts = 0;
	unsigned long long door2_call_ts = 0;

	unsigned long long door_call_ts;
	GPIO_LEVEL level;

	printf("***** gpio detection task create sccess ! *****\n");
	while (1)
	{
		/***** hook state 检测 *****/
		if ((gpio_level_read(gpio_group_pin[0], &level) == true) && (level != gpio_group_level[0]))
		{
			usleep(10 * 1000);
			if ((gpio_level_read(gpio_group_pin[0], &level) == true) && (level != gpio_group_level[0]))
			{
				power_amplifier_enable(false);
				gpio_group_level[0] = level;
				hook_state = level == GPIO_LEVEL_HIGH ? 1 : 0;
				lv_msg_send_cmd(MSG_EVENT_CMD_CUSTOM, hook_state, 0);
			}
		}

		/***** door1 检测 *****/
		door_call_ts = user_timestamp_get();
		if ((gpio_level_read(gpio_group_pin[1], &level) == true) && (level != gpio_group_level[1]))
		{
			gpio_group_level[1] = level;
			if ((level == GPIO_LEVEL_LOW) && (abs(door_call_ts - door1_call_ts) > DOOR_CALL_DELAY))
			{
				door1_call_ts = user_timestamp_get();
				printf("=====================>>> door1 call \n");
				lv_msg_send_cmd(MSG_EVENT_CMD_DOOR1_CALL, 0, 0);
			}
		}

		/***** door2 检测 *****/
		door_call_ts = user_timestamp_get();
		if ((gpio_level_read(gpio_group_pin[2], &level) == true) && (level != gpio_group_level[2]))
		{
			gpio_group_level[2] = level;
			if ((level == GPIO_LEVEL_LOW) && (abs(door_call_ts - door2_call_ts) > DOOR_CALL_DELAY))
			{
				door2_call_ts = user_timestamp_get();
				printf("=====================>>> door2 call \n");
				lv_msg_send_cmd(MSG_EVENT_CMD_DOOR2_CALL, 0, 0);
			}
		}

		usleep(10 * 1000);
	}
	return NULL;
}

// #define POWER_LED_PIN 3
// // 电源指示灯
// void power_led_enable(bool en)
// {
// 	gpio_level_set(POWER_LED_PIN, en ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
// }

#define RING_VOL0_PIN 3
#define RING_VOL1_PIN 4
#define RING_VOL2_PIN 5
// 铃声音量设置
void ring_volume_set(int vol)
{
	switch (vol)
	{
	case 0:
		power_amplifier_enable(false);
		// gpio_level_set(RING_VOL0_PIN, GPIO_LEVEL_HIGH);
		// gpio_level_set(RING_VOL1_PIN, GPIO_LEVEL_HIGH);
		// gpio_level_set(RING_VOL2_PIN, GPIO_LEVEL_HIGH);
		break;
	case 1:
		power_amplifier_enable(true);
		gpio_level_set(RING_VOL0_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL1_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL2_PIN, GPIO_LEVEL_HIGH);

		break;
	case 2:
		power_amplifier_enable(true);
		gpio_level_set(RING_VOL0_PIN, GPIO_LEVEL_HIGH);
		gpio_level_set(RING_VOL1_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL2_PIN, GPIO_LEVEL_LOW);
		break;
	case 3:
		power_amplifier_enable(true);
		gpio_level_set(RING_VOL0_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL1_PIN, GPIO_LEVEL_HIGH);
		gpio_level_set(RING_VOL2_PIN, GPIO_LEVEL_LOW);
		break;
	case 4:
		power_amplifier_enable(true);
		gpio_level_set(RING_VOL0_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL1_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL2_PIN, GPIO_LEVEL_LOW);
		break;
	case 5:
		power_amplifier_enable(true);
		gpio_level_set(RING_VOL0_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL1_PIN, GPIO_LEVEL_LOW);
		gpio_level_set(RING_VOL2_PIN, GPIO_LEVEL_LOW);
		break;
	default:
		printf("ring output volume setting failed %d\n", vol);
		return;
		break;
	}
}

/***
** 日期: 2022-05-20 14:29
** 作者: leo.liu
** 函数作用：door1锁控制
** 返回参数说明：
***/
#define DOOR1_UNLOCK_PIN 74
void door1_unlock_pin_ctrl(bool en)
{
	gpio_level_set(DOOR1_UNLOCK_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-05-20 14:29
** 作者: leo.liu
** 函数作用：door2锁控制
** 返回参数说明：
***/
#define DOOR2_UNLOCK_PIN 73
void door2_unlock_pin_ctrl(bool en)
{
	gpio_level_set(DOOR2_UNLOCK_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

#define POWER_AMPLIFIER_PIN 9
// 功放使能
bool power_amplifier_enable(bool en)
{
	if (en == false)
	{
		return gpio_level_set(POWER_AMPLIFIER_PIN, GPIO_LEVEL_LOW);
	}
	else
	{
		return gpio_level_set(POWER_AMPLIFIER_PIN, hook_state == false ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
	}
}

/***
** 日期: 2022-05-12 11:04
** 作者: leo.liu
** 函数作用： door1 电源
** 返回参数说明：
***/
#define DOOR1_POWER_PIN 69
void door1_power_enable(bool en)
{
	gpio_level_set(DOOR1_POWER_PIN, en ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-05-12 11:04
** 作者: leo.liu
** 函数作用： door2 电源
** 返回参数说明：
***/
#define DOOR2_POWER_PIN 70
void door2_power_enable(bool en)
{
	gpio_level_set(DOOR2_POWER_PIN, en ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-05-12 11:04
** 作者: leo.liu
** 函数作用： CCTV1电源
** 返回参数说明：
***/
#define CCTV1_POWER_PIN 71
void CCTV1_power_enable(bool en)
{
	gpio_level_set(CCTV1_POWER_PIN, en ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-05-12 11:04
** 作者: leo.liu
** 函数作用： CCTV2电源
** 返回参数说明：
***/
#define CCTV2_POWER_PIN 32
void CCTV2_power_enable(bool en)
{
	gpio_level_set(CCTV2_POWER_PIN, en ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-05-20 14:11
** 作者: leo.liu
** 函数作用：音频传输到door1
** 返回参数说明：
***/
#define AUDIO_DOOR1_PIN 77
void audio_to_outdoor1_pin_ctrl(bool en)
{
	gpio_level_set(AUDIO_DOOR1_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}
/***
** 日期: 2022-05-20 14:13
** 作者: leo.liu
** 函数作用：音频传输到door2
** 返回参数说明：
***/
#define AUDIO_DOOR2_PIN 78
void audio_to_outdoor2_pin_ctrl(bool en)
{
	gpio_level_set(AUDIO_DOOR2_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

// 内线音频通道（管理员）
#define AUDIO_INTERNAL_LINE_ADMIN_CHANNEL 79

void audio_internal_line_admin_channel_ctrl(bool en)
{
	gpio_level_set(AUDIO_INTERNAL_LINE_ADMIN_CHANNEL, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

// 内线音频通道（普通）
#define AUDIO_INTER_LINE_SELECT_PIN 6

void audio_to_inter_line_select_pin_ctrl(bool en)
{
	gpio_level_set(AUDIO_INTER_LINE_SELECT_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

// /***
// ** 日期: 2022-05-20 17:09
// ** 作者: leo.liu
// ** 函数作用：控制总线音频输出到下一台设备
// ** 返回参数说明：
// ***/
// #define AUDIO_INTER_L1_PIN 69
// void audio_to_inter_1_pin_ctrl(bool en)
// {
// 	gpio_level_set(AUDIO_INTER_L1_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
// }

// /***
// ** 日期: 2022-05-20 17:09
// ** 作者: leo.liu
// ** 函数作用：控制总线音频输出到下一台设备
// ** 返回参数说明：
// ***/
// #define AUDIO_INTER_L2_PIN 70
// void audio_to_inter_2_pin_ctrl(bool en)
// {
// 	gpio_level_set(AUDIO_INTER_L2_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
// }

// /***
// ** 日期: 2022-05-20 14:29
// ** 作者: leo.liu
// ** 函数作用：室内机锁控制
// ** 返回参数说明：
// ***/
// #define GATE_UNLOCK_PIN 71
// void gate_unlock_pin_ctrl(bool en)
// {
// 	gpio_level_set(GATE_UNLOCK_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
// }

/***
** 日期: 2022-05-20 14:29
** 作者: leo.liu
** 函数作用：大门锁控制
** 返回参数说明：
***/
#define PARKIN_UNLOCK_PIN 68
void gate_unlock_pin_ctrl(bool en)
{
	gpio_level_set(PARKIN_UNLOCK_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-05-20 14:05
** 作者: leo.liu
** 函数作用：呼梯继电器控制
** 返回参数说明：
***/
#define ELEVATOR_ON_PIN 99
void elevator_on_pin_ctrl(bool en)
{
	gpio_level_set(ELEVATOR_ON_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

// /***
// ** 日期: 2022-05-20 14:05
// ** 作者: leo.liu
// ** 函数作用：禁止铃声输出到户外机
// ** 返回参数说明：
// ***/
// #define RING_TO_OUTDOOR_MUTE_PIN 73
// void ring_to_outdoor_mute_pin_ctrl(bool en)
// {
// 	gpio_level_set(RING_TO_OUTDOOR_MUTE_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
// }

#define CTRL_485_PIN 8
// 458芯片发送 或 接收 选择
// mask:0x00->接收 0x01->发送
bool rs485_send_recv_ctrl(int mask)
{
	return gpio_level_set(CTRL_485_PIN, mask == 0x01 ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

// cctv音视频开关
#define CCTV_AUDIO_VIDEO_ENABLE_PIN 35
// （false：cctv1  true：cctv2）
void cctv_audio_video_enable_pin_ctrl(bool en)
{
	gpio_level_set(CCTV_AUDIO_VIDEO_ENABLE_PIN, en == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

/***
** 日期: 2022-04-28 10:11
** 作者: leo.liu
** 函数作用：应用层对GPIO进行初始化
** 返回参数说明：
***/
void layout_gpio_init(void)
{
	/***** 功放IO初始化 *****/
	gpio_direction_set(POWER_AMPLIFIER_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(POWER_AMPLIFIER_PIN, true);
	power_amplifier_enable(false);

	/***** 初始化背光 *****/
	gpio_direction_set(BL_CTRL_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(BL_CTRL_PIN, true);
	backlight_enable(true);

	/***** 初始化tp9950复位 *****/
	gpio_direction_set(TP_RESET_GPIO, GPIO_DIR_OUT);
	gpio_pull_enable(TP_RESET_GPIO, true);

	/***** 听筒状态检测gpio初始化 *****/
	gpio_direction_set(HOOK_STATE_PIN, GPIO_DIR_IN);

	/***** 门口机call机检测gpio初始化 *****/
	gpio_direction_set(DOOR1_CALL_PIN, GPIO_DIR_IN);
	// gpio_pull_enable(DOOR1_CALL_PIN, true);
	gpio_direction_set(DOOR2_CALL_PIN, GPIO_DIR_IN);
	// gpio_pull_enable(DOOR2_CALL_PIN, true);

	/***** 铃声音量设置 *****/
	gpio_direction_set(RING_VOL0_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(RING_VOL0_PIN, true);
	gpio_direction_set(RING_VOL1_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(RING_VOL1_PIN, true);
	gpio_direction_set(RING_VOL2_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(RING_VOL2_PIN, true);
	ring_volume_set(2);

	/***** 门口机1/2开锁IO初始化 *****/
	gpio_direction_set(DOOR1_UNLOCK_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(DOOR1_UNLOCK_PIN, true);
	door1_unlock_pin_ctrl(false);
	gpio_direction_set(DOOR2_UNLOCK_PIN, GPIO_DIR_OUT); // 新增door2初始化
	gpio_pull_enable(DOOR2_UNLOCK_PIN, true);
	door2_unlock_pin_ctrl(false);

	/***** 初始化户外机电源 *****/
	gpio_direction_set(DOOR1_POWER_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(DOOR1_POWER_PIN, true);
	gpio_direction_set(DOOR2_POWER_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(DOOR2_POWER_PIN, true);
	gpio_direction_set(CCTV1_POWER_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(CCTV1_POWER_PIN, true);
	gpio_direction_set(CCTV2_POWER_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(CCTV2_POWER_PIN, true);
	door1_power_enable(false);
	door2_power_enable(false);
	CCTV1_power_enable(false);
	CCTV2_power_enable(false);
	/***** 声音传输到门口机 *****/
	gpio_direction_set(AUDIO_DOOR1_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(AUDIO_DOOR1_PIN, true);
	audio_to_outdoor1_pin_ctrl(true);
	gpio_direction_set(AUDIO_DOOR2_PIN, GPIO_DIR_OUT); // 新增door2初始化
	gpio_pull_enable(AUDIO_DOOR2_PIN, true);
	audio_to_outdoor2_pin_ctrl(true);

	/***** 声音传输到inter*****/
	gpio_direction_set(AUDIO_INTER_LINE_SELECT_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(AUDIO_INTER_LINE_SELECT_PIN, true);

	gpio_direction_set(AUDIO_INTERNAL_LINE_ADMIN_CHANNEL, GPIO_DIR_OUT);
	gpio_pull_enable(AUDIO_INTERNAL_LINE_ADMIN_CHANNEL, true);

	audio_to_inter_line_select_pin_ctrl(false);
	audio_internal_line_admin_channel_ctrl(false);
	/***** 大门锁控制 *****/
	gpio_direction_set(PARKIN_UNLOCK_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(PARKIN_UNLOCK_PIN, true);
	gate_unlock_pin_ctrl(false);

	/***** RS485控制 *****/
	gpio_direction_set(CTRL_485_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(CTRL_485_PIN, true);
	rs485_send_recv_ctrl(0x00);

	/***** 切换CCTV *****/
	gpio_direction_set(CCTV_AUDIO_VIDEO_ENABLE_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(CCTV_AUDIO_VIDEO_ENABLE_PIN, true);
	cctv_audio_video_enable_pin_ctrl(false);

	/***** 呼梯继电器初始化 *****/
	gpio_direction_set(ELEVATOR_ON_PIN, GPIO_DIR_OUT);
	gpio_pull_enable(ELEVATOR_ON_PIN, false);
	gpio_level_set(ELEVATOR_ON_PIN, GPIO_LEVEL_LOW); // 初始低电平（未呼梯）

	pthread_t thread_t;
	pthread_create(&thread_t, user_pthread_atter_get(), gpio_det_task, NULL);
}

/***
** 日期: 2022-05-20 17:03
** 作者: leo.liu
** 函数作用：通话打开
** 返回参数说明：
***/
bool door_audio_talk(AUDIO_CH ch)
{
	switch (ch)
	{
	case AUDIO_CH_DOOR1:
		audio_to_outdoor1_pin_ctrl(true);
		ring_volume_set(user_data_get()->setting.door1_ring_volume);
		break;
	case AUDIO_CH_DOOR2:
		audio_to_outdoor2_pin_ctrl(true);
		ring_volume_set(user_data_get()->setting.door2_ring_volume);
		break;
	case AUDIO_CH_INTER:
		audio_to_outdoor1_pin_ctrl(false);
		audio_to_outdoor2_pin_ctrl(false);
		audio_to_inter_line_select_pin_ctrl(true);
		audio_internal_line_admin_channel_ctrl(false);
		ring_volume_set(user_data_get()->setting.inter_ring_volume);
		break;
	case AUDIO_CH_GUARD:
		audio_to_outdoor1_pin_ctrl(false);
		audio_to_outdoor2_pin_ctrl(false);
		audio_to_inter_line_select_pin_ctrl(false);
		audio_internal_line_admin_channel_ctrl(true);
		ring_volume_set(user_data_get()->setting.inter_ring_volume);
		break;
	case AUDIO_CH_CLOSE:
	default:
		audio_to_outdoor1_pin_ctrl(false);
		audio_to_outdoor2_pin_ctrl(false);
		audio_to_inter_line_select_pin_ctrl(false);
		audio_internal_line_admin_channel_ctrl(false);
		power_amplifier_enable(false);
		break;
	}
	return true;
}

/***
** 日期: 2022-05-21 09:07
** 作者: leo.liu
** 函数作用：铃声输出到户外机控制
** 返回参数说明：
***/
bool call_ring_to_outdoor_ctrl(AUDIO_CH ch, bool en)
{
	// ring_to_outdoor_mute_pin_ctrl(false);
	// if (en == false)
	// {
	// 	// ring_to_outdoor_mute_pin_ctrl(true);
	// 	audio_to_outdoor1_pin_ctrl(false);
	// 	audio_to_outdoor2_pin_ctrl(false);
	// 	audio_to_inter_1_pin_ctrl(false);
	// 	audio_to_inter_2_pin_ctrl(false);
	// 	return true;
	// }

	if (en == false)
	{
		// ring_to_outdoor_mute_pin_ctrl(true);
		// audio_to_outdoor1_pin_ctrl(false);
		// audio_to_outdoor2_pin_ctrl(false);
		return true;
	}
	// power_amplifier_enable(true);
	// ring_to_outdoor_mute_pin_ctrl(false);

	switch (ch)
	{
	case AUDIO_CH_DOOR1:
		audio_to_outdoor1_pin_ctrl(true);
		break;
	case AUDIO_CH_DOOR2:
		audio_to_outdoor2_pin_ctrl(true);
		break;
	default:
		break;
	}
	return true;
}

/***
** 日期: 2022-05-21 08:52
** 作者: leo.liu
** 函数作用：door 1开锁处理
** 返回参数说明：
***/
void monitor_unlock_open(int num, MON_CH ch)
{
	if (ch == MON_CH_DOOR1)
	{
		door1_unlock_pin_ctrl(true); // 门口机1开锁（GPIO74）
	}
	else if (ch == MON_CH_DOOR2)
	{
		door2_unlock_pin_ctrl(true); // 新增：门口机2开锁（GPIO73）
	}
}
/***
** 日期: 2022-05-21 09:01
** 作者: leo.liu
** 函数作用：关锁
** 返回参数说明：
***/
void monitor_unlcok_close(void)
{
	door1_unlock_pin_ctrl(false);
	door2_unlock_pin_ctrl(false);
}
/***
**   日期:2022-05-26 16:47:55
**   作者: leo.liu
**   函数作用：判断sensor1电平是否正常
**   参数说明:
***/
bool sercurity_sensor1_normal_check(void)
{
	GPIO_LEVEL level;
	if (gpio_level_read(SENSOR1_DET_PIN, &level) == false)
	{
		return false;
	}
	return level == GPIO_LEVEL_LOW ? true : false;
}
/***
**   日期:2022-05-26 16:47:55
**   作者: leo.liu
**   函数作用：判断sensor2电平是否正常
**   参数说明:
***/
bool sercurity_sensor2_normal_check(void)
{
	GPIO_LEVEL level;
	if (gpio_level_read(SENSOR2_DET_PIN, &level) == false)
	{
		return false;
	}
	return level == GPIO_LEVEL_LOW ? true : false;
}
/***
**   日期:2022-05-23 09:31:58
**   作者: leo.liu
**   函数作用：打开声音录制
**   参数说明:
***/
void monitor_record_pin_enable(bool en)
{
	if (en == false)
	{
		return;
	}
	audio_to_outdoor1_pin_ctrl(true);
	audio_to_outdoor2_pin_ctrl(true);
}

// false:挂断 true:抬起
bool hook_state_get(void)
{
	return hook_state;
}