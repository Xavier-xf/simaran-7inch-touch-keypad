#!/bin/sh

# 运行flash里的app
run_flash_app()
{
    echo "run flash app"
    /app/app/TABA.BIN &
    # echo 0 > /app/data/update_flag
    echo "finish"
    exit
}

# 挂载sd卡到/mnt/tf/目录下
mount_sd_card()
{
    if [ -e /dev/mmcblk0 ]; then
        echo "SD card inserted, starting to mount"
        mkdir /mnt/tf
        mount /dev/mmcblk0 /mnt/tf/
    else    
        echo "SD card not inserted"
        run_flash_app
    fi
}

# 检测升级包是否存在，存在则启动升级脚本
upgrade()
{
    # if [ -e /mnt/tf/SAT_TABAOS ]; then
    #     echo "Upgrade package exists, starting upgrade"
    #     cp -f /mnt/tf/SAT_TABAOS /tmp
    #     tar -zxvf /tmp/SAT_TABAOS -C /tmp
    #     cd /tmp
    #     echo 1 > /app/data/update_flag
    #     ./update.sh
    # else
    #     echo "Upgrade package does not exist"
        run_flash_app
    # fi
}



# /app/data/update_flag 文件是升级的标志，文件不存在 或 内容为1 表示已升级
if [ -e /app/data/update_flag ]; then

    update_flag=$(cat /app/data/update_flag)
    if [ $update_flag -eq 1 ]; then
        echo "Upgraded"
        run_flash_app
    else
        echo "Not upgraded"
        sleep 1 #延时1s等待sd卡设备节点产生
        mount_sd_card
        upgrade
    fi
else
    touch /app/data/update_flag
    run_flash_app
fi


