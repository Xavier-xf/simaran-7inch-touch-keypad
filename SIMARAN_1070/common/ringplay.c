#include "ringplay.h"
#include "user_common.h"
#include "audio_output.h"
#include <pthread.h>
#include "user_common.h"
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "ak_ao.h"
#include "ak_mem.h"

#include "mad/mad.h"
#include "audio_input.h"

#define RING_FILE_BASE_PATH "/app/app/rings"

/***** 送入声卡最大一帧数据 *****/
#define AUDIO_FRAME_MAX (1152 * 2)

#define MP3_READ_BUFFER_MAX (36 * 1024)
typedef struct
{
	const void *file;
	/***** 编码类型 0:pcm,1:mp3 *****/
	char codec_id;
	enum ak_audio_channel_type ch;
	enum ak_audio_sample_rate rate;
	/***** 源类型：0：file,1:ram *****/
	char type;
} ring_info;

typedef struct
{
	int index;
	int volume;
	bool loop;
	ringplay_callback start_func;
	ringplay_callback finish_func;
} ringplay_queue_info;

typedef struct
{
	long type;
	ringplay_queue_info msg;
} ringplay_queue;

typedef struct
{
	/***** 源类型：0：file,1:ram *****/
	char type;
	union
	{
		int fd;
		const unsigned char *ram_base;
	} handle;

	/***** 文件读取位置 *****/
	int read_pos;
	/***** 将数据存于buffer中 *****/
	unsigned char *start;
	/***** 文件大小 *****/
	unsigned long file_size;

	int read_length;
} mp3_mad;

