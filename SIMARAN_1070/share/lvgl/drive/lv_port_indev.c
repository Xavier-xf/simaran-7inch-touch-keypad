/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "drive/lv_port_indev.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <stdlib.h>
/*********************
 *      DEFINES
 *********************/
static lv_key_t last_key = LV_KEY_NONE; // 上一次按键

#define ADC_KEY_IS_FREE(value) (value == 3299)
#define ADC_KEY_IS_PRESS(value) (value < 3290) // 2025.2.12  hare->value != 3299
#define ADC_KEY_IS_HOME(value) ((value > 650) && (value < 850))
#define ADC_KEY_IS_LEFT(value) ((value > 1200) && (value < 1400))
#define ADC_KEY_IS_SELECT(value) ((value > 2400) && (value < 2600))
#define ADC_KEY_IS_RIGHT(value) ((value > 2650) && (value < 2850))
#define ADC_KEY_IS_BACK(value) ((value > 2900) && (value < 3100))

#define FILE_PATH_MAX 64
#define AK_SAR_ADC_PATCH "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw"

lv_group_t *g_global_group = NULL;
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static int ak_sardac_read_channel_data(int channel, char *result, int count)
{
    int fd = -1;
    int ret = 0;
    char path[FILE_PATH_MAX] = {0};

    memset(path, 0, FILE_PATH_MAX);
    sprintf(path, AK_SAR_ADC_PATCH, channel);

    if (!access(path, F_OK))
    {
        fd = open(path, O_RDONLY);
        if (fd < 0)
        {
            printf("open %s: %s\n", path, strerror(errno));
            return -EPERM;
        }

        ret = read(fd, result, count);
        if (ret < 0)
        {
            close(fd);
            printf("read %s: %s\n", path, strerror(errno));
            return ret;
        }

        close(fd);
    }
    else
    {
        return -ENOENT;
    }
    return 0;
}
#if 0
static void touchpad_init(void);
static bool touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y);

static void mouse_init(void);
static bool mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y);

static void keypad_init(void);
static bool keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static uint32_t keypad_get_key(void);

static void encoder_init(void);
static bool encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void encoder_handler(void);
#else
static void button_init(void);
static bool button_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

#endif
/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t *indev_touchpad;
lv_indev_t *indev_mouse;
lv_indev_t *indev_keypad;
lv_indev_t *indev_encoder;
lv_indev_t *indev_button;

// static int32_t encoder_diff;
// static lv_indev_state_t encoder_state;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_key_t lv_get_button_last_key(void)
{
    return last_key;
}
// static int evdev_fd;
// static int evdev_root_x;
// static int evdev_root_y;
// static int evdev_button;

// static int evdev_key_val;

void lv_port_indev_init(void)
{
    /* Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    lv_indev_drv_t indev_drv;
#if 0
    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);


    /*------------------
     * Mouse
     * -----------------*/

    /*Initialize your touchpad if you have*/
    mouse_init();

    /*Register a mouse input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;
    indev_mouse = lv_indev_drv_register(&indev_drv);

    /*Set cursor. For simplicity set a HOME symbol now.*/
    lv_obj_t * mouse_cursor = lv_img_create(lv_disp_get_scr_act(NULL), NULL);
    lv_img_set_src(mouse_cursor, LV_SYMBOL_HOME);
    lv_indev_set_cursor(indev_mouse, mouse_cursor);

    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv);

    /* Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     * add objects to the group with `lv_group_add_obj(group, obj)`
     * and assign this input device to group to navigate in it:
     * `lv_indev_set_group(indev_keypad, group);` */

    /*------------------
     * Encoder
     * -----------------*/

    /*Initialize your encoder if you have*/
    encoder_init();

    /*Register a encoder input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read;
    indev_encoder = lv_indev_drv_register(&indev_drv);

    /* Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     * add objects to the group with `lv_group_add_obj(group, obj)`
     * and assign this input device to group to navigate in it:
     * `lv_indev_set_group(indev_encoder, group);` */
#else
    /*------------------
     * Button
     * -----------------*/

    /*Initialize your button if you have*/
    button_init();

    /*Register a button input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = button_read;
    indev_button = lv_indev_drv_register(&indev_drv);

    // 创建组并关联输入设备
    g_global_group = lv_group_create();
    lv_indev_set_group(indev_button, g_global_group);
#endif
}
lv_group_t *lv_group_get_default(void)
{
    // 如果全局组未初始化
    if (g_global_group == NULL)
    {
        g_global_group = lv_group_create();
        lv_indev_set_group(indev_button, g_global_group);
    }
    return g_global_group;
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
#if 0
/*------------------
 * Touchpad
 * -----------------*/
