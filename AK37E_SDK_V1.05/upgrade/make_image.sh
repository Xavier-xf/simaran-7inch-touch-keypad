#!/bin/sh

upgrade_bin_version=$(date +"%Y%m%d%H%M%S")
export upgrade_bin_version

# 制作根文件系统，拷贝库、脚本、驱动等文件到根文件系统下
make_rootfs()
{
        cd ../rootfs/
        ./rootfs_build.sh
        cd -
}

# #创建一个用于保存升级文件的目录
create_platform()
{
	if [ -d "platform" ]; then
                echo "remove platform dir"
                rm -rf "platform"
                mkdir "platform"
        else
                echo "mkdir "rootfs""
                mkdir "platform"
        fi
}

# 制作config、tuya、data可读写分区的镜像文件
make_jffs2_images()
{
        ./mkfs.jffs2 -d ../rootfs/rootfs/etc/config -s 256 -e 4096 -m none -o platform/config.jffs2
        ./mkfs.jffs2 -d ../rootfs/rootfs/app/tuya -s 256 -e 4096 -m none -o platform/tuya.jffs2
        ./mkfs.jffs2 -d ../rootfs/rootfs/app/data -s 256 -e 4096 -m none -o platform/data.jffs2
}

# 制作usr、app、root只读分区的镜像文件
make_squashfs_images()
{
        mkdir -p tmpfs
        cp -af ../rootfs/rootfs/* tmpfs

        ./mksquashfs tmpfs/usr platform/usr.sqsh4 -noappend -comp xz
        ./mksquashfs tmpfs/app/app platform/app.sqsh4 -noappend -comp xz

        rm -rf tmpfs/usr/* tmpfs/etc/config/* tmpfs/app/tuya/* tmpfs/app/data/* tmpfs/app/app/*
        ./mksquashfs tmpfs/ platform/root.sqsh4 -noappend -comp xz

        rm -rf tmpfs
}

# 拷贝 uboot、设备树、uImage 到升级目录 platform/ 下
copy_uboot_kernel_images()
{
        cp  ../os/ubd/u-boot.bin                                                platform/
        cp  ../os/kbd/arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_EP.dtb        platform/
        cp  ../os/kbd/arch/arm/boot/uImage                                      platform/
}

# 拷贝 分区表、logo文件 到升级目录 platform/ 下
copy_env_logo_images()
{
        cd ../tools/envtool/
        rm env.img
        cp env_ak3760e_nor.cfg env_ak3760e_nor_tmp.cfg
        echo "uprade_image_version $upgrade_bin_version" >>env_ak3760e_nor_tmp.cfg
        tr '\000' '\377' </dev/zero | dd of=./env.img bs=1024 count=4
        ./fw_setenv -s env_ak3760e_nor_tmp.cfg
        ./fw_printenv
        cp env.img     ../../upgrade/platform/env_ak3760e_nor.img
        cd -
        cp logo/ep_logo.rgb           platform/
}

# 把 升级脚本 和 进度条显示程序 也放进去打包压缩
images_compress()
{
        # cp ../rootfs/scripts/flash/update.nor.sh        platform/update.sh
        # cp upgrade_progress/upgrade_progress            platform/
        # rm -rf SAT_TABA2070OS
        # cd platform/
        # tar -zcvf SAT_TABA2070OS *
        # mv SAT_TABA2070OS ../
        # cd ../
        ./partition_image.sh
}

make_rootfs

create_platform

make_jffs2_images

make_squashfs_images

copy_uboot_kernel_images

copy_env_logo_images

images_compress


