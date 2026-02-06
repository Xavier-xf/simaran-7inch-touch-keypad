/*
 * TP2850.c    tp9950 Camera Driver
 *
 * Copyright anyka
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-image-sizes.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>


#include "../include/ak_video_priv_cmd.h"
#include "../include_internal/ak_video_priv_cmd_internal.h"
#include "../include/ak_isp_drv.h"
#include "../include/ak_sensor.h"
#include "../include/sensor_cmd.h"
#include "../common/sensor_sys.h"
#include "../common/sensor_i2c.h"

#include "tp2850.h"


#define DEBUG            0  //printk debug information on/off

/*
 *
SENSOR_PWDN_LEVEL:	the io level when sensor power down
SENSOR_RESET_LEVEL:	the io level when sensor reset
SENSOR_I2C_ADDR:	the sensor slave address, 7 bits width.
the i2c bus left shift 1bits.
SENSOR_ID:	the sensor ID.
SENSOR_MCLK:	mclk, unit：MHz。24MHz or 27MHz mclk for most sensors.
NOTE: 27MHz mclk is inaccurate.
SENSOR_REGADDR_BYTE:	sensor register address bytes width, 1 byte or 2 bytes.
SENSOR_DATA_BYTE:		register's value bytes width, 1 byte or 2bytes.
SENSOR_OUTPUT_WIDTH:	use for isp cut window MAX width.
SENSOR_OUTPUT_HEIGHT:	use for isp cut window MAX height.
SENSOR_VALID_OFFSET_X:	use for isp cut window left offset.
SENSOR_VALID_OFFSET_Y:	use for isp cut window top offset.
SENSOR_BUS_TYPE:		sensor output data type.
SENSOR_IO_INTERFACE:	sensor output data bus type.
SENSOR_IO_LEVEL:		sensor output data io level.
MAX_FPS:				current sensor configurtion support max fps.
EXP_EFFECT_FRAMES
DELAY_FLAG
 *
 * */
#define SENSOR_PWDN_LEVEL		0 
#define SENSOR_RESET_LEVEL		0
#define SENSOR_I2C_ADDR			0x44 
#define SENSOR_ID				0x2850
#define SENSOR_MCLK				27
#define SENSOR_REGADDR_BYTE		1
#define SENSOR_DATA_BYTE		1

#if 0
#define SENSOR_OUTPUT_WIDTH		1280
#define SENSOR_OUTPUT_HEIGHT	720
#else
static int SENSOR_OUTPUT_WIDTH = 1280;//960
static int SENSOR_OUTPUT_HEIGHT = 720; //256
static int SENSOR_OUTPUT_FPS = 30;
#endif


//#define SENSOR_VALID_OFFSET_X	1
static int SENSOR_VALID_OFFSET_X = 0;
#define SENSOR_VALID_OFFSET_Y	0//0
#define SENSOR_BUS_TYPE			BUS_TYPE_YUV
#if defined(CONFIG_MACH_AK37E)
#define SENSOR_IO_INTERFACE		DVP_INTERFACE_BT656
#else
#define SENSOR_IO_INTERFACE		DVP_INTERFACE
#endif
#define SENSOR_IO_LEVEL			IO_LEVEL_3V3
#define MAX_FPS					25

#define EXP_EFFECT_FRAMES		1
#define DELAY_FLAG				(0xffff)

/*
 *  * check_id - check hardware sensor ID whether it meets this driver
 *  * 0-no check, force to meet
 *  * others-check, if not meet return fail
 *  */
static int check_id = 0;
module_param(check_id, int, 0644);

static int force_output = 0;
module_param(force_output, int, 0644);

static int force_check = 0;
module_param(force_check, int, 0644);

enum input_res {
	INPUT_SD480I = 0,
	INPUT_SD576I,
	INPUT_HD720P,
	INPUT_HD1080P,
	INPUT_HD960P_HD800P,
};

struct res_to_timing_info {
	int fps;
	int res;
	struct isp_timing_info isp_timing;
};

/*
 * Struct
 */
struct regval_list {
	u8 reg_num;
	u8 value;
};

struct  tp9950_win_size {
	char				*name;
	u32				width;
	u32				height;
	const struct regval_list	*regs;
};

/*
 * store sensor fps informations
 * @current_fps: current fps
 * @to_fps: should changed to the fps
 * @to_fps_value: the fps paramter
 */
struct sensor_fps_info {
	int current_fps;
	int to_fps;
	int to_fps_value;
};

/*
 * the host callbacks for sensor
 * @isp_timing_cb_info: isp timing callback.
 * 	a few sensor need the callback when fps had changed
 */
struct host_callbacks {
	struct isp_timing_cb_info isp_timing_cb_info;
};

/*
 * aec paramters. some sensors need record paramters.
 */
struct tp9950_aec_parms {

};

struct tp9950_priv {
	struct i2c_client *client;
	struct v4l2_subdev		subdev;
	struct v4l2_ctrl_handler	hdl;
	u32	cfmt_code;
	const struct tp9950_win_size	*win;

