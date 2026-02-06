#include "video_encode.h"
#include <pthread.h>
#include <string.h>
#include "video_input.h"
#include <unistd.h>
#include "user_common.h"
#include "video_input.h"
#include "tuya/tuya_api.h"
#include "tuya_ring_buffer.h"



#include "ak_common_graphics.h"
#include "ak_common.h"
#include "video_decode.h"
#include "ak_mem.h"
#include "ak_tde.h"
#include "ak_vi.h"

bool video_record_video_write(unsigned char *data, int size);
static int mjpeg_encode_handle_id = -1;
static bool mjpeg_encode_enable = false;
static char jpg_encode_mask = 0x00;
static pthread_mutex_t jpg_encode_mutex;
static int jpg_frame_count = 0;
static void (*mjpeg_encode_output_func)(struct video_stream *farme) = NULL;
static int jpg_skip_frame_count = 0;
/***
** 日期: 2022-05-18 09:19
** 作者: leo.liu
** 函数作用：关闭编码设备
** 返回参数说明：
***/
static void mjpeg_encode_device_close(void)
{
	ak_venc_close(mjpeg_encode_handle_id);
	mjpeg_encode_handle_id = -1;
}
/***
** 日期: 2022-05-18 09:19
** 作者: leo.liu
** 函数作用：开启编码设备
** 返回参数说明：
***/
static bool mjpeg_encode_device_open(void)
{
	struct venc_param ve_param;
	ve_param.width = SUB_VIDEO_PIXEL_WIDTH;
	ve_param.height = SUB_VIDEO_PIXEL_HIGHT;
	ve_param.fps = 20;  // fps set    20 25
	ve_param.goplen = 40;  // gop set  40 50
	ve_param.target_kbps = 1024;//4096;///1024; // k bps
	ve_param.max_kbps = 2048;//10240;//2048; // max kbps
	ve_param.br_mode = BR_MODE_VBR;//BR_MODE_CBR; // br mode  (BR_MODE_VBR   3.27)
	ve_param.minqp = 25;  // qp set      25  40
	ve_param.maxqp = 45;  // qp max value <=100 50 100
	// ve_param.initqp       = (ve_param.minqp + ve_param.maxqp) / 2;  //qp value
	ve_param.jpeg_qlevel = JPEG_QLEVEL_LOW; // jpeg qlevel     (JPEG_QLEVEL_DEFAULT  3.27   JPEG_QLEVEL_HIGH)
	ve_param.chroma_mode = CHROMA_4_2_0;     // chroma mode
	ve_param.max_picture_size = 15 * 1024;//0;//30 * 1024;     // 0 means default
	ve_param.enc_level = 40;//30;      // enc level
	ve_param.smart_mode = SMART_LTR;     // smart mode set
	ve_param.smart_goplen = 100;      // smart mode value
	ve_param.smart_quality = 50;      // quality 50
	ve_param.smart_static_value = 0;     // value
	ve_param.enc_out_type = MJPEG_ENC_TYPE;     // enc type
	/* jpeg enc profile */
	ve_param.initqp = 40;//50;<=100
	ve_param.profile = PROFILE_JPEG;
	if (ak_venc_open(&ve_param, &mjpeg_encode_handle_id))
	{
		printf("open venc fail \n\r");
		return false;
	}
	return true;
}




static void *mjpeg_encode_task(void *arg)
{
	unsigned long long pre_timestamp = 0;
	printf("***** jpg encode task create sccess ! *****\n");
	while (1)
	{
		pthread_mutex_lock(&jpg_encode_mutex);
		if ((mjpeg_encode_enable == false) && (mjpeg_encode_handle_id != -1))
		{
			mjpeg_encode_device_close();
			jpg_encode_mask = 0;
			mjpeg_encode_output_func = NULL;
		}
		else if ((mjpeg_encode_enable == true) && (mjpeg_encode_handle_id == -1))
		{
			mjpeg_encode_device_open();
		}
		else if ((mjpeg_encode_enable == true) && (mjpeg_encode_handle_id != -1) && (jpg_encode_mask != 0))
		{
			unsigned long long timestamp;

			unsigned char *data = video_input_sub_resident_buffer_get(&timestamp);
			if (timestamp != pre_timestamp)
			{
				pre_timestamp = timestamp;
				struct video_stream stream;
				memset(&stream, 0, sizeof(struct video_stream));
				if (ak_venc_encode_frame(mjpeg_encode_handle_id, data, SUB_VIDEO_PIXEL_HIGHT * SUB_VIDEO_PIXEL_WIDTH * 3 / 2, NULL, &stream) == 0)
				{
					if (jpg_skip_frame_count > 0)
					{
						jpg_skip_frame_count--;
					}
					else
					{
						if (jpg_encode_mask & 0x01)
						{
							video_record_video_write(stream.data, stream.len);
						}
						if (jpg_encode_mask & 0x02)
						{
							// if (tuya_api_online_check() == true)
							// {
							// 	tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, stream.data, stream.len, stream.frame_type == FRAME_TYPE_I ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME, user_timestamp_get());
							// }
						}
						if ((jpg_encode_mask & 0x04) && (mjpeg_encode_output_func != NULL))
						{
							pthread_mutex_unlock(&jpg_encode_mutex);
							mjpeg_encode_output_func(&stream);
							pthread_mutex_lock(&jpg_encode_mutex);
						}
					}
				}
				ak_venc_release_stream(mjpeg_encode_handle_id, &stream);
			}
		}
		pthread_mutex_unlock(&jpg_encode_mutex);
		usleep(10 * 1000);
	}
	return NULL;
}

