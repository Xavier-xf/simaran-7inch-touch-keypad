#ifndef _LAYOUT_SETTING_H_
#define _LAYOUT_SETTING_H_
#include "layout_define.h"

/**
 * @brief 封装的环形进度条创建函数
 * @param parent        父对象（如lv_scr_act()表示屏幕顶层）
 * @param task_cb       进度条更新任务回调（如factory_reset_arc_loader）
 * @param arc_size      环形进度条大小（宽=高，默认150px）
 * @param bg_color      背景弧颜色（默认浅灰色0xE0E0E0）
 * @param indic_color   指示器弧颜色（默认蓝色0x007AFF）
 * @param line_width    弧的线宽（默认10px）
 * @param task_period   任务执行周期（ms，默认20ms，控制动画速度）
 * @param align_x       水平偏移（默认0，中心对齐）
 * @param align_y       垂直偏移（默认0，中心对齐）
 * @return lv_obj_t*    创建的环形进度条对象（失败返回NULL）
 */
lv_obj_t *lv_arc_loader_create(lv_obj_t *parent,
                               lv_task_cb_t task_cb,
                               uint16_t arc_size,
                               lv_color_t bg_color,
                               lv_color_t indic_color,
                               uint8_t line_width,
                               uint32_t task_period,
                               int32_t align_x,
                               int32_t align_y);
/**
 * @brief 更新环形进度条旋转角度
 * @param arc 进度条对象
 * @return bool 返回true表示已完成一圈，需要销毁进度条
 */
bool arc_loader_update(lv_obj_t *arc, int *start_angle_ptr);
/**
 * @brief 删除进度条及其相关资源
 * @param arc 进度条对象
 */
void arc_loader_delete(lv_obj_t *arc);

lv_obj_t *setting_back_btn_create(lv_obj_t *parent, void (*back_btn_up)(lv_obj_t *));
lv_obj_t *setting_logo_img_create(lv_obj_t *parent);
/***
** 日期: 2022-04-26 17:09
** 作者: leo.liu
** 函数作用：创建设置页面的返回按钮
** 返回参数说明：
***/
bool setting_cancel_btn_create(void (*obj_click_up_callback)(lv_obj_t *));
/***
** 日期: 2022-04-28 11:59
** 作者: leo.liu
** 函数作用：创建右边按钮基础函数
** 返回参数说明：
***/
lv_obj_t *setting_right_btn_base_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, const char *sub_string, obj_click_data *click_data, unsigned int sub_obj_id);

/***
**   日期:2022-07-04 17:00:03
**   作者: leo.liu
**   函数作用：销毁消息框
**   参数说明:
***/
bool setting_adjust_msgdialog_msg_bg_delete(int id);
bool setting_msgdialog_msg_bg_delete(int id);
/***
** 日期: 2022-04-28 10:40
** 作者: leo.liu
** 函数作用：创建setting的顶部文本
** 返回参数说明：
***/
lv_obj_t *setting_head_label_create(const char *string);
/***
 ** 日期: 2022-04-28 15:32
 ** 作者: leo.liu
 ** 函数作用：创建按键矩阵
 ** 返回参数说明：
 ***/
lv_obj_t *setting_msgdialog_btnmatrix_create(lv_obj_t *parent, int obj_id, const char *string_map[]);
lv_obj_t *setting_adjust_msgdialog_msg_bg_create(int id);
/***
** 日期: 2022-04-29 15:56
** 作者: leo.liu
** 函数作用：消息框的背景创建
** 返回参数说明：
***/
lv_obj_t *setting_msgdialog_msg_bg_create(int id, int x, int y);
/***
** 日期: 2022-04-28 11:59
** 作者: leo.liu
** 函数作用：创建设置图标
** 返回参数说明：focus
    0,不显示，
    1:显示字符串，并且根据click_data显示right icon
    2:显示checkbox
    3:显示switch
    4:多选框
***/
lv_obj_t *setting_sub_btn_base_create(lv_obj_t *parent, int x, int y, int w, int h, const char *main_string, obj_click_data *click_data, bool focus, int type);
/***
** 日期: 2022-05-06 10:22
** 作者: leo.liu
** 函数作用：滚动基础函数
** 返回参数说明：
***/