	int gpio_reset;
	int gpio_pwdn;
	struct sensor_cb_info cb_info;
	struct sensor_fps_info fps_info;

	struct host_callbacks hcb;
	struct tp9950_aec_parms aec_parms;

	enum scan_method method;

	AK_ISP_SENSOR_REG_INFO *preg_info;
	int num;
	int fps_res;

	struct v4l2_ctrl *ctrl_sensor_get_id;

};


spinlock_t readid_lock;
spinlock_t reg_lock;

static AK_ISP_SENSOR_CB tp9950_cb;

static struct res_to_timing_info res_to_timing[] = {
	{60, INPUT_SD480I, {9000,	1,	1600,	18}},	/*INPUT_SD480I NTSC */ 
	{50, INPUT_SD576I, {7680,	1,	1500,	24}},	/*INPUT_SD576I PAL */
	{25, INPUT_HD720P, {9599,	1,	3373,	18}},	/*INPUT_HD720P@25fps*/
	{30, INPUT_HD720P, {8000,	1,	1774,	17}},	/*15 INPUT_HD720P@30fps*/
	{25, INPUT_HD1080P, {6480,	1,	1728,	28}},	/*INPUT_HD1080P@25*/
	{30, INPUT_HD1080P, {6480,	1,	1728,	3}},	/*INPUT_HD1080i@50*/
	{50, INPUT_HD960P_HD800P, {6480,	1,	1728,	3}},		/*INPUT_HD1080P@30*/
};


static const struct v4l2_ctrl_ops tp9950_ctrl_ops;
static const struct v4l2_ctrl_config config_sensor_get_id = {
	.ops	= &tp9950_ctrl_ops,
	.id		= SENSOR_GET_ID,
	.name	= "get sensor id",
	.type	= V4L2_CTRL_TYPE_INTEGER,
	.min	= 0,
	.max	= 0xfffffff,
	.step	= 1,
	.def	= 0,
	.flags	= V4L2_CTRL_FLAG_VOLATILE,/*set V4L2_CTRL_FLAG_VOLATILE avoid get cached*/
};

static u32 tp9950_codes[] = {
	MEDIA_BUS_FMT_YUYV8_2X8,
	MEDIA_BUS_FMT_UYVY8_2X8,
	MEDIA_BUS_FMT_RGB565_2X8_BE,
	MEDIA_BUS_FMT_RGB565_2X8_LE,
};

typedef struct
{
	unsigned int	count;
	unsigned int	mode;
	unsigned int	scan;
	unsigned int	gain[4];
	unsigned int 	std;
	unsigned int	state;
	unsigned int	force;
	unsigned char	addr;
	unsigned int	fps;
} tp9950au_info;

	
static int __tp9950_sensor_probe_id_func(struct i2c_client *client);  //use IIC bus
static int tp9950_sensor_read_id_func(void *arg);	//no use IIC bus
static int tp9950_sensor_get_resolution_func(void *arg, int *width, int *height);
static int call_sensor_sys_init(void);
static int call_sensor_sys_deinit(void);


/*
 * General functions
 */
static struct tp9950_priv *to_tp9950(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct tp9950_priv,subdev);
}

static struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct tp9950_priv, hdl)->subdev;
}

#if 0
/*
 * General functions
 */
unsigned int ConvertACPV1Data(unsigned char dat)
{
	unsigned int i, tmp=0;

	for(i = 0; i < 8; i++)
	{
		tmp <<= 3;

		if(0x01 & dat) tmp |= 0x06;
		else tmp |= 0x04;

		dat >>= 1;
	}

	return tmp;
}
#endif

/*
 * XXXX_s_stream -
 * set steaming enable/disable
 * soc_camera_ops functions
 *
 * @sd:				subdev
 * @enable:			enable flags
 */
static int tp9950_s_stream(struct v4l2_subdev *sd, int enable)
{
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif	
	return 0;
}

static int tp9950_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tp9950_priv *priv = to_tp9950(client);

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	switch (ctrl->id) {
		case SENSOR_GET_ID:
			ctrl->val = SENSOR_ID;//((SENSOR_ID << 8) | SENSOR_TYPE);
			break;

		default:
			ret = -EINVAL;
			pr_err("%s id:%d no support\n", __func__, ctrl->id);
			break;
	}

	return ret;
}

/*
 * XXXX_s_stream -
 * set ctrls
 * soc_camera_ops functions
 *
 * @ctrl:			pointer to ctrl
 */
static int tp9950_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tp9950_priv *priv = to_tp9950(client);

#if (DEBUG)
	printk(KERN_ERR "%s set 0x%x\n", __func__, ctrl->id);
#endif

	switch (ctrl->id) {
		case SENSOR_GET_ID:  //unuse here, xiaozijie
			break;
		case V4L2_CID_HFLIP:
			break;
		case V4L2_CID_VFLIP:
			break;
		case V4L2_CID_BRIGHTNESS:
			break;
		case V4L2_CID_CONTRAST:
			break;
		default:
			pr_err("%s cmd:%x not support\n", __func__, ctrl->id);
			//return -EINVAL;
			break;
	}

	return 0;
}

