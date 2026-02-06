#include "user_intercom.h"
#include <pthread.h>
#include "uart_ctrl.h"
#include "lv_msg_event.h"
#include "user_standby.h"
#include "user_gpio.h"
#include "layout_define.h"
#include "ringplay.h"
#include "tuya/tuya_api.h"
#include "user_common.h"
#include <time.h>

typedef bool (*_InterPhoneReceiveEventHanle)(unsigned int send_id);

typedef struct
{
	char str[30];
	unsigned char cmd;
	_InterPhoneReceiveEventHanle handler;
} intercom_param_info;

typedef struct
{
	uint16_t send_id;
	uint16_t recv_id;
	char cmd;
	intercom_state_t state; // 现在没啥用
} intercom_trans_info;

static bool intercom_event_cmd_process(unsigned int send_id, unsigned int cmd);
static void intercom_standard_format_send(unsigned int receive_id, unsigned char cmd);

/*******************************************************************
 * @brief  : 串口读写句柄
 *******************************************************************/
static int intercom_uart_fd = -1;

/*******************************************************************
 * @brief  : 本机intercom状态
 *******************************************************************/
static intercom_state_t intercom_state = INTERCOM_STATE_IDLE;

/*******************************************************************
 * @brief  : 与自己建立通讯的号码
 *******************************************************************/
static int intercom_number = 0;

/*******************************************************************
 * @brief  : 忙线标志，有别的机器占线了
 *******************************************************************/
static bool intercom_line_is_busy = false;

/*******************************************************************
 * @brief  : 单元忙标志，对方进入监控了
 *******************************************************************/
static bool intercom_unit_is_busy = false;

/*******************************************************************
 * @brief  : 命令可发送的次数，限制发送的次数，预防总线拥堵
 *******************************************************************/
static int send_cmd_count = 0;

/*******************************************************************
 * @brief  : 总线接收到的信息
 *******************************************************************/
static intercom_trans_info recv_info;

/*******************************************************************
 * @brief  : 最后一次建立通讯的状态是 呼进
 *******************************************************************/
bool intercom_call_in_flag = false;

/***
**   日期:2022-05-31 08:39:46
**   作者: leo.liu
**   函数作用：发送命令处理
**   参数说明:
***/
static void intercom_uart_event_send(unsigned int id, unsigned int cmd)
{
	lv_msg_send_cmd(MSG_EVENT_CMD_INTERCOM, id, cmd);
}

