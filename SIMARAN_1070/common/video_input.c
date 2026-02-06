#include "video_input.h"
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"

#include <fcntl.h>

#include "ak_common_graphics.h"
#include "ak_common.h"
#include "ak_mem.h"
#include "ak_tde.h"
#include "ak_vi.h"
#include "user_time.h"
#include "video_decode.h"
#include "user_common.h"
#include <math.h>
#include "video_encode.h"
static void video_input_sub_display_frame_write(struct video_input_frame *frame);

static sem_t video_input_sem;
pthread_mutex_t video_input_mutex;
pthread_mutex_t video_main_display_mutex;
pthread_mutex_t video_sub_display_mutex;
/***** 用于线程间通讯指令（false:关闭 true：打开） *****/
static bool video_input_enable = false;
/***** isp的状态 *****/
static bool video_input_state = false;
static video_isp_param video_isp_info;

static unsigned char *video_input_resident_buffer = NULL;
static unsigned long long video_input_resident_timestmap = 0;
// static unsigned char *video_input_buffer_2 = NULL;

static int video_input_skip_frame_count = 0;

static unsigned char *video_sub_input_resident_buffer = NULL;
static unsigned long long video_sub_input_resident_timestmap = 0;

static int new_zoom = 100;
static int old_zoom = 100;
static int pos_offset_x = 0;
static int pos_offset_y = 0;

unsigned int crop_pos_x, crop_pos_y, crop_width, crop_hight;
int crop_hight_gap = 0;
int crop_width_gap = 0;
unsigned int new_area = 0;
unsigned int old_area = 0;
unsigned int crop_old_hight = 0;
unsigned int crop_old_width = 0;

static int video_input_display_pos_x = 0;
static int video_input_display_pos_y = 0;
static int video_input_display_pos_w = 0;
static int video_input_display_pos_h = 0;

/***
** 日期: 2022-05-12 15:23
** 作者: leo.liu
** 函数作用：关闭isp
** 返回参数说明：
***/
static bool video_isp_close(void)
{
	//	ak_vi_disable_chn(VIDEO_CHN1);
	ak_vi_disable_chn(VIDEO_CHN0);
	ak_vi_disable_dev(VIDEO_DEV0);
	ak_vi_close(VIDEO_DEV0);
	return true;
}

/***
** 日期: 2022-05-12 15:37
** 作者: leo.liu
** 函数作用：打开isp
** 返回参数说明：
***/
static bool video_isp_open(video_isp_param *param)
{
	if (ak_vi_open(VIDEO_DEV0) != 0)
	{
		printf("vi open failed \n");
		return false;
	}

	/// ak_vi_load_sensor_cfg(VIDEO_DEV0, param->isp_conf_file);

	VI_DEV_ATTR dev_attr;
	memset(&dev_attr, 0, sizeof(VI_DEV_ATTR));
	dev_attr.dev_id = VIDEO_DEV0;
	dev_attr.crop.left = 0;
	dev_attr.crop.top = 0;
	dev_attr.crop.width = param->main_width;
	dev_attr.crop.height = param->main_hgith;

	dev_attr.max_width = param->main_width;
	dev_attr.max_height = param->main_hgith;
	dev_attr.frame_rate = 25;
	dev_attr.sub_max_width = 18;
	// param->sub_width;
	dev_attr.sub_max_height = 18;
	// param->sub_hight;

	if (ak_vi_set_dev_attr(VIDEO_DEV0, &dev_attr))
	{
		printf("vi device set device attribute failed!\n");
		ak_vi_close(VIDEO_DEV0);
		return false;
	}

	VI_CHN_ATTR chn_attr;
	memset(&chn_attr, 0, sizeof(VI_CHN_ATTR));
	chn_attr.chn_id = VIDEO_CHN0;
	chn_attr.res.width = param->main_width;
	chn_attr.res.height = param->main_hgith;
	chn_attr.frame_depth = 3;
	chn_attr.frame_rate = 0; // param->fps;
	if (ak_vi_set_chn_attr(VIDEO_CHN0, &chn_attr))
	{
		printf("vi device set channel [%d] attribute failed!\n", VIDEO_CHN0);
		ak_vi_close(VIDEO_DEV0);
		return -1;
	}
#if 0
	/***********************
	用于h.264编码
	***********************/
	VI_CHN_ATTR chn_attr_sub;
	memset(&chn_attr_sub, 0, sizeof(VI_CHN_ATTR));
	chn_attr_sub.chn_id = VIDEO_CHN1;
	chn_attr_sub.res.width = param->sub_width;
	chn_attr_sub.res.height = param->sub_hight;
	chn_attr_sub.frame_depth = 3;
	chn_attr_sub.frame_rate = param->fps;

	if (ak_vi_set_chn_attr(VIDEO_CHN1, &chn_attr_sub))
	{
		printf("vi device set channel [%d] attribute failed!\n", VIDEO_CHN1);
		ak_vi_close(VIDEO_DEV0);
		return -1;
	}
#endif
	/***********************************************************************
	setp1:将向 tp9950_sensor_get_parameter_func,获取接口为DVP
	setp2:将向 tp9950_sensor_get_parameter_func,未知
	setp3:将向 tp9950_sensor_get_parameter_func,未知
	setp4:将向 tp9950_sensor_set_power_on_func 复位开启tp9950的电源
	setp5:将向 tp9950_sensor_init_func 设置一些参数(帧数，制式类型(SENSOR_TYPE)
		  制式的分辨率（SENSOR_OUTPUT_WIDTH,SENSOR_OUTPUT_HEIGHT）原厂需要刷写屏参)
		  并且设置 set_timing();
	**************************************************************************/
	if (ak_vi_enable_dev(VIDEO_DEV0))
	{
		printf("vi device enable device  failed!\n");
		ak_vi_close(VIDEO_DEV0);
		return false;
	}

	if (ak_vi_enable_chn(VIDEO_CHN0))
	{
		printf("vi channel[%d] enable failed!\n", VIDEO_CHN0);
		ak_vi_close(VIDEO_DEV0);
		return false;
	}
#if 0
	if (ak_vi_enable_chn(VIDEO_CHN1))
	{
		printf("vi channel[%d] enable failed!\n", VIDEO_CHN1);
		ak_vi_close(VIDEO_DEV0);
		return false;
	}
#endif
	return true;
}

