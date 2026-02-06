/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_ai_demo.c
* Description: This is a simple example to show how the AI module working.
* Notes:
* History: V1.0.1
*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <getopt.h>

#include "ak_ai.h"
#include "ak_common.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include "ak_vqe.h"

#ifdef AK_RTOS
#include <kernel.h>
#endif

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

static const char save_def_parth[] = "/mnt/";

static FILE *fp = NULL;
static int sample_rate = 8000;
static int volume = 5;
static int save_time = 20000;        // set save time(ms)
static char *save_path= (char *)save_def_parth;    // set save path
static int channel_num = AUDIO_CHANNEL_MONO;
static int vqe_handle_id = -1;


/*     ***********************************
    ***********************************
    *
    use this demo
    must follow this:
    1. make sure the driver is insmode;
    2. mount the T card;
    3. the file path is exit;
    *
    ***********************************
    ***********************************
*/

static const char ac_option_hint[  ][ LEN_HINT ] = {
    "	打印帮助信息" ,
    "	采集时间" ,
    "	采样率[8000 12000 11025 16000 22050 24000 32000 44100 48000]" ,
    "	通道数[1 2]" ,
    "	音量[-90~20db]" ,
    "	文件保存路径" ,
};

static struct option option_long[ ] = {
    { "help"        , no_argument       , NULL , 'h' } , //"       打印帮助信息" ,
    { "second"      , required_argument , NULL , 't' } , //"[SEC]  采集时间" ,
    { "sample-rate" , required_argument , NULL , 's' } , //"[NUM]  采样率[8000 16000 32000 44100 48000]" ,
    { "channel-number" , required_argument , NULL , 'c' } , //"[NUM]  通道数[1 2]" ,
    { "volume"      , required_argument , NULL , 'v' } , //"[NUM]  音量[-90~20db]" ,
    { "path"        , required_argument , NULL , 'p' } , //"[PATH] 文件保存路径" ,
    {0, 0, 0, 0}
};

/*
 * check_dir: check whether the 'path' was exist.
 * path[IN]: pointer to the path which will be checking.
 * return: 1 on exist, 0 is not.
 */
