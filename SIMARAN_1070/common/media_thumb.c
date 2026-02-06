#include "user_common.h"
#include "media_thumb.h"
#include "video_decode.h"
#include "avilib.h"
#include "ak_mem.h"
#include "user_file.h"
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include "video_input.h"

#include "lvgl/lvgl.h"

static unsigned char *thumb_media_buffer = NULL;
static int thumb_media_pos_x;
static int thumb_media_pos_y;
static int thumb_media_pos_w;
static int thumb_media_pos_h;

static bool thumb_media_decode_finish = true;
/***
**   日期:2022-05-23 11:20:23
**   作者: leo.liu
**   函数作用：解码完成回调函数
**   参数说明:
***/
static void thumb_media_thume_decode_func(struct ak_vdec_frame *frame)
{
	if ((thumb_media_decode_finish == true) || (thumb_media_buffer == NULL))
	{
		return;
	}

	struct ak_tde_layer src, dst;
	tde_layer_layer_init(src, GP_FORMAT_YUV420SP,
			     frame->frame_obj.data.pitch_width, frame->frame_obj.data.pitch_height,
			     0, 0, frame->width, frame->height);
	ak_mem_dma_vaddr2paddr(frame->frame_obj.data.data, (unsigned long *)&src.phyaddr);

	tde_layer_layer_init(dst, GP_FORMAT_RGB888, thumb_media_pos_w, thumb_media_pos_h, 0, 0, thumb_media_pos_w, thumb_media_pos_h);
	unsigned char *data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_AO, thumb_media_pos_w * thumb_media_pos_h * 3);
	ak_mem_dma_vaddr2paddr(data, (unsigned long *)&dst.phyaddr);
	ak_tde_opt_scale(&src, &dst);

	unsigned char *dst_data = thumb_media_buffer + thumb_media_pos_y * LV_HOR_RES_MAX * 4 + thumb_media_pos_x * 4;
	unsigned char *src_data = data;
	for (int y = 0; y < thumb_media_pos_h; y++)
	{
		for (int x = 0; x < thumb_media_pos_w; x++)
		{
			dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 3] = 0xFF;
			dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 2] = src_data[y * thumb_media_pos_w * 3 + x * 3 + 2];
			dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 1] = src_data[y * thumb_media_pos_w * 3 + x * 3 + 1];
			dst_data[y * LV_HOR_RES_MAX * 4 + x * 4 + 0] = src_data[y * thumb_media_pos_w * 3 + x * 3 + 0];
		}
	}
	ak_mem_dma_free(data);
	thumb_media_decode_finish = true;
	printf("read: %dx%d write:%d,%d %dx%d\n", frame->width, frame->height, thumb_media_pos_x, thumb_media_pos_y, thumb_media_pos_w, thumb_media_pos_h);
}

/***
**   日期:2022-05-23 11:04:28
**   作者: leo.liu
**   函数作用：初始化缩略图设备
**   参数说明:
***/
bool thumb_media_open(void)
{
	if (thumb_media_buffer != NULL)
	{
		return false;
	}
	thumb_media_decode_finish = true;
	jpg_decode_open(thumb_media_thume_decode_func);
	// h264_decode_open(thumb_media_thume_decode_func);
	thumb_media_buffer = video_input_resident_buffer_get(NULL);
	static rom_bin_info img = rom_bin_raw_get();
	rom_bin_raw_init(img, thumb_media_buffer, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img);
	return true;
}
/***
**   日期:2022-05-23 11:16:52
**   作者: leo.liu
**   函数作用：关闭缩略图
**   参数说明:
***/
bool thumb_media_close(void)
{
	if (thumb_media_buffer == NULL)
	{
		return false;
	}
	jpg_decode_close();
	//	h264_decode_close();
	lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);
	thumb_media_buffer = NULL;
	return true;
}

