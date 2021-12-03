/************************************************************
Copyright (C), 2012-2020, 深圳英可瑞科技开发有限公司
文 件 名：Display.h
版    本：1.00
创建日期：2012-03-22
作    者：郭数理
功能描述：显示相关的定义头文件

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-22  1.00     创建
**************************************************************/

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "Type.h"
#include "../Drivers/Rtc.h"

/*
缩写     全称                   翻译
mmi      Man-Machine Interface  人机接口
win      window                 窗口/页面
cap      capicity               容量
his      history                历史
batt     battery                电池组
         cell                   单体电池
insu     insulation             绝缘
res      resistance             电阻
rec      rectifier              整流模块
curr     current                电流/当前
disp     display                显示
val      vaule                  数值
sel      select                 选择
*/


#define MMI_NAME_MAX_LEN     25       //名称最长24个ASCII字符，12个中文字符，多一个'\0'字符
#define MMI_WIN_ICON_MAX_CNT 2        //页面最大的图标数量为2
#define MMI_WIN_ITEM_MAX_CNT 12       //页面最大的条目数量为11

#define MMI_WIN_TYPE_MASK    0xFF00   //页面ID的高8位表示类型
#define MMI_WIN_TYPE_OFFSET  8
#define MMI_WIN_INDEX_MASK   0x00FF   //页面ID的低8位表示页面结构数组的索引
#define MMI_WIN_ID_NULL      0xFFFF   //无效页面ID
        

/* 界面类型，主要用于按键处理 */
enum
{
	MMI_WIN_TYPE_MAIN_WIN = 0,              // 主界面
	MMI_WIN_TYPE_MENU,                      // 菜单界面
	MMI_WIN_TYPE_SET,                       // 设置界面
	MMI_WIN_TYPE_ABOUT,                     // 关于菜单
	MMI_WIN_TYPE_PASSWORD,                  // 输入密码界面
	MMI_WIN_TYPE_PARAM_RESTORE,             // 参数恢复界面
	MMI_WIN_TYPE_BATT_CAP_RESTORE,          // 电池容量恢复界面
	MMI_WIN_TYPE_HIS_FAULT_CLEAR,           // 历史告警记录清除界面
	MMI_WIN_TYPE_EXCEPTION_CLEAR,           // 掉电末复归告警记录清除界面
	MMI_WIN_TYPE_RECORD_CLEAR,              // 电池充放电记录清除界面
	MMI_WIN_TYPE_CONFIRM,                   // 确认操作提示界面
	MMI_WIN_TYPE_DOING,                     // 正在操作提示界面
	MMI_WIN_TYPE_RUN_INFO,                  // 单页实时数据界面

	/* 以下是实时数据多页界面类型 */
	MMI_WIN_TYPE_CELL1_VOLT,                // 一组单体电池电压界面，多页界面
	MMI_WIN_TYPE_CELL2_VOLT,                // 二组单体电池电压界面，多页界面
	MMI_WIN_TYPE_RECT,                      // 整流模块界面，多页界面
	MMI_WIN_TYPE_DCDC_MODULE,               // 通信模块界面，多页界面
	MMI_WIN_TYPE_DCAC_MODULE,               // 逆变模块界面，多页界面
	MMI_WIN_TYPE_DC_FEEDER,                 // 直流馈线支路信息界面，多页界面
	MMI_WIN_TYPE_DCDC_FEEDER,               // 通信馈线支路信息界面，多页界面
	MMI_WIN_TYPE_DCAC_FEEDER,               // 逆变馈线支路信息界面，多页界面
	MMI_WIN_TYPE_AC_FEEDER,                 // 交流馈线支路信息界面，多页界面
	MMI_WIN_TYPE_SWITCH,                    // 开关状态界面，多页界面
	MMI_WIN_TYPE_EC_SWITCH,                 // 电操开关状态界面，多页界面

	MMI_WIN_TYPE_CURR_FAULT,                // 当前告警界面，多页界面
	MMI_WIN_TYPE_HIS_FAULT,                 // 历史告警界面，多页界面
	MMI_WIN_TYPE_EXCEPTION,                 // 掉电末复归故障界面，多页界面
	MMI_WIN_TYPE_BATT_RECORD,               // 电池充放电记录，多页界面
	
};

