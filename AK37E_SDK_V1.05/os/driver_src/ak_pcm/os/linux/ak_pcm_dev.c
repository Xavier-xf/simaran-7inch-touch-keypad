/*
 *  pcm for anyka chip
 *  Copyright (c) by Anyka, Inc.
 *  Create by chen_yanhong 2020-06-19
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 * 备注
 * 本文件可拆解以下部分
 * 1. 作为平台设备的驱动akpcm_driver关联的接口的实现
 * 2. 一系列的字符设备的生命周期管理的相关接口的实现
 * 3. 运行时结构体的注册注销等
 * 4. 文件操作句柄接口等
 * 5. 与外部模块的消息链路的实现等
 */
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <asm-generic/gpio.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <mach/ak_l2.h>

#include "ak_pcm_defs.h"
#include "capture.h"
#include "playback.h"
#include "loopback.h"
#include "soundcard.h"
#include "ak_pcm_sys.h"

#define AKPCM_CLASS      "akpcm"
#define AKPCM_MAJOR      0
#define AKPCM_MAX_TYPE_SUPPORT_COUNT  2 /*MAX count for single device type*/

#define AKPCM_DEFAULT_PLAYBACK_DEV  (SND_CARD_DAC_PLAYBACK)
#define AKPCM_DEFAULT_CAPTURE_DEV  (SND_CARD_ADC_CAPTURE)

static int pcm_major = AKPCM_MAJOR;
static struct class *pcm_class;
static struct akpcm *g_akpcm;

static struct akpcm_runtime *akpcm_runtime_register(struct device_node *np,
	int snd_dev, int snd_type);
static int akpcm_runtime_unregister(struct akpcm_runtime *runtime);

#ifdef AKPCM_DEV_HOTPLUG
/*
* 为消除ak_pcm驱动跟外部的音频驱动之间加载的先后顺序
* 添加此处内核消息链表 用于收发相关的信息
* 命令的详细含义可以参看文档##ANYCLOUD_音频驱动I2S模块设计_V1.0.0##中相关描述
*/
#include <mach/ak_notifier.h>
#include <linux/notifier.h>

extern int register_ak_notifier(struct notifier_block *block);
extern int unregister_ak_notifier(struct notifier_block *block);
extern int ak_notifier_call_chain(unsigned long action, void *data);

static int akpcm_notifier_call(struct notifier_block *nb, unsigned long action, void *data)
{
	unsigned int event_group = AK_NOTIFIER_EVENT_GROUP(action);
	unsigned int event = AK_NOTIFIER_EVENT_VAL(action);

	ak_pcm_info("akpcm: group#%d event#%d", event_group, event);

	if (event_group != AK_NOTIFIER_EVENT_GROUP_AUDIO)
		return NOTIFY_DONE;

	switch (event) {
		case AK_NOTIFIER_AUDIO_SUB_ALIVE:
			/*
			* 这里接受到子设备驱动注册消息 作为回应 将本驱动已加载的消息发送给外部设备驱动 并提供设备节点的创建接口
			*/
			ak_notifier_call_chain(AK_NOTIFIER_EVENT_MK(AK_NOTIFIER_EVENT_GROUP_AUDIO, AK_NOTIFIER_AUDIO_ALIVE), akpcm_runtime_register);
			break;
		case AK_NOTIFIER_AUDIO_SUB_REMOVE:
			/*
			* 子设备驱动注销 此处将关联的设备节点/内存等资源给释放掉
			*/
			akpcm_runtime_unregister((struct akpcm_runtime *)data);
			break;
	}

	return NOTIFY_OK;
}

static struct notifier_block akpcm_notifier_block = {
	.notifier_call = akpcm_notifier_call,
};

#endif

/***********************************************************************************************************/
/*
* 设备节点文件操作的接口句柄
*/
static int akpcm_playback_open(struct inode *inode, struct file *filp)
{
	struct akpcm_runtime *rt = container_of(inode->i_cdev, struct akpcm_runtime, cdev);

	filp->private_data = rt;

	return playback_open(rt);
}

