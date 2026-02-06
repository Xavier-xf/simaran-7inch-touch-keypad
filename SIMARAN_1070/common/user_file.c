#include "user_file.h"
#include "ak_mem.h"
#include <dirent.h>
#include <pthread.h>
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include "fcntl.h"
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "user_common.h"
#include "user_time.h"
#include "lv_msg_event.h"
#include "user_data.h"

/***** sd照片文件最大数目 *****/
#define SD_PHOTO_NUM_MAX 1024

/***** sd视频文件最大数目 *****/
#define SD_VIDEO_NUM_MAX 1024 // 256

/***** sd call记录文件最大数目 *****/
#define SD_CALL_NUM_MAX 50

/***** sd媒体文件最大数目 *****/
#define SD_MEDIA_NUM_MAX (SD_PHOTO_NUM_MAX + SD_VIDEO_NUM_MAX) // 1000

/***** flash的照片最大数目 *****/
#define FLASH_PHOTO_MAX 200

#define FLASH_CALL_MAX 300 // flash中call记录最大数量

/***** 文件最大路径 *****/
#define MEDIA_PATH_MAX 128

/***** 文件名最大数目 *****/
#define FILE_NAME_MAX 23

// static int sd_media_total = 0;
static int sd_video_total = 0;
static int sd_photo_total = 0;

static int sd_media_new_total = 0;
static int sd_video_new_total = 0;
static int sd_photo_new_total = 0;

static file_info *p_media_sd = NULL;
static file_info *p_video_sd = NULL;
static file_info *p_photo_sd = NULL;

static int sd_media_video_total = 0;
static int sd_media_video_new_total = 0;

static int flash_media_total = 0;
static int flash_media_new_total = 0;
static file_info *p_media_flash = NULL;

/***** 判定sd卡是否插入 *****/
static bool is_media_sd_insert = false;
static bool is_media_sd_full = false;
static pthread_mutex_t file_list_mutex;

static bool sd_format_flg = false;
static bool copy_to_sd_flag = false;
static int copied_to_sd_munber = 0;
static file_type del_file_type_flag = FILE_TYPE_NONE;

static int sd_call_total = 0;
static int sd_call_new_total = 0;
static file_info *p_call_sd = NULL;

static int flash_call_total = 0;
static int flash_call_new_total = 0;
static file_info *p_call_flash = NULL;

/***
** 日期: 2022-05-17 10:58
** 作者: leo.liu
** 函数作用：缓存文件路径
** 返回参数说明：
***/
#define CHACHE_NAME ".conf"
#define SD_MEDIA_CACHE_PATH SD_MEDIA_PATH CHACHE_NAME

#define SD_VIDEO_CACHE_PATH SD_VIDEO_PATH CHACHE_NAME
#define SD_PHOTO_CACHE_PATH SD_PHOTO_PATH CHACHE_NAME

#define FLASH_MEDIA_CACHE_PATH FLASH_PHOTO_PATH CHACHE_NAME

#define FLASH_CALL_CACHE_PATH FLASH_CALL_PATH CHACHE_NAME