/***
** 日期: 2022-05-12 15:54
** 作者: leo.liu
** 函数作用：显示任务
** 返回参数说明：
***/
static void *video_display_task(void *arg)
{
	struct video_input_frame frame;
	printf("***** video display create sccess ! *****\n");

	// unsigned long long prev =0;
	while (1)
	{
		video_input_lock();
		if (video_input_state == true)
		{
			memset(&frame, 0, sizeof(struct video_input_frame));
			if (ak_vi_get_frame(VIDEO_CHN0, &frame) == 0)
			{

				if (video_input_skip_frame_count)
				{
					lv_video_mode_enable(false);
					video_input_skip_frame_count--;
				}
				else
				{
					video_main_display_lock();
					// video_input_resident_buffer_write(frame.vi_frame.data, LV_HOR_RES_MAX, LV_VER_RES_MAX, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, GP_FORMAT_YUV420SP);
					// if ((mjpeg_encode_status_get() == true))
					{
						video_input_sub_display_frame_write(&frame);
					}

					video_input_resident_buffer_write(frame.vi_frame.data, video_isp_info.main_width, video_isp_info.main_hgith, crop_pos_x, crop_pos_y, crop_width, crop_hight, GP_FORMAT_YUV420P);
					//	printf("===========================================video_input[%llu]==========[%d]\n",user_timestamp_get() - prev,video_input_skip_frame_count);
					//	prev = user_timestamp_get();
					video_main_display_unlock();
					if (lv_fb_video_display_preview_get() == true)
					{
						lv_video_mode_enable(true);
						screen_force_refresh();
					}
				}
				ak_vi_release_frame(VIDEO_CHN0, &frame);
			}
		}
		video_input_unlock();
		usleep(1 * 1000);
	}
	return NULL;
}