/***
**   日期:2022-05-23 11:27:27
**   作者: leo.liu
**   函数作用：等待缩略图解码结束
**   参数说明:
***/
static bool thumb_media_decode_finish_wait(void)
{
	int count = 100;
	while (thumb_media_decode_finish == false)
	{
		usleep(10 * 1000);
		count--;
		if (count == 0)
		{
			return false;
		}
	}
	return true;
}
/***
**   日期:2022-05-23 11:36:36
**   作者: leo.liu
**   函数作用：加载照片
**   参数说明:
***/
static bool thumb_media_jpg_load(const char *file)
{
	int fd = open(file, O_RDONLY);
	if (fd < 0)
	{
		return false;
	}
	int size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	if (size <= 0)
	{
		close(fd);
		return false;
	}
	unsigned char *data = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, size);
	read(fd, data, size);
	close(fd);
	jpg_decode_stream_write(data, size);
	ak_mem_free(data);
	return true;
}
/***
**   日期:2022-05-23 11:36:52
**   作者: leo.liu
**   函数作用：加载视频
**   参数说明:
***/
static bool thumb_media_video_load(const char *file)
{
	jpg_decode_buffer_clear();
	avi_t *avi_handle = AVI_open_input_file(file, 1);
	if (avi_handle == NULL)
	{
		printf("open avi:%s failed \n", file);
		return false;
	}
	int total_frame = AVI_video_frames(avi_handle);
	for (int i = 0; i < total_frame; i++)
	{
		if (AVI_set_video_position(avi_handle, i) != 0)
		{
			AVI_close(avi_handle);
			return false;
		}
		int frame_size = AVI_frame_size(avi_handle, i);
		if (frame_size <= 0)
		{
			AVI_close(avi_handle);
			return false;
		}
		unsigned char *data = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, frame_size);
		int is_keyframe = 0;
		if (AVI_read_frame(avi_handle, (char *)data, &is_keyframe) < 0)
		{
			ak_mem_free(data);
			AVI_close(avi_handle);
			return false;
		}

		jpg_decode_stream_write(data, frame_size);
		ak_mem_free(data);
		usleep(20 * 1000);
		if (thumb_media_decode_finish == true)
		{
			break;
		}
	}
	AVI_close(avi_handle);
	return true;
}

/***
**   日期:2022-05-23 11:32:30
**   作者: leo.liu
**   函数作用：获取文件类型
**   参数说明:
***/
static file_type thumb_media_type_get(const char *file)
{
	char *p = strrchr(file, '.');
	if (p == NULL)
	{
		return FILE_TYPE_NONE;
	}
	int str_len = strlen(p);
	if (str_len != 4)
	{
		return FILE_TYPE_NONE;
	}
	if ((p[1] == 'j') || (p[1] == 'J'))
	{
		return FILE_TYPE_PHOTO;
	}
	else if ((p[1] == 'a') || (p[1] == 'A'))
	{
		return FILE_TYPE_VIDEO;
	}
	return FILE_TYPE_NONE;
}

/***
**   日期:2022-05-23 11:18:45
**   作者: leo.liu
**   函数作用：缩略图显示
**   参数说明:
***/
bool thumb_media_load(int x, int y, int w, int h, const char *file)
{
	printf("decode one frame:%s \n", file);
	file_type type = thumb_media_type_get(file);
	if (type == FILE_TYPE_NONE)
	{
		return false;
	}
	thumb_media_decode_finish = false;
	thumb_media_pos_x = x;
	thumb_media_pos_y = y;
	thumb_media_pos_w = w;
	thumb_media_pos_h = h;
	if (type != FILE_TYPE_VIDEO)
	{
		thumb_media_jpg_load(file);
	}
	else
	{
		thumb_media_video_load(file);
	}
	if (thumb_media_decode_finish_wait() == false)
	{
		printf("thumb media wait decode finish tiemout ... \n");
		return false;
	}
	return true;
}

/***
**   日期:2022-05-23 13:49:23
**   作者: leo.liu
**   函数作用：清除缩略图buffer
**   参数说明:
***/
bool thumb_media_buffer_clear(void)
{
	video_input_resident_bzero();
	return true;
}
