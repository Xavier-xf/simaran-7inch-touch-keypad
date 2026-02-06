#!/bin/sh
# 一键自动编译打包脚本
# 支持通过选项选择不同的编译操作

# 编译uboot函数
build_uboot() {
    echo "开始编译uboot..."
    cd os/uboot/ || { echo "uboot目录不存在"; return 1; }
    ./uboot_build.sh || { echo "uboot编译失败"; return 1; }
    cd - || return 1
    echo "uboot编译完成"
}

# 编译内核函数
build_kernel() {
    echo "开始编译内核..."
    cd os/kernel/ || { echo "kernel目录不存在"; return 1; }
    ./kernel_build.sh || { echo "内核编译失败"; return 1; }
    cd - || return 1
    echo "内核编译完成"
}

# 制作分区表函数
make_env() {
    echo "开始制作分区表..."
    cd tools/envtool/ || { echo "envtool目录不存在"; return 1; }
    ./build.sh || { echo "分区表制作失败"; return 1; }
    cd - || return 1
    echo "分区表制作完成"
}

# 编译文件系统并打包函数
make_image() {
    echo "开始编译文件系统并打包..."
    cd upgrade || { echo "upgrade目录不存在"; return 1; }
    ./make_image.sh || { echo "文件系统编译打包失败"; return 1; }
    cd - || return 1
    
    echo "更新USB烧录文件..."
    echo "删除旧的usb烧录文件"
    rm -rf tools/burntool/platform || { echo "删除旧文件失败"; return 1; }
    
    echo "拷贝新的usb烧录文件到tools/burntool"
    cp -rf upgrade/platform tools/burntool/ || { echo "拷贝新文件失败"; return 1; }
    echo "文件系统打包及烧录文件更新完成"
}

# 执行所有操作
build_all() {
    echo "开始执行全量编译..."
    build_uboot || return 1
    build_kernel || return 1
    make_env || return 1
    make_image || return 1
    echo "全量编译完成"
}

# 显示使用说明
usage() {
    echo "========================================================"
    echo "Usage :"
    echo "    ./auto_build.sh -k     编译内核"
    echo "    ./auto_build.sh -u     编译uboot"
    echo "    ./auto_build.sh -i     制作镜像及更新烧录文件"
    echo "    ./auto_build.sh -all   执行所有编译操作"
    echo "    ./auto_build.sh -e     制作环境分区表"
    echo "========================================================"
}

# 主函数
main() {
    local option=$1
    
    # 检查是否提供了选项
    if [ $# -ne 1 ]; then
        echo "请提供一个选项参数"
        usage
        return 1
    fi

    case $option in
        "-k")
            build_kernel
            ;;
        "-u")
            build_uboot
            ;;
        "-i")
            make_image
            ;;
        "-e")
            make_env
            ;;
        "-all")
            build_all
            ;;
        *)
            echo "无效的选项: $option"
            usage
            return 1
            ;;
    esac
}

# 调用主函数
main "$@"