static void video_input_sub_display_frame_write(struct video_input_frame *frame)
{
	struct ak_tde_layer src, dst;
	tde_layer_layer_init(src, GP_FORMAT_YUV420P, video_isp_info.main_width, video_isp_info.main_hgith, video_isp_info.main_crop_pos_x, video_isp_info.main_crop_pos_y, video_isp_info.main_crop_width, video_isp_info.main_crop_hight);
	src.phyaddr = frame->phyaddr;

	tde_layer_layer_init(dst, GP_FORMAT_YUV420P, SUB_VIDEO_PIXEL_WIDTH, SUB_VIDEO_PIXEL_HIGHT, 0, 0, SUB_VIDEO_PIXEL_WIDTH, SUB_VIDEO_PIXEL_HIGHT);
	ak_mem_dma_vaddr2paddr(video_sub_input_resident_buffer, (unsigned long *)&dst.phyaddr);
	ak_tde_opt_scale(&src, &dst);
	video_sub_input_resident_timestmap = user_timestamp_get();
}
#if 0
/***
** 日期: 2022-05-12 15:54
** 作者: leo.liu
** 函数作用：编码采集
** 返回参数说明：
***/
static void *video_sub_display_task(void *arg)
{
	struct video_input_frame frame;
	printf("***** video sub display create sccess ! *****\n");
	while (1)
	{
		video_input_lock();
		if (video_input_state == true)
		{
			memset(&frame, 0, sizeof(struct video_input_frame));
			if (ak_vi_get_frame(VIDEO_CHN1, &frame) == 0)
			{
				video_input_sub_display_frame_write(&frame);
				ak_vi_release_frame(VIDEO_CHN1, &frame);
			}
		}
		video_input_unlock();
		usleep(1 * 1000);
	}
	return NULL;
}
#endif
/***
** 日期: 2022-05-14 17:34
** 作者: leo.liu
** 函数作用：清空buf
** 返回参数说明：
***/
void video_input_resident_bzero(void)
{
	unsigned int *data = (unsigned int *)video_input_resident_buffer;
	int size = LV_HOR_RES_MAX * LV_VER_RES_MAX;
	while (size--)
	{
		*data = 0xFF000000;
		data++;
	}
}

/***
** 日期: 2022-05-12 15:54
** 作者: leo.liu
** 函数作用：视频输入任务
** 返回参数说明：
***/
static void *video_input_task(void *arg)
{
	printf("***** video input create sccess ! *****\n");

	while (1)
	{
		sem_wait(&video_input_sem);
		video_input_lock();

		if (video_input_enable == true)
		{
			printf("##### open isp device #####\n");
			if (video_isp_open(&video_isp_info) == true)
			{
				crop_width = ((unsigned int)(video_isp_info.main_crop_width * (1 / sqrt(new_zoom / 100.0)))) & (~0x1);
				crop_hight = ((unsigned int)(video_isp_info.main_crop_hight * (1 / sqrt(new_zoom / 100.0)))) & (~0x1);
				crop_pos_x = (unsigned int)(video_isp_info.main_crop_pos_x + pos_offset_x);
				crop_pos_y = (unsigned int)(video_isp_info.main_crop_pos_y + pos_offset_y);

				video_input_state = true;
				video_input_skip_frame_count = 5;
			}
		}
		else
		{
			printf("##### colse isp device #####\n");
			lv_video_mode_enable(false);

			video_isp_close();

			video_input_resident_bzero();

			video_input_state = false;
		}
		video_input_unlock();
	}

	return NULL;
}

/***
** 日期: 2022-05-12 16:24
** 作者: leo.liu
** 函数作用：初始化video onput 任务
** 返回参数说明：
***/
bool video_input_init(void)
{
	system("insmod /usr/modules/sensor_tp9950.ko");
	system("insmod /usr/modules/ak_image_capture.ko");

	pthread_t thread_t;
	pthread_mutex_init(&video_input_mutex, NULL);
	pthread_mutex_init(&video_main_display_mutex, NULL);
	pthread_mutex_init(&video_sub_display_mutex, NULL);
	sem_init(&video_input_sem, 0, 0);

	video_input_resident_buffer = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VI, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);

	video_sub_input_resident_buffer = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VI, SUB_VIDEO_PIXEL_WIDTH * SUB_VIDEO_PIXEL_HIGHT * 3 / 2);

	video_input_display_pos_x = 0;
	video_input_display_pos_y = 0;
	video_input_display_pos_w = LV_HOR_RES_MAX;
	video_input_display_pos_h = LV_VER_RES_MAX;
	pthread_create(&thread_t, user_pthread_atter_get(), video_input_task, NULL);
	pthread_create(&thread_t, user_pthread_atter_get(), video_display_task, NULL);
	//	pthread_create(&thread_t, user_pthread_atter_get(), video_sub_display_task, NULL);
	return true;
}

/***
** 日期: 2022-05-12 15:21
** 作者: leo.liu
** 函数作用：打开isp
** 返回参数说明：
***/
bool video_input_open(video_isp_param *param)
{
	video_input_lock();
	if (video_input_state == true)
	{
		pthread_mutex_unlock(&video_input_mutex);
		return false;
	}
	memcpy(&video_isp_info, param, sizeof(video_isp_param));
	video_input_enable = true;
	sem_post(&video_input_sem);
	video_input_unlock();
	return true;
}

