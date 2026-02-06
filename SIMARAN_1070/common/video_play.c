#include "video_play.h"
#include "avilib.h"
#include <pthread.h>
#include "unistd.h"
#include "ak_mem.h"
#include "video_decode.h"
#include "video_input.h"
#include "audio_output.h"
#include "user_time.h"
#include "lvgl/lv_obj.h"
#include "lvgl/lv_disp.h"
#include "user_common.h"

decode_finish_callback jpg_decode_read_frame_func_register(decode_finish_callback read_frame);

static decode_finish_callback old_decode_finish_func = NULL;

static const rom_bin_info *p_scr_act_img = NULL;

#define AUDIO_FRAME_SIZE 512
/*****  播放状态 *****/
static VIDEO_PLAY_STATUS video_play_status = VIDEO_PLAY_STATE_IDLE;
static pthread_mutex_t video_play_mutex;
/*****  播放设备句柄 *****/
static avi_t *avi_handle_id = NULL;
/*****  播放文件 *****/
static char video_play_file[128] = {0};
/*****  一帧视频时长 *****/
static int video_one_frame_duration = 0;
/*****  一帧音频时长 *****/
static int audio_one_frame_duration = 0;
/*****  播放总帧数 *****/
static int video_frame_total = 0;
/*****  视频当前播放帧 *****/
static int video_frame_video_index = 0;
/*****  音频当前帧 *****/
static int video_frame_audio_index = 0;
/*****  标志已经播放完成 *****/
static bool video_play_eof_flg = false;
/*****  标记第一帧解码显示完成 *****/
static char video_first_play = 0x00;
/*****  是否有音频数据 *****/
static bool video_play_has_audio = false;
/*****  外部参考始终 *****/
static unsigned long long video_play_clock_base = 0;
/***** 开启功放 *****/
extern void power_amplifier_enable(bool);
extern void ring_volume_set(int vol);
/***
**   日期:2022-05-24 15:17:02
**   作者: leo.liu
**   函数作用：关闭播放设备
**   参数说明:
***/
static void video_play_device_close(void)
{
	AVI_close(avi_handle_id);
	avi_handle_id = NULL;
}
/***
**   日期:2022-05-24 15:17:10
**   作者: leo.liu
**   函数作用：打开播放设备
**   参数说明:
***/
static void video_play_device_open(void)
{
	avi_handle_id = AVI_open_input_file(video_play_file, 1);
	video_frame_total = AVI_video_frames(avi_handle_id);
	video_frame_video_index = 0;
	video_frame_audio_index = 0;
	video_play_eof_flg = false;
	video_one_frame_duration = (int)1000 / (AVI_frame_rate(avi_handle_id) + 0.0001);
	audio_one_frame_duration = (int)AUDIO_FRAME_SIZE * 8 * 1000.0 / (8000 * 16 * 1);
	jpg_decode_buffer_clear();
	while (audio_output_buffer_query() > 0)
	{
		usleep(1000);
	}

	int audio_channels = AVI_audio_channels(avi_handle_id);
	int audio_rate = AVI_audio_rate(avi_handle_id);
	int audio_byte = AVI_audio_bits(avi_handle_id);
	video_play_has_audio = false;
	if ((audio_channels == 1) && (audio_rate == 8000) && (audio_byte == 16))
	{
		video_play_has_audio = true;
		audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_8000);
		audido_output_volume_set(60);
		/***** 执行用户回调函数 *****/
		power_amplifier_enable(true);
	}
}

/***
**   日期:2022-05-25 10:54:13
**   作者: leo.liu
**   函数作用：发送一帧视频帧
**   参数说明:
***/
static void video_play_send_video_frame(void)
{
	AVI_set_video_position(avi_handle_id, video_frame_video_index);
	int size = AVI_frame_size(avi_handle_id, video_frame_video_index);
	unsigned char *data = ak_mem_alloc(MODULE_ID_VDEC, size);
	int keyframe;
	size = AVI_read_frame(avi_handle_id, (char *)data, &keyframe);
	jpg_decode_stream_write(data, size);
	ak_mem_free(data);
}