static int akpcm_playback_close(struct inode *inode, struct file *filp)
{
	struct akpcm_runtime *rt = filp->private_data;

	return playback_close(rt);
}

static long akpcm_playback_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct akpcm_runtime *rt = filp->private_data;

	return playback_ioctl(rt, cmd, arg);
}

static ssize_t akpcm_playback_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct akpcm_runtime *rt = filp->private_data;

	return playback_write(rt, buf, count, f_pos, filp->f_flags);
}

static int akpcm_capture_open(struct inode *inode, struct file *filp)
{
	struct akpcm_runtime *rt = container_of(inode->i_cdev, struct akpcm_runtime, cdev);

	filp->private_data = rt;

	return capture_open(rt);
}

static int akpcm_capture_close(struct inode *inode, struct file *filp)
{
	struct akpcm_runtime *rt = filp->private_data;

	return capture_close(rt);
}

static long akpcm_capture_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct akpcm_runtime *rt = filp->private_data;

	return capture_ioctl(rt, cmd, arg);
}

static ssize_t akpcm_capture_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct akpcm_runtime *rt = filp->private_data;

	return capture_read(rt, buf, count, f_pos, filp->f_flags);
}

static int akpcm_loopback_open(struct inode *inode, struct file *filp)
{
	struct akpcm_runtime *rt = container_of(inode->i_cdev, struct akpcm_runtime, cdev);

	filp->private_data = rt;

	return loopback_open(rt);
}

static int akpcm_loopback_close(struct inode *inode, struct file *filp)
{
	struct akpcm_runtime *rt = filp->private_data;

	return loopback_close(rt);
}

static long akpcm_loopback_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct akpcm_runtime *rt = filp->private_data;

	return loopback_ioctl(rt, cmd, arg);
}

static ssize_t akpcm_loopback_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct akpcm_runtime *rt = filp->private_data;

	return loopback_read(rt, buf, count, f_pos, filp->f_flags);
}

static const struct file_operations pcm_fops[AK_PCM_DEV_MAX] =
{
	[0] = {
		.owner   = THIS_MODULE,
		.open    = akpcm_playback_open,
		.release = akpcm_playback_close,
		.unlocked_ioctl = akpcm_playback_ioctl,
		.write   = akpcm_playback_write,
	},
	[1] = {
		.owner   = THIS_MODULE,
		.open    = akpcm_capture_open,
		.release = akpcm_capture_close,
		.unlocked_ioctl = akpcm_capture_ioctl,
		.read    = akpcm_capture_read,
	},
	[2] = {
		.owner   = THIS_MODULE,
		.open    = akpcm_loopback_open,
		.release = akpcm_loopback_close,
		.unlocked_ioctl = akpcm_loopback_ioctl,
		.read    = akpcm_loopback_read,
	},
};
/***********************************************************************************************************/


static char* akpcm_device_type[] =
{
	"p",
	"c",
	"l",
};

/*
 * akpcm_class_setup -
 * create pcm class node
 *
 * return 0 if successfully or negative error value if fail.
 * 创建对应的设备节点对应的class
 */
static int akpcm_class_setup(void)
{
	int err = 0;
	dev_t devno;

	pcm_class = class_create(THIS_MODULE, AKPCM_CLASS);
	if (IS_ERR(pcm_class)) {
		err = PTR_ERR(pcm_class);
		ak_pcm_err("akpcm: can't register %s\n", AKPCM_CLASS);
		goto exit_class_setup;
	}

	if (pcm_major) {
		devno = MKDEV(pcm_major, 0);
		err = register_chrdev_region(devno, AK_PCM_DEV_MAX, "pcmchar");
		if (err < 0) {
			ak_pcm_err("akpcm: can't alloc chrdev\n");
			goto exit_class_setup;
		}
	} else {
		err = alloc_chrdev_region(&devno, 0, AK_PCM_DEV_MAX, "pcmchar");
		if (err < 0) {
			ak_pcm_err("akpcm: can't region chrdev\n");
			goto exit_class_setup;
		}
		pcm_major = MAJOR(devno);
	}

	ak_pcm_info("akpcm: pcm_major %d", pcm_major);

	return 0;

exit_class_setup:
	class_destroy(pcm_class);

	return err;
}