/***
**   日期:2022-05-31 08:22:16
**   作者: leo.liu
**   函数作用：校验intercom和
**   参数说明:
***/
static bool intercom_uart_check_sum(char *buf)
{
	recv_info.send_id = (((uint16_t)buf[0] << 8) & 0xFF00) + ((uint16_t)buf[1] & 0xFF);
	recv_info.recv_id = (((uint16_t)buf[2] << 8) & 0xFF00) + ((uint16_t)buf[3] & 0xFF);
	recv_info.cmd = buf[4];
	recv_info.state = buf[5];
	unsigned char checknum = (recv_info.send_id + recv_info.recv_id + recv_info.cmd + recv_info.state) & 0xFF;
	// printf("===========>>> send_id:[%d] recv_id:[%d] \n", send_id, recv_id);

	if (checknum != buf[6]) // 和检验错误
	{
		printf("checksum error : check number error \n\r");
		return false;
	}
	else if (recv_info.cmd == CMD_INTERPHONE_BUSY_CHECK) // 忙检测命令
	{
		if (intercom_state != INTERCOM_STATE_IDLE && intercom_call_in_flag == false && !intercom_line_is_busy) // 自己在占线中，返回Line Busy
		{
			if (((OwnID == GUARD_INTERCOM_NUMBER || intercom_number == GUARD_INTERCOM_NUMBER) &&
				 (recv_info.send_id == GUARD_INTERCOM_NUMBER || recv_info.recv_id == GUARD_INTERCOM_NUMBER)) ||
				((OwnID == GUARD_2_INTERCOM_NUMBER || intercom_number == GUARD_2_INTERCOM_NUMBER) &&
				 (recv_info.send_id == GUARD_2_INTERCOM_NUMBER || recv_info.recv_id == GUARD_2_INTERCOM_NUMBER)))
			{
				printf("intercom guard bus is busy \n");
				intercom_standard_format_send(recv_info.send_id, CMD_INTERPHONE_LINE_BUSY);
				return false;
			}
			else if (((OwnID != GUARD_INTERCOM_NUMBER && intercom_number != GUARD_INTERCOM_NUMBER) &&
					  (recv_info.send_id != GUARD_INTERCOM_NUMBER && recv_info.recv_id != GUARD_INTERCOM_NUMBER)) ||
					 ((OwnID != GUARD_2_INTERCOM_NUMBER && intercom_number != GUARD_2_INTERCOM_NUMBER) &&
					  (recv_info.send_id != GUARD_2_INTERCOM_NUMBER && recv_info.recv_id != GUARD_2_INTERCOM_NUMBER)))
			{
				printf("intercom user bus is busy \n");
				intercom_standard_format_send(recv_info.send_id, CMD_INTERPHONE_LINE_BUSY);
				return false;
			}
		}
		else if (cur_layout_get() == pLAYOUT(camera) && recv_info.recv_id == OwnID) // 自己在监控中，返回Unit Busy
		{
			intercom_standard_format_send(recv_info.send_id, CMD_INTERPHONE_UNIT_BUSY);
			return false;
		}
		if (intercom_state != INTERCOM_STATE_IDLE && recv_info.recv_id == OwnID && !intercom_line_is_busy) // 自己在，返回Line Busy
		{
			printf("intercom own is busy \n");
			intercom_standard_format_send(recv_info.send_id, CMD_INTERPHONE_LINE_BUSY);
			return false;
		}
		return false;
	}
	else if (recv_info.cmd == CMD_INTERPHONE_LINE_BUSY && recv_info.recv_id == OwnID && intercom_state == INTERCOM_STATE_CALL) // 收到忙线命令
	{
		intercom_line_is_busy = true;
		intercom_state = INTERCOM_STATE_IDLE;
		printf("================>>> line busy\n");
		return false;
	}
	else if (recv_info.cmd == CMD_INTERPHONE_UNIT_BUSY && recv_info.recv_id == OwnID && intercom_state == INTERCOM_STATE_CALL) // 收到单元忙命令
	{
		intercom_unit_is_busy = true;
		intercom_state = INTERCOM_STATE_IDLE;
		printf("================>>> unit busy\n");
		return false;
	}
	else if (recv_info.send_id == OwnID) // 发送方和自己同房号
	{
		printf(" check error : send id is OwnID: %d \n", OwnID);

		switch (recv_info.cmd)
		{
		case CMD_INTERPHONE_QUIT: // 队友退出响铃状态，我也退出
			if (intercom_number == recv_info.recv_id)
				intercom_state = INTERCOM_STATE_IDLE;
			break;
		case CMD_INTERPHONE_ANSWER: // 队友进入响铃状态，我也进入
			if (intercom_state == INTERCOM_STATE_IDLE &&
				intercom_state != INTERCOM_STATE_CALLING_IN &&
				intercom_state != INTERCOM_STATE_TALKING)
				intercom_uart_event_send(recv_info.recv_id, CMD_INTERPHONE_CALL);
			break;
		case CMD_INTERPHONE_TALKING: // 队友进入通话状态，我退出
			if (intercom_state != INTERCOM_STATE_TALKING && intercom_number == recv_info.recv_id)
				intercom_state = INTERCOM_STATE_IDLE;
			break;
		default:
			break;
		}

		return false;
	}
	else if (recv_info.recv_id != OwnID) // 接收方不是自己
	{
		return false;
	}

	return true;
}
/***
**	 日期:2022-05-31 08:16:24
**	 作者: leo.liu
**	 函数作用：读取串口格式命令
**	 参数说明:
***/
static bool intercom_cmd_read(char *buffer)
{
	if (uart_read(intercom_uart_fd, &buffer[0], 1) < 0)
	{
		return false;
	}
	if ((buffer[0] == CODE_START) && (uart_read(intercom_uart_fd, &buffer[1], 8) == 8))
	{
		if ((buffer[8] == CODE_STOP) && (intercom_uart_check_sum(&buffer[1]) == true))
		{
			return true;
		}
	}
	return false;
}

