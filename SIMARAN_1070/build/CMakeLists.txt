cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_C_COMPILER "arm-anykav500-linux-uclibcgnueabi-gcc")
set(CMAKE_CXX_COMPILER "arm-anykav500-linux-uclibcgnueabi-g++")
# set(CMAKE_C_COMPILER "/home/wxj/LiuJun/meiou_AK37D/tools/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc")
# set(CMAKE_CXX_COMPILER "/home/wxj/LiuJun/meiou_AK37D/tools/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-g++")

# 设置编译器，此处为ARM平台，如果你安装了编译链就用名称，没安装请用绝对路径
set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../)

SET(leo_proj TABA.BIN)
project(${leo_proj})

# 工程名
SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -Os -s -Wall -std=gnu11  -Werror")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Os  -s -Wall -std=c++1y -Werror")

include_directories(
	${PROJECT_PATH}include/
	${PROJECT_PATH}include/anyka37d
	${PROJECT_PATH}include/common
	${PROJECT_PATH}include/tuya
	${PROJECT_PATH}share/
	${PROJECT_PATH}share/include
	${PROJECT_PATH}share/lvgl
	${PROJECT_PATH}common/
	${PROJECT_PATH}layout/
	${PROJECT_PATH}res/
)

# 添加src目录
aux_source_directory(${PROJECT_PATH}layout/ DIR_SRCS)

add_subdirectory(${PROJECT_PATH}common ${CMAKE_CURRENT_SOURCE_DIR}/common/)
add_subdirectory(${PROJECT_PATH}share ${CMAKE_CURRENT_SOURCE_DIR}/share/)

set(VAR ${VAR} lvgl)
set(VAR ${VAR} common)
set(VAR ${VAR} freetype)
set(VAR ${VAR} mad)

# set(VAR ${VAR} xls)
# set(VAR ${VAR} tuya)
set(VAR ${VAR} zlib)
add_executable(${leo_proj} ${DIR_SRCS})
message("VAR : ${VAR}")
target_link_libraries(${leo_proj} ${VAR})

# target_link_libraries(${leo_proj} m)
# target_link_libraries(${leo_proj} pthread)
# target_link_libraries(${leo_proj} dl)
set(ANYKA_LIB_PATH ${PROJECT_PATH}/lib/anyka37d/)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_tde.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_mem.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_osal.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_thread.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_common.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_log.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_dbg.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_ao.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_vqe.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libakaudiofilter.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libmpi_vdec.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libakv_decode.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_vi.so)

# target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_vpss.so)
# target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_isp_sdk.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libmpi_venc.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libakv_encode.so)
target_link_libraries(${leo_proj} ${ANYKA_LIB_PATH}/libplat_ai.so)

#
#set(ANYKA_LIB_PATH ${PROJECT_PATH}/lib/tuya/)
#target_link_libraries(${leo_proj} -Wl,--start-group
#	${ANYKA_LIB_PATH}/libtuya_ipc.a
#	${ANYKA_LIB_PATH}/libmbedtls.a
#	tuya
#	xls
#	-Wl,--end-group)
