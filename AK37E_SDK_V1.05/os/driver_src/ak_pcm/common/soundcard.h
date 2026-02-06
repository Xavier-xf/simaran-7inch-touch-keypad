/*
 * @file  soundcard.h
 * @brief sound operation interface
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author	wushangsong
 * @date  2020-06-20
 * @version 1.0
 */

#ifndef __AKPCM_SOUNDCARD_H__
#define __AKPCM_SOUNDCARD_H__

#include "ak_pcm_uapi.h"

#define MIXER_VOL_HP				0
#define MIXER_VOL_LI				1
#define MIXER_VOL_MIC				2
#define MIXER_VOL_END				2

#define DEV_POWER_MIC				0
#define DEV_POWER_LINEIN			1
#define DEV_POWER_HP				2

enum snd_mux {
	SND_MUX_SRC_MUTE = 0,
	SND_MUX_SRC_MIC,
	SND_MUX_SRC_LINEIN,
	SND_MUX_SRC_DAC,

	SND_MUX_DST_MUTE,
	SND_MUX_DST_HP
};

typedef struct snd_param {
	int speaker_gpio;
	unsigned int speaker_en_level;

	/*dac clk*/
	struct clk *sddac_gclk;
	struct clk *sddac_clk;
	struct clk *sddachs_clk;

	/*adc clk*/
	struct clk *sdadc_gclk;
	struct clk *sdadc_clk;
	struct clk *sdadchs_clk;

	struct clk *pdm_gclk;
	struct clk *pdm_i2sm;
	struct clk *pdm_hsclk;

	void __iomem  *sysctrl_base;		//0x0800,0000
	//void __iomem	*dac_base;
	//void __iomem	*adc_base;
	void __iomem  *adda_cfg_base;		//0x2011,0000
	void __iomem  *pdm_base; //0x201E,0000
	void __iomem  *i2s_base;
} SND_PARAM;

typedef struct ak_snd_card {
	SND_CONTROLLER *controllers[SND_CARD_TYPE_MAX];
	SND_PARAM *param;
} AK_SND_CARD;

typedef struct snd_card {
	enum snd_card_type type;
	SND_CONTROLLER *controller;
} SND_CARD;

/*
* SND_CARD guidance
* snd_card_init/snd_card_deinit:info from dts to soundcard
* snd_card_prepare/snd_card_release:software resource apply and release
* snd_card_open/snd_card_close:hardware open and close
* snd_card_hwparams:param configure to hardware
* snd_card_set_format:format to hardware
* snd_card_dma_xfer:dma tranfer between host and device
* snd_card_dma_set_callback:callback setup
* snd_card_dma_status:wait dma finish
*/
#ifdef AKPCM_DEV_HOTPLUG
int snd_card_register(snd_dev dev, struct snd_controller *controller);
int snd_card_unregister(snd_dev dev);
#endif
int snd_card_init(SND_PARAM *param);
int snd_card_deinit(void);
int snd_card_prepare(snd_dev dev);
int snd_card_release(snd_dev dev);
int snd_card_open(snd_dev dev);
int snd_card_close(snd_dev dev);
int snd_card_resume(snd_dev dev);
int snd_card_hwparams(snd_dev dev, enum snd_hwparams_type type, int addr, unsigned int param);
int snd_card_dma_xfer(snd_dev dev, unsigned long ram_addr, int length);
int snd_card_dma_set_callback(snd_dev dev, void (*func)(unsigned long), unsigned long data);
int snd_card_dma_status(snd_dev dev);
unsigned long snd_card_set_format(snd_dev dev, SND_FORMAT *format);

/*
*  SDADC functions
*/
int adc_open(snd_dev dev);
int adc_close(snd_dev dev);
unsigned long adc_setformat(snd_dev dev, SND_FORMAT *format);

/*
*  SDDAC functions
*/
int dac_open(snd_dev dev);
int dac_close(snd_dev dev);
int analog_sethwparam(snd_dev dev, enum snd_hwparams_type type, int addr, unsigned int param);
unsigned long dac_setformat(snd_dev dev, SND_FORMAT *format);

#if defined(AK_AUDIO_PDM_SUPPORT)
/*
*  PDM functions
*/
int pdm_open(snd_dev dev);
int pdm_close(snd_dev dev);
unsigned long codec_pdm_set_format(snd_dev dev, SND_FORMAT *format);
int pdm_sethwparam(snd_dev dev, enum snd_hwparams_type type, int addr, unsigned int param);
#endif //AK_AUDIO_PDM_SUPPORT

#endif //__AKPCM_SOUNDCARD_H__
