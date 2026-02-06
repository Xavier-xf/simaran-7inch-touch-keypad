#include "user_monitor.h"
#include "user_gpio.h"
// #include "gsm6502.h"
#include "tp9950.h"
#include "video_input.h"
#include "unistd.h"
#include "video_decode.h"
#include "user_intercom.h"

/***
** 日期: 2022-05-12 15:17
** 作者: leo.liu
** 函数作用：监控状态
** 返回参数说明：
***/
static bool monitor_enable = false;
bool is_monior_enable(void)
{
	return monitor_enable;
}

static bool monitor_valid_ch[MON_CH_TOTAL] = {false};

static MON_CH montor_channel = MON_CH_NONE;
/***
** 日期: 2022-05-12 11:32
** 作者: leo.liu
** 函数作用：设置监控通道
** 返回参数说明：
***/
bool monitor_channel_set(MON_CH ch)
{
	montor_channel = ch;
	return true;
}

/***
** 日期: 2022-05-12 11:33
** 作者: leo.liu
** 函数作用：获取监控通道
** 返回参数说明：
***/
MON_CH monitor_channel_get(void)
{
	return montor_channel;
}

/***
** 日期: 2022-05-12 11:17
** 作者: leo.liu
** 函数作用：打开户外机电源
** 返回参数说明：
***/
static void outdoor_power_enable(MON_CH ch, bool en)
{
	switch (ch)
	{
	case MON_CH_DOOR1:
		door1_power_enable(en);
		door2_power_enable(false);
		CCTV1_power_enable(false);
		CCTV2_power_enable(false);
		// door_audio_video_select_pin_ctrl(false);
		// door_audio_video_enable_pin_ctrl(true);
		break;
	case MON_CH_DOOR2:
		door2_power_enable(en);
		door1_power_enable(false);
		CCTV1_power_enable(false);
		CCTV2_power_enable(false);
		// door_audio_video_select_pin_ctrl(true);
		// door_audio_video_enable_pin_ctrl(true);
		break;
	case MON_CH_CCTV1:
		CCTV1_power_enable(en);
		door1_power_enable(false);
		door2_power_enable(false);
		CCTV2_power_enable(false);
		// door_audio_video_select_pin_ctrl(true);
		// door_audio_video_enable_pin_ctrl(true);
		break;
	case MON_CH_CCTV2:
		CCTV2_power_enable(en);
		door1_power_enable(false);
		door2_power_enable(false);
		CCTV1_power_enable(false);
		// door_audio_video_select_pin_ctrl(true);
		// door_audio_video_enable_pin_ctrl(true);
		break;
	default:
		door1_power_enable(false);
		door2_power_enable(false);
		CCTV1_power_enable(false);
		CCTV2_power_enable(false);
		// door_audio_video_select_pin_ctrl(false);
		// door_audio_video_enable_pin_ctrl(false);
		break;
	}
}

/***
** 日期: 2022-05-12 11:23
** 作者: leo.liu
** 函数作用：开启模拟视频流向
** 返回参数说明：
** flg:0x01:only ch,0x02:obly inter, 0x03:ch and inter
***/
#define SWITCH_DATA_SET(data, flg, out1, out2, in) \
	data[out1][in] = (flg & 0x01) ? 1 : 0;         \
	data[out2][in] = (flg & 0x02) ? 1 : 0;

void video_switch_enable(MON_CH ch, char flg)
{
	// unsigned char data[6][8] = {{0}, {0}};
	// if (ch == MON_CH_DOOR1)
	// {
	// 	SWITCH_DATA_SET(data, flg, OUT_BT601, OUT_INT_BT601, IN_DOOR1);
	// }
	// else if (ch == MON_CH_DOOR2)
	// {
	// 	SWITCH_DATA_SET(data, flg, OUT_BT601, OUT_INT_BT601, IN_DOOR2);
	// }
	// else if (ch == MON_CH_CCTV1)
	// {
	// 	SWITCH_DATA_SET(data, flg, OUT_BT601, OUT_INT_BT601, IN_CCTV1);
	// }
	// else if (ch == MON_CH_CCTV2)
	// {
	// 	SWITCH_DATA_SET(data, flg, OUT_BT601, OUT_INT_BT601, IN_CCTV2);
	// }
	// else if (ch == MON_CH_INTERCOM)
	// {
	// 	SWITCH_DATA_SET(data, flg, OUT_BT601, OUT_INT_BT601, IN_IN1_DOOR);
	// }
	// gsm6502_open(data);
}
/***
**   日期:2022-06-01 17:21:28
**   作者: leo.liu
**   函数作用：关闭模拟视频通道
**   参数说明:
***/
void video_switch_disable(void)
{
	// gsm6502_close();
}
/***
** 日期: 2022-05-12 11:34
** 作者: leo.liu
** 函数作用：开启监控
** 返回参数说明：mask:0x01:indoor, 0x02:intercom
***/
bool monitor_open(bool preview, unsigned char mask)
{
	if (montor_channel == MON_CH_NONE)
	{
		return false;
	}
	// monitor_close();
	video_display_preview_enable(false);
	video_input_skip_frame_count_set(1000);
	// tp9950_vin_enable(montor_channel, false);
	// outdoor_power_enable(montor_channel, false);
	// monitor_enable = false;
	// video_switch_enable(montor_channel, mask);
	// if (OwnID == 1)
	// {

	// if (montor_channel == MON_CH_DOOR1 || montor_channel == MON_CH_DOOR2)
	outdoor_power_enable(montor_channel, true);
	// }
	if (mask & 0x01)
	{
		video_display_preview_enable(preview);
		tp9950_vin_enable(montor_channel, true);
		// tp9950_vin1_enable(true);
		tp9950_restart_det();
	}
	monitor_enable = true;
	return true;
}
/***
**   日期:2022-06-11 11:30:03
**   作者: leo.liu
**   函数作用：设置监控位置
**   参数说明:
***/
bool monitor_preivew_pos_set(int x, int y, int w, int h)
{
	return video_input_display_pos(x, y, w, h);
}

/***
** 日期: 2022-05-12 11:36
** 作者: leo.liu
** 函数作用：关闭监控
** 返回参数说明：
***/
bool monitor_close(void)
{
	// video_switch_disable();
	video_display_preview_enable(false);
	ak_sleep_ms(50);
	video_input_skip_frame_count_set(1000);
	ak_sleep_ms(50);
	tp9950_vin_enable(montor_channel, false);
	// tp9950_vin1_enable(false);
	outdoor_power_enable(MON_CH_NONE, false);
	monitor_enable = false;
	return true;
}

/***
** 日期: 2022-05-16 16:25
** 作者: leo.liu
** 函数作用：判断此通道时候有信号
** 返回参数说明：
***/
bool monitor_valid_channel_check(MON_CH channel)
{
	return monitor_valid_ch[channel];
}

/***
** 日期: 2022-05-16 16:38
** 作者: leo.liu
** 函数作用：设置通道有效
** 返回参数说明：
***/
void monitor_valid_channel_set(MON_CH ch, bool sate)
{
	monitor_valid_ch[ch] = sate;
}
