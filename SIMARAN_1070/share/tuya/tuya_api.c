#include "tuya_api.h"
#include <pthread.h>
#include <string.h>
#include "tuya_ipc_api.h"
#include "wifi_api.h"
#include "tuya_uuid_and_key.h"
#include "tuya_cloud_wifi_defs.h"
#include "tuya_cloud_base_defs.h"
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "lv_msg_event.h"
#include "unistd.h"
#include "tuya_ipc_p2p.h"
#include "user_common.h"
static char tuya_pid_string[64] = {0};
static bool is_tuya_online = false;
/***
**   日期:2022-06-07 10:44:05
**   作者: leo.liu
**   函数作用：满足tuya接口需求，暂未用到
**   参数说明:
***/
static void tuya_dp_query_func(IN CONST TY_DP_QUERY_S *dp_query)
{
}
/***
**   日期:2022-06-07 10:45:18
**   作者: leo.liu
**   函数作用：升级接口，暂未用到
**   参数说明:
***/
static void tuya_upgrade_func(CONST FW_UG_S *fw)
{
}
/***
**   日期:2022-06-07 10:47:29
**   作者: leo.liu
**   函数作用：重启接口，暂未用到
**   参数说明:
***/
static void tuya_sdk_restart_func(void)
{
}

/***
**   日期:2022-06-07 19:30:28
**   作者: leo.liu
**   函数作用：通道切换处理
**   参数说明:
***/
/************************************************************************************************************
下发指令的格式为：
{"cmd":1,"cc":1,"chs":[{"id":1,"n":"door1"},{"id":2,"n":"door2"},{"id":3,"n":"CCTV1"},{"id":4,"n":"CCTV2"}]}

反馈指令的格式为:
"{\\\"res\\\":1,\\\"err\\\":0,\\\"cc\\\":1,\\\"chs\\\":[{\\\"id\\\":1,\\\"n\\\":\\\"door1\\\"},{\\\"id\\\":2,\\\"n\\\":\\\"door2\\\"},{\\\"id\\\":3,\\\"n\\\":\\\"CCTV1\\\"},{\\\"id\\\":4,\\\"n\\\":\\\"CCTV2\\\"}]}\"}"
************************************************************************************************************/

/***
**   日期:2022-06-07 10:43:19
**   作者: leo.liu
**   函数作用：dp处理接口
**   参数说明:
***/
static void tuya_dp_handle_func(IN CONST TY_RECV_OBJ_DP_S *dp_rev)
{
	TY_OBJ_DP_S *dp_data = (TY_OBJ_DP_S *)(dp_rev->dps);
	switch (dp_data->dpid)
	{
	case 231:
	{
		//*****  切换通道 *****/
		char *down_head_str = {"{\"cmd\":1,\"cc\":"};
		if (strncmp(dp_data->value.dp_str, down_head_str, strlen(down_head_str)) != 0)
		{
			printf("String error \n\r");
			return;
		}
		int channel_id = dp_data->value.dp_str[strlen(down_head_str)] - 48;
		lv_msg_send_cmd(MSG_EVENT_CMD_TUYA, TUYA_EVENT_CMD_CH_CHANGE, channel_id);
	}
	break;
	case 134:
	{
		/*****  移动侦测开关 *****/
		tuya_ipc_dp_report(NULL, dp_data->dpid, PROP_BOOL, &(dp_data->value.dp_bool), 1);
		lv_msg_send_cmd(MSG_EVENT_CMD_TUYA, TUYA_EVENT_CMD_MOTION_ENBALE, dp_data->value.dp_bool);
	}
	case 232:
	{
		/*****  开锁 *****/
		unsigned int value = 0;
		sscanf(dp_data->value.dp_str, "%d", &value);
		lv_msg_send_cmd(MSG_EVENT_CMD_TUYA, TUYA_EVENT_CMD_DOOR_OPEN, value);
		break;
	}
	default:
	{
		printf("############### dp unkonw:%d ###############\n", dp_data->dpid);
	}
	}
}
/***
**   日期:2022-06-07 10:44:30
**   作者: leo.liu
**   函数作用：tuya状态改变接口
**   参数说明:
***/
static void tuya_status_channge_func(IN CONST BYTE_T stat)
{
	switch ((stat))
	{
#if defined(WIFI_GW) && (WIFI_GW == 1)
	case STAT_CLOUD_CONN:  // for wifi ipc
	case STAT_MQTT_ONLINE: // for low-power wifi ipc
#endif
#if defined(WIFI_GW) && (WIFI_GW == 0)
	case GB_STAT_CLOUD_CONN: // for wired ipc
#endif
	{
		/*****  tuya 上线 *****/
		static bool tuya_p2p_enable = false;
		if (tuya_p2p_enable == false)
		{
			tuya_p2p_enable = true;
			extern void tuya_p2p_transfer_enable(void);
			tuya_p2p_transfer_enable();
			tuya_ipc_upload_skills();
		}
		lv_msg_send_cmd(MSG_EVENT_CMD_TUYA, TUYA_EVENT_CMD_ONLINE, 0);
		is_tuya_online = true;
		printf("############### tuya  in-line###############\n");
	}
	break;
	case WF_START_AP_ONLY:
	case STAT_MQTT_OFFLINE:
	{
		/*****  tuya 离线 *****/
		is_tuya_online = false;
		printf("############### tuya  off-line###############\n");
	}
	break;
	default:
	{
		printf("############### status change unkonw:%d ###############\n", stat);
		break;
	}
	}
}
/***
**   日期:2022-06-07 10:46:35
**   作者: leo.liu
**   函数作用：删除tuya配置文件
**   参数说明:
***/
static void tuya_reset_system_func(GW_RESET_TYPE_E type)
{
	remove(TUYA_UUID_AND_KEY_CONF_PATH "tuya_user.db_bak");
	remove(TUYA_UUID_AND_KEY_CONF_PATH "tuya_user.db");
	remove(TUYA_UUID_AND_KEY_CONF_PATH "tuya_enckey.db");
	system("sync");
	printf("##### tuya unbundle \n");
	system("reboot");
}

