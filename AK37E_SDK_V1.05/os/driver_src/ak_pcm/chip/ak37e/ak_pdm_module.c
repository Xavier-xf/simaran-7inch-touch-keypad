/*
 * @file ak_pdm_module.c
 * @brief the source code of pdm controller
 * This file is the source code of PDM controller
 * Copyright (C) 2021 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2021-06-10
 * @version 1.0
 */

#include "pcm_port.h"
#include "ak_pdm_module.h"
#include "soundcard.h"

extern AK_SND_CARD g_ak_snd_card;

static int pdm_set_controller(unsigned int work_mode)
{
	SND_PARAM *codec = g_ak_snd_card.param;
	unsigned long reg_val;

	reg_val = __raw_readl(codec->pdm_base + PDM_CTRL_REG);
	reg_val &= ~(STEREO_MODE_EN | (0x1 << MONO_CHANNEL_SEL) | PDM_RX_STATUS | PDM_EN | \
				(0x1 << SAMPLE_MODE) | (0x1 << PDM_I2S_SEL));
	/* set pcm config register
	* 1 config pdm work mode (pdm_mode 2:mono_right, 0:mono_left, 1:stereo)
	* 2 enable pdm l2-mode receive
	*/
	reg_val |= (((work_mode & 0x3) << CHANNEL_SEL) | L2_MODE_EN);
	__raw_writel(reg_val, codec->pdm_base + PDM_CTRL_REG);

	return 0;
}

static void pdm_enable_disable(bool enable)
{
	SND_PARAM *codec = g_ak_snd_card.param;
	unsigned long reg_val;

	reg_val = __raw_readl(codec->pdm_base + PDM_CTRL_REG);
	if (enable) {
		reg_val |= (PDM_EN | FILTER_EN);
	} else {
		reg_val &= ~(PDM_EN | FILTER_EN);
	}
	__raw_writel(reg_val, codec->pdm_base + PDM_CTRL_REG);
}

int pdm_open(snd_dev dev)
{
	SND_PARAM *codec = g_ak_snd_card.param;

	pcm_clk_enable(codec->pdm_gclk);

	return 0;
}

int pdm_close(snd_dev dev)
{
	SND_PARAM *codec = g_ak_snd_card.param;
	unsigned  long reg_val;

	pdm_enable_disable(false);

	pcm_clk_disable(codec->pdm_gclk);
	pcm_clk_disable(codec->pdm_i2sm);
	pcm_clk_disable(codec->pdm_hsclk);

	/*
	* update the pdm_i2sm rate in clk core.
	*/
	pcm_clk_get_rate(codec->pdm_i2sm);

	return 0;
}

unsigned long codec_pdm_set_format(snd_dev dev, SND_FORMAT *format)
{
	SND_PARAM *codec = g_ak_snd_card.param;
	unsigned long sample_rate = 0;

	sample_rate = pdm_hw_setformat(codec, format->sample_rate);

	/*
	* set pdm controller,pdm_mode 2:mono_right, 0:mono_left, 1:stereo
	*/
	switch (format->channel) {
		case AK_PCM_TRACK_MONO_LEFT:
			pdm_set_controller(PDM_MODE_MONO_LEFT);
			break;
		case AK_PCM_TRACK_MONO_RIGHT:
			pdm_set_controller(PDM_MODE_MONO_RIGHT);
			break;
		case AK_PCM_TRACK_STEREO:
			pdm_set_controller(PDM_MODE_STEREO);
			break;
		default:
			pdm_set_controller(PDM_MODE_MONO_LEFT);
			break;
	}

	/* start captrue */
	pdm_enable_disable(true);

	return sample_rate;
}

int pdm_sethwparam(snd_dev dev, enum snd_hwparams_type type, int addr, unsigned int param)
{
	return 0;
}
