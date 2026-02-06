cmd_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb := mkdir -p arch/arm/boot/dts/ ; /opt/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.pre.tmp -nostdinc -I/home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts -I/home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include -I/home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.dts.tmp /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb -b 0 -i /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/  -d arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.dtc.tmp arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.dts.tmp ; cat arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.pre.tmp arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.dtc.tmp > arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d

source_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb := /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dts

deps_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb := \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_ak37e.dtsi \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_ak37e_common.dtsi \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include/dt-bindings/clock/ak37e-clock.h \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_ak37e_pinctrl.dtsi \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include/dt-bindings/pinctrl/ak_37e_pinctrl.h \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_norflash.dtsi \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_nandflash.dtsi \
  /home/xiaoxiao/workspace/EP-Linux/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_lcd.dtsi \

arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb: $(deps_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb)

$(deps_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb):