/***
** 日期: 2022-05-12 15:44
** 作者: leo.liu
** 函数作用：关闭isp
** 返回参数说明：
***/
bool video_input_close(void)
{
	video_input_lock();
	if (video_input_state == false)
	{
		pthread_mutex_unlock(&video_input_mutex);
		return false;
	}
	video_input_enable = false;
	sem_post(&video_input_sem);
	video_input_unlock();

	/***** 等待isp关闭 *****/
	while (video_input_state == true)
	{
		usleep(10 * 1000);
	}
	return true;
}

/***
** 日期: 2022-05-13 17:48
** 作者: leo.liu
** 函数作用：获取常驻视频buffer
** 返回参数说明：
***/
unsigned char *video_input_resident_buffer_get(unsigned long long *timestamp)
{
	if (timestamp != NULL)
	{
		*timestamp = video_input_resident_timestmap;
	}
	return video_input_resident_buffer;
}

/***
** 日期: 2022-05-13 17:48
** 作者: leo.liu
** 函数作用：获取常驻视频子buffer
** 返回参数说明：
***/
unsigned char *video_input_sub_resident_buffer_get(unsigned long long *timestamp)
{
	if (timestamp != NULL)
	{
		*timestamp = video_sub_input_resident_timestmap;
	}
	return video_sub_input_resident_buffer;
}

/***
** 日期: 2022-05-14 16:16
** 作者: leo.liu
** 函数作用：设置过滤帧数
** 返回参数说明：
***/
void video_input_skip_frame_count_set(int count)
{
	video_input_skip_frame_count = count;
	video_input_resident_bzero();
}