/***
**   日期:2022-05-31 08:27:58
**   作者: leo.liu
**   函数作用：处理户户通命令
**   参数说明:
***/
void intercom_uart_cmd_proc(const char *buf)
{
	char cmd = buf[5];
	if (intercom_state != INTERCOM_STATE_IDLE && intercom_state != INTERCOM_STATE_CALL && recv_info.send_id != intercom_number) // 发送方还没有跟自己建立通讯
	{
		printf("===============>>> 当前通话设备:[%d] 插线设备:[%d] 当前状态:[%d]\n", intercom_number, recv_info.send_id, intercom_state);
		return;
	}
	intercom_uart_event_send(recv_info.send_id, cmd);
}

static void intercom_cmd_send_event_handler(void)
{
	static unsigned long long prev_ts;
	static int delay = 200;
	unsigned long long curr_ts = user_timestamp_get();

	if (abs(curr_ts - prev_ts) > delay)
	{
		delay = rand() % 200 + 100;
		prev_ts = curr_ts;
		switch (intercom_state)
		{
		case INTERCOM_STATE_CALL:
			intercom_call_in_flag = false;
			if (send_cmd_count > 0)
			{
				intercom_standard_format_send(intercom_number, CMD_INTERPHONE_BUSY_CHECK);
			}
			else
			{
				intercom_standard_format_send(intercom_number, CMD_INTERPHONE_CALL);
			}
			break;
		case INTERCOM_STATE_CALLING_IN:
			if (send_cmd_count > 0)
				intercom_standard_format_send(intercom_number, CMD_INTERPHONE_ANSWER);
			break;
		case INTERCOM_STATE_CALLING_OUT:
			break;
		case INTERCOM_STATE_TALKING:
			if (intercom_call_in_flag && send_cmd_count > 0)
				intercom_standard_format_send(intercom_number, CMD_INTERPHONE_TALKING);
			break;
		case INTERCOM_STATE_HUNG_UP:
			if (send_cmd_count > 0)
				intercom_standard_format_send(intercom_number, CMD_INTERPHONE_QUIT);
			else
				intercom_state = INTERCOM_STATE_IDLE;
			break;
		case INTERCOM_STATE_IDLE:

			break;
		default:
			break;
		}
		// printf("====================>>> intercom_call_in_flag:[%d]\n", intercom_call_in_flag);
	}
}