/***
**   日期:2022-06-07 14:28:37
**   作者: leo.liu
**   函数作用：音视频循环队列初始化
**   参数说明:
***/
void tuya_ringbuffer_init(IPC_MEDIA_INFO_S *ringbuffer)
{
	memset(ringbuffer, 0, sizeof(IPC_MEDIA_INFO_S));
	ringbuffer->channel_enable[E_CHANNEL_VIDEO_MAIN] = TRUE;		 /* Whether to enable local HD video streaming */
	ringbuffer->video_fps[E_CHANNEL_VIDEO_MAIN] = 25;			 /* FPS */
	ringbuffer->video_gop[E_CHANNEL_VIDEO_MAIN] = 30;			 /* GOP */
	ringbuffer->video_bitrate[E_CHANNEL_VIDEO_MAIN] = TUYA_VIDEO_BITRATE_1M; /* Rate limit */
	ringbuffer->video_width[E_CHANNEL_VIDEO_MAIN] = 640;			 /* Single frame resolution of width*/
	ringbuffer->video_height[E_CHANNEL_VIDEO_MAIN] = 360;			 /* Single frame resolution of height */
	ringbuffer->video_freq[E_CHANNEL_VIDEO_MAIN] = 90000;			 /* Clock frequency */
	ringbuffer->video_codec[E_CHANNEL_VIDEO_MAIN] = TUYA_CODEC_VIDEO_H264;
	// TUYA_CODEC_VIDEO_H264; // TUYA_CODEC_VIDEO_MJPEG;
	//  TUYA_CODEC_VIDEO_H264; /* Encoding format */

	ringbuffer->channel_enable[E_CHANNEL_AUDIO] = TRUE;		      /* Whether to enable local sound collection */
	ringbuffer->audio_codec[E_CHANNEL_AUDIO] = TUYA_CODEC_AUDIO_G711U;    // TUYA_CODEC_AUDIO_PCM; //TUYA_CODEC_AUDIO_PCM /* Encoding format */
	ringbuffer->audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_8K;     // TUYA_AUDIO_SAMPLE_8K /* Sampling Rate */
	ringbuffer->audio_databits[E_CHANNEL_AUDIO] = TUYA_AUDIO_DATABITS_16; /* Bit width */
	ringbuffer->audio_channel[E_CHANNEL_AUDIO] = TUYA_AUDIO_CHANNEL_MONO; // TUYA_AUDIO_CHANNEL_MONO;/* channel */
	ringbuffer->audio_fps[E_CHANNEL_AUDIO] = 25;			      /* Fragments per second */
}
/***
**   日期:2022-06-07 10:23:20
**   作者: leo.liu
**   函数作用：tuya sdk初始化
**   参数说明:
***/
static void tuya_sdk_init(void)
{
	unsigned char uuid[128] = {0};
	unsigned char key[128] = {0};
	tuya_uuid_and_key_read(uuid, key);
	printf("\n\n#####[uuid:%s] [key:%s]#####\n\n\n", uuid, key);

	IPC_MEDIA_INFO_S ringbuffer;
	tuya_ringbuffer_init(&ringbuffer);
	tuya_ipc_ring_buffer_init(E_CHANNEL_VIDEO_MAIN, ringbuffer.video_bitrate[E_CHANNEL_VIDEO_MAIN], ringbuffer.video_fps[E_CHANNEL_VIDEO_MAIN], 0, NULL);
	tuya_ipc_ring_buffer_init(E_CHANNEL_AUDIO, ringbuffer.audio_sample[E_CHANNEL_AUDIO] * ringbuffer.audio_databits[E_CHANNEL_AUDIO] / 1024, ringbuffer.audio_fps[E_CHANNEL_AUDIO], 0, NULL);
	/*****  tuya信息初始化 *****/
	TUYA_IPC_ENV_VAR_S env;
	memset(&env, 0, sizeof(TUYA_IPC_ENV_VAR_S));
	strcpy(env.storage_path, TUYA_UUID_AND_KEY_CONF_PATH);
	strcpy(env.product_key, tuya_pid_string);
	strcpy(env.uuid, (char *)uuid);
	strcpy(env.auth_key, (char *)key);
	strcpy(env.dev_sw_version, "1.2.3");
	strcpy(env.dev_serial_num, "tuya_ipc");

	/*****  dp回调处理接口 *****/
	env.dev_obj_dp_cb = tuya_dp_handle_func;
	env.dev_dp_query_cb = tuya_dp_query_func;
	/*****  tuya状态改变接口 *****/
	env.status_changed_cb = tuya_status_channge_func;
	env.gw_ug_cb = tuya_upgrade_func;
	env.gw_rst_cb = tuya_reset_system_func,
	env.gw_restart_cb = tuya_sdk_restart_func;
	env.mem_save_mode = false;
	/*****  初始化sdk *****/
	tuya_ipc_init_sdk(&env);

	tuya_ipc_start_sdk(WIFI_INIT_NULL, NULL);
}