/*
 * XXXX_s_power -
 * set power operation
 * soc_camera_ops functions
 *
 * @sd:			subdev
 * @on:			power flags
 */
static int tp9950_core_s_power(struct v4l2_subdev *sd, int on)
{
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif	
	return 0;
}

/*
 * XXXX_get_sensor_id -
 * get sensor ID
 * private callback functions
 *
 * @ctrl:			pointer to ctrl
 */
static int tp9950_get_sensor_id(struct v4l2_control *ctrl)
{
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif	
	ctrl->value = tp9950_sensor_read_id_func(NULL);   //no use IIC bus
	return 0;
}

/*
 * XXXX_get_sensor_id -
 * get sensor callback
 * private callback functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int tp9950_get_sensor_cb(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tp9950_priv *priv = to_tp9950(client);

#if (DEBUG)	
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	ctrl->value = (int)&priv->cb_info;
	return 0;
}

/*
 * XXXX_core_g_ctrl -
 * get control
 * core functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int tp9950_core_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int ret;

#if (DEBUG)
	printk(KERN_ERR "%s %x\n", __func__, ctrl->id);
#endif	

	switch (ctrl->id) {
		case GET_SENSOR_ID:
			ret = tp9950_get_sensor_id(ctrl);
			break;

		case GET_SENSOR_CB:
			ret = tp9950_get_sensor_cb(sd, ctrl);
			break;

		default:
			pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
			ret = -1;
			break;
	}

	return ret;
}

/*
 * XXXX_set_isp_timing_cb -
 * set isp timing callback function
 * private callback functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int tp9950_set_isp_timing_cb(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tp9950_priv *priv = to_tp9950(client);
	struct isp_timing_cb_info *isp_timing_cb_info = (void *)ctrl->value;

	pr_info("%s\n", __func__);
	//pr_info("%s isp_timing_cb_info = %p\n", __func__, isp_timing_cb_info);;

	memcpy(&priv->hcb.isp_timing_cb_info,
			isp_timing_cb_info,
			sizeof(struct isp_timing_cb_info));

	return 0;
}

/*
 * XXXX_core_s_ctrl -
 * set core s_ctrl
 * core functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int tp9950_core_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int ret;

#if (DEBUG)	
	pr_info("%s ctrl->id = 0x%x\n", __func__,ctrl->id);
#endif

	switch (ctrl->id) {
		case SET_ISP_MISC_CALLBACK:
			ret = tp9950_set_isp_timing_cb(sd, ctrl);
			break;

		default:
			pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
			ret = -1;
			break;
	}

	return ret;
}

static long tp9950_core_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tp9950_priv *priv = to_tp9950(client);
	int ret = 0;

#if (DEBUG)
	pr_info("%s cmd= 0x%x\n", __func__, cmd);
#endif

	switch (cmd) {
		case AK_SENSOR_SET_INIT:
			/*pr2000 regs in driver, so returen success*/
			break;

		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}


/*
 * XXXX_get_fmt -
 * get format
 * pad functions
 *
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @format:			return format
 */
static int tp9950_get_fmt(struct v4l2_subdev *sd,
		struct v4l2_subdev_pad_config *cfg,
		struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct tp9950_priv *priv = to_tp9950(client);
	static struct tp9950_win_size win = {
		.name = "default",
		.width = 0,//SENSOR_OUTPUT_WIDTH,
		.height = 0,//SENSOR_OUTPUT_HEIGHT,
		.regs = NULL,
	};
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif
	win.width = SENSOR_OUTPUT_WIDTH;
	win.height = SENSOR_OUTPUT_HEIGHT;

	if (format->pad)
		return -EINVAL;

	if (!priv->win) {
		priv->win = &win;
		priv->cfmt_code = MEDIA_BUS_FMT_UYVY8_2X8;
	}

	mf->width	= priv->win->width;
	mf->height	= priv->win->height;
	mf->code	= priv->cfmt_code;

	switch (mf->code) {
		case MEDIA_BUS_FMT_RGB565_2X8_BE:
		case MEDIA_BUS_FMT_RGB565_2X8_LE:
			mf->colorspace = V4L2_COLORSPACE_SRGB;
			break;
		default:
		case MEDIA_BUS_FMT_YUYV8_2X8:
		case MEDIA_BUS_FMT_UYVY8_2X8:
			mf->colorspace = V4L2_COLORSPACE_JPEG;
	}
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

/*
 * XXXX_set_fmt -
 * set format
 * pad functions
 *
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @format:			format to set
 */
static int tp9950_set_fmt(struct v4l2_subdev *sd,
		struct v4l2_subdev_pad_config *cfg,
		struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif
	if (format->pad)
		return -EINVAL;

	mf->field	= V4L2_FIELD_NONE;

	switch (mf->code) {
	case MEDIA_BUS_FMT_RGB565_2X8_BE:
	case MEDIA_BUS_FMT_RGB565_2X8_LE:
		mf->colorspace = V4L2_COLORSPACE_SRGB;
		break;
	default:
		mf->code = MEDIA_BUS_FMT_UYVY8_2X8;
	case MEDIA_BUS_FMT_YUYV8_2X8:
	case MEDIA_BUS_FMT_UYVY8_2X8:
		mf->colorspace = V4L2_COLORSPACE_JPEG;
	}

	if (cfg)
		cfg->try_fmt = *mf;
	return 0;
}

/*
 * XXXX_set_fmt -
 * set format
 * pad functions
 *
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @sel:			return selection
 */
static int tp9950_get_selection(struct v4l2_subdev *sd,
		struct v4l2_subdev_pad_config *cfg,
		struct v4l2_subdev_selection *sel)
{
	sel->r.left = SENSOR_VALID_OFFSET_X;
	sel->r.top = SENSOR_VALID_OFFSET_Y;
	sel->r.width = SENSOR_OUTPUT_WIDTH;
	sel->r.height = SENSOR_OUTPUT_HEIGHT;

	return 0;
}

/*
 * XXXX_enum_bus -
 * enum bus
 * pad functions
 *
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @code:			return code of bus type
 */
static int tp9950_enum_mbus_code(struct v4l2_subdev *sd,
		struct v4l2_subdev_pad_config *cfg,
		struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->pad || code->index >= ARRAY_SIZE(tp9950_codes))
		return -EINVAL;

	//code->code = MEDIA_BUS_FMT_YUYV8_1_5X8;//pr2000_codes[code->index];
	code->code = MEDIA_BUS_FMT_YUYV8_2X8;//pr2000_codes[code->index];
	//code->code = MEDIA_BUS_FMT_SBGGR8_1X8;
	//code->code = V4L2_PIX_FMT_YUV420;

	return 0;
}

