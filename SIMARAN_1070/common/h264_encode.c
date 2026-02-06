#include "video_encode.h"
#include <pthread.h>
#include "ak_ai.h"
#include "string.h"
#include <unistd.h>
#include "user_time.h"
#include "video_input.h"
#include "tuya/tuya_api.h"
#include "tuya_ring_buffer.h"
#include "user_common.h"
bool video_record_video_write(unsigned char *data, int size);

static char h264_encode_mask = 0x00;
static int h264_encode_handle_id = -1;
static pthread_mutex_t h264_encode_mutex;

static int skip_frame_count = 0;
/***
** 日期: 2022-05-19 11:46
** 作者: leo.liu
** 函数作用：开启音频采集
** 返回参数说明：
***/
static void h264_encode_device_open(void)
{
#if 1
	struct venc_param ve_param;
	ve_param.width = SUB_VIDEO_PIXEL_WIDTH;
	ve_param.height = SUB_VIDEO_PIXEL_HIGHT;
	ve_param.fps = 25;					 // fps set
	ve_param.goplen = 50;					 // gop set
	ve_param.target_kbps = 800;				 // 800;		 //k bps
	ve_param.max_kbps = 1024;				 // 1024;		 //max kbps
	ve_param.br_mode = BR_MODE_VBR;				 // BR_MODE_VBR;//BR_MODE_CBR; //br mode
	ve_param.minqp = 25;					 // qp set
	ve_param.maxqp = 50;					 // qp max value
	ve_param.initqp = (ve_param.minqp + ve_param.maxqp) / 2; // qp value
	ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;		 // jpeg qlevel
	ve_param.chroma_mode = CHROMA_4_2_0;			 // chroma mode
	ve_param.max_picture_size = 0;				 // 0 means default
	ve_param.enc_level = 50;				 // 50;				 //enc level
	ve_param.smart_mode = 0;				 // smart mode set
	ve_param.smart_goplen = 100;				 // smart mode value
	ve_param.smart_quality = 50;				 // quality
	ve_param.smart_static_value = 0;			 // value
	ve_param.enc_out_type = H264_ENC_TYPE;			 // enc type
	ve_param.profile = PROFILE_MAIN;
#else
	struct venc_param ve_param;
	ve_param.width = SUB_VIDEO_PIXEL_WIDTH;
	ve_param.height = SUB_VIDEO_PIXEL_HIGHT;
	ve_param.fps = 20;		// fps set
	ve_param.goplen = 40;		// gop set
	ve_param.target_kbps = 1024;	// k bps
	ve_param.max_kbps = 2048;	// max kbps
	ve_param.br_mode = BR_MODE_VBR; // br mode
	ve_param.minqp = 25;		// qp set
	ve_param.maxqp = 50;		// qp max value
	// ve_param.initqp       = (ve_param.minqp + ve_param.maxqp) / 2;  //qp value
	ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT; // jpeg qlevel
	ve_param.chroma_mode = CHROMA_4_2_0;	    // chroma mode
	ve_param.max_picture_size = 30 * 1024;	    // 0 means default
	ve_param.enc_level = 30;		    // enc level
	ve_param.smart_mode = SMART_DISABLE;	    // smart mode set
	ve_param.smart_goplen = 100;		    // smart mode value
	ve_param.smart_quality = 50;		    // quality
	ve_param.smart_static_value = 0;	    // value
	ve_param.enc_out_type = MJPEG_ENC_TYPE;	    // enc type
	/* jpeg enc profile */
	ve_param.initqp = 50;
	ve_param.profile = PROFILE_JPEG;
#endif
	ak_venc_open(&ve_param, &h264_encode_handle_id);
}