/*
 * akpcm_class_cleanup -
 * remove pcm class node
 * 资源注销
 */
static void akpcm_class_cleanup(void)
{
	if (pcm_class) {
		class_destroy(pcm_class);
		pcm_class = NULL;
	}
}

/*
* pcmC{x}D{y}[cpl]
* C{x}: 0 -- internel dac&dac
*       1 -- i2s
*       2 -- pdm
* D{y}: ..
* 关于设备节点的命名规则 可参考##ANYCLOUD_音频驱动模块设计##中的相关说明
*/
static int akpcm_get_namespace(int dev_type, int *c_idx, int *d_idx)
{
	int err = 0;

	switch (dev_type) {
		case SND_CARD_DAC_PLAYBACK:
		case SND_CARD_ADC_CAPTURE:
		case SND_CARD_DAC_LOOPBACK:
			*c_idx = 0;
			*d_idx = 0;
			break;
		case SND_CARD_I2S0_SEND:
		case SND_CARD_I2S0_RECV:
		case SND_CARD_I2S0_LOOPBACK:
			*c_idx = 1;
			*d_idx = 0;
			break;
		case SND_CARD_I2S1_RECV:
			*c_idx = 1;
			*d_idx = 1;
			break;
		case SND_CARD_PDM_RECV:
			*c_idx = 2;
			*d_idx = 0;
			break;
		default:
			err = -EINVAL;
			break;
	}

	return 0;
}

/*
 * akpcm_cdev_setup -
 * create pcm cdev node
 *
 * @runtime: device runtime, provide the cdev member
 * @dev_type: which device type for runtime related with the enum akpcm_device
 *
 * return 0 if successfully or negative error value if fail.
 * 创建指定的字符设备节点 该节点向上层的应用层提供文件操作
 */
static int akpcm_cdev_setup(struct akpcm_runtime *runtime, int snd_type, int snd_dev)
{
	struct device *dev;
	int err = 0, c_idx = 0, d_idx = 0;
	dev_t devno = MKDEV(pcm_major, snd_dev);

	if (akpcm_get_namespace(snd_dev, &c_idx, &d_idx))
		return -EINVAL;

	cdev_init(&(runtime->cdev), &pcm_fops[snd_type]);
	err = cdev_add(&(runtime->cdev), devno, 1);
	if (err)
		return err;

	/* ak_pcm_info("akpcm: create pcmC%dD%d%s", 0, index, akpcm_device_type[dev_type]); */

	runtime->dev = device_create(pcm_class , NULL, devno, runtime, "pcmC%dD%d%s", c_idx, d_idx, akpcm_device_type[snd_type]);
	if (IS_ERR(runtime->dev)) {
		err = PTR_ERR(runtime->dev);
		ak_pcm_err("akpcm: can't create pcmC%dD%d%s", c_idx, d_idx, akpcm_device_type[snd_type]);
	}

	return err;
}

/*
 * akpcm_cdev_release -
 * release pcm cdev node
 *
 * @cdev: the cdev which to release
 * @dev_type: which device type for runtime related with the enum akpcm_device
 * 字符设备节点注销
 */
static void akpcm_cdev_release(struct cdev *cdev, int dev_type, int snd_dev)
{
	dev_t devno = MKDEV(pcm_major, snd_dev);

	device_destroy(pcm_class, devno);
	cdev_del(cdev);
}

/*
 * akpcm_playback_runtime_init -
 * init the playback rumtime
 *
 * @pcm: the platform private data which manager the runtimes
 *
 * return 0 if successfully or negative error value if fail.
 * 播放模块运行时的初始化函数
 */