#define EVDEV_NAME "/dev/input/event0"
/*Initialize your touchpad*/
static void touchpad_init(void)
{
    /*Your code comes here*/
    evdev_fd = open(EVDEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY);
    if (evdev_fd == -1)
    {
        perror("unable open evdev interface:");
        return;
    }

    fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);

    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_key_val = 0;
    evdev_button = LV_INDEV_STATE_REL;
}

/* Will be called by the library to read the touchpad */
static bool touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    bool enable = false;
    struct input_event in;
    while (read(evdev_fd, &in, sizeof(struct input_event)) > 0)
    {

        if (in.type == EV_REL)
        {
            if (in.code == REL_X)
#if EVDEV_SWAP_AXES
                evdev_root_y += in.value;
#else
                evdev_root_x += in.value;
#endif
            else if (in.code == REL_Y)
#if EVDEV_SWAP_AXES
                evdev_root_x += in.value;
#else
                evdev_root_y += in.value;
#endif
        }
        else if (in.type == EV_ABS)
        {
            if (in.code == ABS_X)
#if EVDEV_SWAP_AXES
                evdev_root_y = in.value;
#else
                evdev_root_x = in.value;
#endif
            else if (in.code == ABS_Y)
#if EVDEV_SWAP_AXES
                evdev_root_x = in.value;
#else
                evdev_root_y = in.value;
#endif
            else if (in.code == ABS_MT_POSITION_X)
#if EVDEV_SWAP_AXES
                evdev_root_y = in.value;
#else
                evdev_root_x = in.value;
#endif
            else if (in.code == ABS_MT_POSITION_Y)
#if EVDEV_SWAP_AXES
                evdev_root_x = in.value;
#else
                evdev_root_y = in.value;
#endif
            else if (in.code == 48)
            {
                if (in.value == 0)
                {
                    evdev_button = LV_INDEV_STATE_REL;
                }
                else
                {
                    evdev_button = LV_INDEV_STATE_PR;
                }
                printf("type:%d,code:%d,value:%d \n", in.type, in.code, in.value);
                enable = true;
            }
        }
        else if (in.type == EV_KEY)
        {

            if (in.code == BTN_MOUSE || in.code == BTN_TOUCH)
            {
                if (in.value == 0)
                    evdev_button = LV_INDEV_STATE_REL;
                else if (in.value == 1)
                    evdev_button = LV_INDEV_STATE_PR;
            }
            else if (drv->type == LV_INDEV_TYPE_KEYPAD)
            {
                data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
                switch (in.code)
                {
                case KEY_BACKSPACE:
                    data->key = LV_KEY_BACKSPACE;
                    break;
                case KEY_ENTER:
                    data->key = LV_KEY_ENTER;
                    break;
                case KEY_UP:
                    data->key = LV_KEY_UP;
                    break;
                case KEY_LEFT:
                    data->key = LV_KEY_PREV;
                    break;
                case KEY_RIGHT:
                    data->key = LV_KEY_NEXT;
                    break;
                case KEY_DOWN:
                    data->key = LV_KEY_DOWN;
                    break;
                default:
                    data->key = 0;
                    break;
                }
                evdev_key_val = data->key;
                evdev_button = data->state;
                return false;
            }
        }
    }

    if (drv->type == LV_INDEV_TYPE_KEYPAD)
    {
        /* No data retrieved */
        data->key = evdev_key_val;
        data->state = evdev_button;
        return false;
    }
    if (drv->type != LV_INDEV_TYPE_POINTER)
    {
        return false;
    }
    /*Store the collected data*/

#if EVDEV_SCALE
    dsfsd data->point.x = map(evdev_root_x, 0, EVDEV_SCALE_HOR_RES, 0, lv_disp_get_hor_res(drv->disp));
    data->point.y = map(evdev_root_y, 0, EVDEV_SCALE_VER_RES, 0, lv_disp_get_ver_res(drv->disp));
#endif
#if EVDEV_CALIBRATE
    sdfsdf data->point.x = map(evdev_root_x, EVDEV_HOR_MIN, EVDEV_HOR_MAX, 0, lv_disp_get_hor_res(drv->disp));
    data->point.y = map(evdev_root_y, EVDEV_VER_MIN, EVDEV_VER_MAX, 0, lv_disp_get_ver_res(drv->disp));
#else
    data->point.x = evdev_root_x; // evdev_root_y;
    data->point.y = evdev_root_y; // evdev_root_x ? lv_disp_get_ver_res(drv->disp) - evdev_root_x : 0;
#endif

    data->state = evdev_button;

    if (data->point.x < 0)
        data->point.x = 0;
    if (data->point.y < 0)
        data->point.y = 0;
    // if (data->point.y > 10)
    //     data->point.y -= 10;