/***
**   日期:2022-05-30 18:02:20
**   作者: leo.liu
**   函数作用：intercom接收任务
**   参数说明:
***/
static void *intercom_receive_task(void *arg)
{
	printf("***** intercom receive task create success*****\n");
	char buffer[9] = {0};
	srand((unsigned)time(NULL));
	rand_delay();

	while (1)
	{
		if (intercom_cmd_read(buffer) == true)
		{
			intercom_uart_cmd_proc(buffer);
		}
		intercom_cmd_send_event_handler();
		usleep(1000);
	}
	return NULL;
}
/***
**   日期:2022-05-30 17:54:02
**   作者: leo.liu
**   函数作用：初始化intercom
**   参数说明:
***/
bool intercom_init(void)
{
	intercom_uart_fd = uart_open("ttySAK2", 9600, 8, 1, 'n');
	if (intercom_uart_fd < 0)
	{
		printf("open ttySAK2 faild \n");
		usleep(1000 * 1000);
		return false;
	}
	pthread_t thread_t;
	layout_intercom_event_callback_register(intercom_event_cmd_process);
	pthread_create(&thread_t, user_pthread_atter_get(), intercom_receive_task, NULL);
	return true;
}
/************************************************************************ intercom 通用接口 ***********************************************************************/
static void intercom_standard_format_send(unsigned int receive_id, unsigned char cmd)
{
	send_cmd_count--;
	char buf[9];
	buf[0] = CODE_START;
	buf[1] = (OwnID >> 8) & 0xFF;
	buf[2] = OwnID & 0xFF;
	buf[3] = (receive_id >> 8) & 0xFF;
	buf[4] = receive_id & 0xFF;
	buf[5] = cmd;
	buf[6] = intercom_state;
	buf[7] = (OwnID + receive_id + cmd + intercom_state) & 0xff;
	buf[8] = CODE_STOP;
	rs485_send_recv_ctrl(0x01);
	uart_write(intercom_uart_fd, buf, 9);
	rs485_send_recv_ctrl(0x00);
	printf("===========>>> send_id:[%d] recv_id:[%d] cmd:[%d] checknum:[%d]\n", OwnID, receive_id, buf[5], buf[6]);
}
/************************************************************************** 接收处理范围 **************************************************************************/