/***
** 日期: 2022-05-18 08:36
** 作者: leo.liu
** 函数作用：设备初始化
** 返回参数说明：
***/
bool mjpeg_encode_init(void)
{
	pthread_t thread_t;
	pthread_mutex_init(&jpg_encode_mutex, NULL);
	pthread_create(&thread_t, user_pthread_atter_get(), mjpeg_encode_task, NULL);
	return true;
}

/***
** 日期: 2022-05-18 08:36
** 作者: leo.liu
** 函数作用：开启设备
** 返回参数说明：
***/
bool mjpeg_encode_open(void (*callback)(struct video_stream *farme), unsigned char mask)
{
	pthread_mutex_lock(&jpg_encode_mutex);
	if ((jpg_encode_mask & mask) != 0x00)
	{
		pthread_mutex_unlock(&jpg_encode_mutex);
		return false;
	}
	if (callback != NULL)
	{
		mjpeg_encode_output_func = callback;
	}
	jpg_encode_mask |= mask;
	mjpeg_encode_enable = true;
	pthread_mutex_unlock(&jpg_encode_mutex);
	return true;
}

/***
** 日期: 2022-05-18 08:38
** 作者: leo.liu
** 函数作用：关闭编码
** 返回参数说明：
***/
bool mjpeg_encode_close(unsigned char mask)
{
	// printf("===============[%s]=========[%d]\n",__func__,__LINE__);
	pthread_mutex_lock(&jpg_encode_mutex);
	// printf("===============[%s]=========[%d]\n",__func__,__LINE__);
	if ((jpg_encode_mask & mask) == 0x00)
	{
		// printf("===============[%s]=========[%d]\n",__func__,__LINE__);
		pthread_mutex_unlock(&jpg_encode_mutex);
		return true;
	}
	jpg_encode_mask &= (~mask);
	if (jpg_encode_mask == 0x00)
	{
		mjpeg_encode_enable = false;
	}
	// printf("===============[%s]=========[%d]\n",__func__,__LINE__);
	pthread_mutex_unlock(&jpg_encode_mutex);
	// printf("===============[%s]=========[%d]\n",__func__,__LINE__);
	return true;
}

/***
** 日期: 2022-05-19 09:50
** 作者: leo.liu
** 函数作用：编码状态
** 返回参数说明：
***/
bool mjpeg_encode_status_get(void)
{
	return ((mjpeg_encode_handle_id != -1) || (mjpeg_encode_enable == true));
}
/***
**   日期:2022-06-08 08:17:53
**   作者: leo.liu
**   函数作用：设置过滤前面的n帧
**   参数说明:
***/
bool jpg_encode_skip_frame(int n)
{
	pthread_mutex_lock(&jpg_encode_mutex);
	if (jpg_encode_mask == 0x00)
	{
		pthread_mutex_unlock(&jpg_encode_mutex);
		return true;
	}
	jpg_frame_count = n;
	pthread_mutex_unlock(&jpg_encode_mutex);
	return true;
}
/***
**   日期:2022-05-26 08:39:18
**   作者: leo.liu
**   函数作用：编码采集开关
**   参数说明:
***/
void jpg_encode_capture_enable(bool en)
{
	pthread_mutex_lock(&jpg_encode_mutex);
	mjpeg_encode_enable = en;
	pthread_mutex_unlock(&jpg_encode_mutex);
}
