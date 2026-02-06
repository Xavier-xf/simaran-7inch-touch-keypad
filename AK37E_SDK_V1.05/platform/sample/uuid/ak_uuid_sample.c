#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <malloc.h>
#include <unistd.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include "ak_uuid.h"

#ifdef AK_RTOS
#include <kernel.h>
#endif

/**
 * Preconditions:
 * 1. T card is already mounted
 * 2. mic or linein must ready
 */
#ifdef AK_RTOS
static int uuid_sample(int argc, char *argv[])
#else
int main(int argc, char **argv)
#endif
{
    /* start the application */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );
    
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    ak_print_normal(MODULE_ID_APP, "** uuid demo version: %s **\n", ak_uuid_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    
    int length = -1;
    int ret =-1;
    
    /* uuid open */
    ret = ak_uuid_open();
    if(ret < 0){
        ak_print_error_ex(MODULE_ID_APP," ak_uuid_open failed\n");
        goto exit;
    }
    unsigned char* global_ptr = NULL;
    ret = ak_uuid_get_global_id(&global_ptr);          // 获取全球id数据
    ret = ak_uuid_get_global_id_length(&length);       // 获取全球id长度
     ak_print_notice_ex(MODULE_ID_APP,"global length= %d\n",length);
    if(ret < 0 ){
        goto exit;
    }
    
    ak_print_notice_ex(MODULE_ID_APP,"global_id=");
    for(int i =0;i< length;i++){
        printf("\033[1;33m %x\033[m", global_ptr[i]);
    }
    printf("\n");

    /* close uuid */
    ak_uuid_close();
exit:
    ak_sdk_exit();
    return ret;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(uuid_sample, ak_uuid_sample, UUID Sample);
#endif