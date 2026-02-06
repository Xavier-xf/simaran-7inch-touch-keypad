```markdown
# simaran-7inch-touch-keypad

基于 AK37E 平台的 7寸触摸按键设备项目（SIMARAN）

## 项目结构说明

- `AK37E_SDK_V1.05/`          厂商基础 SDK（内核、uboot、文件系统打包工具等）
- `SIMARAN_1070/`             应用层代码（UI、业务逻辑、资源文件等）
- `SIMARAN_1070.tar.gz`       应用层代码压缩包（历史备份）

## 编译环境准备

1. 操作系统：Ubuntu 18.04 / 20.04 推荐（其他版本可能需要调整工具链兼容性）
2. 安装交叉编译工具链（已包含在 SDK 内）
   路径示例：`/opt/arm-anykav500-linux-uclibcgnueabi/`
3. 确保有足够的磁盘空间（建议 ≥ 50GB）

## 内核 / 系统编译步骤（底层固件）

位置：`AK37E_SDK_V1.05/`

常用一键编译脚本：`auto_build.sh`

支持的常用参数：

```bash
# 编译内核
./auto_build.sh -k

# 编译 uboot
./auto_build.sh -u

# 制作环境分区表（env）
./auto_build.sh -e

# 制作文件系统镜像 + 更新 USB 烧录文件
./auto_build.sh -i

# 全流程编译（最常用）
./auto_build.sh -all
```

**完整编译一次底层固件建议执行：**

```bash
cd AK37E_SDK_V1.05
./auto_build.sh -all
```

完成后会在 `upgrade/platform/` 目录下生成烧录所需的文件。

## 应用层编译与打包

位置：`SIMARAN_1070/`

主要产物：`TABA.BIN`（主程序） + 资源文件

编译步骤：

```bash
# 进入应用层目录
cd SIMARAN_1070

# 清理旧的构建文件（可选）
make clean

# 编译应用（使用 cmake + make）
make

# 编译完成后会自动：
# 1. 把 TABA.BIN + 字体 + 资源 等拷贝到 upgrade/app/
# 2. 调用 make_image.sh 打包成 app.sqsh4
# 3. 把需要更新的文件同步到 SDK 的 rootfs 目录
# 4. 调用 SDK 的 auto_build.sh -i 更新镜像
```

**最常用的一条命令：**

```bash
cd SIMARAN_1070
make
```

执行完成后，新的 `app.sqsh4` 已经被更新到 SDK 目录，接下来只需要执行：

```bash
cd ../AK37E_SDK_V1.05
./auto_build.sh -i
```

即可更新可烧录的完整固件镜像。

## 快速修改应用后重新打包流程（推荐）

```bash
# 1. 修改代码 / 资源
# 2. 进入应用目录编译 & 打包
cd SIMARAN_1070
make

# 3. 更新系统镜像（只更新 app 分区部分）
cd ../AK37E_SDK_V1.05
./auto_build.sh -i
```

## 生成的最终烧录文件

位置：`AK37E_SDK_V1.05/upgrade/platform/`

主要文件：

- `u-boot.bin`
- `uImage`
- `EVB_CBDM_AK3760E_V1.0.1_EP.dtb`
- `env_ak3760e_nor.img`
- `root.sqsh4` / `usr.sqsh4` / `app.sqsh4`
- `config.jffs2` / `data.jffs2` / `tuya.jffs2`
- `ep_logo.rgb`
- ...

烧录工具一般使用 SDK 自带的 `tools/burntool/` 相关工具。

## 注意事项

- 交叉编译工具链路径不要修改，脚本中写死了
- 每次编译应用层后建议都执行一次 `./auto_build.sh -i`
- 完整固件编译耗时较长，日常开发主要改动应用层即可
- 如需只更新应用，可直接使用 `app.sqsh4` 进行分区升级

祝开发顺利～

```