static struct akpcm_runtime* akpcm_playback_runtime_init(struct device_node *np,
	int snd_dev, int snd_type)
{
	int err;
	struct akpcm_runtime *runtime = NULL;

	runtime = kzalloc(sizeof(struct akpcm_runtime), GFP_KERNEL);
	if (runtime == NULL) {
		ak_pcm_err("playback rt allocate failed");
		return NULL;
	}

	err = akpcm_cdev_setup(runtime, AK_PCM_DEV_PLAYBACK, snd_dev);
	if (err) {
		ak_pcm_err("playback cdev fail %d", err);
		goto exit_playback_runtime_init;
	}

	init_waitqueue_head(&(runtime->wq));
	clear_bit(STATUS_START_STREAM, &(runtime->runflag));
	clear_bit(STATUS_WORKING, &(runtime->runflag));
	clear_bit(STATUS_PAUSING, &(runtime->runflag));
	clear_bit(STATUS_OPENED, &(runtime->runflag));
	clear_bit(STATUS_PREPARED, &(runtime->runflag));
	runtime->dma_area = NULL;
	runtime->cache_buff = NULL;
	runtime->buffer_bytes = 0;
	runtime->cfg.rate = 8000;
	runtime->cfg.channels = 2;
	runtime->cfg.sample_bits = 16;
	runtime->cfg.period_bytes = 16384;
	runtime->cfg.periods = 32;
	runtime->snd_dev = snd_dev;
	runtime->snd_type = snd_type;

#ifdef CONFIG_PM
	runtime->pm_ops.resume = playback_pm_resume;
	runtime->pm_ops.suspend = playback_pm_suspend;
#endif

	if (of_property_read_u32(np, "hp_gain", &runtime->hp_gain_cur)) {
		runtime->hp_gain_cur = DEFAULT_HPVOL;
	}

	/* ak_pcm_func("playback_runtime rt 0x%p dev %d", runtime, snd_dev); */

	return runtime;

exit_playback_runtime_init:
	kfree(runtime);

	return NULL;
}

/*
 * akpcm_capture_runtime_init -
 * init the capture rumtime
 *
 * @pcm: the platform private data which manager the runtimes
 *
 * return 0 if successfully or negative error value if fail.
 * 采集模块运行时的初始化函数
 */
static struct akpcm_runtime* akpcm_capture_runtime_init(struct device_node *np,
	int snd_dev, int snd_type)
{
	int err;
	struct akpcm_runtime *runtime = NULL;
	struct device_node *npp = of_get_parent(np);

	runtime = kzalloc(sizeof(struct akpcm_runtime), GFP_KERNEL);
	if (runtime == NULL) {
		ak_pcm_err("capture rt allocate failed");
		return NULL;
	}

	err = akpcm_cdev_setup(runtime, AK_PCM_DEV_CAPTURE, snd_dev);
	if (err) {
		ak_pcm_err("capture cdev fail %d", err);
		goto exit_capture_runtime_init;
	}

	init_waitqueue_head(&(runtime->wq));
	clear_bit(STATUS_START_STREAM, &(runtime->runflag));
	clear_bit(STATUS_WORKING, &(runtime->runflag));
	clear_bit(STATUS_PAUSING, &(runtime->runflag));
	clear_bit(STATUS_OPENED, &(runtime->runflag));
	clear_bit(STATUS_PREPARED, &(runtime->runflag));
	runtime->dma_area = NULL;
	runtime->cache_buff = NULL;
	runtime->buffer_bytes = 0;
	runtime->cfg.rate = 8000;
	runtime->cfg.channels = 1;
	runtime->cfg.sample_bits = 16;
	runtime->cfg.period_bytes = 16384;
	runtime->cfg.periods = 16;
	runtime->snd_dev = snd_dev;
	runtime->snd_type = snd_type;

#ifdef CONFIG_PM
	runtime->pm_ops.resume = capture_pm_resume;
	runtime->pm_ops.suspend = capture_pm_suspend;
#endif

	if (of_property_read_u32(np, "mic_gain", &runtime->mic_gain_cur)) {
		runtime->mic_gain_cur = DEFAULT_MICVOL;
	}

	if (of_property_read_u32(np, "linein_gain", &runtime->linein_gain_cur)) {
		runtime->linein_gain_cur = DEFAULT_LINEINVOL;
	}

	/* ak_pcm_func("capture_runtime rt 0x%p dev %d", runtime, snd_dev); */

	return runtime;

exit_capture_runtime_init:
	kfree(runtime);

	return NULL;
}

