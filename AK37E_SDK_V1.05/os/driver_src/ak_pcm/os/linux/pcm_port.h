#ifndef __PCM_PORT_H__
#define __PCM_PORT_H__

#include <linux/cdev.h>
#include <linux/cpufreq.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <mach/ak_l2.h>
#include "soundcard.h"

#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/hardirq.h>
#include <asm-generic/gpio.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/clk-provider.h>

#define pcm_port_debug(fmt, arg...) \
	pr_debug("[%llu]:[%s:%d]: "fmt" \n\n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)
#define pcm_port_func(fmt, arg...) \
	pr_info("[%llu]:[%s]: "fmt" \n", ktime_to_ms(ktime_get_boottime()), __func__, ##arg)
#define pcm_port_info(fmt, arg...) \
	pr_info("[%llu]:[%s:%d]: "fmt" \n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)
#define pcm_port_warn(fmt, arg...) \
	pr_warn("[%llu]:[%s:%d] warn! "fmt" \n\n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)
#define pcm_port_err(fmt, arg...) \
	pr_err("[%llu]:[%s:%d] error! "fmt" \n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)

#define MHz	1000000UL

int pcm_clk_enable(struct clk *pclk);
void pcm_clk_disable(struct clk *pclk);
unsigned long pcm_clk_get_rate(struct clk *pclk);

unsigned long dac_hw_setformat(SND_PARAM *clk_param, unsigned long sample_rate);
unsigned long adc_hw_setformat(SND_PARAM *clk_param, unsigned long sample_rate);
unsigned long pdm_hw_setformat(SND_PARAM *clk_param, unsigned long sample_rate);

#endif  //__PCM_PORT_H__