/* 界面ID定义 */
#define MMI_WIN_ID_MAIN_WINDOW              (0 |  (MMI_WIN_TYPE_MAIN_WIN<<8))            // 主界面-1段母线数据
#define MMI_WIN_ID_MAIN_MENU                (1 |  (MMI_WIN_TYPE_MENU<<8))                // 主菜单           
#define MMI_WIN_ID_DC_RUN_INFO_MENU         (2 |  (MMI_WIN_TYPE_MENU<<8))                // 直流系统信息查询菜单
#define MMI_WIN_ID_FEEDER_RUN_INFO_MENU     (3 |  (MMI_WIN_TYPE_MENU<<8))                // 馈线柜信息查询菜单
#define MMI_WIN_ID_DCDC_RUN_INFO_MENU       (4 |  (MMI_WIN_TYPE_MENU<<8))                // 通信系统信息查询菜单
#define MMI_WIN_ID_DCAC_RUN_INFO_MENU       (5 |  (MMI_WIN_TYPE_MENU<<8))                // 逆变系统信息查询菜单
#define MMI_WIN_ID_SET_MENU                 (6 |  (MMI_WIN_TYPE_MENU<<8))                // 参数设置菜单
#define MMI_WIN_ID_DC_SET_MENU              (7 |  (MMI_WIN_TYPE_MENU<<8))                // 直流参数设置菜单
#define MMI_WIN_ID_FEEDER_SET_MENU          (8 |  (MMI_WIN_TYPE_MENU<<8))                // 馈线柜参数设置菜单


#define MMI_WIN_ID_NO_CONFIG                (9 |  (MMI_WIN_TYPE_RUN_INFO<<8))            // 末配置此项信息提示窗口

#define MMI_WIN_ID_AC_TRIPHASE_INFO         (10 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 三相交流信息显示页面
#define MMI_WIN_ID_AC_UNIPHASE_INFO         (11 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 单相交流信息显示页面
#define MMI_WIN_ID_BATT_TOTAL_INFO          (12 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 一组电池运行数据
#define MMI_WIN_ID_BATT_GROUP_INFO          (13 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 二组电池运行数据
#define MMI_WIN_ID_BMS_GROUP1_INFO          (14 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 一组电池巡检数据
#define MMI_WIN_ID_BMS1_CELL_INFO           (15 | (MMI_WIN_TYPE_CELL1_VOLT<<8))         // 一组单体电压数据
#define MMI_WIN_ID_BMS_GROUP2_INFO          (16 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 二组电池巡检数据
#define MMI_WIN_ID_BMS2_CELL_INFO           (17 | (MMI_WIN_TYPE_CELL2_VOLT<<8))         // 二组单体电压数据
#define MMI_WIN_ID_PB_CB_INSU_INFO          (18 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 一段合控母绝缘状态查询
#define MMI_WIN_ID_BUS_INSU_INFO            (19 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 一段母线绝缘查询
#define MMI_WIN_ID_RECT_INFO                (20 | (MMI_WIN_TYPE_RECT<<8))               // AC/DC模块信息查询
#define MMI_WIN_ID_SWITCH_INFO              (21 | (MMI_WIN_TYPE_SWITCH<<8))             // 开关状态查询
#define MMI_WIN_ID_DC_FEEDER_INFO           (22 | (MMI_WIN_TYPE_DC_FEEDER<<8))          // 一段馈电柜馈出支路信息
#define MMI_WIN_ID_DCDC_INFO                (23 | (MMI_WIN_TYPE_DCDC_MODULE<<8))        // 通信模块信息查询
#define MMI_WIN_ID_DCDC_FEEDER_INFO         (24 | (MMI_WIN_TYPE_DCDC_FEEDER<<8))        // 通信屏馈出支路信息查询
#define MMI_WIN_ID_DCAC_INFO                (25 | (MMI_WIN_TYPE_DCAC_MODULE<<8))        // 逆变模块信息查询
#define MMI_WIN_ID_DCAC_FEEDER_INFO         (26 | (MMI_WIN_TYPE_DCAC_FEEDER<<8))        // 逆变屏馈出支路信息查询