/***
**   日期:2022-06-07 10:19:09
**   作者: leo.liu
**   函数作用：tuya网络检测
**   参数说明:
***/
static void *tuya_api_task(void *arg)
{
	bool conneted = false;

	if ((wifi_device_connection_stauts(NULL, NULL, NULL, &conneted, NULL) == false) || (conneted == false))
	{
		wifi_device_conneting();
		wifi_device_open();
	}

	while ((wifi_device_connection_stauts(NULL, NULL, NULL, &conneted, NULL) == false) || (conneted == false))
	{
		usleep(100 * 1000);
	}
	tuya_sdk_init();
	while (1)
	{
		usleep(100 * 1000);
	}
	return NULL;
}

/***
**   日期:2022-06-07 10:12:44
**   作者: leo.liu
**   函数作用：tuya api初始化
**   参数说明:
***/
bool tuya_api_init(const char *pid)
{
	pthread_t thread_t;
	memset(tuya_pid_string, 0, sizeof(tuya_pid_string));
	strcpy(tuya_pid_string, pid);
	pthread_create(&thread_t, user_pthread_atter_get(), tuya_api_task, NULL);
	return true;
}

/***
**   日期:2022-06-07 15:52:37
**   作者: leo.liu
**   函数作用：获取tuya在线状态
**   参数说明:
***/
bool tuya_api_online_check(void)
{
	if (is_tuya_online == false)
	{
		return false;
	}
	return tuya_ipc_get_mqtt_status() ? true : false;
}

