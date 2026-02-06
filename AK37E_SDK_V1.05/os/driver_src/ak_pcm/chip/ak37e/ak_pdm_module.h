/*
 * @file ak_pdm_module.h
 * @brief anyka pdm module interface
 * Copyright (C) 2021 Anyka (Guangzhou) Software Technology Co., LTD
 * @date  2021-03-18
 * @version 1.0
 */

#ifndef __AK_PDM_MODULE_H__
#define __AK_PDM_MODULE_H__

//#include "pcm_port.h"
#include "ak_pcm_defs.h"
//#include "soundcard.h"

extern struct snd_controller pdm_controller;

//#define	drv_printk_LR()	printk(KERN_ERR "########[pdm] file:%s,function:%s,line:%d\n",__FILE__,__func__,__LINE__)


#define ENABLE							1
#define DISABLE							0

#define PDM_MODE_MONO_LEFT				(0)
#define PDM_MODE_STEREO					(1)
#define PDM_MODE_MONO_RIGHT				(2)

#define PDM_CTRL_REG					(0x0)

// registers
#define CLOCK_GATE_CTRL_REG				0x001c
#define SOFT_RST_CTRL_REG				0x0020
#define CLOCK_AUDIO_PLL_CTRL			0x01B8
#define CLOCK_GATE_SOFT_RESET1			0x00FC
#define DAC_ADC_AUDIO_CLK_REG			0x01BC

// bits
#define	GCLK_GAT_CFG_ADC_CTRL1			(14)
#define PDM_CONTROLLER_RST				(1 << 14)	// reset pdm controller
#define	AUDIO_PLL_DISBALE				(15)
#define	AUDIO_PLL_REF_CLK_ENABLE		(16)
#define PDM_OLCLK_RST					(1 << 31)	// reset the module drivn by pdm_i2sm_clk
#define PDM_HCLK_RST					(1 << 30)	// reset the module driven by pdm_hsclk

/* pdm online clock cfg (audio pll channel clock control) */
#define PDM_OLCLK_CFG_VLD				(1 << 29)	// enable the adjustment of pdm_i2sm_clk divider of audio pll
#define PDM_OLCLK_EN					(1 << 28)	// enable pdm_i2sm_clk
#define PDM_OLCLK_DIV_CFG				(20)		// [27:20]pdm_i2sm_clk_div_num_cfg

/* pdm hclock cfg (adc/dac clock control of audio pll) */
#define PDM_HCLK_CFG_VLD				(1 << 29)	// enable the adjustment of pdm_hsclk divider of audio pll
#define PDM_HCLK_EN						(1 << 28)	// enable pdm hsclock
#define PDM_HCLK_DIV_CFG				(20)		// [27:20]pdm_hsclk_div_num_cfg

/* pdm interface control register */
#define MONO_CHANNEL_SEL				(18)		// 0:select left channel data, 1:select right channel data
#define STEREO_MODE_EN					(1 << 17)	// pdm stereo mode enable
#define CHANNEL_SEL						(17)		// [18:17] 2:mono_right, 0:mono_left, 1:stereo

#define FILTER_EN						(1 << 16)	// pdm filter enable
#define DECIMATION_RATIO_CFG			(13)		// [15:13] this filed conctrls the decimation ratio of sinc filter
#define SAMPLE_MODE						(12)		// pdm channel polarity

#define PDM_I2S_SEL						(4)			// receive port select
#define PDM_RX_EN						(~(1 << 4))	// pdm receive enable
#define I2S_RX_EN						(1 << 4)	// i2s receive enable

#define PDM_RX_STATUS					(1 << 2)	// pdm host receive interrupt
#define L2_MODE_EN						(1 << 1)	// enable the L2 mode
#define PDM_EN							(1 << 0)	// enable pdm control interface


#endif //__AK_PDM_MODULE_H__
