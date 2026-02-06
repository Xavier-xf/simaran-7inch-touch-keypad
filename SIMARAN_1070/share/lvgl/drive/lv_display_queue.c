#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "unistd.h"
//#include "anyka/ak_common_graphics.h"


#define DISP_BUF_MAX 3


#define QUEUE_IS_FULL(w, r, size) ((((w) + 1) % (size)) == (r))

typedef  unsigned char  AK_GP_FORMAT;
typedef struct
{
	int width;
	int height;
	uint8_t* data;
	AK_GP_FORMAT fmt;
}disp_node;

typedef struct
{
	int ridx;
	int widx;
	disp_node node[DISP_BUF_MAX];
}display_info;



static display_info disp_buf = 
{
	.ridx = 0,
	.widx = 0,
	.node = {
		{0,0,NULL,0},
	}
};


int display_queue_push(uint8_t* buf,int w,int h,AK_GP_FORMAT fmt)
{
	if(QUEUE_IS_FULL(disp_buf.widx,disp_buf.ridx,DISP_BUF_MAX))
	{
		return -1;
	}

	disp_buf.node[disp_buf.widx].width = w;
	disp_buf.node[disp_buf.widx].height = h;
	disp_buf.node[disp_buf.widx].fmt = fmt;
	disp_buf.node[disp_buf.widx].data = buf;
	disp_buf.widx = (disp_buf.widx + 1)%DISP_BUF_MAX;
//	printf("111111111 %dx%d \n",disp_buf.node[disp_buf.widx].width,disp_buf.node[disp_buf.widx].height);
	return 0;
}


int display_queue_get(uint8_t** buf, unsigned short* w, unsigned short*h,AK_GP_FORMAT* fmt)
{
	if(disp_buf.ridx == disp_buf.widx)
	{
		return -1;
	}

	*buf = disp_buf.node[disp_buf.ridx].data;
	*w = disp_buf.node[disp_buf.ridx].width;
	*h = disp_buf.node[disp_buf.ridx].height;
	*fmt = disp_buf.node[disp_buf.ridx].fmt;
	disp_buf.ridx = (disp_buf.ridx + 1)%DISP_BUF_MAX;
	return 0;
}






