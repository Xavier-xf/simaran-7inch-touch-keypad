#!/bin/sh
#在应用层只需修改app分区即可



#创建一个用于保存升级文件的目录
create_platform()
{
	if [ -d "platform" ]; then
                echo "remove platform dir"
                # rm -rf "platform"
                # mkdir "platform"
        else
                echo "mkdir platform"
                mkdir "platform"
        fi
}

#用mksquashfs工具将app目录打包成 app.sqsh4 文件系统
make_squashfs_images()
{
        ./tools/mksquashfs ./app platform/app.sqsh4 -noappend -comp xz
}

#把 升级脚本 和 进度条显示程序 也放进去打包压缩
images_compress()
{
        cp scripts/update.sh                    platform/
        cp upgrade_progress/upgrade_progress    platform/

        rm -rf SAT_TABAOS
        cd platform/
        tar -zcvf SAT_TABAOS *
        mv SAT_TABAOS ../
        cd ../
}
# 文件处理：删除目标文件并拷贝新文件
handle_files() {
    # 源目录和目标目录
    local source_dir=/home/xiaoxiao/workspace/SIMARAN/SIMARAN_1070/upgrade/app
    local dest_dir=/home/xiaoxiao/workspace/SIMARAN/AK37E_SDK_V1.05/rootfs/resource/app/app
    
    # 用空格分隔文件列表
    local files="rom.bin sat_leo.ttf TABA.BIN persian.ttf"

    echo "开始处理文件拷贝..."

    # 检查目录是否存在
    [ ! -d "$source_dir" ] && { echo "错误：源目录 $source_dir 不存在"; exit 1; }
    [ ! -d "$dest_dir" ] && { echo "错误：目标目录 $dest_dir 不存在"; exit 1; }

    # 删除目标目录文件
    for file in $files; do
        [ -f "$dest_dir/$file" ] && { echo "删除 $dest_dir/$file"; rm -f "$dest_dir/$file"; }
    done

    # 拷贝源文件到目标目录
    for file in $files; do
        if [ -f "$source_dir/$file" ]; then
            echo "拷贝 $source_dir/$file 到 $dest_dir"
            cp -f "$source_dir/$file" "$dest_dir/" || { echo "拷贝 $file 失败"; exit 1; }
        else
            echo "警告：源文件 $source_dir/$file 不存在，跳过"
        fi
    done
}

# 执行外部make_image.sh脚本
execute_external_script() {
        local external_script="/home/xiaoxiao/workspace/SIMARAN/AK37E_SDK_V1.05"
    
        echo "开始执行外部脚本：$external_script"


        cd $external_script
        # 执行外部脚本
        ./auto_build.sh -i
        cd /home/xiaoxiao/workspace/SIMARAN/SIMARAN_1070
        echo "外部脚本执行完成"
}

create_platform

make_squashfs_images

images_compress

# 执行文件处理
handle_files

execute_external_script  # 拷贝完成后执行外部脚本