static void *video_play_task(void *arg)
{
	printf("***** video play create sccess ! *****\n");
	unsigned char audio_data[AUDIO_FRAME_SIZE] = {0};
	//	unsigned long long video_play_clock_base = 0;
	while (1)
	{
		pthread_mutex_lock(&video_play_mutex);
		if ((video_play_status == VIDEO_PLAY_STATE_IDLE) && (avi_handle_id != NULL))
		{
			video_play_device_close();
		}
		else if ((video_play_status != VIDEO_PLAY_STATE_IDLE) && (avi_handle_id == NULL))
		{
			video_first_play = 0x00;
			video_play_device_open();
		}

		if ((video_play_status == VIDEO_PLAY_STATE_PLAY) && (avi_handle_id != NULL))
		{
			if (video_frame_video_index < video_frame_total)
			{
				if (video_first_play == 0x00)
				{
					video_play_send_video_frame();
					usleep(10 * 1000);
				}
				else if (video_first_play == 0x01)
				{
					video_first_play = 0x02;
					// video_play_status = VIDEO_PLAY_STATE_PAUSE;
					// video_frame_video_index = 0;
					// video_frame_audio_index = 0;
					// AVI_set_video_position(avi_handle_id, video_frame_video_index);
					// AVI_set_audio_position(avi_handle_id, video_frame_audio_index);
					video_play_clock_base = user_timestamp_get();
				}
				else
				{
					unsigned long long cur_pts = user_timestamp_get();
					if (video_play_has_audio == true)
					{
						// unsigned long long video_play_clock_base = cur_pts - video_frame_audio_index * audio_one_frame_duration;
						unsigned long long v_pts = video_play_clock_base + (video_frame_video_index + 1) * video_one_frame_duration;
						// printf("play video :%llu\n",cur_pts >v_pts?( cur_pts - v_pts):(v_pts- v_pts));
						if (((cur_pts >= v_pts) /* && ((cur_pts - v_pts) < 10)*/) || ((cur_pts < v_pts) && ((v_pts - cur_pts) < 10)))
						{
							video_play_send_video_frame();
							video_frame_video_index++;
							// printf("decode video frame:%d \n", video_frame_video_index);
							// printf("play video :%llu \n",cur_pts >v_pts?( cur_pts - v_pts):(v_pts- v_pts));
						}
						else if ((cur_pts > v_pts) && ((cur_pts - v_pts) > 100))
						{
							video_frame_video_index++;
							AVI_set_video_position(avi_handle_id, video_frame_video_index);
							// printf("video frame skip index:%d ---%llu\n", video_frame_video_index, cur_pts - v_pts);
						}
#if 0
						unsigned long long a_pts = video_play_clock_base + (video_frame_audio_index + 1) * audio_one_frame_duration;
						if (((cur_pts >= a_pts) /* && ((cur_pts - v_pts) < 10)*/) || ((cur_pts < a_pts) && ((a_pts - cur_pts) < 10)))
						{
							int size = (int)AVI_read_audio(avi_handle_id, (char *)audio_data, AUDIO_FRAME_SIZE);
							if (size > 0)
							{
								audio_output_write(audio_data, size);			
							}
							video_frame_audio_index++;
							printf("decode audio frame:%d \n", video_frame_audio_index);
							// printf("play video :%llu \n",cur_pts >v_pts?( cur_pts - v_pts):(v_pts- v_pts));
						}
						else if ((cur_pts > a_pts) && ((cur_pts - a_pts) > 100))
						{
							video_frame_audio_index++;
							AVI_set_video_position(avi_handle_id, video_frame_audio_index * AUDIO_FRAME_SIZE);
							printf("audio frame skip index:%d ---%llu\n", video_frame_audio_index, cur_pts - a_pts);
						}

#else
						int buffer_query = audio_output_buffer_query();
						// printf("======buffer_query %d [%llu]\n", buffer_query, user_timestamp_get());
						if (buffer_query < 1024 * 4)
						{
							int size = (int)AVI_read_audio(avi_handle_id, (char *)audio_data, AUDIO_FRAME_SIZE);
							// printf("=================size:%d", size);
							if (size > 0)
							{
								audio_output_write(audio_data, size);
							}
							video_frame_audio_index++;
							// printf("decode audio frame:%d \n", video_frame_audio_index);
						}
#endif
					}
					else
					{
						static unsigned long long video_pre_pts = 0;
						if (video_pre_pts == 0)
						{
							video_pre_pts = user_timestamp_get();
						}
						unsigned long long v_next_pts = video_pre_pts + video_one_frame_duration;
						if (cur_pts >= v_next_pts)
						{
							video_play_send_video_frame();
							video_frame_video_index++;
							video_pre_pts = user_timestamp_get();
							// printf("decode video frame:%d \n", video_frame_video_index);
						}
					}
				}
			}
			else if (video_play_eof_flg == false)
			{
				video_play_eof_flg = true;
				video_play_status = VIDEO_PLAY_STATE_PAUSE;
				video_frame_video_index = 0;
				video_frame_audio_index = 0;
				AVI_set_video_position(avi_handle_id, video_frame_video_index);
				AVI_set_audio_position(avi_handle_id, video_frame_audio_index);
				jpg_decode_buffer_clear();
			}
		}
		else if ((video_play_status == VIDEO_PLAY_STATE_PAUSE) && (avi_handle_id != NULL))
		{
			video_play_send_video_frame();
			screen_force_refresh();
			usleep(40 * 1000);
		}
		pthread_mutex_unlock(&video_play_mutex);
		usleep(1 * 1000);
	}
	return NULL;
}