/*
 * XXXX_g_crop -
 * get crop
 * video functions
 *
 * @sd:				subdev
 * @a:				return crop
 */
static int tp9950_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	/*return sensor cut window size&offset*/
	a->c.left	= SENSOR_VALID_OFFSET_X;
	a->c.top	= SENSOR_VALID_OFFSET_Y;
	a->c.width	= SENSOR_OUTPUT_WIDTH;
	a->c.height	= SENSOR_OUTPUT_HEIGHT;
	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return 0;
}

/*
 * XXXX_cropcap -
 * get crop capbilities
 * video functions
 *
 * @sd:				subdev
 * @a:				return crop capbilities
 */
static int tp9950_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	tp9950_sensor_get_resolution_func(NULL, &a->bounds.width, &a->bounds.height);
	a->bounds.left			= SENSOR_VALID_OFFSET_X;
	a->bounds.top			= SENSOR_VALID_OFFSET_Y;
	//a->bounds.width			= UXGA_WIDTH;
	//a->bounds.height		= UXGA_HEIGHT;
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;
#if (DEBUG)
	pr_info("%s bounds.width:%d, bounds.height:%d\n",
			__func__, a->bounds.width, a->bounds.height);
#endif
	return 0;
}

/*
 * XXXX_video_probe -
 * do some process in subdev probe
 * locale functions
 *
 * @sd:				subdev
 * @a:				return crop capbilities
 */
static int tp9950_video_probe(struct i2c_client *client)
{
	int ret;
	struct tp9950_priv *priv = to_tp9950(client);

#if (DEBUG)	
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	ret = v4l2_ctrl_handler_setup(&priv->hdl);
	return ret;
}

/*ctrl ops*/
static const struct v4l2_ctrl_ops tp9950_ctrl_ops = {
	.s_ctrl 			= tp9950_s_ctrl,
	.g_volatile_ctrl	= tp9950_g_volatile_ctrl,
};

/*core ops*/
static struct v4l2_subdev_core_ops tp9950_subdev_core_ops = {
	.s_power	= tp9950_core_s_power,
	.g_ctrl		= tp9950_core_g_ctrl,
	.s_ctrl		= tp9950_core_s_ctrl,
	.ioctl		= tp9950_core_ioctl,
};

/*
 * XXXX_g_mbus_config -
 * get buf config
 * video functions
 *
 * @sd:				subdev
 * @cfg:			return config
 */
static int tp9950_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
#if (DEBUG)
	printk(KERN_ERR "%s %d\n",__func__, __LINE__);
#endif
	cfg->flags = V4L2_MBUS_PCLK_SAMPLE_FALLING | V4L2_MBUS_MASTER |
		V4L2_MBUS_VSYNC_ACTIVE_HIGH | V4L2_MBUS_HSYNC_ACTIVE_HIGH |
		V4L2_MBUS_DATA_ACTIVE_HIGH;//V4L2_MBUS_PCLK_SAMPLE_RISING
	cfg->type = V4L2_MBUS_PARALLEL;
	cfg->flags = 0;

	return 0;
}

