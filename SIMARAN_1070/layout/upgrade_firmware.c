#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "user_data.h"

#define UPGRADE_PACKAGE_FILE "/mnt/tf/SAT_TABAOS"

void upgrade_check_firmware(void)
{
	if(user_data_get()->upgrade_success_flag)
	{
		printf("================>>>已升级\n");
		return;
	}
	system("mkdir /mnt/tf");
	system("mount /dev/mmcblk0 /mnt/tf/");
	if (access(UPGRADE_PACKAGE_FILE, F_OK) != 0)
	{
		printf("================>>>无升级文件\n");
		return;
	}
	printf("================>>>开始升级\n");
	/***** 开始升级，true标志位 *****/
	user_data_get()->upgrade_success_flag = true;
	user_data_save();
	system("cp -f " UPGRADE_PACKAGE_FILE " /tmp");
	system("tar -zxvf /tmp/SAT_TABAOS -C /tmp");
	system("cd /tmp;./update.sh");
	printf("================>>>升级结束\n");
}