#define SD_CALL_CACHE_PATH SD_CALL_PATH CHACHE_NAME
/***
** 日期: 2022-05-17 11:48
** 作者: leo.liu
** 函数作用：文件信息同步
** 返回参数说明：
***/
static bool _media_file_sync(file_type type)
{
	// system("rm -rf "SD_MEDIA_CACHE_PATH);

	if (type == FILE_TYPE_PHOTO)
	{
		system("rm -rf " SD_PHOTO_CACHE_PATH);
		int fd = open(SD_PHOTO_CACHE_PATH, O_CREAT | O_WRONLY);
		if (fd < 0)
		{
			printf("open %s failed \n", SD_PHOTO_CACHE_PATH);
			return false;
		}
		write(fd, p_photo_sd, sd_photo_total * sizeof(file_info));
		close(fd);
	}
	else if (type == FILE_TYPE_VIDEO)
	{
		system("rm -rf " SD_VIDEO_CACHE_PATH);
		int fd = open(SD_VIDEO_CACHE_PATH, O_CREAT | O_WRONLY);
		if (fd < 0)
		{
			printf("open %s failed \n", SD_VIDEO_CACHE_PATH);
			return false;
		}
		printf("================sd_video_total[%d]\n", sd_video_total);
		write(fd, p_video_sd, sd_video_total * sizeof(file_info));
		close(fd);
	}
	else if (type == FILE_TYPE_FLASH_PHOTO)
	{
		system("rm -rf " FLASH_MEDIA_CACHE_PATH);
		int fd = open(FLASH_MEDIA_CACHE_PATH, O_CREAT | O_WRONLY);
		if (fd < 0)
		{
			printf("open %s failed \n", FLASH_MEDIA_CACHE_PATH);
			return false;
		}
		write(fd, p_media_flash, flash_media_total * sizeof(file_info));
		close(fd);
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型同步
	{
		system("rm -rf " SD_CALL_CACHE_PATH);
		int fd = open(SD_CALL_CACHE_PATH, O_CREAT | O_WRONLY);
		if (fd < 0)
		{
			printf("open %s failed \n", SD_CALL_CACHE_PATH);
			return false;
		}
		write(fd, p_call_sd, sd_call_total * sizeof(file_info));
		close(fd);
	}
	else if (type == FILE_TYPE_FLASH_CALL)
	{ // 新增flash call同步
		system("rm -rf " FLASH_CALL_CACHE_PATH);
		int fd = open(FLASH_CALL_CACHE_PATH, O_CREAT | O_WRONLY);
		if (fd < 0)
		{
			printf("open %s failed \n", FLASH_CALL_CACHE_PATH);
			return false;
		}
		write(fd, p_call_flash, flash_call_total * sizeof(file_info));
		close(fd);
	}
	system("sync");
	return true;
}
/***
** 日期: 2022-05-17 10:53
** 作者: leo.liu
** 函数作用：根据.conf扫描其中的文件信息
** 返回参数说明：
***/
static bool _media_file_load(file_info *pinfo, const char *cache_dir, int *ptotal, int *pnew_total, int num_file_max, const char *file_path, file_type type)
{
	(*ptotal) = (*pnew_total) = 0;

	int fd = open(cache_dir, O_RDONLY);
	if (fd < 0)
	{
		printf("open %s failed \n", cache_dir);
		return false;
	}

	struct stat statbuf;
	stat(cache_dir, &statbuf);
	int total = statbuf.st_size / sizeof(file_info);
	total = total > num_file_max ? num_file_max : total;

	char file[128];
	bool need_sync = false;
	int read_len = 0;
	file_info info;
	int read_size = sizeof(file_info);
	// read(fd, pinfo, sizeof(file_info) * total);//取出conf文件里保存的结构体数组
	while ((read_len = read(fd, &info, read_size)) == read_size)
	{
		sprintf(file, "%s%s", file_path, info.file_name);
		// *ptotal = total;
		// for (int i = 0; i < total; i++)//统计新媒体文件数量
		// {
		if (access(file, F_OK) == 0)
		{
			*pinfo = info;
			(*ptotal)++;
			if (info.type == FILE_TYPE_VIDEO)
			{
				sd_media_video_total++;
			}
			if (info.is_new == true)
			{
				(*pnew_total)++;
				if (info.type == FILE_TYPE_VIDEO)
				{
					sd_media_video_new_total++;
				}
			}
			pinfo++;
		}
		else if (need_sync == false)
		{
			printf("%s not exist \n", file);
			need_sync = true;
		}
		// }
	}

	close(fd);
	if (need_sync == true)
	{
		_media_file_sync(type);
	}
	printf("scanf %s :total:%d,new total:%d\n", cache_dir, *ptotal, *pnew_total);
	return true;
}

/***
** 日期: 2022-05-17 10:48
** 作者: leo.liu
** 函数作用：媒体文件扫描
** 返回参数说明：
***/
static bool _scan_media_file(file_type type)
{
	char *cache_dir = NULL;
	char *file_path = NULL;
	file_info *info;
	int *p_total;
	int *p_new_total;
	int num_file_max = 0;
	if (type == FILE_TYPE_FLASH_PHOTO)
	{
		cache_dir = FLASH_MEDIA_CACHE_PATH;
		info = p_media_flash;
		flash_media_total = flash_media_new_total = 0;
		p_total = &flash_media_total;
		p_new_total = &flash_media_new_total;
		num_file_max = FLASH_PHOTO_MAX;
		file_path = FLASH_PHOTO_PATH;
		if (access(file_path, F_OK) != 0)
		{
			/***** 文件夹不存在 *****/
			system("mkdir " FLASH_PHOTO_PATH);
			printf("mkdir " FLASH_PHOTO_PATH "\n");
			return false;
		}
	}
	else if (type == FILE_TYPE_PHOTO)
	{
		cache_dir = SD_PHOTO_CACHE_PATH;
		info = p_photo_sd;
		sd_photo_total = sd_photo_new_total = 0;
		// sd_media_video_total = sd_media_video_new_total = 0;//wxj,计算media目录视频数量
		p_total = &sd_photo_total;
		p_new_total = &sd_photo_new_total;
		num_file_max = SD_PHOTO_NUM_MAX;
		file_path = SD_PHOTO_PATH;
		if (access(file_path, F_OK) != 0)
		{
			/***** 文件夹不存在 *****/
			system("mkdir " SD_PHOTO_PATH);
			printf("mkdir " SD_PHOTO_PATH "\n");
			return false;
		}
	}
	else if (type == FILE_TYPE_VIDEO)
	{
		cache_dir = SD_VIDEO_CACHE_PATH;
		info = p_video_sd;
		sd_video_total = sd_media_new_total = 0;
		sd_media_video_total = sd_media_video_new_total = 0; // wxj,计算media目录视频数量
		p_total = &sd_video_total;
		p_new_total = &sd_video_new_total;
		num_file_max = SD_VIDEO_NUM_MAX;
		file_path = SD_VIDEO_PATH;
		if (access(file_path, F_OK) != 0)
		{
			/***** 文件夹不存在 *****/
			system("mkdir " SD_VIDEO_PATH);
			printf("mkdir " SD_VIDEO_PATH "\n");
			return false;
		}
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型扫描
	{
		cache_dir = SD_CALL_CACHE_PATH;
		info = p_call_sd;
		sd_call_total = sd_call_new_total = 0;
		p_total = &sd_call_total;
		p_new_total = &sd_call_new_total;
		num_file_max = SD_CALL_NUM_MAX;
		file_path = SD_CALL_PATH;
		if (access(file_path, F_OK) != 0)
		{
			/***** 文件夹不存在 *****/
			system("mkdir " SD_CALL_PATH);
			printf("mkdir " SD_CALL_PATH "\n");
			return false;
		}
	}
	else if (type == FILE_TYPE_FLASH_CALL)
	{ // 新增flash call扫描
		cache_dir = FLASH_CALL_CACHE_PATH;
		info = p_call_flash;
		flash_call_total = flash_call_new_total = 0;
		p_total = &flash_call_total;
		p_new_total = &flash_call_new_total;
		num_file_max = FLASH_CALL_MAX;
		file_path = FLASH_CALL_PATH;
		if (access(file_path, F_OK) != 0)
		{
			system("mkdir " FLASH_CALL_PATH);
			printf("mkdir " FLASH_CALL_PATH "\n");
			return false;
		}
	}
	else
	{
		return false;
	}
	return _media_file_load(info, cache_dir, p_total, p_new_total, num_file_max, file_path, type);
}

/***
** 日期: 2022-05-17 11:07
** 作者: leo.liu
** 函数作用：判断sd卡是否已经插入
** 返回参数说明：
***/
static bool _media_sd_insert_check(void)
{
	return (access("/dev/mmcblk0", F_OK) == 0) ? true : false;
}

/***
** 日期: 2022-05-17 11:12
** 作者: leo.liu
** 函数作用：创建文件夹
** 返回参数说明：
***/
static void _media_dir_create(const char *path)
{
	char buf[128] = {0};
	sprintf(buf, "mkdir %s", path);
	system(buf);
}

/***
** 日期: 2022-05-17 11:24
** 作者: leo.liu
** 函数作用：格式化sd
** 返回参数说明：
***/
static void _media_sd_format(void)
{
	printf("##### start fomrat sdcard ##### \n");
	system("rm -rf " SD_MEDIA_PATH);
	system("rm -rf " SD_VIDEO_PATH);
	system("rm -rf " SD_PHOTO_PATH);
	system("rm -rf " SD_CALL_PATH); // 添加删除call文件夹
	system("umount " SD_BASE_PATH);

	system("mkdosfs -F 32 -I /dev/mmcblk0 -n EP");
	system("sync");
	printf("##### finish fomrat sdcard ##### \n");
}

/***
** 日期: 2022-05-17 11:34
** 作者: leo.liu
** 函数作用：判断sd卡是否已经满
** 返回参数说明：
***/
static bool _media_sd_full_check(void)
{
	struct statfs sta;

	int total = 0;
	int free = 0;

	if (0 != statfs(SD_BASE_PATH, &sta))
	{
		return false;
	}

	if (sta.f_bsize >= 1024)
	{
		total = (int)((sta.f_bsize / 1024) * sta.f_blocks);
		free = (int)((sta.f_bsize / 1024) * sta.f_bavail);
	}
	else
	{
		total = (int)((sta.f_blocks / 1024) * sta.f_bsize);
		free = (int)((sta.f_bavail / 1024) * sta.f_bsize);
	}
	printf("============= sdcard disk:total:%0.2fM/ free:%0.2fM =============\n", total / 1024.0, free / 1024.0);
	return (free < 102400) ? true : false;
}

/***
** 日期: 2022-05-17 11:39
** 作者: leo.liu
** 函数作用：删除一个文件
** 返回参数说明：
***/
static bool _media_file_delete(file_type type, int index)
{
	file_info *info = NULL;
	file_info *array = NULL;
	char file_path[MEDIA_PATH_MAX] = {0};
	int *ptotal = NULL;
	int *p_new_total = NULL;
	if (type == FILE_TYPE_FLASH_PHOTO)
	{
		if (index >= flash_media_total)
		{
			return false;
		}
		array = p_media_flash;
		info = &p_media_flash[index];
		strcpy(file_path, FLASH_PHOTO_PATH);
		strcat(file_path, info->file_name);

		ptotal = &flash_media_total;
		p_new_total = &flash_media_new_total;
	}
	else if (type == FILE_TYPE_PHOTO)
	{
		if (index >= sd_photo_total)
		{
			return false;
		}
		array = p_photo_sd;
		info = &p_photo_sd[index];
		strcpy(file_path, SD_PHOTO_PATH);
		strcat(file_path, info->file_name);

		ptotal = &sd_photo_total;
		p_new_total = &sd_photo_new_total;
	}
	else if (type == FILE_TYPE_VIDEO)
	{
		if (index >= sd_video_total)
		{
			return false;
		}
		array = p_video_sd;
		info = &p_video_sd[index];
		strcpy(file_path, SD_VIDEO_PATH);
		strcat(file_path, info->file_name);

		ptotal = &sd_video_total;
		p_new_total = &sd_video_new_total;
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型删除
	{
		if (index >= sd_call_total)
		{
			return false;
		}
		array = p_call_sd;
		info = &p_call_sd[index];
		strcpy(file_path, SD_CALL_PATH);
		strcat(file_path, info->file_name);

		ptotal = &sd_call_total;
		p_new_total = &sd_call_new_total;
	}
	else if (type == FILE_TYPE_FLASH_CALL) // 新增flash call删除
	{
		if (index >= flash_call_total)
		{
			return false;
		}
		array = p_call_flash;
		info = &p_call_flash[index];
		strcpy(file_path, FLASH_CALL_PATH);
		strcat(file_path, info->file_name);

		ptotal = &flash_call_total;
		p_new_total = &flash_call_new_total;
	}
	if ((*ptotal) <= 0)
	{
		return false;
	}

	if ((info->is_new == true) && ((*p_new_total) > 0))
	{
		(*p_new_total)--;
		if (info->type == FILE_TYPE_VIDEO)
		{
			sd_media_video_new_total--;
		}
	}
	if (access(file_path, F_OK) == 0)
	{
		remove(file_path);
	}
	// printf("remove %d->%s \n", index, file_path);
	printf("---------------[%s]--------[%d]\n", __func__, __LINE__);
	if (info->type == FILE_TYPE_VIDEO)
	{
		sd_media_video_total--;
	}
	printf("---------------[%s]--------[%d]\n", __func__, __LINE__);
	if (index < ((*ptotal) - 1))
	{
		memmove(&array[index], &array[index + 1], (((*ptotal) - 1) - index) * sizeof(file_info));
	}
	(*ptotal)--;
	printf("---------------[%s]--------[%d]\n", __func__, __LINE__);
	_media_file_sync(type);
	printf("---------------[%s]--------[%d]\n", __func__, __LINE__);
	return true;
}

/*******************************************************************
 * @brief  : 移动flash的照片到SD卡
 * @return  {*}
 *******************************************************************/
static void media_copy_flash_photo_to_sd_process(void)
{
	if (access(SD_BACKUP_PATH, F_OK) != 0)
	{

		mkdir(SD_BACKUP_PATH, 0777);
		system("sync");
	}

	char cmd[64] = {0};
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	dir = opendir(FLASH_PHOTO_PATH);
	if (dir != NULL)
	{
		while ((ptr = readdir(dir)) != NULL)
		{
			if (strstr(ptr->d_name, PHOTO_DOT) != NULL)
			{
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "cp -rf " FLASH_PHOTO_PATH "%s " SD_BACKUP_PATH, ptr->d_name);
				system(cmd);
				copied_to_sd_munber++;
			}
		}
		closedir(dir);
	}

	system("sync");
	_scan_media_file(FILE_TYPE_FLASH_PHOTO);
}

/***
**   日期:2022-06-15 11:14:00
**   作者: feian.liu
**   函数作用：获知SD卡是否有p1分区，如果有，则显示为错误的SD卡
**   参数说明:
***/
bool sd_card_repartition(void)
{
	uint64_t block0_sector = 0, p1_sector = 0, p2_sector = 0, sector_size = 0;

	FILE *fp = popen("fdisk -l "
					 "/dev/"
					 "mmcblk0",
					 "r");

	if (fp == NULL)
	{
		return false;
	}
	char buffer[256] = {0};
	while (fgets(buffer, sizeof(buffer), fp))
	{
		if (strstr(buffer, "/dev/mmcblk0:") != NULL)
		{
			char *p_sectors_str = strrchr(buffer, ',') + 1;
			sscanf(p_sectors_str, " %llu sectors", &block0_sector);
		}
		if (strstr(buffer, "Units: sectors of") != NULL) // 获取SD卡扇区大小
		{
			uint64_t a, b;
			sscanf(buffer, "Units: sectors of %llu * %llu = %llu bytes", &a, &sector_size, &b);
		}
		if (strstr(buffer, "/dev/mmcblk0p1") != NULL)
		{
			char str[8][32];
			for (int i = 0; i < 8; i++)
			{
				memset(str[i], 0, 32);
			}
			sscanf(buffer, "%s\t%s\t%s\t%s\t%s\t%llu\t%s\t%s", str[0], str[1], str[2], str[3], str[4], &p1_sector, str[6], str[7]);
		}
		else if (strstr(buffer, "/dev/mmcblk0p2") != NULL)
		{
			char str[8][32];
			for (int i = 0; i < 8; i++)
			{
				memset(str[i], 0, 32);
			}
			sscanf(buffer, "%s\t%s\t%s\t%s\t%s\t%llu\t%s\t%s", str[0], str[1], str[2], str[3], str[4], &p2_sector, str[6], str[7]);
		}
		memset(buffer, 0, sizeof(buffer));
	}
	pclose(fp);

	if (p1_sector != 0 || p2_sector != 0)
	{
		user_data_get()->sd_card_pattion = true;
	}
	else
	{
		user_data_get()->sd_card_pattion = false;
	}
	return false;
}

/***
** 日期: 2022-05-17 10:46
** 作者: leo.liu
** 函数作用：文件系统检测任务
** 返回参数说明：
***/
static void *media_filelist_task(void *arg)
{
	bool insert = false;
	printf("***** media file task create sccess ! *****\n");
	while (1)
	{
		pthread_mutex_lock(&file_list_mutex);
		insert = _media_sd_insert_check();
		if (insert != is_media_sd_insert)
		{
			is_media_sd_insert = insert;
			if (is_media_sd_insert == true)
			{
				if (access(SD_BASE_PATH, F_OK) != 0)
				{
					_media_dir_create(SD_BASE_PATH);
				}

				sd_card_repartition();

				/***** 挂载媒体 *****/
				// system("umount -f " SD_BASE_PATH);
				system("mount /dev/mmcblk0 " SD_BASE_PATH);
				if (access(SD_MEDIA_PATH, F_OK) != 0)
				{
					_media_dir_create(SD_MEDIA_PATH);
				}
				if (access(SD_VIDEO_PATH, F_OK) != 0)
				{
					_media_dir_create(SD_VIDEO_PATH);
				}
				if (access(SD_PHOTO_PATH, F_OK) != 0)
				{
					_media_dir_create(SD_PHOTO_PATH);
				}
				if (access(SD_CALL_PATH, F_OK) != 0)
				{
					_media_dir_create(SD_CALL_PATH);
				}
				is_media_sd_full = false;
				if (_media_sd_full_check() == true)
				{
					printf("sd space full \n");
					is_media_sd_full = true;
				}
				_scan_media_file(FILE_TYPE_VIDEO);
				_scan_media_file(FILE_TYPE_PHOTO);
				// 添加call文件扫描
				_scan_media_file(FILE_TYPE_CALL);
			}
			else
			{
				is_media_sd_full = false;
				system("umount -l " SD_BASE_PATH);
			}
			lv_msg_send_cmd(MSG_EVENT_CMD_SD_STATE, 0, 0);
		}
		pthread_mutex_unlock(&file_list_mutex);

		if (sd_format_flg == true)
		{
			if (is_media_sd_insert == false)
			{
				sd_format_flg = false;
			}
			else
			{
				pthread_mutex_lock(&file_list_mutex);
				is_media_sd_insert = false;
				is_media_sd_full = false;
				pthread_mutex_unlock(&file_list_mutex);
				_media_sd_format();
				sd_format_flg = false;
			}
		}
		if (copy_to_sd_flag == true)
		{
			if (is_media_sd_insert == false)
			{
				copy_to_sd_flag = false;
			}
			else
			{
				printf("===========================>> 开始拷贝文件\n");
				pthread_mutex_lock(&file_list_mutex);
				media_copy_flash_photo_to_sd_process();
				pthread_mutex_unlock(&file_list_mutex);
				copy_to_sd_flag = false;
			}
		}
		if (del_file_type_flag != FILE_TYPE_NONE)
		{
			media_file_delete_all(del_file_type_flag, false);
			del_file_type_flag = FILE_TYPE_NONE;
		}
		usleep(50 * 1000);
	}
	return NULL;
}

/***
** 日期: 2022-05-17 10:40
** 作者: leo.liu
** 函数作用：媒体文件初始化
** 返回参数说明：
***/
void media_file_list_init(void)
{
	p_media_sd = (file_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(file_info) * SD_MEDIA_NUM_MAX);
	p_media_flash = (file_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(file_info) * FLASH_PHOTO_MAX);
	p_video_sd = (file_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(file_info) * SD_VIDEO_NUM_MAX);
	p_photo_sd = (file_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(file_info) * SD_PHOTO_NUM_MAX);
	p_call_sd = (file_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(file_info) * SD_CALL_NUM_MAX); // 新增
	p_call_flash = (file_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(file_info) * FLASH_CALL_MAX);

	pthread_mutex_init(&file_list_mutex, NULL);

	_scan_media_file(FILE_TYPE_FLASH_PHOTO);
	_scan_media_file(FILE_TYPE_FLASH_CALL);
	pthread_t thread_t;
	pthread_create(&thread_t, user_pthread_atter_get(), media_filelist_task, NULL);
}

/***
** 日期: 2022-05-17 11:26
** 作者: leo.liu
** 函数作用：判断sd插入
** 返回参数说明：
***/
bool media_sdcard_insert_check(void)
{
	pthread_mutex_lock(&file_list_mutex);
	bool insert = is_media_sd_insert;
	pthread_mutex_unlock(&file_list_mutex);
	return insert;
}
/***
** 日期: 2022-05-17 14:40
** 作者: leo.liu
** 函数作用：判断sd文件是否已经满
** 返回参数说明：
***/
bool media_sdcard_full_check(void)
{
	pthread_mutex_lock(&file_list_mutex);
	bool full = is_media_sd_full;
	pthread_mutex_unlock(&file_list_mutex);
	return full;
}
/***
** 日期: 2022-05-17 11:28
** 作者: leo.liu
** 函数作用：创建一个文件
** 返回参数说明：
***/
bool media_file_create(file_type type, char ch, char mode, bool is_door_station, int call_num, char *file_path)
{
	pthread_mutex_lock(&file_list_mutex);
	if ((type == FILE_TYPE_CALL || type == FILE_TYPE_PHOTO || type == FILE_TYPE_VIDEO) &&
		(is_media_sd_insert == false))
	{
		pthread_mutex_unlock(&file_list_mutex);
		return false;
	}

	file_info *p_array = NULL;
	char *path = NULL;
	if (type == FILE_TYPE_VIDEO)
	{
		int file_max = SD_VIDEO_NUM_MAX;
		/***** 判断sd容量 *****/
		printf("=================sd_video_total=[%d]=====[%s]\n", sd_video_total, __func__);
		while ((sd_video_total >= file_max) ||
			   (_media_sd_full_check() == true))
		{
			for (int i = 0; i < sd_video_total; i++) // 覆盖文件
			{
				if (p_video_sd[i].type == type)
				{
					_media_file_delete(type, i);
					break;
				}
			}

			if (sd_video_total < 1)
			{
				break;
			}
		}

		path = SD_VIDEO_PATH;
		p_array = &p_video_sd[sd_video_total];
		sd_video_total++;
		sd_video_new_total++;
		// if(type == FILE_TYPE_VIDEO)
		{
			sd_media_video_total++;
			sd_media_video_new_total++;
		}
	}
	else if (type == FILE_TYPE_PHOTO)
	{
		int file_max = SD_PHOTO_NUM_MAX;
		/***** 判断sd容量 *****/
		while (sd_photo_total >= file_max ||
			   (_media_sd_full_check() == true))
		{
			for (int i = 0; i < sd_photo_total; i++)
			{
				if (p_photo_sd[i].type == type)
				{
					// printf("删除，覆盖照片文件\n");
					_media_file_delete(type, i);
					break;
				}
			}
			// printf("退出覆盖照片文件\n");
			if (sd_photo_total < 1)
			{
				break;
			}
		}

		path = SD_PHOTO_PATH;
		p_array = &p_photo_sd[sd_photo_total];
		sd_photo_total++;
		sd_photo_new_total++;
		// printf("=======>>>>>>sd_photo_total==[%d]\n",sd_photo_total);
		// printf("=======>>>>>>sd_photo_new_total==[%d]\n",sd_photo_new_total);
		// if(type == FILE_TYPE_VIDEO)
		// {
		// 	sd_media_video_total++;
		// 	sd_media_video_new_total++;
		// }
	}
	else if (type == FILE_TYPE_FLASH_PHOTO)
	{
		if (flash_media_total >= FLASH_PHOTO_MAX)
		{
			_media_file_delete(type, 0);
		}
		path = FLASH_PHOTO_PATH;
		p_array = &p_media_flash[flash_media_total];
		flash_media_total++;
		flash_media_new_total++;
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型创建
	{
		int file_max = SD_CALL_NUM_MAX;
		/***** 判断sd容量 *****/
		while (sd_call_total >= file_max ||
			   (_media_sd_full_check() == true))
		{
			for (int i = 0; i < sd_call_total; i++)
			{
				if (p_call_sd[i].type == type)
				{
					_media_file_delete(type, i);
					break;
				}
			}

			if (sd_call_total < 1)
			{
				break;
			}
		}

		path = SD_CALL_PATH;
		p_array = &p_call_sd[sd_call_total];
		sd_call_total++;
		sd_call_new_total++;
	}
	else if (type == FILE_TYPE_FLASH_CALL)
	{ // 新增flash call创建
		if (flash_call_total >= FLASH_CALL_MAX)
		{
			_media_file_delete(type, 0); // 删除最旧的记录
		}
		path = FLASH_CALL_PATH;
		p_array = &p_call_flash[flash_call_total];
		flash_call_total++;
		flash_call_new_total++;
	}

	p_array->ch = ch;
	p_array->is_new = true;
	p_array->mode = mode;
	p_array->type = type;

	struct tm date;
	user_time_read(&date);

	int count = 0;
	do
	{

		if (type == FILE_TYPE_CALL || type == FILE_TYPE_FLASH_CALL)
		{
			p_array->answered = false; // 默认未接听
			p_array->is_door_station = is_door_station;
			p_array->call_num = call_num; // 直接存储call_num
			// call记录文件名格式

			if (is_door_station)
			{
				// 门口机文件名：保留原有格式，加入"DS"标识
				snprintf(p_array->file_name, MEDIA_PATH_MAX, "%02d%02d%02d-%02d%02d%02d-%c%s",
						 date.tm_year % 100, date.tm_mon, date.tm_mday,
						 date.tm_hour, date.tm_min, date.tm_sec,
						 ch, CALL_DOT);
			}
			else
			{
				// 普通call_num文件名：加入"CN"和call_num
				snprintf(p_array->file_name, MEDIA_PATH_MAX, "%02d%02d%02d-%02d%02d%02d-%d%s",
						 date.tm_year % 100, date.tm_mon, date.tm_mday,
						 date.tm_hour, date.tm_min, date.tm_sec,
						 call_num, CALL_DOT);
			}
		}
		else
		{
			// 原有文件名生成逻辑
			snprintf(p_array->file_name, MEDIA_PATH_MAX, "%02d%02d%02d-%02d%02d%02d-%d%d%s",
					 date.tm_year % 100,
					 date.tm_mon,
					 date.tm_mday,
					 date.tm_hour,
					 date.tm_min,
					 date.tm_sec,
					 ch,
					 mode,
					 type == FILE_TYPE_VIDEO ? VIDEO_DOT : PHOTO_DOT);
		}

		strcpy(file_path, path);

		strcat(file_path, p_array->file_name);

		date.tm_sec++;
		date.tm_sec %= 60;
		count++;

	} while (access(file_path, F_OK) == 0);
	// printf("---------------[%s]--------[%d]\n",__func__,__LINE__);
	if (count > 60)
	{
		pthread_mutex_unlock(&file_list_mutex);
		return false;
	}
	// printf("---------------[%s]--------[%d]\n",__func__,__LINE__);
	_media_file_sync(type);
	// printf("---------------[%s]--------[%d]\n",__func__,__LINE__);
	pthread_mutex_unlock(&file_list_mutex);
	// printf("---------------[%s]--------[%d]\n",__func__,__LINE__);
	return true;
}

/***
** 日期: 2022-05-17 13:36
** 作者: leo.liu
** 函数作用：文件总数获取
** 返回参数说明：
***/
bool media_file_total_get(file_type type, int *total, int *new_total)
{
	bool reslut = true;
	pthread_mutex_lock(&file_list_mutex);
	if (type == FILE_TYPE_FLASH_PHOTO)
	{
		if (total != NULL)
		{
			*total = flash_media_total;
		}
		if (new_total != NULL)
		{
			*new_total = flash_media_new_total;
		}
	}
	else if (type == FILE_TYPE_FLASH_CALL) // 新增flash call统计
	{
		if (total != NULL)
		{
			*total = flash_call_total;
		}
		if (new_total != NULL)
		{
			*new_total = flash_call_new_total;
		}
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型统计
	{
		if (is_media_sd_insert == false)
		{
			reslut = false;
			goto finish;
		}

		if (total != NULL)
		{
			*total = sd_call_total;
		}
		if (new_total != NULL)
		{
			*new_total = sd_call_new_total;
		}
	}
	else
	{
		if (is_media_sd_insert == false)
		{
			reslut = false;
			goto finish;
		}

		if (total != NULL)
		{
			if (type == FILE_TYPE_PHOTO)
				// *total = sd_media_total - sd_media_video_total;
				*total = sd_photo_total;
			else if (type == FILE_TYPE_VIDEO)
				// *total = sd_media_video_total;
				*total = sd_video_total;
		}
		if (new_total != NULL)
		{
			if (type == FILE_TYPE_PHOTO)
				// *new_total = sd_media_new_total - sd_media_video_new_total;
				*new_total = sd_photo_new_total;
			else if (type == FILE_TYPE_VIDEO)
				// *new_total = sd_media_video_new_total;
				*new_total = sd_video_new_total;
		}
	}
finish:
	pthread_mutex_unlock(&file_list_mutex);
	return reslut;
}
/***
** 日期: 2022-05-17 13:44
** 作者: leo.liu
** 函数作用：清除新文件标志
** 返回参数说明：
***/
bool media_file_new_clear(file_type type, int index)
{
	pthread_mutex_lock(&file_list_mutex);
	if (type == FILE_TYPE_FLASH_PHOTO)
	{
		if (index >= flash_media_total)
		{
			pthread_mutex_unlock(&file_list_mutex);
			return false;
		}

		if ((p_media_flash[index].is_new != false) && (flash_media_new_total > 0))
		{
			flash_media_new_total--;
			p_media_flash[index].is_new = false;
			_media_file_sync(type);
		}
	}
	else if (type == FILE_TYPE_PHOTO)
	{
		if ((is_media_sd_insert == false) || (index >= sd_photo_total))
		{
			pthread_mutex_unlock(&file_list_mutex);
			return false;
		}
		if ((p_photo_sd[index].is_new != false) && (sd_photo_new_total > 0))
		{
			sd_photo_new_total--;
			p_photo_sd[index].is_new = false;
			_media_file_sync(type);
		}
	}
	else if (type == FILE_TYPE_VIDEO)
	{
		if ((is_media_sd_insert == false) || (index >= sd_video_total))
		{
			pthread_mutex_unlock(&file_list_mutex);
			return false;
		}
		if ((p_video_sd[index].is_new != false) && (sd_video_new_total > 0))
		{
			sd_video_new_total--;
			p_video_sd[index].is_new = false;
			_media_file_sync(type);
		}
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型清除新标志
	{
		if ((is_media_sd_insert == false) || (index >= sd_call_total))
		{
			pthread_mutex_unlock(&file_list_mutex);
			return false;
		}
		if ((p_call_sd[index].is_new != false) && (sd_call_new_total > 0))
		{
			sd_call_new_total--;
			p_call_sd[index].is_new = false;
			_media_file_sync(type);
		}
	}
	else if (type == FILE_TYPE_FLASH_CALL) // 新增flash call清除新标志
	{
		if (index >= flash_call_total)
		{
			pthread_mutex_unlock(&file_list_mutex);
			return false;
		}
		if ((p_call_flash[index].is_new != false) && (flash_call_new_total > 0))
		{
			flash_call_new_total--;
			p_call_flash[index].is_new = false;
			_media_file_sync(type);
		}
	}
	pthread_mutex_unlock(&file_list_mutex);
	return true;
}

/***
** 日期: 2022-05-17 13:53
** 作者: leo.liu
** 函数作用：删除一个文件
** 返回参数说明：
***/
bool media_file_delete(file_type type, int index)
{
	pthread_mutex_lock(&file_list_mutex);
	bool reslut = _media_file_delete(type, index);
	pthread_mutex_unlock(&file_list_mutex);
	return reslut;
}
/***
** 日期: 2022-05-17 13:54
** 作者: leo.liu
** 函数作用：判断该文件是否存在
** 返回参数说明：
***/
bool media_file_bad_check(const char *file)
{
	pthread_mutex_lock(&file_list_mutex);
	if (access(file, F_OK) != 0)
	{
		if (strstr(file, SD_VIDEO_PATH) != NULL)
		{
			for (int i = (sd_video_total - 1); i >= 0; i++)
			{
				if (strstr(file, p_video_sd[i].file_name) != NULL)
				{
					_media_file_delete(FILE_TYPE_VIDEO, i);
					break;
				}
			}
		}
		else if (strstr(file, SD_PHOTO_PATH) != NULL)
		{
			for (int i = (sd_photo_total - 1); i >= 0; i++)
			{
				if (strstr(file, p_photo_sd[i].file_name) != NULL)
				{
					_media_file_delete(FILE_TYPE_PHOTO, i);
					break;
				}
			}
		}
		else if (strstr(file, FLASH_PHOTO_PATH) != NULL)
		{
			for (int i = (flash_media_total - 1); i >= 0; i++)
			{
				if (strstr(file, p_media_flash[i].file_name) != NULL)
				{
					_media_file_delete(FILE_TYPE_FLASH_PHOTO, i);
					break;
				}
			}
		}
		else if (strstr(file, SD_CALL_PATH) != NULL)
		{
			for (int i = (sd_call_total - 1); i >= 0; i++)
			{
				if (strstr(file, p_call_sd[i].file_name) != NULL)
				{
					_media_file_delete(FILE_TYPE_CALL, i);
					break;
				}
			}
		}
		else if (strstr(file, FLASH_CALL_PATH) != NULL) // 新增flash call坏文件检查
		{
			for (int i = (flash_call_total - 1); i >= 0; i--)
			{
				if (strstr(file, p_call_flash[i].file_name) != NULL)
				{
					_media_file_delete(FILE_TYPE_FLASH_CALL, i);
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&file_list_mutex);
	return true;
}
/***
**   日期:2022-05-23 09:51:21
**   作者: leo.liu
**   函数作用：获取格式化sd卡状态
**   参数说明:
***/
bool media_format_sd_state(void)
{
	bool state = sd_format_flg;
	return state;
}
/***
**   日期:2022-05-23 09:58:49
**   作者: leo.liu
**   函数作用：格式话sd卡
**   参数说明:
***/
void media_format_sd(void)
{
	sd_format_flg = true;
}

/*******************************************************************
 * @brief  : 获取移动flash照片到SD卡的状态
 * @return  {*}
 *******************************************************************/
bool media_copy_flash_photo_to_sd_state(int *copied)
{
	bool state = copy_to_sd_flag;
	if (copied != NULL)
	{
		*copied = copied_to_sd_munber;
	}
	return state;
}
/*******************************************************************
 * @brief  : 开始移动flash照片到SD卡
 * @return  {*}
 *******************************************************************/
void media_copy_flash_photo_to_sd(void)
{
	copy_to_sd_flag = true;
	copied_to_sd_munber = 0;
}
// 开始删除文件
void media_file_delete_all_start(file_type type)
{
	del_file_type_flag = type;
}
// 删除文件状态
bool media_file_delete_all_state(void)
{
	bool state = del_file_type_flag == FILE_TYPE_NONE ? false : true;
	return state;
}
/***
**   日期:2022-05-23 14:34:43
**   作者: leo.liu
**   函数作用：获取文件信息
**   参数说明:
***/
const file_info *media_file_info_get(file_type type, int index)
{
	const file_info *pinfo = NULL;
	pthread_mutex_lock(&file_list_mutex);
	if (type == FILE_TYPE_FLASH_PHOTO)
	{
		if (index < flash_media_total)
		{
			pinfo = &p_media_flash[index];
		}
		else
		{
			printf("find flash media index:%d failed \n", index);
		}
	}
	else if (type == FILE_TYPE_PHOTO)
	{
		if ((is_media_sd_insert == true) && (index < sd_photo_total))
		{
			pinfo = &p_photo_sd[index];
		}
		else
		{
			printf("find sd media index:%d failed \n", index);
		}
	}
	else if (type == FILE_TYPE_VIDEO)
	{
		if ((is_media_sd_insert == true) && (index < sd_video_total))
		{
			pinfo = &p_video_sd[index];
		}
		else
		{
			printf("find sd media index:%d failed \n", index);
		}
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型获取信息
	{
		if ((is_media_sd_insert == true) && (index < sd_call_total))
		{
			pinfo = &p_call_sd[index];
		}
		else
		{
			printf("find call record index:%d failed \n", index);
		}
	}
	else if (type == FILE_TYPE_FLASH_CALL) // 新增flash call获取信息
	{
		if (index < flash_call_total)
		{
			pinfo = &p_call_flash[index];
		}
		else
		{
			printf("find flash call index:%d failed \n", index);
		}
	}
	pthread_mutex_unlock(&file_list_mutex);
	return pinfo;
}

// 新增：创建call记录的函数
bool media_call_record_create(char ch, int call_num, bool is_door_station, char *file_path)
{
	// 优先尝试使用flash存储
	if (media_file_create(FILE_TYPE_FLASH_CALL, ch, 0, is_door_station, call_num, file_path))
	{
		return true;
	}

	// 如果flash存储失败，回退到SD卡存储
	if (is_media_sd_insert)
	{
		return media_file_create(FILE_TYPE_CALL, ch, 0, is_door_station, call_num, file_path);
	}

	return false;
}

// 新增：更新call记录接听状态
bool media_file_call_answered_update(const char *file_name)
{
	pthread_mutex_lock(&file_list_mutex);

	// 先在flash call记录中查找
	for (int i = 0; i < flash_call_total; i++)
	{
		if (strcmp(p_call_flash[i].file_name, file_name) == 0)
		{
			if (p_call_flash[i].answered == false)
			{
				p_call_flash[i].answered = true;
				if (p_call_flash[i].is_new == true && flash_call_new_total > 0)
				{
					p_call_flash[i].is_new = false;
					flash_call_new_total--;
				}
				_media_file_sync(FILE_TYPE_FLASH_CALL);
				pthread_mutex_unlock(&file_list_mutex);
				return true;
			}
			break;
		}
	}

	// 如果在flash中没找到，再到SD卡中查找（向后兼容）
	if (is_media_sd_insert == true)
	{
		for (int i = 0; i < sd_call_total; i++)
		{
			if (strcmp(p_call_sd[i].file_name, file_name) == 0)
			{
				if (p_call_sd[i].answered == false)
				{
					p_call_sd[i].answered = true;
					if (p_call_sd[i].is_new == true && sd_call_new_total > 0)
					{
						p_call_sd[i].is_new = false;
						sd_call_new_total--;
					}
					_media_file_sync(FILE_TYPE_CALL);
					pthread_mutex_unlock(&file_list_mutex);
					return true;
				}
				break;
			}
		}
	}

	pthread_mutex_unlock(&file_list_mutex);
	return false;
}

/***
**   日期:2022-06-15 11:14:00
**   作者: leo.liu
**   函数作用：删除所有文件
**   参数说明:
***/
bool media_file_delete_all(file_type type, bool all)
{
	pthread_mutex_lock(&file_list_mutex);
	if (type == FILE_TYPE_FLASH_PHOTO)
	{
		system("rm -rf " FLASH_PHOTO_PATH);
		system("rm -rf " FLASH_MEDIA_CACHE_PATH);
		_scan_media_file(type);
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型删除所有
	{
		if (is_media_sd_insert == true)
		{
			system("rm -rf " SD_CALL_PATH "*" CALL_DOT);
			sd_call_total = 0;
			sd_call_new_total = 0;
			_media_file_sync(FILE_TYPE_CALL);
		}
	}
	else if (type == FILE_TYPE_FLASH_CALL) // 新增flash call删除所有
	{
		system("rm -rf " FLASH_CALL_PATH "*" CALL_DOT);
		system("rm -rf " FLASH_CALL_CACHE_PATH);
		_scan_media_file(type);
	}
	else if (all == true)
	{
		system("rm -rf " SD_MEDIA_PATH);
		system("rm -rf " SD_MEDIA_CACHE_PATH);
		_scan_media_file(type);
	}
	else if (is_media_sd_insert == true)
	{
		int total = 0, new_total = 0;
		file_info *buffer = NULL;
		// unsigned long long os_time = user_timestamp_get();
		if (type == FILE_TYPE_PHOTO)
		{
			// for (int i = 0; i < sd_media_total; i++)
			// {
			// 	if (p_media_sd[i].type == FILE_TYPE_VIDEO)
			// 	{
			// 		memcpy(&p_media_sd[total], &p_media_sd[i], sizeof(file_info));//把视频的结构体信息往前移
			// 		total++;
			// 		if(p_media_sd[i].is_new)
			// 			new_total++;
			// 	}
			// }
			// if(total > 0)
			// {
			// 	buffer = (file_info *)malloc(sizeof(file_info) * total);
			// 	memcpy(buffer, p_photo_sd, sizeof(file_info) * total);
			// 	memset(p_photo_sd, 0, sizeof(file_info) * SD_MEDIA_NUM_MAX);
			// 	memcpy(p_photo_sd, buffer, sizeof(file_info) * total);
			// }
			// else
			// {
			// 	memset(p_photo_sd, 0, sizeof(file_info) * SD_MEDIA_NUM_MAX);
			// }
			// sd_media_video_total = sd_media_total = total;
			// sd_media_video_new_total = sd_media_new_total = new_total;
			sd_photo_total = total;
			sd_photo_new_total = new_total;
			// printf("================>>>> 111:[%llu]  file_type:[%d]\n", user_timestamp_get() - os_time, type);
			system("rm -rf " SD_PHOTO_PATH "*" PHOTO_DOT);
			_media_file_sync(FILE_TYPE_PHOTO);
		}
		else if (type == FILE_TYPE_VIDEO)
		{
			// for (int i = 0; i < sd_media_total; i++)
			// {
			// 	if (p_media_sd[i].type == FILE_TYPE_PHOTO)
			// 	{
			// 		memcpy(&p_media_sd[total], &p_media_sd[i], sizeof(file_info));
			// 		total++;
			// 		if(p_media_sd[i].is_new)
			// 			new_total++;
			// 	}
			// }
			// if(total > 0)
			// {
			// buffer = (file_info *)malloc(sizeof(file_info) * total);
			// memcpy(buffer, p_video_sd, sizeof(file_info) * total);
			// memset(p_video_sd, 0, sizeof(file_info) * SD_MEDIA_NUM_MAX);
			// memcpy(p_video_sd, buffer, sizeof(file_info) * total);
			// }
			// else
			// {
			// 	memset(p_video_sd, 0, sizeof(file_info) * SD_MEDIA_NUM_MAX);
			// }
			sd_video_total = total;
			sd_video_new_total = new_total;
			// sd_media_video_total = sd_media_video_new_total = 0;
			// printf("================>>>> 111:[%llu]  file_type:[%d]\n", user_timestamp_get() - os_time, type);
			system("rm -rf " SD_VIDEO_PATH "*" VIDEO_DOT);
			printf("++++++++++++++++++++++sd_video_total[%d]\n", sd_video_total);
			_media_file_sync(FILE_TYPE_VIDEO);
		}
		if (buffer != NULL)
			free(buffer);
		// printf("================>>>> 222:[%llu]\n", user_timestamp_get() - os_time);
	}
	pthread_mutex_unlock(&file_list_mutex);
	return true;
}

/***
**   日期:2022-06-15 11:14:00
**   作者: leo.liu
**   函数作用：获取SD卡所有媒体文件
**   参数说明:
***/
bool sd_media_all_file_total_get(int *total, int *new_total)
{
	bool reslut = true;
	pthread_mutex_lock(&file_list_mutex);
	if (is_media_sd_insert == false)
	{
		reslut = false;
		goto finish;
	}

	if (total != NULL)
	{
		// *total = sd_media_total;
		*total = sd_photo_total + sd_video_total;
	}
	if (new_total != NULL)
	{
		// *new_total = sd_media_new_total;
		*new_total = sd_photo_new_total + sd_video_new_total;
	}
finish:
	pthread_mutex_unlock(&file_list_mutex);
	return reslut;
}

/***
**   日期:2022-06-15 11:14:00
**   作者: leo.liu
**   函数作用：快速单独获取媒体文件数量
**   参数说明:
***/
int high_speed_media_file_total_get(file_type type)
{
	if (type == FILE_TYPE_NONE)
		return 0;

	int total = 0;
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	if (type == FILE_TYPE_VIDEO)
	{
		dir = opendir("/mnt/tf/video/");
		if (dir == NULL)
			return 0;
		while ((ptr = readdir(dir)) != NULL)
		{
			if (strstr(ptr->d_name, VIDEO_DOT) != NULL)
			{
				total++;
			}
		}
	}
	else if (type == FILE_TYPE_PHOTO)
	{
		dir = opendir("/mnt/tf/photo/");
		if (dir == NULL)
			return 0;
		while ((ptr = readdir(dir)) != NULL)
		{
			if (strstr(ptr->d_name, PHOTO_DOT) != NULL)
			{
				total++;
			}
		}
	}
	else if (type == FILE_TYPE_FLASH_PHOTO)
	{
		dir = opendir("/app/data/photo/");
		if (dir == NULL)
			return 0;
		while ((ptr = readdir(dir)) != NULL)
		{
			if (strstr(ptr->d_name, PHOTO_DOT) != NULL)
			{
				total++;
			}
		}
	}
	else if (type == FILE_TYPE_CALL) // 新增call类型快速统计
	{
		dir = opendir("/mnt/tf/call/");
		if (dir == NULL)
			return 0;
		while ((ptr = readdir(dir)) != NULL)
		{
			if (strstr(ptr->d_name, CALL_DOT) != NULL)
			{
				total++;
			}
		}
	}
	else if (type == FILE_TYPE_FLASH_CALL) // 新增flash call快速统计
	{
		dir = opendir(FLASH_CALL_PATH);
		if (dir == NULL)
			return 0;
		while ((ptr = readdir(dir)) != NULL)
		{
			if (strstr(ptr->d_name, CALL_DOT) != NULL)
			{
				total++;
			}
		}
		closedir(dir);
	}
	closedir(dir);
	printf("media file total:[%d]\n\r", total);
	return total;
}