/*set crop*/
static int tp9950_s_crop(struct v4l2_subdev *sd, const struct v4l2_crop *crop)
{
#if (DEBUG)
	printk(KERN_ERR "%s %d, left:%d, top:%d, width:%d, height:%d\n",
			__func__, __LINE__, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
#endif
	return 0;
}

/*
 * XXXX_s_crop -
 * set crop
 * video functions
 *
 * @sd:				subdev
 * @crop:			crop to set
 */
static struct v4l2_subdev_video_ops tp9950_subdev_video_ops = {
	.s_stream			= tp9950_s_stream,
	.cropcap			= tp9950_cropcap,
	.g_crop				= tp9950_g_crop,
	.g_mbus_config		= tp9950_g_mbus_config,
	.s_crop				= tp9950_s_crop,
};

/*pad ops*/
static const struct v4l2_subdev_pad_ops tp9950_subdev_pad_ops = {
	.enum_mbus_code 	= tp9950_enum_mbus_code,
	.get_fmt			= tp9950_get_fmt,
	.set_fmt			= tp9950_set_fmt,
	.get_selection		= tp9950_get_selection,
};

/*sensor driver subdev ops*/
static struct v4l2_subdev_ops tp9950_subdev_ops = {
	.core	= &tp9950_subdev_core_ops,
	.video	= &tp9950_subdev_video_ops,
	.pad	= &tp9950_subdev_pad_ops,
};









/************************************
创建/pro/tp9950配置
***********************************/
#define TP9550_DEVICES "TP9950"
struct proc_dir_entry* tp9950_ctl = NULL;
typedef struct{

	int width;
	int height;
	int fps;
} tp9950_info;


static int tp9950_info_show(struct seq_file* seq,void*v){

	seq_puts(seq,"hello ,my name is leo");
	return 0;
}

static int tp9950_pro_open(struct inode* inode,struct file* file){

	return single_open(file,tp9950_info_show,inode->i_private);
}



static ssize_t tp9950_pro_write(struct file* file,const char __user* buffer,size_t count,loff_t*pos){

	tp9950_info info;
	int info_size = sizeof(tp9950_info);
	if(info_size != count){
		printk("%s:tp9950 write fail\n\r",__func__);
		return 0;
	}
	copy_from_user(&info, buffer, count);
	SENSOR_OUTPUT_WIDTH = info.width;
	SENSOR_OUTPUT_HEIGHT = info.height;
	SENSOR_OUTPUT_FPS = info.fps;

	SENSOR_VALID_OFFSET_X = (SENSOR_OUTPUT_WIDTH == 1280)?1:0;
#if (DEBUG)
	printk(KERN_ERR"TP9950 width:%d,height:%d, fps:%d\n",SENSOR_OUTPUT_WIDTH,SENSOR_OUTPUT_HEIGHT,SENSOR_OUTPUT_FPS);
#endif
	return count;
}

struct file_operations tp9950_ops = {

	.owner = THIS_MODULE,
    .open = tp9950_pro_open,
    .release = single_release,
    .write = tp9950_pro_write,
};



static int tp9950_pro_probe(void){

#if (DEBUG)
	printk("%s \n",__func__);
#endif
	tp9950_ctl = proc_create(TP9550_DEVICES,0777,NULL,&tp9950_ops);
	if(tp9950_ctl == NULL){
		printk("pro_create fialed \n\r");
		proc_remove(tp9950_ctl);
	 	return -EINVAL;
	}
	return 0;
}

static int tp9950_pro_remove(void){

#if (DEBUG)
	printk("%s \n",__func__);
#endif
	proc_remove(tp9950_ctl);
	
	return 0;
}












/*
 * i2c_driver functions
 */
static int tp9950_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	int ret = 0, i = 0, val = 0;
	struct tp9950_priv	*priv;
	struct i2c_adapter	*adapter = to_i2c_adapter(client->dev.parent);
#if (DEBUG)
    printk(KERN_ERR"TP9950 driver version %d.%d.%d loaded\n",
           (TP2850_VERSION_CODE >> 16) & 0xff,
           (TP2850_VERSION_CODE >>  8) & 0xff,
           TP2850_VERSION_CODE & 0xff);
#endif	
  //  printk(KERN_ERR"Compiled %s.%s.\n", __DATE__, __TIME__);
	
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&adapter->dev,"TP9950: I2C-Adapter doesn't support SMBUS\n");
		return -EIO;
	}
	
	priv = devm_kzalloc(&client->dev, sizeof(struct tp9950_priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&client->dev,"Failed to allocate memory for private data!\n");
		return -ENOMEM;
	}


	/*
	 * the current sensor slave address to client.
	 * the address from dts may incorrect
	 * */
	client->addr 		= SENSOR_I2C_ADDR;
	priv->cb_info.cb 	= &tp9950_cb;
	priv->cb_info.arg 	= priv;
	priv->client 		= client;
	
	if (!client->dev.of_node) {
		dev_err(&client->dev, "Missing platform_data for driver\n");
		ret = -EINVAL;
		goto err_clk;
	}