/***
**   日期:2022-05-24 14:27:18
**   作者: leo.liu
**   函数作用：视频播放初始化
**   参数说明:
***/
bool video_play_init(void)
{

	pthread_mutex_init(&video_play_mutex, NULL);
	pthread_t thread_t;
	pthread_create(&thread_t, user_pthread_atter_get(), video_play_task, NULL);
	return true;
}

static void video_play_decode_read_frame_func(struct ak_vdec_frame *frame)
{
	video_main_display_lock();
	video_input_resident_buffer_write(frame->frame_obj.data.data, frame->frame_obj.data.pitch_width, frame->frame_obj.data.pitch_height, 0, 0, frame->width, frame->height, GP_FORMAT_YUV420SP);
	video_main_display_unlock();
	if (video_first_play == 0x02)
	{
		screen_force_refresh();
	}
	else if (video_first_play == 0x00)
	{
		video_first_play = 0x01;
	}
}

/***
**   日期:2022-05-24 14:27:45
**   作者: leo.liu
**   函数作用：视频开始播放
**   参数说明:
***/
bool video_play_start(const char *file)
{
	printf("===========>>>[%s]<<<===========\n", __func__);
	pthread_mutex_lock(&video_play_mutex);
	power_amplifier_enable(true);
	ring_volume_set(4);
	if (video_play_status != VIDEO_PLAY_STATE_IDLE)
	{
		pthread_mutex_unlock(&video_play_mutex);
		return false;
	}
	memset(video_play_file, 0, sizeof(video_play_file));
	strcpy(video_play_file, file);
	video_play_status = VIDEO_PLAY_STATE_PLAY;
	if (old_decode_finish_func == NULL)
	{
		old_decode_finish_func = jpg_decode_read_frame_func_register(video_play_decode_read_frame_func);
	}
	if (p_scr_act_img == NULL)
	{
		p_scr_act_img = lv_obj_get_style_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN);
	}
	lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);
	lv_video_mode_enable(true);
	video_display_preview_enable(true);
	pthread_mutex_unlock(&video_play_mutex);
	return true;
}
/***
**   日期:2022-05-24 14:28:35
**   作者: leo.liu
**   函数作用：视频停止播放
**   参数说明:
***/
bool video_play_stop(void)
{
	printf("===========>>>[%s]<<<===========\n", __func__);
	pthread_mutex_lock(&video_play_mutex);
	power_amplifier_enable(false);
	if (video_play_status == VIDEO_PLAY_STATE_IDLE)
	{
		pthread_mutex_unlock(&video_play_mutex);
		return false;
	}
	video_play_status = VIDEO_PLAY_STATE_IDLE;
	if (old_decode_finish_func != NULL)
	{
		jpg_decode_read_frame_func_register(old_decode_finish_func);
		old_decode_finish_func = NULL;
	}
	if (p_scr_act_img != NULL)
	{
		lv_obj_set_style_local_pattern_image(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, p_scr_act_img);
		p_scr_act_img = NULL;
	}
	lv_video_mode_enable(false);
	video_display_preview_enable(false);

	pthread_mutex_unlock(&video_play_mutex);
	return true;
}
/***
**   日期:2022-05-24 14:28:51
**   作者: leo.liu
**   函数作用：暂停播放
**   参数说明:
***/
bool video_play_pause(void)
{
	printf("===========>>>[%s]<<<===========\n", __func__);
	pthread_mutex_lock(&video_play_mutex);
	if (video_play_status == VIDEO_PLAY_STATE_IDLE)
	{
		pthread_mutex_unlock(&video_play_mutex);
		return false;
	}
	if (video_play_status == VIDEO_PLAY_STATE_PLAY)
	{
		video_play_status = VIDEO_PLAY_STATE_PAUSE;
	}
	else if (video_play_status == VIDEO_PLAY_STATE_PAUSE)
	{
		video_play_status = VIDEO_PLAY_STATE_PLAY;
		if (video_play_eof_flg == true)
		{
			video_play_eof_flg = false;
		}
		video_play_clock_base = user_timestamp_get() - video_frame_video_index * video_one_frame_duration;
	}
	pthread_mutex_unlock(&video_play_mutex);
	return true;
}
/***
**   日期:2022-05-24 14:30:09
**   作者: leo.liu
**   函数作用：获取视频播放状态
**   参数说明:
***/
VIDEO_PLAY_STATUS video_play_status_get(void)
{
	pthread_mutex_lock(&video_play_mutex);
	VIDEO_PLAY_STATUS state = video_play_status;
	pthread_mutex_unlock(&video_play_mutex);
	return state;
}
/***
**   日期:2022-05-24 14:32:04
**   作者: leo.liu
**   函数作用：获取播放时长
**   参数说明:
***/
bool video_play_duration_get(int *cur, int *total)
{
	pthread_mutex_lock(&video_play_mutex);
	if (video_play_status == VIDEO_PLAY_STATE_IDLE)
	{
		pthread_mutex_unlock(&video_play_mutex);
		return false;
	}
	*total = video_frame_total * video_one_frame_duration;
	if (video_play_eof_flg == true)
	{
		*cur = *total;
	}
	else
	{
		*cur = video_frame_video_index * video_one_frame_duration;
	}
	pthread_mutex_unlock(&video_play_mutex);
	return true;
}
/**
 * 非播放状态下获取视频总时长（不播放，仅读元信息）
 */