#define MMI_WIN_ID_CURR_FAULT_INFO          (27 | (MMI_WIN_TYPE_CURR_FAULT<<8))          // 当前故障界面
#define MMI_WIN_ID_NO_CURR_FAULT            (28 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 无当前故障提示界面
#define MMI_WIN_ID_HIS_FAULT_INFO           (29 | (MMI_WIN_TYPE_HIS_FAULT<<8))           // 历史故障界面
#define MMI_WIN_ID_NO_HIS_FAULT             (30 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 无历史故障提示界面
#define MMI_WIN_ID_EXCEPTION_INFO           (31 | (MMI_WIN_TYPE_EXCEPTION<<8))           // 掉电末复归故障界面
#define MMI_WIN_ID_NO_EXCEPTION             (32 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 无掉电末复归故障提示界面
#define MMI_WIN_ID_RECORD                   (33 | (MMI_WIN_TYPE_BATT_RECORD<<8))         // 电池充放电记录界面
#define MMI_WIN_ID_NO_RECORD                (34 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 无电池充放电记录界面

#define MMI_WIN_ID_INPUT_PASSWORD           (35 | (MMI_WIN_TYPE_PASSWORD<<8))            // 输入密码界面
#define MMI_WIN_ID_PASSWORD_ERROR           (36 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 密码错误提示窗口

#define MMI_WIN_ID_PARAM_OUT_RANGE          (37 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 提示参数范围超出界限

#define MMI_WIN_ID_DC_SYSTEM_SET            (38 | (MMI_WIN_TYPE_SET<<8))                 // 直流系统设置界面
#define MMI_WIN_ID_AC_PARAM_SET             (39 | (MMI_WIN_TYPE_SET<<8))                 // 交流参数配置
#define MMI_WIN_ID_BATT_PARAM_SET           (40 | (MMI_WIN_TYPE_SET<<8))                 // 电池配置
#define MMI_WIN_ID_DC_PARAM_SET             (41 | (MMI_WIN_TYPE_SET<<8))                 // 直流参数配置
#define MMI_WIN_ID_RECT_PARAM_SET           (42 | (MMI_WIN_TYPE_SET<<8))                 // 整流模块参数配置
#define MMI_WIN_ID_BATT_METER1_SET          (43 | (MMI_WIN_TYPE_SET<<8))                 // 一组电池巡检配置
#define MMI_WIN_ID_BATT_METER2_SET          (44 | (MMI_WIN_TYPE_SET<<8))                 // 二组电池巡检配置

#define MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET (45 | (MMI_WIN_TYPE_SET<<8))              // 馈电柜馈线模块数量配置
#define MMI_WIN_ID_FEEDER_PANEL_MODULE_SET	(46 | (MMI_WIN_TYPE_SET<<8))                 // 馈电柜馈线模块配置

#define MMI_WIN_ID_AC_THR_SET               (47 | (MMI_WIN_TYPE_SET<<8))                 // 交流门限设置
#define MMI_WIN_ID_DC_THR_SET               (48 | (MMI_WIN_TYPE_SET<<8))                 // 直流门限设置
#define MMI_WIN_ID_BATT_THR_SET             (49 | (MMI_WIN_TYPE_SET<<8))                 // 电池门限值设置
#define MMI_WIN_ID_INSU_THR_SET             (50 | (MMI_WIN_TYPE_SET<<8))                 // 绝缘门限报警设置

#define MMI_WIN_ID_BATT_CHARGE_SET          (51 | (MMI_WIN_TYPE_SET<<8))                 // 电池充电参数设置
#define MMI_WIN_ID_BATT_TO_FLO_SET          (52 | (MMI_WIN_TYPE_SET<<8))                 // 电池转浮充判据
#define MMI_WIN_ID_BATT_TO_EQU_SET          (53 | (MMI_WIN_TYPE_SET<<8))                 // 电池转均充判据
#define MMI_WIN_ID_BATT_DISCHARGE_END_SET   (54 | (MMI_WIN_TYPE_SET<<8))                 // 电池核容终止设置
#define MMI_WIN_ID_BATT_DISCHARGE_SET       (55 | (MMI_WIN_TYPE_SET<<8))                 // 电池放电曲线设置

#define MMI_WIN_ID_SYSTEM_CTL_SET           (56 | (MMI_WIN_TYPE_SET<<8))                 // 系统控制方式
#define MMI_WIN_ID_BATT_CTL_SET             (57 | (MMI_WIN_TYPE_SET<<8))                 // 电池充电方式
#define MMI_WIN_ID_RECT_ON_OFF              (58 | (MMI_WIN_TYPE_SET<<8))                 // 整流模块开关机