/*
 *
 *	dp233,用于设备端上报接收和挂断通话的dp点，因为app端下发通话和挂断已经有其他dp点使用，故不能作为其app下发的dp点。
 *
 *	1(字符串类型):接收通话
 *	   设备端上报到app:设备端已接受通话。
 *
 *	2(字符串类型):挂断通话
 *		设备端上报到app:设备端已挂断通话。
 *
 * 	整体逻辑:
 *		在门铃按下后，设备和app都能够同时预览门铃画面。
 *
 *		场景1:	在设备端按下挂断后->将上传dp233(字符串内容:2)，app端同步挂断。
 *
 *		场景2:	在设备端按下接收通话 ->将上传dp233(字符串内容:1)，app端应该是挂断操作（这个动作需客户端确认）
 *
 *	备注:在每次离开预览页面都会上报dp233+2。每次接受通话后都会上报dp233+1
 *
 */
/***
**   日期:2022-06-07 15:51:54
**   作者: leo.liu
**   函数作用：app退出监控
**   参数说明: 1(字符串类型):接收通话
**	   设备端上报到app:设备端已接受通话。
***/
bool tuya_api_preview_quit(void)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	tuya_ipc_dp_report(NULL, 233, PROP_STR, "1", 1);
	return true;
}
/***
**   日期:2022-06-07 15:58:20
**   作者: leo.liu
**   函数作用：app与
**   参数说明:2(字符串类型):挂断通话
**		设备端上报到app:设备端已挂断通话。
***/
bool tuya_api_monitor_handup(void)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	tuya_ipc_dp_report(NULL, 233, PROP_STR, "2", 1);
	return true;
}
/***
**   日期:2022-06-07 16:10:44
**   作者: leo.liu
**   函数作用：强制时间同步
**   参数说明:
***/
bool tuya_api_app_sync_utc_time(void)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	TIME_T time_utc;
	INT_T time_zone;
	tuya_ipc_get_service_time_force(&time_utc, &time_zone);
	return true;
}

