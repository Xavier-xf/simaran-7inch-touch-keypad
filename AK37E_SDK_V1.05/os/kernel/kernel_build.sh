CROSS_COMPILE="/opt/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-"
mkdir ../kbd
make O=../kbd/ anycloud_ak37e_taba_defconfig CROSS_COMPILE=$CROSS_COMPILE
make O=../kbd/ dtbs  modules uImage -j8 CROSS_COMPILE=$CROSS_COMPILE