static void h264_encode_device_close(void)
{
	ak_venc_close(h264_encode_handle_id);
	h264_encode_handle_id = -1;
}
/***
** 日期: 2022-05-19 11:34
** 作者: leo.liu
** 函数作用：音频采集线程
** 返回参数说明：
***/
static void *h264_encode_task(void *arg)
{
	struct video_stream vstream;
	unsigned long long pre_timestamp = 0;
	printf("***** h264 encode task create sccess ! *****\n");

	while (1)
	{
		pthread_mutex_lock(&h264_encode_mutex);
		if ((h264_encode_mask == 0x00) && (h264_encode_handle_id != -1))
		{
			h264_encode_device_close();
			printf("### h264 encode close success ! ###\n");
		}
		else if ((h264_encode_mask) && (h264_encode_handle_id == -1))
		{
			h264_encode_device_open();
			skip_frame_count = skip_frame_count == 0 ? 5 : skip_frame_count;
		}

		if ((h264_encode_mask) && (h264_encode_handle_id != -1))
		{
			unsigned long long timestamp;
			unsigned char *data = video_input_sub_resident_buffer_get(&timestamp);
			if (timestamp != pre_timestamp)
			{
				pre_timestamp = timestamp;
				if (ak_venc_encode_frame(h264_encode_handle_id, data, SUB_VIDEO_PIXEL_WIDTH * SUB_VIDEO_PIXEL_HIGHT * 3 / 2, NULL, &vstream) == 0)
				{
					if (skip_frame_count > 0)
					{
						skip_frame_count--;
						if (skip_frame_count == 0)
						{
							ak_venc_request_idr(h264_encode_handle_id);
						}
					}
					else
					{
						if (h264_encode_mask & 0x01)
						{
							video_record_video_write(vstream.data, vstream.len);
						}
						if (h264_encode_mask & 0x02)
						{
						#if 0	
							if (tuya_api_online_check() == true)
							{
								tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, vstream.data, vstream.len, vstream.frame_type == FRAME_TYPE_I ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME, user_timestamp_get());
							}
						#endif
						}
					}
					ak_venc_release_stream(h264_encode_handle_id, &vstream);
				}
			}
		}
		pthread_mutex_unlock(&h264_encode_mutex);
		usleep(10 * 1000);
	}
	return NULL;
}

/***
** 日期: 2022-05-19 11:32
** 作者: leo.liu
** 函数作用：音频采集设备
** 返回参数说明：
***/
bool h264_encode_init(void)
{
	pthread_t thread_t;
	pthread_mutex_init(&h264_encode_mutex, NULL);
	pthread_create(&thread_t, user_pthread_atter_get(), h264_encode_task, NULL);
	return true;
}

/***
** 日期: 2022-05-19 11:33
** 作者: leo.liu
** 函数作用：音频打开采集
** 返回参数说明：
***/
bool h264_encode_open(unsigned char mask)
{
	pthread_mutex_lock(&h264_encode_mutex);
	if (h264_encode_mask & mask)
	{
		pthread_mutex_unlock(&h264_encode_mutex);
		return false;
	}
	h264_encode_mask |= mask;
	pthread_mutex_unlock(&h264_encode_mutex);
	return true;
}
/***
** 日期: 2022-05-19 11:34
** 作者: leo.liu
** 函数作用：音频关闭
** 返回参数说明：
***/
bool h264_encode_close(unsigned char mask)
{
	pthread_mutex_lock(&h264_encode_mutex);
	if ((h264_encode_mask & mask) == 0x00)
	{
		pthread_mutex_unlock(&h264_encode_mutex);
		return true;
	}
	h264_encode_mask &= (~mask);
	pthread_mutex_unlock(&h264_encode_mutex);
	return true;
}
/***
**   日期:2022-06-08 08:17:53
**   作者: leo.liu
**   函数作用：设置过滤前面的n帧
**   参数说明:
***/
bool h264_encode_skip_frame(int n)
{
	pthread_mutex_lock(&h264_encode_mutex);
	if (h264_encode_mask == 0x00)
	{
		pthread_mutex_unlock(&h264_encode_mutex);
		return true;
	}
	skip_frame_count = n;
	pthread_mutex_unlock(&h264_encode_mutex);
	return true;
}