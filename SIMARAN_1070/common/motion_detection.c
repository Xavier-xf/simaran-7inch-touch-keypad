#include "motion_detection.h"
#include "ak_mem.h"
#include "tuya_ipc_video_proc.h"
#include "video_input.h"
#include "video_decode.h"

#define MOTION_WIDTH 320
#define MOTION_HIGHT 240
/***
**   日期:2022-06-09 09:27:35
**   作者: leo.liu
**   函数作用：设备开启了检测功能
**   参数说明:
***/
static bool is_motion_detection_start = false;
/***
**   日期:2022-06-09 09:19:12
**   作者: leo.liu
**   函数作用：初始化标志
**   参数说明:
***/
static bool is_motion_detection_init = false;
/***
**   日期:2022-06-09 09:19:22
**   作者: leo.liu
**   函数作用：缩放后的数据传入对比算法中
**   参数说明:
***/
static unsigned char *motion_detection_pdata = NULL;
static unsigned char *motion_detection_dst = NULL;
/***
**   日期:2022-06-09 09:19:39
**   作者: leo.liu
**   函数作用：灵敏度设置参考
**   参数说明:
***/
static int motion_detection_sensitivity = 0;
static int motion_detection_threshold = 0;
/***
**   日期:2022-06-09 09:20:34
**   作者: leo.liu
**   函数作用：画面已经触发阈值
**   参数说明:
***/
static bool is_motion_detection_move = false;
/***
**   日期:2022-06-09 09:35:28
**   作者: leo.liu
**   函数作用：过滤前面的帧数
**   参数说明:
***/
static int motion_detection_skip_frame = 0;
/***
**   日期:2022-06-09 09:07:48
**   作者: leo.liu
**   函数作用：移动侦测初始化
**   参数说明:
***/
bool motion_detection_init(void)
{
	if (is_motion_detection_init == true)
	{
		printf("The device has been initialized \n");
		return false;
	}
	if (motion_detection_pdata == NULL)
	{
		motion_detection_pdata = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VI, MOTION_WIDTH * MOTION_HIGHT * 3 / 2);
	}
	if (motion_detection_dst == NULL)
	{
		motion_detection_dst = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VI, MOTION_WIDTH * MOTION_HIGHT * 3 / 2);
	}

	is_motion_detection_init = true;
	is_motion_detection_start = false;
	return true;
}
/***
**   日期:2022-06-09 09:08:19
**   作者: leo.liu
**   函数作用：移动侦测销毁
**   参数说明:
***/
bool motion_detection_destory(void)
{
	if (is_motion_detection_init == false)
	{
		printf("The device is not initialized \n");
		return false;
	}
	if (motion_detection_pdata != NULL)
	{
		ak_mem_dma_free(motion_detection_pdata);
		motion_detection_pdata = NULL;
	}
	if (motion_detection_dst != NULL)
	{
		ak_mem_dma_free(motion_detection_dst);
		motion_detection_dst = NULL;
	}
	is_motion_detection_init = false;
	is_motion_detection_start = false;
	return true;
}
/***
**   日期:2022-06-09 09:08:41
**   作者: leo.liu
**   函数作用：开启侦测
**   参数说明:
***/
bool motion_detection_start(void)
{
	if (is_motion_detection_start == true)
	{
		printf("The device has been activated for motion detection \n");
		return false;
	}
	motion_detection_skip_frame = 10;
	is_motion_detection_start = true;
	is_motion_detection_move = false;
	return true;
}
/***
**   日期:2022-06-09 09:08:59
**   作者: leo.liu
**   函数作用：移动侦测停止
**   参数说明:
***/
bool motion_detection_stop(void)
{
	if (is_motion_detection_start == false)
	{
		printf("The device is not turned on \n");
		return false;
	}
	is_motion_detection_move = false;
	is_motion_detection_start = false;
	return true;
}
/***
**   日期:2022-06-09 09:41:58
**   作者: leo.liu
**   函数作用：将数据缩放到固定大小
**   参数说明:
***/
static void motion_detection_data_scale(unsigned char *src_data, unsigned char *dst_data)
{
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_YUV420SP;
	src.pos_height = src.height = SUB_VIDEO_PIXEL_HIGHT;
	src.pos_width = src.width = SUB_VIDEO_PIXEL_WIDTH;
	src.pos_left = src.pos_top = 0;
	ak_mem_dma_vaddr2paddr(src_data, (unsigned long *)&src.phyaddr);

	dst.format_param = GP_FORMAT_YUV420SP;
	dst.pos_height = dst.height = MOTION_HIGHT;
	dst.pos_width = dst.width = MOTION_WIDTH;
	dst.pos_left = dst.pos_top = 0;
	ak_mem_dma_vaddr2paddr(dst_data, (unsigned long *)&dst.phyaddr);
	ak_tde_opt_scale(&src, &dst);
}
/***
**   日期:2022-06-21 08:44:29
**   作者: leo.liu
**   函数作用：判断是否移动
**   参数说明:
***/
static bool motion_detection_move(void)
{
	int i, j, k;
	int piexl_diffret = 0;
	int cell_diffret = 0;
	for (i = 0; i < MOTION_WIDTH; i += 8)
	{
		for (j = 0; j < MOTION_HIGHT; j++)
		{
			for (k = i; k < i + 8; k++)
			{
				if (abs(motion_detection_pdata[MOTION_WIDTH * j + k] - motion_detection_dst[MOTION_WIDTH * j + k]) > motion_detection_threshold)
				{
					piexl_diffret++;
				}
			}

			if (!(j % 8))
			{
				if (piexl_diffret > 8)
				{
					cell_diffret++;
				}
				piexl_diffret = 0;
			}
		}
	}

	struct ak_tde_layer src, dst;
	tde_layer_layer_init(src, GP_FORMAT_YUV420SP, MOTION_WIDTH, MOTION_HIGHT, 0, 0, MOTION_WIDTH, MOTION_HIGHT);
	ak_mem_dma_vaddr2paddr(motion_detection_pdata, (unsigned long *)&src.phyaddr);
	tde_layer_layer_init(dst, GP_FORMAT_YUV420SP, MOTION_WIDTH, MOTION_HIGHT, 0, 0, MOTION_WIDTH, MOTION_HIGHT);
	ak_mem_dma_vaddr2paddr(motion_detection_dst, (unsigned long *)&dst.phyaddr);
	ak_tde_opt_blit(&src, &dst);
	if (cell_diffret > motion_detection_sensitivity)
	{
		printf("\n\n\n===================>> motion detection move \n\n\n\n");
		return true;
	}
	return false;
}
/***
**   日期:2022-06-09 09:09:20
**   作者: leo.liu
**   函数作用：移动侦测检测是否移动
**   参数说明:
***/
bool motion_detection_check(void)
{
	if (is_motion_detection_init == false)
	{
		return false;
	}
	if (is_motion_detection_start == false)
	{
		return false;
	}
	if (video_input_state_get() == false)
	{
		return false;
	}
	if (is_motion_detection_move == true)
	{
		return true;
	}
	static unsigned long long pre_timestamp = 0;
	unsigned long long timestamp;
	unsigned char *data = video_input_sub_resident_buffer_get(&timestamp);
	if (timestamp != pre_timestamp)
	{
		pre_timestamp = timestamp;
		motion_detection_data_scale(data, motion_detection_pdata);
		if ((motion_detection_move() == true) && (motion_detection_skip_frame == 0))
		{
			printf("#####  move #####\n");
			is_motion_detection_move = true;
			return true;
		}
		else if (motion_detection_skip_frame)
		{
			motion_detection_skip_frame--;
			if (motion_detection_skip_frame == 0)
			{
				printf("##### start move ##### \n");
			}
		}
	}
	return false;
}
/***
**   日期:2022-06-09 09:09:43
**   作者: leo.liu
**   函数作用：移动侦测设置灵敏度
**   参数说明:
*    sensivity:取值在0-1200 每个8x8的宏块中有一个移动就+1,在320x240的像素块 中总共有1200个宏块
*    threshold:取值范围在0-255
***/
bool motion_detection_sensivity(int sensivity, int threshold)
{
	/*****  这个参数固定 *****/
	motion_detection_sensitivity = sensivity;
	motion_detection_threshold = threshold;
	return true;
}