//	priv->cb_info.cb->sensor_init_func(priv,NULL);
	
	/*subdev init*/
	v4l2_i2c_subdev_init(&priv->subdev, client, &tp9950_subdev_ops);
	priv->subdev.flags |= /*V4L2_SUBDEV_FL_HAS_EVENTS | */V4L2_SUBDEV_FL_HAS_DEVNODE;
	v4l2_ctrl_handler_init(&priv->hdl, 5);
	v4l2_ctrl_new_std(&priv->hdl, &tp9950_ctrl_ops,
			V4L2_CID_VFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &tp9950_ctrl_ops,
			V4L2_CID_HFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &tp9950_ctrl_ops,
		V4L2_CID_BRIGHTNESS, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &tp9950_ctrl_ops,
		V4L2_CID_CONTRAST, 0, 1, 1, 0);


	/*private ctrl*/
	priv->ctrl_sensor_get_id = v4l2_ctrl_new_custom(&priv->hdl, &config_sensor_get_id, NULL);
	if (!priv->ctrl_sensor_get_id)
		pr_err("creat ctrl_sensor_get_id fail\n");

	priv->subdev.ctrl_handler = &priv->hdl;
	if (priv->hdl.error) {
		dev_err(&client->dev, "Hdl error\n");
		ret = priv->hdl.error;
		goto err_clk;
	}
	
	ret = tp9950_video_probe(client);
	if (ret < 0) {
		dev_err(&client->dev, "TP9950 video probe fail\n");
		goto err_videoprobe;
	}
	
	/*register async subdev*/
	ret = v4l2_async_register_subdev(&priv->subdev);
	if (ret < 0) {
		dev_err(&client->dev, "v4l2 async register subdev fail, ret:%d\n", ret);
		goto err_videoprobe;
	}

	dev_info(&adapter->dev, "TP9950 Probed success, subdev:%p\n", &priv->subdev);

	spin_lock_init(&readid_lock);
	spin_lock_init(&reg_lock);	

	/*ak platform need export some informations from sensor*/
	//call_sensor_sys_init();

	tp9950_pro_probe();

#if (DEBUG)
    printk("TP9950 Driver Init Successful!\n");
#endif
	return 0;

err_videoprobe:
	v4l2_ctrl_handler_free(&priv->hdl);
err_clk:
	return ret;
}

static int tp9950_remove(struct i2c_client *client)
{
	struct tp9950_priv *priv = to_tp9950(client);

	if (!priv) {
		pr_err("%s had remove\n", __func__);
		return 0;
	}

	/*unregister async subdev*/
	v4l2_async_unregister_subdev(&priv->subdev);
	/*unregister subdev*/
	v4l2_device_unregister_subdev(&priv->subdev);
	/*unregister handler*/
	v4l2_ctrl_handler_free(&priv->hdl);
	/*unregister sys node*/
	call_sensor_sys_deinit();
	
	tp9950_pro_remove();
	
	priv = NULL;
	return 0;
}

