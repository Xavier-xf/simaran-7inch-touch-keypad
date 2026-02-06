
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <mqueue.h>
#include <ctype.h>

#include "ak_common_graphics.h"
#include "ak_common.h"
#include "ak_tde.h"

#include "layout_define.h"
#include "lv_msg_event.h"
#include "ringplay.h"
#include "user_common.h"
extern void lv_init(void);
extern void lv_display_init(void);
extern void lv_port_indev_init(void);

extern void upgrade_check_firmware(void);

// 快速进入系统的标志位
bool LEO_FAST_ENTER_SYSTEM_FLAG = false;

/***
** 日期: 2022-04-22 15:31
** 作者: leo.liu
** 函数作用：打印当前项目的相关信息
** 返回参数说明：
***/
static void project_printf(void)
{
	printf("********************************\n");
	printf("***** project: CDV1000QT   *****\n");
	printf("***** author:  leo		   *****\n");
	printf("***** date:    2022/04/22  *****\n");
	printf("********************************\n");

	system("insmod /usr/modules/ak_fb.ko");
	system("insmod /usr/modules/ak_gui.ko");
	system("insmod /usr/modules/ts_gsl.ko");
}

static void start_daemon_process(void)
{
	FILE *fp = popen("ps aux | grep daemon_process.sh | grep -v grep | wc -l", "r");
	char buffer[3] = {0};
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	if (buffer[0] != '1')
	{
		system("killall daemon_process.sh");
		system("/usr/sbin/daemon_process.sh &");
	}
}

/***
** 日期: 2022-04-23 08:12
** 作者: leo.liu
** 函数作用：平台接口初始化
** 返回参数说明：
***/
static void platform_sdk_init(void)
{
	sdk_run_config config;

	memset(&config, 0, sizeof(sdk_run_config));
	ak_sdk_init(&config);
	/***** 打开tde 模块 ******/
	ak_tde_open();
}

static void *lv_sys_tick_task(void *arg)
{
	struct timeval tv1, tv2;
	gettimeofday(&tv2, NULL);

	while (1)
	{
		gettimeofday(&tv1, NULL);
		lv_tick_inc(tv1.tv_sec * 1000 + tv1.tv_usec / 1000 - tv2.tv_sec * 1000 - tv2.tv_usec / 1000);
		gettimeofday(&tv2, NULL);
		usleep(1000);
	}

	return NULL;
}

/***
** 日期: 2022-04-25 14:22
** 作者: leo.liu
** 函数作用：lvgl的任务调度以及心跳包线程
** 返回参数说明：
***/
static void lv_task_scheduling_start(void)
{
	pthread_t task_id;

	goto_layout(pLAYOUT(logo));
	pthread_create(&task_id, user_pthread_atter_get(), lv_sys_tick_task, NULL);

	while (1)
	{
		lv_task_handler();
		usleep(1000);
	}
}

/***
** 日期: 2022-04-22 15:30
** 作者: leo.liu
** 函数作用：主函数入口
** 返回参数说明：
***/
int main(int argc, char *argv[])
{
	project_printf();

	/***** 根据参数决定是否快速进入系统 *****/
	if (argc == 2)
	{
		if (strcmp(argv[1], "leo") == 0)
		{
			LEO_FAST_ENTER_SYSTEM_FLAG = true;
		}
	}

	/***** 启动守护进程 *****/
	start_daemon_process();
	/******** 将内部电容值4pF改为6pF，在4pF的时候会出现时间快过网络时间 *******/
	system("devmem 0x0800021C w 0x81F302DF");

	/***** 获取用户数据 *****/
	user_data_init();

	/***** 固件升级 *****/
	// upgrade_check_firmware();

	/***** 升级完成，复位标志位 *****/
	// user_data_get()->upgrade_success_flag = false;
	// user_data_save();

	/***** 初始化安凯sdk *****/
	platform_sdk_init();

	/***** 设置字库 *****/
	extern void lv_ft_font_set_type(int type);
	lv_ft_font_set_type(user_data_get()->setting.language);

	/***** 初始化LVGL *****/
	lv_init();
	lv_display_init();
	lv_port_indev_init();

	/***** 开启app系统调度 *****/
	lv_event_task_init();
	lv_task_scheduling_start();
	return 0;
}
