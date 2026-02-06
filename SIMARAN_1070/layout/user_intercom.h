#ifndef USER_INTERCOM_H_
#define USER_INTERCOM_H_
#include <stdbool.h>
#include "user_data.h"
#include "user_monitor.h"
#define CODE_START 0x1F // start cmd
#define CODE_STOP 0XF1	// stop  cmd

#define GUARD_INTERCOM_NUMBER 0xFFFF//警卫机1号码
#define GUARD_2_INTERCOM_NUMBER 0xFFFE//警卫机2号码

/////////////////////////////	cmd		//////////////////////////////////////////////


//本机号码
#define OwnID (user_data_get()->device_id)

typedef enum
{
    INTERCOM_STATE_IDLE = 0x00,
    INTERCOM_STATE_CALL,
    INTERCOM_STATE_CALLING_IN,
    INTERCOM_STATE_CALLING_OUT,
    INTERCOM_STATE_TALKING,
    INTERCOM_STATE_HUNG_UP,
}intercom_state_t;

// interphone call interphone
#define CMD_INTERPHONE_CALL 0XA0
#define CMD_INTERPHONE_ANSWER 0XA1
#define CMD_INTERPHONE_TALKING 0XA2
#define CMD_INTERPHONE_QUIT 0XA3
#define CMD_INTERPHONE_LINE_BUSY 0XA4
#define CMD_INTERPHONE_UNIT_BUSY 0XA5
#define CMD_INTERPHONE_BUSY_CHECK 0XA6


bool intercom_init(void);

void rand_delay(void);

int intercom_number_get(void);

void intercom_number_set(int num);

intercom_state_t intercom_state_get(void);

void intercom_state_set(intercom_state_t state);

bool intercom_line_busy_state_get(void);

bool intercom_unit_busy_state_get(void);

#endif