/***
** 日期: 2022-05-14 16:41
** 作者: leo.liu
** 函数作用：将数据写入视频buf
** 返回参数说明：
***/
void video_input_resident_buffer_write(unsigned char *data, int w, int h, int posx, int posy, int posw, int posh, AK_GP_FORMAT fmt)
{
	if (data == NULL)
	{
		video_input_resident_bzero();
		return;
	}
	struct ak_tde_layer src, dst;
	tde_layer_layer_init(src, fmt, w, h, posx, posy, posw, posh);
	ak_mem_dma_vaddr2paddr(data, (unsigned long *)&src.phyaddr);

	tde_layer_layer_init(dst, GP_FORMAT_RGB888, LV_HOR_RES_MAX, LV_VER_RES_MAX, video_input_display_pos_x, video_input_display_pos_y, video_input_display_pos_w, video_input_display_pos_h);
	ak_mem_dma_vaddr2paddr(video_input_resident_buffer, (unsigned long *)&dst.phyaddr);
	if ((w == LV_VER_RES_MAX) && (h == LV_VER_RES_MAX))
	{
		ak_tde_opt_format(&src, &dst);
	}
	else
	{
		ak_tde_opt_scale(&src, &dst);
	}

	video_input_resident_timestmap = user_timestamp_get();
}
/***
** 日期: 2022-05-16 10:38
** 作者: leo.liu
** 函数作用：获取视频状态
** 返回参数说明：
***/
bool video_input_state_get(void)
{
	extern bool video_format_is_invalid(void);
	if ((video_input_state == true) && (video_input_skip_frame_count == 0) && !video_format_is_invalid())
	{
		return true;
	}
	return false;
}
/***
**   日期:2022-06-11 11:36:39
**   作者: leo.liu
**   函数作用：设置将视频显示在指定的位置
**   参数说明:
***/
bool video_input_display_pos(int x, int y, int w, int h)
{
	video_input_display_pos_x = x * 8 / 8;
	video_input_display_pos_y = y * 8 / 8;
	video_input_display_pos_w = w * 8 / 8;
	video_input_display_pos_h = h * 8 / 8;
	return true;
}

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：设置视频显示百分比倍数
**   参数说明:
***/
bool video_input_display_zoom_set(int multiple)
{
	// if(!video_input_state_get())
	// {
	// 	return false;
	// 	printf("=======video_set_offser_fail(video_input_not_ready)\n");
	// }
	if (multiple > 500)
	{
		printf("========new_zoom range set too large \n");

		return false;
	}
	else if (multiple < 100)
	{
		printf("========new_zoom range set too small \n");

		return false;
	}
	old_zoom = new_zoom;
	new_zoom = multiple;

	crop_width = ((unsigned int)(video_isp_info.main_crop_width * (1 / sqrt(new_zoom / 100.0)))) & (~0x1);
	crop_hight = ((unsigned int)(video_isp_info.main_crop_hight * (1 / sqrt(new_zoom / 100.0)))) & (~0x1);
	crop_old_width = ((unsigned int)(video_isp_info.main_crop_width * (1 / sqrt(old_zoom / 100.0)))) & (~0x1);
	crop_old_hight = ((unsigned int)(video_isp_info.main_crop_hight * (1 / sqrt(old_zoom / 100.0)))) & (~0x1);
	crop_width_gap = crop_old_width - crop_width;
	crop_hight_gap = crop_old_hight - crop_hight;
	new_area = crop_width * crop_hight;
	if (new_area != old_area) // 判断视频裁剪大小是否发生了变动，改变坐标偏移
	{
		pos_offset_x = (int)(pos_offset_x + (int)(crop_width_gap * 0.5)) & (~0x01);
		int max_offset_x = (video_isp_info.main_crop_width - crop_width);
		pos_offset_x = pos_offset_x <= 0 ? 0 : pos_offset_x > max_offset_x ? max_offset_x
																		   : pos_offset_x;

		pos_offset_y = (int)(pos_offset_y + (int)(crop_hight_gap * 0.5)) & (~0x01);
		int max_offset_y = (video_isp_info.main_crop_hight - crop_hight);
		pos_offset_y = pos_offset_y <= 0 ? 0 : pos_offset_y > max_offset_y ? max_offset_y
																		   : pos_offset_y;
	}
	old_area = new_area;
	crop_pos_x = (unsigned int)(video_isp_info.main_crop_pos_x + pos_offset_x);
	crop_pos_y = (unsigned int)(video_isp_info.main_crop_pos_y + pos_offset_y);
	return true;
}

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：获取视频显示倍数
**   参数说明:
***/
int video_input_display_zoom_get()
{
	// if(!video_input_state_get())
	// {
	// 	return false;
	// 	printf("=======video_set_offser_fail(video_input_not_ready)\n");
	// }
	return new_zoom;
}

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：设置视频相对坐标原点的百分比偏移
**   参数说明:填入要偏移的百分比参数0-100，使用这句函数之前最好先使用video_input_display_offset_get获取偏移百分比，再在这数值基础上进行左右加减
***/
bool video_input_display_offset_set(double offset_pct_x, double offset_pct_y)
{
	if (!video_input_state_get())
	{
		return false;
		printf("=======video_set_offser_fail(video_input_not_ready)\n");
	}
	int offset_x = (unsigned int)(offset_pct_x / 100.0 * video_isp_info.main_crop_width) & (~0x01);
	int offset_y = (unsigned int)(offset_pct_y / 100.0 * video_isp_info.main_crop_hight) & (~0x01);

	printf("========offset_pct_x========%f\n", offset_pct_x);
	printf("========offset_pct_y========%f\n", offset_pct_y);
	if ((offset_pct_x >= 100) || (offset_pct_y >= 100))
	{
		printf("========offset range unvalid======offser_pct need positive integer 0 - 100\n");
		return false;
	}
	if (((offset_x + ((unsigned int)((video_isp_info.main_crop_width * (1 / sqrt((double)new_zoom / 100.0)))) & (~0x01))) > video_isp_info.main_crop_width) ||
		((offset_y + ((unsigned int)((video_isp_info.main_crop_hight * (1 / sqrt((double)new_zoom / 100.0)))) & (~0x01))) > video_isp_info.main_crop_hight))
	{
		printf("========offset range unvalid\n");
		return false;
	}
	pos_offset_x = (offset_x & (~0x01));
	pos_offset_y = (offset_y & (~0x01));
	crop_pos_x = (unsigned int)(video_isp_info.main_crop_pos_x + pos_offset_x);
	crop_pos_y = (unsigned int)(video_isp_info.main_crop_pos_y + pos_offset_y);

	printf("=========crop_width====%d======\n", crop_width);
	printf("=========crop_hight====%d======\n", crop_hight);
	printf("=========pos_offset_x====%d======\n", pos_offset_x);
	printf("=========pos_offset_y====%d======\n", pos_offset_y);

	return true;
}

/***
**   日期:2022-11-18 22:40:39
**   作者: xiaoxiao
**   函数作用：获取视频偏移百分比参数
**   参数说明:相对坐标原点
***/
bool video_input_display_offset_get(double *offset_per_x, double *offset_per_y)
{
	if (!video_input_state_get())
	{
		return false;
		printf("=======video_set_offser_fail(video_input_not_ready)\n");
	}
	if (offset_per_x != NULL)
		*offset_per_x = (double)(pos_offset_x * 1.0 / video_isp_info.main_crop_width * 100);
	if (offset_per_y != NULL)
		*offset_per_y = (double)(pos_offset_y * 1.0 / video_isp_info.main_crop_hight * 100);
	return true;
}
