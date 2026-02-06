#ifndef __AKPCM_DEFS_H__
#define __AKPCM_DEFS_H__

#include "../../include/ak_pcm.h"
#include "soundcard.h"
#include "ak_pcm_uapi.h"

#define ak_pcm_debug(fmt, arg...) \
	pr_debug("[%llu]:[%s:%d]: "fmt" \n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)
#define ak_pcm_func(fmt, arg...) \
	pr_info("[%llu]:[%s]: "fmt" \n", ktime_to_ms(ktime_get_boottime()), __func__, ##arg)
#define ak_pcm_info(fmt, arg...) \
	pr_info("[%llu]:[%s:%d]: "fmt" \n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)
#define ak_pcm_warn(fmt, arg...) \
	pr_warn("[%llu]:[%s:%d] warn! "fmt" \n\n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)
#define ak_pcm_err(fmt, arg...) \
	pr_err("[%llu]:[%s:%d] error! "fmt" \n", ktime_to_ms(ktime_get_boottime()), __func__, __LINE__, ##arg)

#define ak_pcm_assert(exp) \
	((exp)? 1 : ak_pcm_info("assert"))

/* bit[0]-opened; bit[1]-prepared , bit[2]-stream; bit[3]-DMA; bit[4] -pausing */
enum run_flag_bit {
	STATUS_OPENED = 0x00,
	STATUS_PREPARED,
	STATUS_START_STREAM,
	STATUS_WORKING,
	STATUS_PAUSING,
};

struct akpcm {
	struct device *dev;

	struct list_head runtime_list;

	SND_PARAM param;
	struct device_node *np;
};

static inline int is_runtime_opened(struct akpcm_runtime *rt) {
	return test_bit(STATUS_OPENED, &(rt->runflag));
}

static inline int is_runtime_ready(struct akpcm_runtime *rt) {
	return test_bit(STATUS_PREPARED, &(rt->runflag));
}

static inline int is_runtime_stream(struct akpcm_runtime *rt) {
	return test_bit(STATUS_START_STREAM, &(rt->runflag));
}

static inline int is_runtime_working(struct akpcm_runtime *rt) {
	return test_bit(STATUS_WORKING, &(rt->runflag));
}

static inline int is_runtime_pausing(struct akpcm_runtime *rt) {
	return test_bit(STATUS_PAUSING, &(rt->runflag));
}

#endif//__AKPCM_DEFS_H__