static int check_dir(const char *path)
{
    struct stat buf = {0};

    if (NULL == path)
        return 0;

    stat(path, &buf);
    if (S_ISDIR(buf.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void print_playing_dot(void)
{
    static unsigned char first_flag = 1;
    static struct ak_timeval cur_time;
    static struct ak_timeval print_time;

    if (first_flag)
    {
        first_flag = 0;
        ak_get_ostime(&cur_time);
        ak_get_ostime(&print_time);
        ak_print_normal(MODULE_ID_APP, "\n.");
    }

    /* get time */
    ak_get_ostime(&cur_time);
    if (ak_diff_ms_time(&cur_time, &print_time) >= 10000)
    {
        ak_get_ostime(&print_time);
        ak_print_normal(MODULE_ID_APP, ".");
    }
}


/*
 * create_pcm_file_name: create pcm file by path+date.
 * path[IN]: pointer to the path which will be checking.
 * file_path[OUT]: pointer to the full path of pcm file.
 * return: void.
 */
static void create_pcm_file_name(const char *path, char *file_path,
                                        int sample_rate, int channel_num)
{
    if (0 == check_dir(path))
    {
        return;
    }

    char time_str[20] = {0};
    struct ak_date date;

    /* get the file path */
    ak_get_localdate(&date);
    ak_date_to_string(&date, time_str);
    sprintf(file_path, "%s%s_%d_%d.pcm", path, time_str, sample_rate, channel_num);
}

/*
 * open_pcm_file: open pcm file.
 * path[IN]: pointer to the path which will be checking.
 * fp[OUT]: pointer of opened pcm file.
 * return: 0 is success, -1 is error.
 */
static int open_pcm_file(int sample_rate, int channel_num, const char *path, FILE **fp)
{
    /* create the pcm file name */
    char file_path[255];
    create_pcm_file_name(path, file_path, sample_rate, channel_num);

    /* open file */
    *fp = fopen(file_path, "w+b");
    if (NULL == *fp)
    {
        ak_print_normal(MODULE_ID_APP, "open pcm file: %s error\n", file_path);
		return -1;
    }
    else
    {
        ak_print_normal(MODULE_ID_APP, "open pcm file: %s OK\n", file_path);
		return 0;
    }
}

/*
 * close_pcm_file: close pcm file.
 * fp[IN]: pointer of opened pcm file.
 * return: void.
 */
static void close_pcm_file(FILE *fp)
{
    if (NULL != fp)
    {
        fclose(fp);
        fp = NULL;
    }
}

/*
 * ai_capture_loop: loop to get and release pcm data, between get and release,
 *                  here we just save the frame to file, on your platform,
 *                  you can rewrite the save_function with your code.
 * ai_handle_id[IN]: pointer to ai handle, return by ak_ai_open()
 * path[IN]: save directory path, if NULL, will not save anymore.
 * save_time[IN]: captured time of pcm data, unit is second.
 */
static void ai_capture_loop(int ai_handle_id, const char *path, int save_time)
{
    unsigned long long start_ts = 0;// use to save capture start time
    unsigned long long end_ts = 0;    // the last frame time
    struct frame frame = {0};
    int ret = AK_FAILED;
    unsigned char *resample_data = NULL;
    unsigned int resample_len = 0;

    ak_print_normal(MODULE_ID_APP, "*** capture start ***\n");

    while (1)
    {
        /* get the pcm data frame */
        ret = ak_ai_get_frame(ai_handle_id, &frame, 0);
        if (ret)
        {
            if (ERROR_AI_NO_DATA == ret)
            {
                ak_sleep_ms(10);
                continue;
            }
            else
            {
                break;
            }
        }
        ret = ak_vqe_process(vqe_handle_id, frame.data, frame.len, &resample_data, &resample_len);
        if (ret)
            break;

        print_playing_dot();

        if (!frame.data || frame.len <= 0)
        {
            ak_sleep_ms(10);
            continue;
        }

        if (NULL != fp)
        {
            if(fwrite(resample_data, resample_len, 1, fp) < 0)
            {
                ak_ai_release_frame(ai_handle_id, &frame);
                ak_print_normal(MODULE_ID_APP, "write file error.\n");
                break;
            }
        }

        /* save the begin time */
        if (0 == start_ts)
        {
            start_ts = frame.ts;
            end_ts = frame.ts;
        }
        end_ts = frame.ts;

        ak_ai_release_frame(ai_handle_id, &frame);

        /* time is up */
        if ((end_ts - start_ts) >= save_time)
        {
            ak_print_normal(MODULE_ID_APP, "*** timp is up ***\n\n");
            break;
        }
    }
    ak_print_normal(MODULE_ID_APP, "*** capture finish ***\n\n");
}

/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint( char *pc_prog_name )
{
    int i;

    printf( "version: \r\n  %s \n\n", ak_vqe_get_version());
	printf( "usage: \r\n  %s  [options] <value>\n\n", pc_prog_name);
	printf( "options: \r\n");
	for(i = 0; i < sizeof(option_long) / sizeof(struct option)-1; i ++)
	{
		printf("  -%c,--%-16s\t%s \n" , option_long[ i ].val , option_long[ i ].name , ac_option_hint[ i ]);
	}
	printf( "\nexample: \r\n"
			"  %s -t 60 -s 8000 -c 1 -v 5 -p /mnt/\n", pc_prog_name);
	printf("\n\n");
    return 0;
}

static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short,
                        int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ )
    {
        c_option = p_option[ i ].val;
        switch( p_option[ i ].has_arg )
        {
        case no_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;

    if (argc < 5 && argc != 1)
    {
        help_hint( argv[ 0 ] );
        c_flag = 0;
        goto parse_option_end;
    }

#ifdef AK_RTOS
	optind = 0;
#endif
    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0)
    {
        switch(i_option)
        {
        case 'h' :                                                          //help
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;
        case 't' :                                                          //second
            save_time = atoi( optarg ) * 1000 ;
            break;
        case 's' :                                                          //sample-rate
            sample_rate = atoi( optarg );
            break;
        case 'c' :                                                          //sample-rate
            channel_num = atoi( optarg );
            break;
        case 'v' :                                                          //volume
            volume = atoi( optarg );
            printf("----sample_rate- %d-----\n", sample_rate);
            break;
        case 'p' :                                                          //path
            save_path = optarg;
            break;
        }
    }
parse_option_end:
    return c_flag;
}

#ifdef AK_RTOS
static void param_init(void)
{
	fp = NULL;
	sample_rate = 8000;
	volume = 5;
	save_time = 20000;		 // set save time(ms)
	save_path= (char *)save_def_parth;    // set save path
	channel_num = AUDIO_CHANNEL_MONO;
}
#endif

/**
 * Preconditions:
 * 1. T card is already mounted
 * 2. mic or linein must ready
 */
#ifdef AK_RTOS
static int ai_resample_sample(int argc, char *argv[])
#else
int main(int argc, char **argv)
#endif
{
    /* start the application */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

#ifdef AK_RTOS
	param_init();
#endif

    if( parse_option( argc, argv ) == 0 )
    {
		return 0;
    }
	ak_print_normal(MODULE_ID_APP, "*****************************************\n");
	ak_print_normal(MODULE_ID_APP, "** ai_resample_sample version: %s **\n", ak_vqe_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");

    if (0 > open_pcm_file(sample_rate, channel_num, save_path, &fp))
    {
        ak_print_error_ex(MODULE_ID_APP,"open_pcm_file create err!\n");
		return 0;
	}

    int ret = -1;

    struct ak_audio_in_param param;
    memset(&param, 0, sizeof(struct ak_audio_in_param));
	
	/*set sample rate*/ 
    param.pcm_data_attr.sample_rate = AK_AUDIO_SAMPLE_RATE_8000;

	/*sample bits only support 16 bit*/ 
    param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;    

	/*channel number*/
    param.pcm_data_attr.channel_num = channel_num;
    param.dev_id = DEV_ADC;

    int ai_handle_id = -1;
    if (ak_ai_open(&param, &ai_handle_id))
    {
        ak_print_normal(MODULE_ID_APP, "*** ak_ai_open failed. ***\n");
        goto exit;
    }

    /* set source, source include mic and linein */
    ret = ak_ai_set_source(ai_handle_id, AI_SOURCE_MIC);
    if (ret)
    {
        ak_print_normal(MODULE_ID_APP, "*** ak_ai_set_source failed. ***\n");
        ak_ai_close(ai_handle_id);
        goto exit;
    }

    ret = ak_ai_set_gain(ai_handle_id, 5);
    if (ret)
    {
        ak_print_normal(MODULE_ID_APP, "*** set ak_ai_set_volume failed. ***\n");
        ak_ai_close(ai_handle_id);
        goto exit;
    }

	/*set volume value*/
    ak_ai_set_volume(ai_handle_id, volume);

    ret = ak_ai_enable_nr(ai_handle_id, AUDIO_FUNC_ENABLE);
    if (ret)
    {
        ak_print_error(MODULE_ID_APP, "*** set ak_ai_set_nr_agc failed. ***\n");
    }

    struct ak_audio_agc_attr agc_attr = {24576, 4, 0, 80, 0, 1};

	/*set agc param*/
    ret = ak_ai_set_agc_attr(ai_handle_id, &agc_attr);
    if (ret)
    {
        ak_print_error(MODULE_ID_APP, "*** set ak_ai_set_nr_agc failed. ***\n");
    }

	/*enable vqe*/
    if (ak_vqe_open(AK_VQE_RESAMPLE, &vqe_handle_id))
    {
        ak_print_error(MODULE_ID_APP, "*** ak_audio_effect_open failed. ***\n");
        ak_ai_close(ai_handle_id);
        goto exit;
    }

    struct ak_audio_data_attr pcm_attr_in;
    struct ak_audio_data_attr pcm_attr_out;

    pcm_attr_in.sample_rate = AK_AUDIO_SAMPLE_RATE_8000;
    pcm_attr_in.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    pcm_attr_in.channel_num = channel_num;

    pcm_attr_out.sample_rate = sample_rate;
    pcm_attr_out.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    pcm_attr_out.channel_num = channel_num;

    if (ak_vqe_set_res_attr(vqe_handle_id, &pcm_attr_in, &pcm_attr_out))
    {
        ak_print_error(MODULE_ID_APP, "*** ak_audio_effect_set_resample_attr failed. ***\n");
        ak_ai_close(ai_handle_id);
        goto exit;
    }

    ret = ak_ai_start_capture(ai_handle_id);
    if (ret)
    {
        ak_print_error(MODULE_ID_APP, "*** ak_ai_start_capture failed. ***\n");
        ak_ai_close(ai_handle_id);
        goto exit;
    }

	/*loop capture audio source*/
    ai_capture_loop(ai_handle_id, save_path, save_time);

    ret = ak_ai_stop_capture(ai_handle_id);
    if (ret)
    {
        ak_print_error(MODULE_ID_APP, "*** ak_ai_stop_capture failed. ***\n");
        ak_ai_close(ai_handle_id);
        goto exit;
    }

    ret = ak_ai_close(ai_handle_id);
    if (ret)
    {
        ak_print_normal(MODULE_ID_APP, "*** ak_ai_close failed. ***\n");
    }

    ret = ak_vqe_close(vqe_handle_id);
    if (ret)
    {
        ak_print_normal(MODULE_ID_APP, "*** ak_audio_effect_close failed. ***\n");
    }

exit:
    /* close file handle */
    close_pcm_file(fp);
    ak_print_normal(MODULE_ID_APP, "******** exit ai demo ********\n");
    ak_sdk_exit();
    return ret;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(ai_resample_sample, ak_ai_resample_sample, ai resample sample);
#endif