#define MMI_WIN_ID_DCDC_MODULE_SET           (59 | (MMI_WIN_TYPE_SET<<8))                 // 通信模块参数配置
#define MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET (60 | (MMI_WIN_TYPE_SET<<8))                // 通信屏馈线模块数量配置
#define MMI_WIN_ID_DCDC_FEEDER_MODULE_SET	 (61 | (MMI_WIN_TYPE_SET<<8))                 // 通信屏馈线模块配置

#define MMI_WIN_ID_DCAC_MODULE_SET           (62 | (MMI_WIN_TYPE_SET<<8))                 // 逆变模块参数配置
#define MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET (63 | (MMI_WIN_TYPE_SET<<8))                // 逆变屏馈线模块数量配置
#define MMI_WIN_ID_DCAC_FEEDER_MODULE_SET	 (64 | (MMI_WIN_TYPE_SET<<8))                 // 逆变屏馈线模块配置

#define MMI_WIN_ID_RELAY_OUT_SET            (65 | (MMI_WIN_TYPE_SET<<8))                 // 干接点输出设定
#define MMI_WIN_ID_ALARM_SET                (66 | (MMI_WIN_TYPE_SET<<8))                 // 报警设定
#define MMI_WIN_ID_BACKSTAGE_SET            (67 | (MMI_WIN_TYPE_SET<<8))                 // 远程通讯设置
#define MMI_WIN_ID_RTC_PASSWORD_SET         (68 | (MMI_WIN_TYPE_SET<<8))                 // RTC时间和密码设定

#define MMI_WIN_ID_HIS_FAULT_CLEAR          (69 | (MMI_WIN_TYPE_HIS_FAULT_CLEAR<<8))     // 历史告警记录清除
#define MMI_WIN_ID_EXCEPTION_CLEAR          (70 | (MMI_WIN_TYPE_EXCEPTION_CLEAR<<8))     // 掉电末复归告警记录清除
#define MMI_WIN_ID_RECORD_CLEAR             (71 | (MMI_WIN_TYPE_RECORD_CLEAR<<8))        // 事件记录清除
#define MMI_WIN_ID_BATT_RESTORE             (72 | (MMI_WIN_TYPE_BATT_CAP_RESTORE<<8))    // 电池容量恢复
#define MMI_WIN_ID_PARAM_RESTORE            (73 | (MMI_WIN_TYPE_PARAM_RESTORE<<8))       // 设置参数恢复

#define MMI_WIN_ID_CONFIRM_OPERATION        (74 | (MMI_WIN_TYPE_CONFIRM<<8))             // 确认操作提示
#define MMI_WIN_ID_OPERATION_SCRESS         (75 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 提示操作已成功

#define MMI_WIN_ID_AC_ADJUST                (76 | (MMI_WIN_TYPE_SET<<8))                 // 交流校准界面
#define MMI_WIN_ID_DC_VOLT_ADJUST           (77 | (MMI_WIN_TYPE_SET<<8))                 // 直流电压校准界面
#define MMI_WIN_ID_LOAD_CURR_ADJUST         (78 | (MMI_WIN_TYPE_SET<<8))                 // 负载电流校准界面
#define MMI_WIN_ID_BATT1_CURR_ADJUST        (79 | (MMI_WIN_TYPE_SET<<8))                 // 一组电池电流校准界面
#define MMI_WIN_ID_BATT2_CURR_ADJUST        (80 | (MMI_WIN_TYPE_SET<<8))                 // 二组电池电流校准界面
#define MMI_WIN_ID_ADJUST_DOING             (81 | (MMI_WIN_TYPE_DOING<<8))               // 校准正在进行提示界面
#define MMI_WIN_ID_ADJUST_SCUESS            (82 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 校准成功提示界面
#define MMI_WIN_ID_ADJUST_FAIL              (83 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 校准失败提示界面
#define MMI_WIN_ID_ADJUST_NEXT_CURR         (84 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 校准下一个电流点提示界面 

#define MMI_WIN_ID_DISPLAY                  (85 | (MMI_WIN_TYPE_SET<<8))                 // 显示设置界面

#define MMI_WIN_ID_ABOUT                    (86 | (MMI_WIN_TYPE_ABOUT<<8))               // 关于菜单