/*
 * akpcm_loopback_runtime_init -
 * init the loopback rumtime
 *
 * @pcm: the platform private data which manager the runtimes
 *
 * return 0 if successfully or negative error value if fail.
 * 回送设备的运行时初始化函数
 */
static struct akpcm_runtime* akpcm_loopback_runtime_init(struct device_node *np,
	int snd_dev, int snd_type)
{
	int err;
	struct akpcm_runtime *runtime = NULL;

	runtime = kzalloc(sizeof(struct akpcm_runtime), GFP_KERNEL);
	if (runtime == NULL) {
		ak_pcm_err("loopback rt allocate failed");
		return NULL;
	}

	err = akpcm_cdev_setup(runtime, AK_PCM_DEV_LOOPBACK, snd_dev);
	if (err) {
		ak_pcm_err("loopback cdev fail %d", err);
		goto exit_loopback_runtime_init;
	}

	init_waitqueue_head(&(runtime->wq));

	runtime->dma_area = NULL;
	runtime->cache_buff = NULL;
	runtime->buffer_bytes = 0;
	runtime->snd_dev = snd_dev;
	runtime->snd_type = snd_type;
	clear_bit(STATUS_START_STREAM, &(runtime->runflag));
	clear_bit(STATUS_WORKING, &(runtime->runflag));
	clear_bit(STATUS_PAUSING, &(runtime->runflag));
	clear_bit(STATUS_OPENED, &(runtime->runflag));
	clear_bit(STATUS_PREPARED, &(runtime->runflag));

#ifdef CONFIG_PM
	runtime->pm_ops.resume = NULL;
	runtime->pm_ops.suspend = NULL;
#endif

	/* ak_pcm_func("loopback_runtime rt 0x%p", runtime); */

	return runtime;

exit_loopback_runtime_init:
	kfree(runtime);

	return NULL;
}

/*
* 该函数用于指定设备的运行时初始化
* np是对应设备节点的of句柄 用于获取相关的dts配置信息
* snd_dev 是对应设备的设备类型
* snd_type 是对应设备的类型
*/
static struct akpcm_runtime *akpcm_runtime_register(struct device_node *np,
	int snd_dev, int snd_type)
{
	struct akpcm_runtime *new_runtime = NULL;

	switch (snd_type) {
		case AK_PCM_DEV_PLAYBACK:
			new_runtime = akpcm_playback_runtime_init(np, snd_dev, snd_type);
			break;
		case AK_PCM_DEV_CAPTURE:
			new_runtime = akpcm_capture_runtime_init(np, snd_dev, snd_type);
			break;
		case AK_PCM_DEV_LOOPBACK:
			new_runtime = akpcm_loopback_runtime_init(np, snd_dev, snd_type);
			break;
		default:
			ak_pcm_err("err snd_type@%d", snd_type);
			break;
	}

	if (new_runtime != NULL) {
		list_add_tail(&new_runtime->list, &g_akpcm->runtime_list);
	}

#ifdef AKPCM_DEV_HOTPLUG
	switch (snd_dev) {
		case SND_CARD_I2S0_SEND:
		case SND_CARD_I2S0_RECV:
		case SND_CARD_I2S1_RECV:
			ak_notifier_call_chain(AK_NOTIFIER_EVENT_MK(AK_NOTIFIER_EVENT_GROUP_AUDIO, AK_NOTIFIER_AUDIO_SUB_ADD), snd_card_register);
			break;
		default:
			break;
	}
#endif

	return new_runtime;
}