/***
** 日期: 2022-04-27 13:33
** 作者: leo.liu
** 函数作用：内置铃声资源
** 返回参数说明：
***/
static rom_bin_info touch_sound_info;
//= rom_bin_info_get(ROM_UI_KEY_SOUND_PCM);
static const ring_info rings_info[] =
	{
		{&touch_sound_info, 0, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 1},
		{"1.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"2.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"3.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"4.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"5.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"6.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"7.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"8.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"10.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"11_e.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
		{"12_c.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0},
};

/***** 强制铃声禁止输出 *****/
static bool is_ringplay_force_mute = false;

/***** 按键音静音开关 *****/
static bool ringplay_touchsound_mute = false;

/***** 按键音音量 *****/
static int ringplay_touchsound_volume = 100;

/***** 铃声播放停止 *****/
static bool is_ringplay_play_stop = false;

/***** 铃声播放状态 *****/
static bool is_ringplay_playing = false;

/***** 铃声播放帧数 *****/
static unsigned int ringplay_mp3_frame_num = 0;

static pthread_mutex_t ringplay_mutex;

/***
** 日期: 2022-04-27 13:55
** 作者: leo.liu
** 函数作用：铃声消息队列头
** 返回参数说明：
***/
static int ringplay_queue_head = -1;

/***
** 日期: 2022-04-27 15:33
** 作者: leo.liu
** 函数作用：等待声卡数据播放完成
** 返回参数说明：
***/
void ringplay_play_buffer_wait(void)
{
	while (is_ringplay_play_stop == false)
	{
		if (audio_output_buffer_query() == 0)
		{
			break;
		}
		usleep(10 * 1000);
	}
}

/***
** 日期: 2022-04-27 15:20
** 作者: leo.liu
** 函数作用：播放来自文件中的音频
** 返回参数说明：
***/
static bool ringplay_decode_pcm_from_file(ringplay_queue_info *info)
{
	unsigned char data[AUDIO_FRAME_MAX] = {0};
	char file_path[128] = {0};
	sprintf(file_path, "%s/%s", RING_FILE_BASE_PATH, (char *)rings_info[info->index].file);
	int fd = open(file_path, O_RDONLY);
	if (fd < 0)
	{
		printf("open %s failed \n", file_path);
		return false;
	}

	int len = 0;
	while ((len = read(fd, data, AUDIO_FRAME_MAX)) > 0)
	{
		if (is_ringplay_play_stop == true)
		{
			break;
		}
		audio_output_write(data, len);
	}
	close(fd);
	return true;
}

/***
** 日期: 2022-04-27 15:21
** 作者: leo.liu
** 函数作用：播放来自内存中的数据
** 返回参数说明：
***/
static bool ringplay_decode_pcm_from_ram(ringplay_queue_info *info)
{
	extern const unsigned char *get_rom_bin_base(void);

	rom_bin_info *rom_info = (rom_bin_info *)rings_info[info->index].file;
	const unsigned char *addr = get_rom_bin_base() + rom_info->offset;
	int play_count = 0;
	int play_size = rom_info->size;
	while (1)
	{
		if (is_ringplay_play_stop == true)
		{
			break;
		}

		if ((play_count + AUDIO_FRAME_MAX) < play_size)
		{
			audio_output_write(&addr[play_count], AUDIO_FRAME_MAX);
			play_count += AUDIO_FRAME_MAX;
		}
		else
		{
			audio_output_write(&addr[play_count], play_size - play_count);
			break;
		}
		usleep(10 * 1000);
	}
	return true;
}

/***
** 日期: 2022-04-27 14:46
** 作者: leo.liu
** 函数作用：解码PCM文件
** 返回参数说明：
***/
static bool ringplay_decodec_pcm(ringplay_queue_info *info)
{
	bool reslut = false;
	/***** 来自文件 *****/
	if (rings_info[info->index].type == 0)
	{
		printf("play :%s \n", (char *)rings_info[info->index].file);
		reslut = ringplay_decode_pcm_from_file(info);
	} /***** 来自内存中 *****/
	else if (rings_info[info->index].type == 1)
	{
		reslut = ringplay_decode_pcm_from_ram(info);
	}
	return reslut;
}

/***
** 日期: 2022-04-29 10:52
** 作者: leo.liu
** 函数作用：mp3解码错误接口
** 返回参数说明：
***/
static enum mad_flow mp3_error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{

	//  mp3_mad *buffer = data;

#if 0
  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
#endif
	return MAD_FLOW_CONTINUE;
}
/***
** 日期: 2022-04-29 10:51
** 作者: leo.liu
** 函数作用:mp3文件输入接口
** 返回参数说明：
***/
static enum mad_flow mp3_input(void *data, struct mad_stream *stream)
{
	if (is_ringplay_play_stop == true)
	{
		return MAD_FLOW_STOP;
	}
	mp3_mad *mp3_info = (mp3_mad *)data;
	if (stream->next_frame)
	{
		mp3_info->read_length = &mp3_info->start[mp3_info->read_length] - stream->next_frame;
		memmove(mp3_info->start, stream->next_frame, mp3_info->read_length);
	}
	int read_size = 0;
	if (mp3_info->type == 0)
	{
		read_size = read(mp3_info->handle.fd, &mp3_info->start[mp3_info->read_length], MP3_READ_BUFFER_MAX - mp3_info->read_length);
		if (read_size <= 0)
		{
			return MAD_FLOW_STOP;
		}
		mp3_info->read_length += read_size;
	}
	else if (mp3_info->type == 1)
	{
		if (mp3_info->read_pos > mp3_info->file_size)
		{
			return MAD_FLOW_STOP;
		}
		read_size = ((mp3_info->read_pos + MP3_READ_BUFFER_MAX) > mp3_info->file_size) ? (mp3_info->file_size - mp3_info->read_pos) : MP3_READ_BUFFER_MAX;
		memcpy(&mp3_info->start[mp3_info->read_length], mp3_info->handle.ram_base + mp3_info->read_pos, read_size);
		mp3_info->read_pos += read_size;
		mp3_info->read_length += read_size;
	}
	mad_stream_buffer(stream, mp3_info->start, mp3_info->read_length);
	return MAD_FLOW_CONTINUE;
}

static inline signed int mp3_scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/***
** 日期: 2022-04-29 10:52
** 作者: leo.liu
** 函数作用：mp3文件输出接口
** 返回参数说明：
***/
static enum mad_flow mp3_output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	if (is_ringplay_play_stop == true)
	{
		return MAD_FLOW_STOP;
	}

	pcm->channels = 1;
	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	unsigned char sample_buffer[AUDIO_FRAME_MAX] = {0};
	int sample_read_size = 0;
	while (nsamples--)
	{
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */
		if (is_ringplay_play_stop == true)
		{
			return MAD_FLOW_STOP;
		}

		sample = mp3_scale(*left_ch++);
		sample_buffer[sample_read_size++] = (sample >> 0) & 0xff;
		sample_buffer[sample_read_size++] = (sample >> 8) & 0xff;

		if (nchannels == 2)
		{
			sample = mp3_scale(*right_ch++);
			sample_buffer[sample_read_size++] = (sample >> 0) & 0xff;
			sample_buffer[sample_read_size++] = (sample >> 8) & 0xff;
		}

		/* output sample(s) in 16-bit signed little-endian PCM */
		// if (sample_read_size >= AUDIO_FRAME_MAX)
		// {
		// audio_output_write((unsigned char *)sample_buffer, sample_read_size);
		// 	sample_read_size = 0;
		// }
	}
	if (sample_read_size)
	{
		static unsigned char pcm_cache_data[4096] = {0};
		static int pcm_cache_size = 0;
		if (ringplay_mp3_frame_num > 5)
		{
			if ((pcm_cache_size + sample_read_size) > 4096)
			{
				audio_output_write((unsigned char *)pcm_cache_data, pcm_cache_size);
				pcm_cache_size = 0;
			}
			memcpy(&pcm_cache_data[pcm_cache_size], sample_buffer, sample_read_size);
			pcm_cache_size += sample_read_size;
		}
		else if (ringplay_mp3_frame_num == 0)
		{
			pcm_cache_size = 0;
		}
		ringplay_mp3_frame_num++;
	}
	return MAD_FLOW_CONTINUE;
}

/***
** 日期: 2022-04-27 14:57
** 作者: leo.liu
** 函数作用：解码mp3文件
** 返回参数说明：
***/
static bool ringplay_decodec_mp3(ringplay_queue_info *info)
{
	bool reslut = false;
	ringplay_mp3_frame_num = 0;
	mp3_mad mp3_info;
	struct mad_decoder decoder;
	mp3_info.read_pos = 0;
	mp3_info.read_length = 0;
	if (rings_info[info->index].type == 0)
	{
		printf("play :%s \n", (char *)rings_info[info->index].file);

		char file_path[128] = {0};
		sprintf(file_path, "%s/%s", RING_FILE_BASE_PATH, (char *)rings_info[info->index].file);
		mp3_info.handle.fd = open(file_path, O_RDONLY);
		if (mp3_info.handle.fd < 0)
		{
			printf("open %s failed \n", file_path);
			return false;
		}

		mp3_info.file_size = lseek(mp3_info.handle.fd, 0, SEEK_END);
		lseek(mp3_info.handle.fd, 0, SEEK_SET);
	}
	else if (rings_info[info->index].type == 1)
	{
		extern const unsigned char *get_rom_bin_base(void);
		rom_bin_info *rom_info = (rom_bin_info *)rings_info[info->index].file;
		mp3_info.handle.ram_base = get_rom_bin_base() + rom_info->offset;
		mp3_info.file_size = rom_info->size;
	}
	mp3_info.start = (unsigned char *)ak_mem_alloc(MODULE_ID_AO, MP3_READ_BUFFER_MAX);
	mp3_info.type = rings_info[info->index].type;
	mad_decoder_init(&decoder, &mp3_info, mp3_input, 0, 0, mp3_output, mp3_error, 0);
	mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	mad_decoder_finish(&decoder);
	if (rings_info[info->index].type == 0)
	{

		close(mp3_info.handle.fd);
	}
	ak_mem_free(mp3_info.start);
	return reslut;
}

/***
** 日期: 2022-04-27 14:41
** 作者: leo.liu
** 函数作用：铃声文件解码
** 返回参数说明：
***/
static bool ringplay_decodec(ringplay_queue_info *info)
{
	bool reslut = true;
	if (rings_info[info->index].codec_id == 0)
	{
		reslut = ringplay_decodec_pcm(info);
	}
	else if (rings_info[info->index].codec_id == 1)
	{
		reslut = ringplay_decodec_mp3(info);
		// audio_output_device_restart();
	}
	/***** 等待声卡数据播放完成 *****/
	ringplay_play_buffer_wait();
	return reslut;
}
/***** 开启功放 *****/
extern void power_amplifier_enable(bool);
/***
** 日期: 2022-04-27 14:19
** 作者: leo.liu
** 函数作用：铃声播放任务，初始化后一直存在
** 返回参数说明：
***/
static void *ringplay_task(void *arg)
{
	/***** 一个临时数据，一个最后播放需要的数据 *****/
	ringplay_queue play_tmp_data, play_data;
	memset(&play_tmp_data, 0, sizeof(ringplay_queue));
	memset(&play_data, 0, sizeof(ringplay_queue));
	/***** 是否需要播放 *****/
	bool is_ringplay_valid = false;

	/***** 空闲计数器 *****/
	bool is_ringplay_idle_start = false;
	unsigned long ringplay_idle_count = 0;

	while (msgrcv(ringplay_queue_head, (void *)&play_tmp_data, sizeof(ringplay_queue_info), 0, IPC_NOWAIT) > 0)
		;

	printf("***** ringplay task create success ! *****\n");
	while (1)
	{
		/***** 从队列中获取最后一个需要播放的音频消息 *****/
		if (msgrcv(ringplay_queue_head, (void *)&play_tmp_data, sizeof(ringplay_queue_info), sizeof(ringplay_queue_info), IPC_NOWAIT) > 0)
		{
			play_data = play_tmp_data;
			if (play_data.msg.index > (sizeof(rings_info) / sizeof(ring_info)))
			{
				continue;
			}
			ringplay_idle_count = 0;

			is_ringplay_valid = true;
			/***** 标记铃声播放开始 *****/

			is_ringplay_playing = true;

		} /***** 开始解码最后一个音频消息 *****/
		else if (is_ringplay_valid == true)
		{
			is_ringplay_valid = false;
			pthread_mutex_lock(&ringplay_mutex);
			is_ringplay_play_stop = false;
			pthread_mutex_unlock(&ringplay_mutex);
			/***** 打开音频设备 *****/
			audio_output_open(rings_info[play_data.msg.index].ch, rings_info[play_data.msg.index].rate);
			audido_output_volume_set(play_data.msg.volume);
			/***** 执行用户回调函数 *****/
			// power_amplifier_enable(true);
			if (play_data.msg.start_func != NULL)
			{
				play_data.msg.start_func(play_data.msg.index);
			}
		loop:

			// audio_input_aec_enable(false);
			/***** 开始解码 *****/
			ringplay_decodec(&play_data.msg);
			/***** 循环播放，中途被打断，取消循环播放 *****/
			if ((play_data.msg.loop == true) && (is_ringplay_play_stop == false))
			{
				goto loop;
			}

			/***** 执行用户回调函数 *****/
			if (play_data.msg.finish_func != NULL)
			{
				play_data.msg.finish_func(play_data.msg.index);
			}
		} /***** 播放完成，开始空闲计时 *****/
		else if (is_ringplay_playing == true)
		{
			//		audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_8000);
			//	audio_input_aec_enable(true);
			is_ringplay_playing = false;
			/***** 开启空闲计数 *****/
			is_ringplay_idle_start = true;
		} /***** 空闲超过10s关闭声卡 *****/
		else if (((is_ringplay_idle_start == true) && (ringplay_idle_count++) == 100))
		{
			is_ringplay_idle_start = false;
			ringplay_idle_count = 0;
		}
		usleep(10 * 1000);
	}
	return NULL;
}

/***
** 日期: 2022-04-27 12:00
** 作者: leo.liu
** 函数作用：铃声播放任务初始化
** 返回参数说明：
***/
bool ringplay_init(void)
{
	static bool is_first = false;
	if (is_first == true)
	{
		return false;
	}
	is_first = true;

	ringplay_queue_head = msg_queue_create();

	pthread_mutex_init(&ringplay_mutex, NULL);
	pthread_t ringplay_pthread_t;
	pthread_create(&ringplay_pthread_t, user_pthread_atter_get(), ringplay_task, NULL);
	return true;
}

/***
** 日期: 2022-04-27 13:31
** 作者: leo.liu
** 函数作用：播放基础函数
** 返回参数说明：
***/

static bool ringplay_playbase(int index, int volume, ringplay_callback start, ringplay_callback finish, bool loop)
{
	if (is_ringplay_force_mute == true)
	{
		return false;
	}
	int reslut = 0;
	ringplay_queue info;
	info.type = sizeof(ringplay_queue_info);
	info.msg.index = index;
	info.msg.volume = volume;
	info.msg.start_func = start;
	info.msg.finish_func = finish;
	info.msg.loop = loop;

	pthread_mutex_lock(&ringplay_mutex);
	is_ringplay_play_stop = true;
	pthread_mutex_unlock(&ringplay_mutex);
	if ((reslut = msgsnd(ringplay_queue_head, &info, info.type, 0)) < 0)
	{
		printf("send ringplay queue fail (%d) send size:%d --- %s\n", reslut, sizeof(info), strerror(errno));
	}
	return true;
}

/***
** 日期: 2022-04-27 13:32
** 作者: leo.liu
** 函数作用：播放按键音
** 返回参数说明：
***/
bool touch_sound_play(ringplay_callback start, ringplay_callback finish)
{
	if (ringplay_touchsound_mute == true)
	{
		return false;
	}
	ringplay_playbase(0, ringplay_touchsound_volume, start, finish, false);
	return true;
}

/***
**   日期:2022-06-02 14:08:49
**   作者: leo.liu
**   函数作用：设置按键音信息
**   参数说明:info,必须是8k的pcm,为rom.bin中的文件
***/
bool touch_sound_rom_info_register(rom_bin_info *info)
{
	if (info == NULL)
	{
		return false;
	}
	memcpy(&touch_sound_info, info, sizeof(rom_bin_info));
	return true;
}
/***
** 日期: 2022-04-27 13:51
** 作者: leo.liu
** 函数作用：获取强制铃声静止输出设置
** 返回参数说明：
***/
bool ringplay_force_mute_get(void)
{
	return is_ringplay_force_mute;
}
/***
** 日期: 2022-04-27 13:51
** 作者: leo.liu
** 函数作用：强制铃声静止输出设置
** 返回参数说明：
***/
void ringplay_force_mute_set(bool en)
{
	is_ringplay_force_mute = en;
}
/***
** 日期: 2022-04-27 14:00
** 作者: leo.liu
** 函数作用：设置是否播放按键音
** 返回参数说明：
***/
void ringplay_touchsound_mute_set(bool en)
{
	ringplay_touchsound_mute = en;
}

/***
** 日期: 2022-04-27 14:02
** 作者: leo.liu
** 函数作用：设置按键音的音量大小
** 返回参数说明：
***/
void ringplay_touchsound_volume_set(int volume)
{
	ringplay_touchsound_volume = volume;
}

/***
** 日期: 2022-04-27 14:12
** 作者: leo.liu
** 函数作用：等待铃声结束超时500ms
** 返回参数说明：
***/
static bool ringplay_timeout_wait(void)
{
	bool reslut = true;
	int timeout = 1000;
	while (is_ringplay_playing == true)
	{
		is_ringplay_play_stop = true;

		timeout--;
		if (timeout == 0 || audio_output_buffer_try_timeout(500) == 0)
		{
			printf("closing ringplay  timeout :%d\n", timeout);
			reslut = false;
			break;
		}
		usleep(1000);
	}
	printf("####ringplay wait finsih#### # \n");

	return reslut;
}

/***
** 日期: 2022-04-27 14:11
** 作者: leo.liu
** 函数作用：停止播放铃声
** 返回参数说明：
***/
void ringplay_play_stop(void)
{
	ringplay_timeout_wait();
}

/***
** 日期: 2022-04-27 14:32
** 作者: leo.liu
** 函数作用：播放铃声
** 返回参数说明：
***/
void ringplay_play_form_index(int index, int volume, ringplay_callback start, ringplay_callback finish, bool loop)
{
	ringplay_playbase(index, volume, start, finish, loop);
}
/***
**   日期:2022-05-28 14:44:09
**   作者: leo.liu
**   函数作用：判断铃声是否播放
**   参数说明:
***/
bool ringplay_ing_check(void)
{
	return is_ringplay_playing;
}