lv_obj_t *setting_time_roller_base(lv_obj_t *parent, int x, int y, int w, int h, int min, int max, int cur, obj_click_data *click_data, int obj_id);
/*************************************************************************
 * @brief  消息弹窗 0>success 1>warning 2>error 3>notice
 * @date   2022-09-03 10:55
 * @author xiaoele
 * @param  str
 * @param  type
 * @param  repeat  0->不重复， n->重复次数 LV_ANIM_REPEAT_INFINITE->无限循环
 * @param  offset  [0]不偏移 [1]向下偏移 [-1]向上偏移
 **************************************************************************/
void msg_toast(const char *str, short int type, int repeat, int offset);
/***
** 日期: 2022-05-09 10:26
** 作者: leo.liu
** 函数作用：输入编辑的数值
** 返回参数说明：
***/
bool setting_password_input_string(lv_obj_t *parent, const char *string, int max_edit);
/***
** 日期: 2022-05-04 09:10
** 作者: leo.liu
** 函数作用：消息框的确认按钮创建
** 返回参数说明：
***/
void setting_msgdialog_msg_confirm_btn_create(lv_obj_t *parent, void (*confirm_func)(lv_obj_t *obj));
/***
** 日期: 2022-05-04 09:10
** 作者: leo.liu
** 函数作用：消息框的确认按钮创建
** 返回参数说明：
***/
void setting_msgdialog_msg_confirm_and_cancel_btn_create(lv_obj_t *parent, void (*cancel_func)(lv_obj_t *obj), void (*confirm_func)(lv_obj_t *obj));

/*************************************************************************
 * @brief  文本域
 * @date   2022-10-21 09:45
 * @author xiaoele
 **************************************************************************/
void setting_msgdialog_msg_create(lv_obj_t *parent, const char *msg_string);
/***
** 日期: 2022-05-09 10:57
** 作者: leo.liu
** 函数作用：删除一个字符
** 返回参数说明：
***/
bool setting_password_del_string(lv_obj_t *parent, int max_edit);
/***
** 日期: 2022-05-09 11:31
** 作者: leo.liu
** 函数作用：复位输入框
** 返回参数说明：
***/
bool setting_password_input_reset(lv_obj_t *parent, int edit_max);
/***
** 日期: 2022-05-09 11:27
** 作者: leo.liu
** 函数作用：获取当前输入的索引
** 返回参数说明：
***/
int setting_password_edit_index_get(lv_obj_t *parent);
/***
** 日期: 2022-05-09 11:22
** 作者: leo.liu
** 函数作用：获取输入字符串
** 返回参数说明：
***/
bool setting_password_get_string(lv_obj_t *parent, char *buffer);
/***
** 日期: 2022-05-07 13:52
** 作者: leo.liu
** 函数作用：数字密码输入键盘
** 返回参数说明：
***/
lv_obj_t *setting_passowrd_num_keyboard_create(lv_obj_t *parent, int x, int y, int w, int h, obj_click_data *click_data);
/***
** 日期: 2022-05-07 16:58
** 作者: leo.liu
** 函数作用：设置密码输入框
** 返回参数说明
** 3x4个控件
** LABEL ：显示文本
** OBJ ：显示圆点
** OBJ :显示下划线
** 要获取控件:可以通过矩阵id获取，如第一个lable:11
***/
bool setting_password_input_label_create(lv_obj_t *parent, int x, int y, int row);
/***
** 日期: 2022-04-28 11:59
** 作者: leo.liu
** 函数作用：创建带双向箭头的设置按钮
** 返回参数说明：
***/
lv_obj_t *setting_double_arrow_btn_create(lv_obj_t *parent, int x, int y, int w, int h,
                                          const char *main_string, const char *sub_string,
                                          obj_click_data *left_arrow_click_data,
                                          obj_click_data *right_arrow_click_data, unsigned int sub_obj_id);
int security_enter_flag;
/***
 * @brief  更新按钮组状态
 * @date   2022-05-09 11:09
 * @author leo.liu
 * @param  parent
 * @param  obj_id
 * @param  add_to_group
 **************************************************************************/
void update_motion_buttons_group_state(lv_obj_t *parent, int obj_id, bool add_to_group);

#endif