#if 1
#if 0
	if (data->point.x >= lv_disp_get_hor_res(drv->disp))
		data->point.x = lv_disp_get_hor_res(drv->disp) - 1;
	if (data->point.y >= lv_disp_get_ver_res(drv->disp))
		data->point.y = lv_disp_get_ver_res(drv->disp) - 1;
#else
    if (data->point.x >= 1024)
        data->point.x = 1024 - 1;
    if (data->point.y >= 600)
        data->point.y = 600 - 1;

    // data->point.x =1024 -data->point.x;
    // data->point.y = 600 -  data->point.y;

    data->point.x = data->point.x;
    data->point.y = data->point.y;
#endif
#endif
    if (enable == true)
    {
        printf("%d, %d,%d \n", data->state, data->point.x, data->point.y);
    }
    return false;
}

/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}

/*------------------
 * Mouse
 * -----------------*/

/* Initialize your mouse */
static void mouse_init(void)
{
    /*Your code comes here*/
}

/* Will be called by the library to read the mouse */
static bool mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the mouse button is pressed or released*/
    if(mouse_is_pressed()) {
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

/*Return true is the mouse button is pressed*/
static bool mouse_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the mouse is pressed*/
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}

/*------------------
 * Keypad
 * -----------------*/

/* Initialize your keypad */
static void keypad_init(void)
{
    /*Your code comes here*/
}

/* Will be called by the library to read the mouse */
static bool keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;

    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
        case 1:
            act_key = LV_KEY_NEXT;
            break;
        case 2:
            act_key = LV_KEY_PREV;
            break;
        case 3:
            act_key = LV_KEY_LEFT;
            break;
        case 4:
            act_key = LV_KEY_RIGHT;
            break;
        case 5:
            act_key = LV_KEY_ENTER;
            break;
        }

        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/

    return 0;
}

/*------------------
 * Encoder
 * -----------------*/

/* Initialize your keypad */
static void encoder_init(void)
{
    /*Your code comes here*/
}

/* Will be called by the library to read the encoder */
static bool encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{

    data->enc_diff = encoder_diff;
    data->state = encoder_state;

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

/*Call this function in an interrupt to process encoder events (turn, press)*/
static void encoder_handler(void)
{
    /*Your code comes here*/

    encoder_diff += 0;
    encoder_state = LV_INDEV_STATE_REL;
}
#else
/*------------------
 * Button
 * -----------------*/

/* Initialize your buttons */

static void button_init(void)
{
    /*Your code comes here*/
}

/* Will be called by the library to read the button */
static bool button_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{

    //  静态变量：保存上一次状态
    static int last_voltage = 0; // 上一次ADC电压

    //  读取当前ADC电压
    char adc_buf[32] = {0};
    int adc_ret = ak_sardac_read_channel_data(0, adc_buf, sizeof(adc_buf));
    int current_voltage = adc_ret == 0 ? atoi(adc_buf) : 0; // 电压值

    // 初始化返回值
    data->state = LV_INDEV_STATE_REL;
    data->key = LV_KEY_NONE;

    // 按键判断与消抖
    if (ADC_KEY_IS_PRESS(current_voltage)) // 检测到按键按下
    {
        // 消抖：当前电压与上一次接近，且间隔超过消抖时间
        if (abs(current_voltage - last_voltage) < 100)
        {
            // 映射ADC按键到LVGL键值
            if (ADC_KEY_IS_LEFT(current_voltage))
            {
                data->key = LV_KEY_PREV; // 左键→切换到上一个焦点
                last_key = LV_KEY_PREV;
            }
            else if (ADC_KEY_IS_RIGHT(current_voltage))
            {
                data->key = LV_KEY_NEXT; // 右键→切换到下一个焦点
                last_key = LV_KEY_NEXT;
            }
            else if (ADC_KEY_IS_SELECT(current_voltage))
            {
                data->key = LV_KEY_ENTER; // 选择键→确认
                last_key = LV_KEY_ENTER;
            }
            else if (ADC_KEY_IS_BACK(current_voltage))
            {
                data->key = LV_KEY_ESC; // 返回键→退出/返回
                last_key = LV_KEY_ESC;
            }
            else if (ADC_KEY_IS_HOME(current_voltage))
            {
                data->key = LV_KEY_HOME; // 主页键→返回主界面
                last_key = LV_KEY_HOME;
            }

            data->state = LV_INDEV_STATE_PR; // 标记为按下状态
        }
    }
    // 更新上一次状态
    last_voltage = current_voltage;

    // 释放时返回空键值
    if (data->state == LV_INDEV_STATE_REL)
    {
        data->key = LV_KEY_NONE;
    }

    return false; // LVGL要求：返回false表示无连续读取需求
}

#endif

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