/*
* 该函数用于运行时的销毁 这里会释放关联的内存资源
* 并对设备节点进行释放 同时注销外部音频驱动关联的控制器操作句柄
* 并将本运行时从全局的运行时链表中给移除掉
*/
static int akpcm_runtime_unregister(struct akpcm_runtime *runtime)
{
	akpcm_cdev_release(&(runtime->cdev), runtime->snd_type, runtime->snd_dev);

	switch (runtime->snd_type) {
		case AK_PCM_DEV_PLAYBACK:
			if (runtime->dma_area) {
				dma_unmap_single(runtime->dev, runtime->dma_addr, runtime->buffer_bytes, DMA_TO_DEVICE);
				kfree(runtime->dma_area);
				runtime->dma_area = NULL;
			}
			if (runtime->cache_buff) {
				vfree(runtime->cache_buff);
				runtime->cache_buff = NULL;
			}
			break;
		case AK_PCM_DEV_CAPTURE:
			if (runtime->dma_area) {
				dma_unmap_single(runtime->dev, runtime->dma_addr, runtime->buffer_bytes, DMA_FROM_DEVICE);
				kfree(runtime->dma_area);
				runtime->dma_area = NULL;
			}
			/*
			* NOTICE:capture donot use the cache_buff
			*/
			if (runtime->cache_buff) {
				kfree(runtime->cache_buff);
				runtime->cache_buff = NULL;
			}
			break;
		case AK_PCM_DEV_LOOPBACK:
			if (runtime->cache_buff != NULL) {
				vfree(runtime->cache_buff);
				runtime->cache_buff = NULL;
			}
			if (runtime->dma_area) {
				vfree(runtime->dma_area);
				runtime->dma_area = NULL;
			}
			break;
		default:
			ak_pcm_err("err snd_type@%d", runtime->snd_type);
			break;
	}

	list_del_init(&runtime->list);
	kfree(runtime);

#ifdef AKPCM_DEV_HOTPLUG
	switch (runtime->snd_dev) {
		case SND_CARD_I2S0_SEND:
		case SND_CARD_I2S0_RECV:
		case SND_CARD_I2S1_RECV:
			snd_card_unregister(runtime->snd_dev);
			break;
		default:
			break;
	}
#endif

	return 0;
}

/*
* 释放运行时链表上的各个运行时
* 将其关联的字符设备/内存资源/控制器操作句柄给释放掉
*/
static int ak_free_runtimes(struct akpcm *pcm)
{
	struct akpcm_runtime *runtime, *t_runtime;

	if (pcm == NULL)
		return -EINVAL;

	list_for_each_entry_safe(runtime, t_runtime, &pcm->runtime_list, list) {
		akpcm_runtime_unregister(runtime);
	}

	return 0;
}

/*
 * akpcm_probe -
 * pcm driver probe
 *
 * @pdev: pointer to platform device
 *
 * return: 0-successful, <0- failed
 */
