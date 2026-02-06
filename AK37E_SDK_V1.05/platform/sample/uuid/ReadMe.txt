ak_uuid_sample测试

Efuse 模块用来读取和设置芯片的唯一ID,唯一ID分为global_ID和custom两个区(37D和330没有custom分区)
本测试用例用来测试global分区
测试步骤:
Step1:
insmod /usr/modules/ak_efuse.ko，成功产生设备节点/dev/efuse

Step2:
   通过cat /sys/kernel/efuse/efuse_OR命令查看当前芯片的id

Step3：
   ./ak_uuid_sample

对比step2和step3的数据，查看step3获取到的数据是否和step2查看到的数据一致