/***
**   日期:2022-05-31 09:01:17
**   作者: leo.liu
**   函数作用：intercom 发送呼叫命令
**   参数说明:
***/
static bool intercom_call_receive_event(unsigned int send_id)
{
	printf("==========>>>> device [%d] call in\n", send_id);
	if (cur_layout_get() == pLAYOUT(camera)) // 没啥用
	{
		intercom_standard_format_send(send_id, CMD_INTERPHONE_UNIT_BUSY);
	}
	else if ((cur_layout_get() != pLAYOUT(intercom_out) && cur_layout_get() != pLAYOUT(intercom_in) && intercom_state == INTERCOM_STATE_IDLE && (user_data_get()->setting.intercom_receive_enable)) || (send_id == 65535))
	{
		call_record_start(false, 0, send_id);
		intercom_number = send_id;
		intercom_call_in_flag = true;
		usleep(400 * 1000);
		if (hook_state_get() == true)
		{
			call_record_answered(send_id);
			goto_layout(pLAYOUT(intercom_talk));
		}
		else
		{
			goto_layout(pLAYOUT(intercom_in));
		}
	}
	return true;
}
static bool intercom_answer_receive_event(unsigned int send_id)
{
	intercom_number = send_id;
	if (intercom_state == INTERCOM_STATE_CALLING_IN)
	{
		printf("================>>> calling in\n");
	}
	else if (intercom_state == INTERCOM_STATE_CALL)
	{
		intercom_call_in_flag = false;
		if (cur_layout_get() != pLAYOUT(intercom_out))
		{
			goto_layout(pLAYOUT(intercom_out));
		}
	}
	else if (intercom_state == INTERCOM_STATE_CALLING_OUT)
	{
		printf("================>>> calling out\n");
	}
	else if (intercom_state == INTERCOM_STATE_IDLE)
	{
		printf("================>>> idle\n");
		// intercom_standard_format_send(intercom_number, CMD_INTERPHONE_QUIT);
	}

	return true;
}
static bool intercom_talking_receive_event(unsigned int send_id)
{
	intercom_number = send_id;
	if (intercom_state == INTERCOM_STATE_IDLE || intercom_state == INTERCOM_STATE_HUNG_UP)
	{
	}
	else if (intercom_state == INTERCOM_STATE_CALL)
	{
		printf("================>>> call out talking\n");
		intercom_call_in_flag = false;
		if (cur_layout_get() != pLAYOUT(intercom_talk))
		{
			call_record_answered(0);
			goto_layout(pLAYOUT(intercom_talk));
		}
	}
	else if (intercom_state == INTERCOM_STATE_CALLING_OUT)
	{
		printf("================>>> call out talking\n");
		intercom_call_in_flag = false;
		if (cur_layout_get() != pLAYOUT(intercom_talk))
		{
			call_record_answered(0);
			goto_layout(pLAYOUT(intercom_talk));
		}
	}
	else if (intercom_state == INTERCOM_STATE_TALKING)
	{
		printf("================>>> talking\n");
	}

	return true;
}
static bool intercom_quit_receive_event(unsigned int send_id)
{
	printf("================>>> hung up\n");
	intercom_state = INTERCOM_STATE_IDLE;
	return true;
}
static bool intercom_line_busy_receive_event(unsigned int send_id) // 没啥用
{
	printf("================>>> line busy\n");
	intercom_line_is_busy = true;
	return true;
}
static bool intercom_unit_busy_receive_event(unsigned int send_id) // 没啥用
{
	intercom_unit_is_busy = true;
	return true;
}
static const intercom_param_info intercom_receive_handle[] =
	{
		{"CMD_INTERPHONE_CALL", CMD_INTERPHONE_CALL, intercom_call_receive_event},
		{"CMD_INTERPHONE_ANSWER", CMD_INTERPHONE_ANSWER, intercom_answer_receive_event},
		{"CMD_INTERPHONE_TALKING", CMD_INTERPHONE_TALKING, intercom_talking_receive_event},
		{"CMD_INTERPHONE_QUIT", CMD_INTERPHONE_QUIT, intercom_quit_receive_event},
		{"CMD_INTERPHONE_LINE_BUSY", CMD_INTERPHONE_LINE_BUSY, intercom_line_busy_receive_event},
		{"CMD_INTERPHONE_UNIT_BUSY", CMD_INTERPHONE_UNIT_BUSY, intercom_unit_busy_receive_event},
};
/***
**   日期:2022-05-31 08:47:35
**   作者: leo.liu
**   函数作用：接收intercom，主线程处理任务
**   参数说明:
***/
static bool intercom_event_cmd_process(unsigned int send_id, unsigned int cmd)
{
	int total = sizeof(intercom_receive_handle) / sizeof(intercom_param_info);
	for (int i = 0; i < total; i++)
	{
		if ((cmd == intercom_receive_handle[i].cmd) && (intercom_receive_handle[i].handler != NULL))
		{
			return intercom_receive_handle[i].handler(send_id);
		}
	}
	printf("not find cmd event (%02x ->%d)\n\r", cmd, send_id);
	return false;
}

int intercom_number_get(void)
{
	return intercom_number;
}
void intercom_number_set(int num)
{
	intercom_number = num;
}
intercom_state_t intercom_state_get(void)
{
	return intercom_state;
}
void intercom_state_set(intercom_state_t state)
{
	switch (state)
	{
	case INTERCOM_STATE_CALLING_IN:
		send_cmd_count = 5;
		break;
	case INTERCOM_STATE_TALKING:
		send_cmd_count = 5;
		break;
	case INTERCOM_STATE_HUNG_UP:
		send_cmd_count = 10;
		break;
	default:
		send_cmd_count = 10;
		break;
	}
	intercom_state = state;
}

bool intercom_line_busy_state_get(void)
{
	return intercom_line_is_busy;
}

bool intercom_unit_busy_state_get(void)
{
	return intercom_unit_is_busy;
}

void intercom_busy_state_reset(void)
{
	intercom_unit_is_busy = false;
	intercom_line_is_busy = false;
}

/*******************************************************************
 * @brief  : 随机延时
 * @return  {*}
 *******************************************************************/
void rand_delay(void)
{
	int delay = rand() % 100 + 50;
	usleep(delay * 1000);
}