static const struct i2c_device_id tp9950_id[] = {
	{ "tp9950", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, tp9950_id);

static const struct of_device_id tp9950_of_match[] = {
	/*donot changed compatible, must the same as dts*/
	{.compatible = "anyka,sensor0", },
	{},
};
MODULE_DEVICE_TABLE(of, tp9950_of_match);

static struct i2c_driver tp9950_i2c_driver = {
	.driver = {
		.name = "anyka-sensor",
		.of_match_table = of_match_ptr(tp9950_of_match),
	},
	.probe    = tp9950_probe,
	.remove   = tp9950_remove,
	.id_table = tp9950_id,
};

module_i2c_driver(tp9950_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for TechPoint TP2850(tp9950) sensor");
MODULE_AUTHOR("xiao_zijie@anyka.com");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.3");

///////////////////////////////////////////////////////////////


static void set_timing(struct tp9950_priv *priv, int fps_res)
{
	int i;

	for (i = 0; i < sizeof(res_to_timing) / sizeof(res_to_timing[0]); i++)
	{
		if (((res_to_timing[i].fps << 8) | res_to_timing[i].res) == fps_res) 
		{
			void *arg = priv->hcb.isp_timing_cb_info.isp_timing_arg;
			SET_ISP_MISC set_isp_timing = priv->hcb.isp_timing_cb_info.set_isp_timing;

			//printk(KERN_ERR "-------------------------->>>set_isp_timing:%p, arg:%p, i:%d\n", set_isp_timing, arg, i);

			if (set_isp_timing)
				set_isp_timing(arg, &res_to_timing[i].isp_timing);
			
			break;
		}
	}

	return;
}



/*
 * The sensor must set callbacks
 *
 * */



static int tp9950_set_scan_method(struct tp9950_priv *priv,int* fps_res){

	/*******
	PAL制式
	********/
	if((SENSOR_OUTPUT_WIDTH == 960)&&(SENSOR_OUTPUT_HEIGHT == 288)&&(SENSOR_OUTPUT_FPS == 50)){
		
		priv->method = SCAN_METHOD_INTERLACED;
		//SENSOR_VALID_OFFSET_X = 0;
		*fps_res = (SENSOR_OUTPUT_FPS<<8)|INPUT_SD576I;
		//tp9950_set_v4l2_format(INPUT_SD576I);

	}else if((SENSOR_OUTPUT_WIDTH == 960)&&(SENSOR_OUTPUT_HEIGHT == 240) && (SENSOR_OUTPUT_FPS == 60)){

		priv->method = SCAN_METHOD_INTERLACED;
		//SENSOR_VALID_OFFSET_X = 0;
		*fps_res = (SENSOR_OUTPUT_FPS<<8)|INPUT_SD480I;
		//tp9950_set_v4l2_format(INPUT_SD480I);

	}else if((SENSOR_OUTPUT_WIDTH == 1280)&&(SENSOR_OUTPUT_HEIGHT == 720) && (SENSOR_OUTPUT_FPS == 25)){

		priv->method = SCAN_METHOD_PROGRESSIVE;
		//SENSOR_VALID_OFFSET_X = 1;
		*fps_res = (SENSOR_OUTPUT_FPS<<8)|INPUT_HD720P;
		//tp9950_set_v4l2_format(INPUT_HD720P);

	}else if((SENSOR_OUTPUT_WIDTH == 1280)&&(SENSOR_OUTPUT_HEIGHT == 720) && (SENSOR_OUTPUT_FPS == 30)){

		priv->method = SCAN_METHOD_PROGRESSIVE;
		//SENSOR_VALID_OFFSET_X = 1;
		*fps_res = (SENSOR_OUTPUT_FPS<<8)|INPUT_HD720P;
		//tp9950_set_v4l2_format(INPUT_HD720P);

	}else if((SENSOR_OUTPUT_WIDTH == 960)&&(SENSOR_OUTPUT_HEIGHT == 1080) && (SENSOR_OUTPUT_FPS == 25)){

		priv->method = SCAN_METHOD_PROGRESSIVE;
		//SENSOR_VALID_OFFSET_X = 0;
		*fps_res = (SENSOR_OUTPUT_FPS<<8)|INPUT_HD1080P;
		//tp9950_set_v4l2_format(INPUT_HD1080P);

	}else if((SENSOR_OUTPUT_WIDTH == 960)&&(SENSOR_OUTPUT_HEIGHT == 1080) && (SENSOR_OUTPUT_FPS == 30)){

		priv->method = SCAN_METHOD_PROGRESSIVE;
	//	SENSOR_VALID_OFFSET_X = 0;
		*fps_res = (SENSOR_OUTPUT_FPS<<8)|INPUT_HD1080P;
		//tp9950_set_v4l2_format(INPUT_HD1080P);
	}else{

		printk(KERN_ERR "%s %d ,signal error \n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

static int tp9950_sensor_init_func(void *arg, const AK_ISP_SENSOR_INIT_PARA *para)
{
	int fps_res = 0;
	int ret = 0;
	struct tp9950_priv *priv = arg;
	struct i2c_client *client = priv->client;
#if DEBUG	
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	tp9950_set_scan_method(priv,&fps_res);

	set_timing(priv, fps_res);
	
	priv->fps_info.current_fps = SENSOR_OUTPUT_FPS;
	priv->fps_info.to_fps = priv->fps_info.current_fps;
	priv->fps_info.to_fps_value = 0;
	return 0;
}

/*read sensor register*/
static int tp9950_sensor_read_reg_func(void *arg, const int reg_addr)
{
	struct tp9950_priv *priv = arg;
	struct i2c_client *client = priv->client;

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	return 0;
}

/*write sensor register*/
static int tp9950_sensor_write_reg_func(void *arg, const int reg_addr, int value)
{
	struct tp9950_priv *priv = arg;
	struct i2c_client *client = priv->client;

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	return 0;
}

/*read sensor register, NO i2c ops*/
static int tp9950_sensor_read_id_func(void *arg)   //no use IIC bus
{
	struct tp9950_priv *priv = arg;

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	return SENSOR_ID;//((SENSOR_ID<<8) | SENSOR_TYPE);
}

/*set sensor again*/
static int tp9950_sensor_update_a_gain_func(void *arg, const unsigned int a_gain)
{
	return 0;
}

/*set sensor dgain*/
static int tp9950_sensor_update_d_gain_func(void *arg, const unsigned int d_gain)
{
	return 0;
}

/*set sensor exp time*/
static int tp9950_sensor_updata_exp_time_func(void *arg, unsigned int exp_time)
{
	return 0;
}

/*sensor timer*/
static int tp9950_sensor_timer_func(void *arg)
{
	return 0;
}

/*standby in*/
static int tp9950_sensor_set_standby_in_func(void *arg)
{
	return 0;
}

/*standby out*/
static int tp9950_sensor_set_standby_out_func(void *arg)
{
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif	
	return 0;
}

/*low level read sensor ID, user i2c ops*/
static int __tp9950_sensor_probe_id_func(struct i2c_client *client)  //use IIC bus
{
	int id = 0;
	int value = 0;

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif
	return SENSOR_ID;//((SENSOR_ID << 8) | SENSOR_TYPE);
}

/*read sensor ID, user i2c ops*/
static int tp9950_sensor_probe_id_func(void *arg)  //use IIC bus
{
	struct tp9950_priv *priv = arg;
	struct i2c_client *client = priv->client;

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	return __tp9950_sensor_probe_id_func(client);
}

/*get resolution*/
static int tp9950_sensor_get_resolution_func(void *arg, int *width, int *height)
{
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif	

	*width = SENSOR_OUTPUT_WIDTH;
	*height = SENSOR_OUTPUT_HEIGHT;
	return 0;
}

/*get mclk*/
static int tp9950_sensor_get_mclk_func(void *arg)
{
#if (DEBUG)	
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	return SENSOR_MCLK;
}

/*get current fps*/
static int tp9950_sensor_get_fps_func(void *arg)
{
	struct tp9950_priv *priv = arg;

#if (DEBUG)	
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif
	return priv->fps_info.current_fps;
}

/*get valid coordinate*/
static int tp9950_sensor_get_valid_coordinate_func(void *arg, int *x, int *y)
{
	*x = SENSOR_VALID_OFFSET_X;
	*y = SENSOR_VALID_OFFSET_Y;
	return 0;
}

/*get bus type*/
static enum sensor_bus_type tp9950_sensor_get_bus_type_func(void *arg)
{
	return SENSOR_BUS_TYPE;
}

/*get self definded params*/
static int tp9950_sensor_get_parameter_func(void *arg, int param, void *value)
{
	int ret = 0;
	struct tp9950_priv *priv = arg;

#if (DEBUG)	
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif

	switch (param) {
		case GET_INTERFACE:
			*((int *)value) = SENSOR_IO_INTERFACE;
			break;

		case GET_IO_LEVEL:
			*((int *)value) = SENSOR_IO_LEVEL;
			break;

		case GET_RESET_GPIO:
			*((int *)value) = priv->gpio_reset;
			break;

		case GET_PWDN_GPIO:
			*((int *)value) = priv->gpio_pwdn;
			break;

		case GET_SCAN_METHOD:
			*((int *)value) = priv->method;
	//		*((int *)value) =  SCAN_METHOD_PROGRESSIVE;
			break;

		default:
			//pr_err("%s param:0x%x not support\n", __func__, param);
			ret = -1;
			break;
	}

	return ret;
}


/*sensor power on*/
static int tp9950_sensor_set_power_on_func(void *arg)
{
	struct tp9950_priv *priv = arg;
	struct i2c_client *client = priv->client;

#if (DEBUG)
	printk(KERN_ERR"%s start\n", __func__);
#endif
	return 0;
}

/*sensor power off*/
static int tp9950_sensor_set_power_off_func(void *arg)
{
	struct tp9950_priv *priv = arg;

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif
	return 0;
}

/*set sensor fps*/
static int tp9950_sensor_set_fps_func(void *arg, const int fps)
{
#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif
	return 0;
}

static AK_ISP_SENSOR_CB tp9950_cb = {
	.sensor_init_func 					= tp9950_sensor_init_func,
	.sensor_read_reg_func 				= tp9950_sensor_read_reg_func,
	.sensor_write_reg_func 				= tp9950_sensor_write_reg_func,
	.sensor_read_id_func 				= tp9950_sensor_read_id_func,
	.sensor_update_a_gain_func 			= tp9950_sensor_update_a_gain_func,
	.sensor_update_d_gain_func 			= tp9950_sensor_update_d_gain_func,
	.sensor_updata_exp_time_func 		= tp9950_sensor_updata_exp_time_func,
	.sensor_timer_func 					= tp9950_sensor_timer_func,
	.sensor_set_standby_in_func			= tp9950_sensor_set_standby_in_func,
	.sensor_set_standby_out_func 		= tp9950_sensor_set_standby_out_func,
	.sensor_probe_id_func				= tp9950_sensor_probe_id_func,
	.sensor_get_resolution_func 		= tp9950_sensor_get_resolution_func,
	.sensor_get_mclk_func 				= tp9950_sensor_get_mclk_func,
	.sensor_get_fps_func 				= tp9950_sensor_get_fps_func,
	.sensor_get_valid_coordinate_func 	= tp9950_sensor_get_valid_coordinate_func,
	.sensor_get_bus_type_func 			= tp9950_sensor_get_bus_type_func,
	.sensor_get_parameter_func 			= tp9950_sensor_get_parameter_func,
	.sensor_set_power_on_func 			= tp9950_sensor_set_power_on_func,
	.sensor_set_power_off_func 			= tp9950_sensor_set_power_off_func,
	.sensor_set_fps_func 				= tp9950_sensor_set_fps_func,
};

static int sensor_id_func(void)
{

#if (DEBUG)
	printk(KERN_ERR "%s %d\n", __func__, __LINE__);
#endif
	return SENSOR_ID;//tp9950_sensor_read_id_func(priv);

}

static char *sensor_if_func(void)
{
	static char ifstr[16] = "dvp"; 

	return ifstr;
}

static int call_sensor_sys_init(void)
{
	return sensor_sys_init(sensor_id_func, sensor_if_func);
}

static int call_sensor_sys_deinit(void)
{
	sensor_sys_exit();
	return 0;
}