static int  akpcm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *children_node;
	struct akpcm *pcm = NULL;
	int err = 0, list_count = 0;
	int enable_level;
	struct resource *res;
	SND_PARAM *param;
	int snd_dev, snd_type;
	struct akpcm_runtime *runtime = NULL;
	struct pinctrl *pinctrl = devm_pinctrl_get(dev);
	struct pinctrl_state *pin_state;

	pcm = (struct akpcm *)kzalloc(sizeof(struct akpcm), GFP_KERNEL);
	if (pcm == NULL) {
		dev_err(dev,"akpcm probe alloc memory fail\n");
		return -ENOMEM;
	}
	g_akpcm = pcm;

	INIT_LIST_HEAD(&pcm->runtime_list);

	param = &pcm->param;
	param->speaker_gpio = of_get_named_gpio(np, "speak-gpios", 0);
	if (param->speaker_gpio >= 0) {
		err = of_property_read_u32(np, "speak-gpios-en", &enable_level);
		if (err < 0) {
			enable_level = 1;
		}
		/* set PA off */
		param->speaker_en_level = enable_level;
	}
	ak_pcm_info("SPEAKER @%d(lv %d)", param->speaker_gpio, param->speaker_en_level);

	pcm->dev = dev;

	platform_set_drvdata(pdev, pcm);

	err = akpcm_class_setup();
	if (err) {
		dev_err(dev,"akpcm_class_setup %d\n", err);
		goto exit_akpcm_class_setup;
	}

	for_each_child_of_node(np, children_node) {
		err = of_property_read_u32(children_node, "snd-dev", &snd_dev);
		if (err) {
			dev_err(dev, "bad prop@snd-dev\n");
			goto exit_runtime_init;
		}
		err = of_property_read_u32(children_node, "snd-type", &snd_type);
		if (err) {
			dev_err(dev, "bad prop@snd-type\n");
			goto exit_runtime_init;
		}
		runtime = akpcm_runtime_register(children_node, snd_dev, snd_type);
		ak_pcm_info("rt#0x%p@snd_dev#%d snd_type#%d", runtime, snd_dev, snd_type);
		if (runtime == NULL) {
			err = -ENODEV;
			goto exit_runtime_init;
		}
		if (runtime->snd_dev == SND_CARD_PDM_RECV) {
			pin_state = pinctrl_lookup_state(pinctrl, "pdm_pins");
			if (IS_ERR(pin_state)) {
				dev_err(dev, "bad prop@pdm_pins\n");
				err = -ENODEV;
				goto exit_runtime_init;
			}
			err = pinctrl_select_state(pinctrl, pin_state);
			if (err) {
				dev_err(dev, "fail pinctrl@pdm_pins\n");
				goto exit_runtime_init;
			}
		}
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);    /* get SD_ADC/DAC clock control register */
	if (!res) {
		dev_err(dev,"no memory resource for analog_ctrl_res\n");
		err = -ENXIO;
		goto exit_akpcm_get_resource;
	}

	param->sysctrl_base = devm_ioremap(dev, res->start, res->end - res->start + 1);
	if (!param->sysctrl_base) {
		dev_err(dev,"could not remap analog_ctrl_res memory\n");
		err = -ENXIO;
		goto exit_akpcm_get_resource;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);     /* get ADC2 mode registers */
	if (!res) {
		dev_err(dev,"no memory resource for adda_cfg_res\n");
		err = -ENXIO;
		goto exit_akpcm_get_resource;
	}

	param->adda_cfg_base = devm_ioremap(dev, res->start, res->end - res->start + 1);
	if (!param->adda_cfg_base) {
		dev_err(dev,"could not remap adda_cfg_res memory\n");
		err = -ENXIO;
		goto exit_akpcm_get_resource;
	}

#ifdef AK_AUDIO_PDM_SUPPORT
	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (!res) {
		dev_err(dev,"no memory resource for pdm_base\n");
		err = -ENXIO;
		goto exit_akpcm_get_resource;
	}

	param->pdm_base = devm_ioremap(dev, res->start, res->end - res->start + 1);
	if (!param->pdm_base) {
		dev_err(dev,"could not remap pdm_base memory\n");
		err = -ENXIO;
		goto exit_akpcm_get_resource;
	}
#endif

	/* adc clock */
	param->sdadc_gclk = of_clk_get_by_name(np, "sdadc_gclk");
	param->sdadc_clk = of_clk_get_by_name(np, "sdadc_clk");
	param->sdadchs_clk = of_clk_get_by_name(np, "sdadchs_clk");

	/* dac clock */
	param->sddac_gclk = of_clk_get_by_name(np, "sddac_gclk");
	param->sddac_clk = of_clk_get_by_name(np, "sddac_clk");
	param->sddachs_clk = of_clk_get_by_name(np, "sddachs_clk");

	param->pdm_gclk = of_clk_get_by_name(np, "pdm_gclk");
	param->pdm_i2sm = of_clk_get_by_name(np, "pdm_i2sm");
	param->pdm_hsclk = of_clk_get_by_name(np, "pdm_hsclk");

	err = snd_card_init(param);
	if (err) {
		dev_err(dev,"sound card init fail %d\n", err);
		goto exit_snd_card_init;
	}

	if (ak_pcm_sys_init(pcm)) {
		dev_err(dev,"sys fs init fail\n");
	}