int video_get_duration_without_play(const char *file)
{
	if (file == NULL || strlen(file) == 0)
		return 0; // 文件路径无效

	pthread_mutex_lock(&video_play_mutex); // 避免与播放线程冲突

	avi_t *temp_avi_handle = NULL;
	int total_duration_ms = 0; // 总时长（毫秒）

	do
	{
		// 1. 临时打开视频文件（仅读元信息，不播放）
		temp_avi_handle = AVI_open_input_file(file, 1); // 第二个参数1：只读模式
		if (temp_avi_handle == NULL)
		{
			printf("临时打开视频文件失败：%s\n", file);
			break; // 打开失败，退出循环
		}

		// 2. 获取总帧数和帧率，计算总时长
		int total_frames = AVI_video_frames(temp_avi_handle);					 // 总帧数
		int frame_rate = (int)1000 / (AVI_frame_rate(temp_avi_handle) + 0.0001); // 帧率（如25帧/秒）
		if (total_frames <= 0 || frame_rate <= 0.0001)
		{
			printf("获取视频参数失败：总帧数=%d，帧率=%d\n", total_frames, frame_rate);
			break;
		}

		// 3. 计算总时长
		total_duration_ms = total_frames * frame_rate;

	} while (0);

	// 4. 无论成功失败，都要关闭临时打开的文件，避免内存泄漏
	if (temp_avi_handle != NULL)
	{
		AVI_close(temp_avi_handle);
		temp_avi_handle = NULL;
	}

	pthread_mutex_unlock(&video_play_mutex); // 释放互斥锁

	return total_duration_ms; // 返回总时长（毫秒）
}

/***
**   日期:2022-06-11 11:41:27
**   作者: leo.liu
**   函数作用：播放显示位置设置
**   参数说明:
***/
bool video_play_display_pos(int x, int y, int w, int h)
{
	return video_input_display_pos(x, y, w, h);
}
