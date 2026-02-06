#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "tuya_ring_buffer.h"
#include "tuya_ipc_media.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_p2p.h"
#include "tuya_g711_utils.h"
#include "lv_msg_event.h"
#include "audio_output.h"
#include "tuya_api.h"
/***
**   日期:2022-06-07 16:46:00
**   作者: leo.liu
**   函数作用：用于标记第一次接收到音频数据
**   参数说明:
***/
static bool tuya_receive_audio_first = false;

/***
**   日期:2022-06-07 14:46:12
**   作者: leo.liu
**   函数作用：p2p状态发生改变
**   参数说明:暂时为用到
***/
STATIC VOID __depereated_online_cb(IN TRANSFER_ONLINE_E status)
{
}

/***
**   日期:2022-06-07 14:47:04
**   作者: leo.liu
**   函数作用：设备端接收app端的音频数据
**   参数说明:
***/
STATIC VOID tuya_device_audio_receive_cb(IN CONST TRANSFER_AUDIO_FRAME_S *p_audio_frame, IN CONST UINT_T frame_no)
{
	if (tuya_receive_audio_first == true)
	{
		/*****  用于挂断其他设备 *****/
		tuya_api_preview_quit();
		audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_8000);
		tuya_receive_audio_first = false;
	}
	unsigned char pcm_data[640] = {0};
	unsigned int pcm_len = 0;
	tuya_g711_decode(TUYA_G711_MU_LAW, (unsigned short *)p_audio_frame->p_audio_buf, p_audio_frame->buf_len, pcm_data, &pcm_len);
	// audio_output_write(pcm_data, pcm_len);
}

/***
**   日期:2022-06-07 14:47:43
**   作者: leo.liu
**   函数作用：p2ps事件接收
**   参数说明:
***/
STATIC INT_T tuya_p2p_event_cb(IN CONST TRANSFER_EVENT_E event, IN CONST PVOID_T args)
{
	switch (event)
	{
	case TRANS_LIVE_VIDEO_START:
	{
		printf("==================================\n");
		if (tuya_ipc_get_client_online_num() > 1)
		{
			break;
		}
		tuya_api_ringbuffer_clear();
		lv_msg_send_cmd(MSG_EVENT_CMD_TUYA, TUYA_EVENT_CMD_VIDEO_START, 0);
	}
	break;
	case TRANS_LIVE_VIDEO_STOP:
	{
		if (tuya_ipc_get_client_online_num() > 1)
		{
			break;
		}
		unsigned char data[1024];
		tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, data, 1024, E_VIDEO_PB_FRAME, user_timestamp_get());
		tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, data, 1024, E_AUDIO_FRAME, user_timestamp_get());
		lv_msg_send_cmd(MSG_EVENT_CMD_TUYA, TUYA_EVENT_CMD_VIDEO_STOP, 0);
	}
	break;
	case TRANS_SPEAKER_START:
	{
		tuya_receive_audio_first = true;
		lv_msg_send_cmd(MSG_EVENT_CMD_TUYA, TUYA_EVENT_CMD_AUDIO_START, 0);
	}
	break;
	default:

		break;
	}
	return 0;
}
/***
**   日期:2022-06-07 14:34:10
**   作者: leo.liu
**   函数作用：p2p接口
**   参数说明:
***/
void tuya_p2p_transfer_enable(void)
{
	TUYA_IPC_TRANSFER_VAR_S p2p_var = {0};
	memset(&p2p_var, 0, sizeof(TUYA_IPC_TRANSFER_VAR_S));
	p2p_var.online_cb = __depereated_online_cb;
	p2p_var.on_rev_audio_cb = tuya_device_audio_receive_cb;

	p2p_var.rev_audio_codec = TUYA_CODEC_AUDIO_G711U; // TUYA_CODEC_AUDIO_PCM;
	p2p_var.audio_sample = TUYA_AUDIO_SAMPLE_8K;
	p2p_var.audio_databits = TUYA_AUDIO_DATABITS_16;
	p2p_var.audio_channel = TUYA_AUDIO_CHANNEL_MONO;
	p2p_var.on_event_cb = tuya_p2p_event_cb;
	p2p_var.live_quality = TRANS_LIVE_QUALITY_MAX; // TRANS_LIVE_QUALITY_MAX;
	p2p_var.max_client_num = 5;

	IPC_MEDIA_INFO_S ringbuffer;
	extern void tuya_ringbuffer_init(IPC_MEDIA_INFO_S * ringbuffer);
	tuya_ringbuffer_init(&ringbuffer);
	memcpy(&p2p_var.AVInfo, &ringbuffer, sizeof(IPC_MEDIA_INFO_S));
	tuya_ipc_tranfser_init(&p2p_var);
}