#define SWITCH_CHANNEL_CMD_HEAD "{\\\"cmd\\\":1,\\\"cc\\\":%d,\\\"chs\\\":["
#define TUYA_CHANNEL_CMD_TAIL "]}\"}"
/***
**   日期:2022-06-07 18:15:39
**   作者: leo.liu
**   函数作用：上传有效通道
**   参数说明:channe:当前监控的通道
***/
bool tuya_api_channel_report(int cur, bool door1_valild, const char *door1_str, bool door2_valild, const char *door2_str, bool cctv1_valid, const char *cctv1_str, bool cctv2_valid, const char *cctv2_str)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	if (((door1_valild == false) && (door2_valild == false) && (cctv1_valid == false) && (cctv2_valid == false)))
	{
		return false;
	}

	char dp_str[512] = {0};
	cur = cur == 0 ? 1 : cur;
	sprintf(dp_str, SWITCH_CHANNEL_CMD_HEAD, cur);
	if (door1_valild == true)
	{
		char temp[128] = {0};
		sprintf(temp, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s", 1, door1_str);
		if (cur == 1)
		{
			strcat(temp, " current");
		}
		strcat(temp, "\\\"},");
		strcat(dp_str, temp);
	}
	if (door2_valild == true)
	{
		char temp[128] = {0};
		sprintf(temp, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s", 2, door2_str);
		if (cur == 2)
		{
			strcat(temp, " current");
		}
		strcat(temp, "\\\"},");
		strcat(dp_str, temp);
	}
	if (cctv1_valid == true)
	{
		char temp[128] = {0};
		sprintf(temp, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s", 3, cctv1_str);
		if (cur == 3)
		{
			strcat(temp, " current");
		}
		strcat(temp, "\\\"},");
		strcat(dp_str, temp);
	}
	if (cctv2_valid == true)
	{
		char temp[128] = {0};
		sprintf(temp, "{\\\"id\\\":%d,\\\"n\\\":\\\"%s", 4, cctv2_str);
		if (cur == 4)
		{
			strcat(temp, " current");
		}
		strcat(temp, "\\\"},");
		strcat(dp_str, temp);
	}
	/*****************************
	以为结束的最后一个","是不需要。
	******************************/
	dp_str[strlen(dp_str) - 1] = '\0';
	strcat(dp_str, TUYA_CHANNEL_CMD_TAIL);
	tuya_ipc_dp_report(NULL, 231, PROP_STR, dp_str, 1);
	// printf("report :dp231( %s ) \n", dp_str);
	return true;
}
/***
**   日期:2022-06-08 08:26:18
**   作者: leo.liu
**   函数作用：使tuya重新连接
**   参数说明:
***/
bool tuya_api_reconnect_network(void)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	tuya_ipc_reconnect_wifi();
	return true;
}
/***
**   日期:2022-06-08 08:36:35
**   作者: leo.liu
**   函数作用：上传门2的开锁模式
**   参数说明:  "1"/"2"
***/
bool tuya_api_door2_unlock_mode_report(int mode)
{
	if (tuya_api_online_check() == false)
	{
		printf("###### device off line ###### \n");
		return false;
	}
	char dp_str[6] = {0};
	sprintf(dp_str, "%d", mode);
	tuya_ipc_dp_report(NULL, 234, PROP_STR, dp_str, 1);
	return true;
}
/***
**   日期:2022-06-08 08:50:31
**   作者: leo.liu
**   函数作用：开锁成功失败反馈
**   参数说明:
***/
bool tuya_api_open_door_success_report(bool ok)
{
	if (tuya_api_online_check() == false)
	{
		printf("###### device off line ###### \n");
		return false;
	}
	tuya_ipc_dp_report(NULL, 232, PROP_STR, ok ? "1" : "0", 1);
	return true;
}
/***
**   日期:2022-06-08 09:32:50
**   作者: leo.liu
**   函数作用：获取app连接设备的个数
**   参数说明:
***/
int tuya_api_client_num(void)
{
	if (tuya_api_online_check() == false)
	{
		return 0;
	}
	return tuya_ipc_get_client_online_num();
}
/***
**   日期:2022-06-08 15:34:00
**   作者: leo.liu
**   函数作用：tuya 循环buffer清除
**   参数说明:
***/
bool tuya_api_ringbuffer_clear(void)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}

	tuya_ipc_ring_buffer_clean_user_state_and_buffer(E_CHANNEL_VIDEO_MAIN, E_USER_P2P_USER);
	tuya_ipc_ring_buffer_clean_user_state_and_buffer(E_CHANNEL_AUDIO, E_USER_P2P_USER);
	return true;
}
/***
**   日期:2022-06-08 11:16:24
**   作者: leo.liu
**   函数作用：上传call机照片
**   参数说明:
***/
bool tuya_api_call_event(int channel, const char *jpg, int size)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	tuya_ipc_door_bell_press(DOORBELL_NORMAL, jpg, size, NOTIFICATION_CONTENT_JPEG);
	tuya_ipc_notify_with_event(jpg, size, NOTIFICATION_CONTENT_JPEG, channel == 1 ? NOTIFICATION_NAME_PASSBY : NOTIFICATION_NAME_CAR);
	return true;
}
/***
**   日期:2022-06-08 16:31:58
**   作者: leo.liu
**   函数作用：上传警报事件
**   参数说明:
***/
bool tuya_api_alarm_event(int channel, const char *jpg, int size)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	tuya_ipc_notify_with_event(jpg, size, NOTIFICATION_CONTENT_JPEG, channel == 3 ? NOTIFICATION_NAME_IO_ALARM : NOTIFICATION_NAME_USER_IO);
	return true;
}
/***
**   日期:2022-06-09 18:22:31
**   作者: leo.liu
**   函数作用：移动侦测事件触发
**   参数说明:
***/
bool tuya_api_motion_event(int channel, const char *jpg, int size)
{
	if (tuya_api_online_check() == false)
	{
		return false;
	}
	tuya_ipc_notify_with_event(jpg, size, NOTIFICATION_CONTENT_JPEG, channel == 1 ? NOTIFICATION_NAME_MOTION : channel == 2 ? NOTIFICATION_NAME_LINGER
													       : channel == 3	? NOTIFICATION_NAME_HUMAN
																: NOTIFICATION_NAME_ABNORMAL_SOUND);
	return true;
}