#define MMI_WIN_ID_NO_FUNCTION              (87 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 系统无此功能提示窗口

#define MMI_WIN_ID_GUIDELINE                (88 | (MMI_WIN_TYPE_MENU<<8))                // 维护指南菜单
#define MMI_WIN_ID_START                    (89 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 开通页面
#define MMI_WIN_ID_COMM_FAULT               (90 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 通讯异常报警页面
#define MMI_WIN_ID_VOLT_FAULT               (91 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 过欠压报警页面
#define MMI_WIN_ID_INSU_FAULT               (92 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 绝缘报警页面
#define MMI_WIN_ID_BACKSTAGE_FAULT          (93 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 后台通讯异常页面
#define MMI_WIN_ID_CONTACT_VENDER           (94 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 联系厂家页面
#define MMI_WIN_ID_REPAIR                   (95 | (MMI_WIN_TYPE_RUN_INFO<<8))            // 关于设备返修页面

#define MMI_WIN_ID_MANUAL_LIMIT_CURR_SET    (96 | (MMI_WIN_TYPE_SET<<8))                 // 手动限流调节设定
#define MMI_WIN_ID_COMM_OFFLINE_SET         (97 | (MMI_WIN_TYPE_SET<<8))                 // 通讯报警条件设定

#define MMI_WIN_ID_INSU_MEAS_SET            (98 | (MMI_WIN_TYPE_SET<<8))                 // 绝缘测量参数配置
#define MMI_WIN_ID_ZERO_CURR_SET            (99 | (MMI_WIN_TYPE_SET<<8))                 // 电流零点修正配置

#define MMI_WIN_ID_MAIN_WINDOW2             (100 | (MMI_WIN_TYPE_MAIN_WIN<<8))           // 主界面-2段母线数据
#define MMI_WIN_ID_DC10_PARAM_SET           (101 | (MMI_WIN_TYPE_SET<<8))                // 直流DC10参数配置

#define MMI_WIN_ID_AC_MODULE_SET            (102 | (MMI_WIN_TYPE_SET<<8))                // 交流系统DC10参数配置
#define MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET (103 | (MMI_WIN_TYPE_SET<<8))               // 交流系统馈线模块数量配置
#define MMI_WIN_ID_ACS_FEEDER_MODULE_SET	(104 | (MMI_WIN_TYPE_SET<<8))                // 交流系统馈电柜馈线模块配置

#define MMI_WIN_ID_ACS_AC_RUN_INFO_MENU     (105 | (MMI_WIN_TYPE_MENU<<8))               // 交流系统三相交流信息显示页面
#define MMI_WIN_ID_ACS_AC_RUN_INFO          (106 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 交流系统信息查询
#define MMI_WIN_ID_ACS_FEEDER_INFO          (107 | (MMI_WIN_TYPE_AC_FEEDER<<8))          // 交流系统馈出支路信息查询

#define MMI_WIN_ID_SWT_ON_OFF               (108 | (MMI_WIN_TYPE_SET<<8))                // 电操开关控制

#define MMI_WIN_ID_ECSWT_INFO               (109 | (MMI_WIN_TYPE_EC_SWITCH<<8))          // 电操开关状态查询

#define MMI_WIN_ID_FEEDER_SET_MENU2         (110 |  (MMI_WIN_TYPE_MENU<<8))              // 二段馈线柜参数设置菜单
#define MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2 (111 | (MMI_WIN_TYPE_SET<<8))            // 二段馈电柜馈线模块数量配置
#define MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2	(112 | (MMI_WIN_TYPE_SET<<8))                // 二段馈电柜馈线模块配置

#define MMI_WIN_ID_PB_CB_INSU_INFO2         (113 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 二段合控母绝缘状态查询
#define MMI_WIN_ID_BUS_INSU_INFO2           (114 | (MMI_WIN_TYPE_RUN_INFO<<8))           // 二段母线绝缘查询
#define MMI_WIN_ID_FEEDER_RUN_INFO_MENU2    (115 | (MMI_WIN_TYPE_MENU<<8))               // 二段馈线柜信息查询菜单
#define MMI_WIN_ID_DC_FEEDER_INFO2          (116 | (MMI_WIN_TYPE_DC_FEEDER<<8))          // 二段馈电柜馈出支路信息

