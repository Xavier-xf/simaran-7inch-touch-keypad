#include "lv_jpg_decode.h"
#include "tjpgd.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "../../../include/anyka37d/ak_common_graphics.h"
#include "../../../include/anyka37d/ak_mem.h"
#include "../../../include/anyka37d/ak_tde.h"

typedef struct
{
	int fd;
	unsigned char *raw_data;
	int raw_width;
	int raw_hight;
} jpg_info;

#define TJPGD_WORKBUFF_SIZE 4096
static size_t jpg_read_func(JDEC *jd, uint8_t *buff, size_t ndata)
{
	jpg_info *info = (jpg_info *)jd->device;
	int read_len = 0;
	if (buff != NULL)
	{
		read_len = read(info->fd, buff, ndata);
	}
	else
	{
		lseek(info->fd, ndata, SEEK_CUR);
		read_len = ndata;
	}
	return read_len;
}
static int jpg_decode_func(JDEC *jd, void *data, JRECT *rect)
{
	jpg_info *info = (jpg_info *)jd->device;
	uint8_t *buf = data;
	const int INPUT_PIXEL_SIZE = 3;
	const int row_width = rect->right - rect->left + 1; // Row width in pixels.
	const int row_size = row_width * INPUT_PIXEL_SIZE;  // Row size (bytes).
	for (int y = rect->top; y <= rect->bottom; y++)
	{
		unsigned char *dst = info->raw_data + y * info->raw_width * INPUT_PIXEL_SIZE + rect->left * INPUT_PIXEL_SIZE;
		memcpy(dst, buf, row_size);
		buf += row_size;
	}
	return 1;
}

/***
**   日期:2022-07-01 13:50:29
**   作者: leo.liu
**   函数作用：获取解码的数据
**   参数说明:
***/
bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h)
{
	jpg_info raw_info;
	raw_info.fd = open(file, O_RDONLY);
	if (raw_info.fd < 0)
	{
		printf("jpg decode :open %s failed \n", file);
		return false;
	}
	JDEC jd_tmp;
	uint8_t *read_data = lv_mem_alloc(TJPGD_WORKBUFF_SIZE);
	JRESULT rc = jd_prepare(&jd_tmp, jpg_read_func, read_data, TJPGD_WORKBUFF_SIZE, &raw_info);
	if (rc != JDR_OK)
	{
		lv_mem_free(read_data);
		printf("jpg prepare %s failed \n", file);
		return false;
	}
	raw_info.raw_width = jd_tmp.width;
	raw_info.raw_hight = jd_tmp.height;
	raw_info.raw_data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, raw_info.raw_width * raw_info.raw_hight * 3);
	rc = jd_decomp(&jd_tmp, jpg_decode_func, 0);
	if (rc != JDR_OK)
	{
		lv_mem_free(read_data);
		ak_mem_dma_free(raw_info.raw_data);
		printf("jpg decode %s failed \n", file);
		return LV_RES_INV;
	}
	lv_mem_free(read_data);

	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_RGB888;
	src.pos_width = src.width = raw_info.raw_width;
	src.pos_height = src.height = raw_info.raw_hight;
	src.pos_left = src.pos_top = 0;
	ak_mem_dma_vaddr2paddr(raw_info.raw_data, (unsigned long *)&src.phyaddr);

#if LV_COLOR_DEPTH == 32
	dst.format_param = GP_FORMAT_BGR888;
#elif LV_COLOR_DEPTH == 16
	dst.format_param = GP_FORMAT_BGR565;
#endif
	dst.pos_width = dst.width = dst_w;
	dst.pos_height = dst.height = dst_h;
	dst.pos_left = dst.pos_top = 0;

	unsigned char *data = (unsigned char *)info->offset;
	ak_mem_dma_vaddr2paddr(data, (unsigned long *)&dst.phyaddr);

	ak_tde_opt_scale(&src, &dst);
	ak_mem_dma_free(raw_info.raw_data);
#if LV_COLOR_DEPTH == 32
	int len = dst_w * dst_h;
	unsigned char *dst_data = data + dst_w * dst_h * 4 - 1;
	unsigned char *src_data = data + dst_w * dst_h * 3 - 1;
	for (int i = 0; i < len; i++)
	{

		*dst_data = 0xFF;
		dst_data--;
		*dst_data = *src_data;
		dst_data--;
		src_data--;
		*dst_data = *src_data;
		dst_data--;
		src_data--;
		*dst_data = *src_data;
		dst_data--;
		src_data--;
	}
#endif
	return true;
}