#ifdef AKPCM_DEV_HOTPLUG
	ak_notifier_call_chain(AK_NOTIFIER_EVENT_MK(AK_NOTIFIER_EVENT_GROUP_AUDIO, AK_NOTIFIER_AUDIO_ALIVE), akpcm_runtime_register);
	register_ak_notifier(&akpcm_notifier_block);
#endif

	ak_pcm_info("successful!");

	return 0;

exit_snd_card_init:
exit_akpcm_get_resource:
exit_runtime_init:
	ak_free_runtimes(pcm);
	akpcm_class_cleanup();
exit_akpcm_class_setup:
	if (err != 0) {
		kfree(pcm);
	}
	return err;
}

/*
 * akpcm_remove -
 * pcm platfrom driver remove
 *
 * @devptr: pointer to platform device
 *
 * return: 0-successful, <0- failed
 */
static int akpcm_remove(struct platform_device *pdev)
{
	struct akpcm *pcm = platform_get_drvdata(pdev);
	dev_t devno = MKDEV(pcm_major, 0);

#ifdef AKPCM_DEV_HOTPLUG
	ak_notifier_call_chain(AK_NOTIFIER_EVENT_MK(AK_NOTIFIER_EVENT_GROUP_AUDIO, AK_NOTIFIER_AUDIO_REMOVE), NULL);
	unregister_ak_notifier(&akpcm_notifier_block);
#endif

	ak_pcm_info("akpcm_remove pcm 0x%p", pcm);

	ak_pcm_sys_exit(pcm);

	snd_card_deinit();

	/* free runtimes */
	ak_free_runtimes(pcm);

	unregister_chrdev_region(devno, AK_PCM_DEV_MAX);
	akpcm_class_cleanup();

	kfree(pcm);

	return 0;
}

#ifdef CONFIG_PM
static int akpcm_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct akpcm_runtime *runtime, *t_runtime;

	list_for_each_entry_safe(runtime, t_runtime, &g_akpcm->runtime_list, list) {
		if (runtime->pm_ops.suspend != NULL)
			runtime->pm_ops.suspend(runtime);
	}

	pinctrl_pm_select_sleep_state(&pdev->dev);

	return 0;
}

static int akpcm_resume(struct platform_device *pdev)
{
	struct akpcm_runtime *runtime, *t_runtime;

	pinctrl_pm_select_default_state(&pdev->dev);

	list_for_each_entry_safe(runtime, t_runtime, &g_akpcm->runtime_list, list) {
		if (runtime->pm_ops.resume != NULL)
			runtime->pm_ops.resume(runtime);
	}

	return 0;
}
#else
#define akpcm_suspend NULL
#define akpcm_resume NULL
#endif

static const struct of_device_id akpcm_of_ids[] = {
	{ .compatible = "anyka,ak39ev330-dac" },
	{ .compatible = "anyka,ak39ev330-adc-dac" },
	{ .compatible = "anyka,ak37d-adc-dac" },
	{ .compatible = "anyka,ak37e-adc-dac" },
	{},
};
MODULE_DEVICE_TABLE(of, akpcm_of_ids);

static struct platform_driver akpcm_driver = {
	.probe      = akpcm_probe,
	.remove     = akpcm_remove,
	.suspend    = akpcm_suspend,
	.resume     = akpcm_resume,
	.driver     = {
		.name   = "ak-codec",
		.of_match_table = of_match_ptr(akpcm_of_ids),
	},
};

static int __init akpcm_init(void)
{
	ak_pcm_info("akpcm_init...");
	return platform_driver_register(&akpcm_driver);
}

static void __exit akpcm_exit(void)
{
	platform_driver_unregister(&akpcm_driver);
}

module_init(akpcm_init);
module_exit(akpcm_exit);

MODULE_DESCRIPTION("Anyka PCM driver");
MODULE_AUTHOR("Anyka Microelectronic Ltd.");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1.13");