#define MMI_WIN_MAX_CNT                     117


/*  数值类型  */
#define MMI_VAL_TYPE_MASK      0xF0
#define MMI_VAL_TYPE_U8_TYPE   0x00
#define MMI_VAL_TYPE_U16_TYPE  0x10
#define MMI_VAL_TYPE_U32_TYPE  0x20
#define MMI_VAL_TYPE_F32_TYPE  0x30

#define MMI_VAL_TYPE_NONE      0x00       // 条目没有数值
#define MMI_VAL_TYPE_ENUM      0x01       // 数值为字符类型
#define MMI_VAL_TYPE_U8        0x02       // 8位整数值按实际显示
#define MMI_VAL_TYPE_U8_2BIT   0x03       // 8位整数值显示为2位
#define MMI_VAL_TYPE_U8_3BIT   0x04       // 8位整数值显示为3位

#define MMI_VAL_TYPE_U16       0x10       // 16位整数值按实际显示
#define MMI_VAL_TYPE_U16_2BIT  0x11       // 16位整数值显示为2位
#define MMI_VAL_TYPE_U16_3BIT  0x12       // 16位整数值显示为3位
#define MMI_VAL_TYPE_U16_4BIT  0x13       // 16位整数值显示为4位

#define MMI_VAL_TYPE_U32       0x20       // 16位整数值按实际显示
#define MMI_VAL_TYPE_U32_2BIT  0x21       // 16位整数值显示为2位
#define MMI_VAL_TYPE_U32_3BIT  0x22       // 16位整数值显示为3位
#define MMI_VAL_TYPE_U32_4BIT  0x23       // 16位整数值显示为4位
#define MMI_VAL_TYPE_U32_5BIT  0x24       // 16位整数值显示为5位

#define MMI_VAL_TYPE_F32_1P    0x30       // 1位小数浮点类型，整数部分按实际长度显示
#define MMI_VAL_TYPE_F32_3W1P  0x31       // 宽度为3的1位小数浮点类型，整数部分显示1位
#define MMI_VAL_TYPE_F32_4W1P  0x32       // 宽度为4的1位小数浮点类型，整数部分显示2位
#define MMI_VAL_TYPE_F32_5W1P  0x33       // 宽度为5的1位小数浮点类型，整数部分显示3位
#define MMI_VAL_TYPE_F32_6W1P  0x34       // 宽度为5的1位小数浮点类型，整数部分显示3位
#define MMI_VAL_TYPE_F32_2P    0x35       // 2位小数浮点类型，整数部分按实际长度显示
#define MMI_VAL_TYPE_F32_4W2P  0x36       // 宽度为4的2位小数浮点类型，整数部分显示1位
#define MMI_VAL_TYPE_F32_5W2P  0x37       // 宽度为5的2位小数浮点类型，整数部分显示2位
#define MMI_VAL_TYPE_F32_6W2P  0x38       // 宽度为6的2位小数浮点类型，整数部分显示3位
#define MMI_VAL_TYPE_F32_3P    0x39       // 3位小数浮点类型，整数部分按实际长度显示
#define MMI_VAL_TYPE_F32_5W3P  0x3A       // 宽度为5的3位小数浮点类型，整数部分显示1位
#define MMI_VAL_TYPE_F32_6W3P  0x3B       // 宽度为6的3位小数浮点类型，整数部分显示2位
#define MMI_VAL_TYPE_F32_7W3P  0x3C       // 宽度为7的3位小数浮点类型，整数部分显示3位
	

/* 窗口状态常量定义 */
enum
{
	MMI_WIN_NORMAL = 0,             // 查看状态
	MMI_WIN_SET                     // 设置状态
};

/* 正反显示常量定义 */
enum
{
	MMI_NORMAL_DISP = 0,           // 正常显示
	MMI_INVERSE_DISP,              // 反白显示
};


/* 条目结构体定义 */
typedef struct
{
	U16_T    u16_name_index;       // 名称字符串索引值
	U8_T     u8_name_x;            // 名称字符串坐标x
	U8_T     u8_name_y;            // 名称字符串坐标y

	U16_T    u16_val_index;        // 如果数值类型为字符，则这个值表字符的起始索引，实际的字符串索引为u16_val_index+*((U8_T *)pv_val)
	                               // 如果数值类型不是字符类型，则这个值无意义，不使用

	U8_T     u8_val_x;             // 数值的坐标x
	U8_T     u8_val_y;             // 数值的坐标y

	U8_T     u8_val_type;          // 数值类型
	U32_T    u32_val_min;          // 最小值，如果数值类型为浮点数，则乘以1000转化为整形
	U32_T    u32_val_max;          // 最大值，如果数值类型为浮点数，则乘以1000转化为整形
	void   	 *pv_val;              // 指向数值的指针
}MMI_ITEM_T;

/* 图标结构体定义 */
typedef struct
{
	U16_T u16_index;               // icon数组索引值
	U8_T  u8_x;                    // x坐标
	U8_T  u8_y;                    // y坐标
}MMI_ICON_T;


typedef struct
{
	U16_T u16_id;                  // 窗口ID号
	U16_T u16_id_father;           // 父窗口的ID号，按"ESC"将返回的窗口
	U16_T u16_id_prev;             // 前一页窗口ID号
	U16_T u16_id_next;             // 后一页窗口ID号

	U8_T u8_sel_father;            // 当前页面在父窗口中的索引值
	U8_T u8_icon_cnt;              // icon数量
	U8_T u8_item_cnt;              // 条目数量

	MMI_ICON_T t_icon[MMI_WIN_ICON_MAX_CNT];
	MMI_ITEM_T t_item[MMI_WIN_ITEM_MAX_CNT];
}MMI_WIN_T;

typedef struct
{
	U16_T u16_curr_id;                          //当前页面ID
	MMI_WIN_T t_curr_win;

	union                                       //序号联合体，用于多页界面翻页处理时记录序号
	{
		U16_T u16_cell;                         //电池序号
		U16_T u16_feeder_module;                //馈线模块号                
		U16_T u16_feeder_branch;                //馈出支路序号
		U16_T u16_curr_fault;                   //当前故障序号           
		U16_T u16_his_fault;                    //历史故障序号
		U16_T u16_exception;                    //掉电末复归故障序号
		U16_T u16_rect_info;                    //整流模块信息序号
		U16_T u16_dcdc_module;                  //通信模块信息序号
		U16_T u16_dcac_module;                  //逆变模块信息序号
		U16_T u16_record;                       //充电记录序号
		U16_T u16_rect_on_off;                  //整流模块开关机序号
		U16_T u16_swt;                          //自带开关序号
		U16_T u16_swt_on_off;                   //电操开关序号
	}u_ordinal;

	U8_T  u8_special_name[MMI_WIN_ITEM_MAX_CNT][MMI_NAME_MAX_LEN];   //多页界面的条目名称一般需要临时生成，这个数组用来保存生成的名称
	U8_T  u8_item_inverse[MMI_WIN_ITEM_MAX_CNT];                     //页面的条目是否反显，MMI_INVERSE_DISP反显，MMI_NORMAL_DISP不反显
	U8_T  u8_buffer[43];                        //用于保存从dataflash中读取到故障名称、支路名称

	U8_T  u8_set_blink;                         //在设置状态下，当前闪烁的状态，MMI_INVERSE_DISP反显，NORMAL_DISPLAY不反显
	U8_T  u8_win_status;	                    //当前窗口的状态，MMI_WIN_NORMAL查看状态，MMI_WIN_SET设置状态 
	U8_T  u8_sel_index;                         //菜单和设置窗口下，当前选择的条目
	U8_T  u8_bit_index;                         //在设置状态下，表示当前设置的那一位
	void *pv_back_item_val;                     //保存正在设置条目的pv_val的值
	U32_T u32_set_value;                        //保存正在设置的值，设置时条目的pv_val指向该值

	RTC_TIME_T t_time;                          //rtc时间变量，用于设置和显示RTC时间
}MMI_WIN_RECORD_T;



/*************************************************************
函数名称: v_disp_share_data_restore		           				
函数功能: 恢复默认数据，将设置参数恢复到默认值						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_disp_cfg_data_restore(void);


/*************************************************************
函数名称: v_disp_cfg_data_init		           				
函数功能: 配置数据初始化						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_disp_cfg_data_init(void);


/*************************************************************
函数名称: v_disp_display_task		           				
函数功能: 显示和按键处理任务，该任务可通过RTX函数os_tsk_create来启动						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
extern __task void v_disp_display_task(void);

#endif
