/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：PublicData.h
版    本：1.00
创建日期：2012-04-13
作    者：郭数理
功能描述：全局ID定义头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-04-13  1.00     创建
**************************************************************/


#ifndef __PUBLIC_DATA_H_
#define __PUBLIC_DATA_H_


#include <rtl.h>
#include "Type.h"
#include "ShareDataStruct.h"

#define HW_VERSION "V1.0"

//公司代号.订制序号.标准软件版本号.订制软件版本序号
#define SW_VERSION "B45.1800.C40.1"

//配置工具版本
#define TOOL_VERSION "V1.16"  //增加后台“交流窜入电压及报警数据”上传


/************************** 型号声明 *******************************************/
#define PRODUCT_TYPE_SC12  0       //SC12
#define PRODUCT_TYPE_SC22  1       //SC22
#define PRODUCT_TYPE_SC32  2       //SC32

extern U8_T          g_u8_product_type;          //产品型号


/************************** 任务ID声明 *****************************************/
extern OS_TID g_tid_display;                     // 显示及按键处理任务ID
extern OS_TID g_tid_dc_sample;                   // 直流采集任务ID
extern OS_TID g_tid_ac_sample;                   // 交流采集任务ID
extern OS_TID g_tid_swt_sample;                  // 开关量采集任务ID
extern OS_TID g_tid_key;                         // 按键判断及消抖任务ID
extern OS_TID g_tid_fault;                       // 故障处理任务ID
extern OS_TID g_tid_batt;                        // 电池处理任务ID
extern OS_TID g_tid_com1_comm;                   // 串口1通信处理任务ID，串口1接整流模块、通信模块、逆变模块、电池巡检
extern OS_TID g_tid_can_comm;                    // CAN口通信处理任务ID
extern OS_TID g_tid_wdt;                         // 看门狗任务ID
extern OS_TID g_tid_bs;                          // 后台通信任务ID
extern OS_TID g_tid_load;                        // 烧写、导出文件任务ID
extern OS_TID g_tid_compare_time;                // 对时任务ID


/************************** 全局共享数据声明 ***********************************/
extern SHARE_DATA_T  g_t_share_data;             // 全局共享数据定义
extern OS_MUT        g_mut_share_data;           // 定义访问全局共享数据的互斥量

/************************** 电操开关数据声明 ***********************************/
extern const RC10_SWT_ITEM_T g_t_swt_sheet[];
extern U16_T u16_public_get_ctrl_swt_num(void);
extern U16_T u16_public_get_first_swt_index(void);
extern U16_T u16_public_get_last_swt_index(void);
extern U16_T u16_public_pre_mv_swt_index(U16_T *p_u16_index, U16_T n);
extern U16_T u16_public_next_mv_swt_index(U16_T *p_u16_index, U16_T n);
extern U16_T u16_public_get_swt_index_from_no(U16_T no);
extern void  v_public_fdl_swt_sync_update(void);


/******************* 显示及按键处理任务事件标志声明 ****************************/
#define KEY_K7                     0x0001
#define KEY_K6                     0x0002
#define KEY_K5                     0x0004
#define KEY_K4                     0x0008
#define KEY_K3                     0x0010
#define KEY_K2                     0x0020
#define KEY_EVT_FLAGS              (KEY_K7 | KEY_K6 | KEY_K5 | KEY_K4 | KEY_K3 | KEY_K2)

#define AC_ADJUST_SCUESS           0x0040     //交流校准成功
#define AC_ADJUST_FAIL             0x0080     //交流校准失败
#define DC_ADJUST_SCUESS           0x0100     //直流校准成功
#define DC_ADJUST_FAIL             0x0200     //直流校准失败
#define DC_ADJUST_FIRST_CURR_COMPLETE 0x0400  //第一个电流校准点校准完成
#define ADJUST_FLAGS               (AC_ADJUST_SCUESS | AC_ADJUST_FAIL | DC_ADJUST_SCUESS | DC_ADJUST_FAIL | DC_ADJUST_FIRST_CURR_COMPLETE)


#define FAULT_BUZZER_BEEP          0x0800    //蜂鸣器开启
#define FAULT_BUZZER_QUEIT         0x1000    //蜂鸣器关闭
#define FAULT_OCCURE               0x2000    //故障发生标志
#define BUZZER_EVT_FLAGS           (FAULT_BUZZER_BEEP | FAULT_BUZZER_QUEIT | FAULT_OCCURE)


/******************* 交流采集任务事件标志 ***************************************/
#define AC_ADJUST_FIRST_PATH_UV    0x0001     //一路UV电压校准
#define AC_ADJUST_FIRST_PATH_VW    0x0002     //一路VW电压校准
#define AC_ADJUST_FIRST_PATH_WU    0x0004     //一路WU电压校准
#define AC_ADJUST_SECOND_PATH_UV   0x0008     //二路UV电压校准
#define AC_ADJUST_SECOND_PATH_VW   0x0010     //二路VW电压校准
#define AC_ADJUST_SECOND_PATH_WU   0x0020     //二路WU电压校准
#define AC_ADJUST_EVT_FLAGS        (AC_ADJUST_FIRST_PATH_UV | AC_ADJUST_FIRST_PATH_VW | AC_ADJUST_FIRST_PATH_WU \
                                    | AC_ADJUST_SECOND_PATH_UV | AC_ADJUST_SECOND_PATH_VW | AC_ADJUST_SECOND_PATH_WU)


/******************* 直流采集任务事件标志 ***************************************/
#define DC_ADJUST_BATT_VOLT            0x0001     //电池电压校准
#define DC_ADJUST_PB_VOLT              0x0002     //合母电压校准
#define DC_ADJUST_CB_VOLT              0x0004     //控母电压校准
#define DC_ADJUST_BUS_NEG_TO_END_VOLT  0x0008     //母线电压校准
#define DC_ADJUST_LOAD_CURR_1          0x0010     //负载电池1校准
#define DC_ADJUST_LOAD_CURR_2          0x0020     //负载电流2校准
#define DC_ADJUST_BATT1_CURR_1         0x0040     //一组电池电流1校准
#define DC_ADJUST_BATT1_CURR_2         0x0080     //一组电池电流2校准
#define DC_ADJUST_BATT2_CURR_1         0x0100     //二组电池电流1校准
#define DC_ADJUST_BATT2_CURR_2         0x0200     //二组电池电流2校准
#define DC_ADJUST_EVT_FLAGS        (DC_ADJUST_BATT_VOLT | DC_ADJUST_PB_VOLT | DC_ADJUST_CB_VOLT | DC_ADJUST_BUS_NEG_TO_END_VOLT \
                                    | DC_ADJUST_LOAD_CURR_1 | DC_ADJUST_LOAD_CURR_2 | DC_ADJUST_BATT1_CURR_1 | \
                                    DC_ADJUST_BATT1_CURR_2 | DC_ADJUST_BATT2_CURR_1 | DC_ADJUST_BATT2_CURR_2)


/******************* 电池管理任务事件标志 **************************************/
#define BATT_CAPACITY_RESTORE      0x0001     //恢复电池容量
#define RECT_SET_CURR_SCUESS1      0x0002     //一组充电模块限流广播命令完成
#define RECT_SET_CURR_SCUESS2      0x0004     //二组充电模块限流广播命令完成


/******************* 看门狗任务事件标志 ****************************************/
#define DISPLAY_FEED_DOG           0x0001
#define AC_SAMPLE_FEED_DOG         0x0002
#define	DC_SAMPLE_FEED_DOG         0x0004
#define	SWT_SAMPLE_FEED_DOG        0x0008
#define	KEY_FEED_DOG               0x0010
#define	FAULT_FEED_DOG             0x0020
#define BATT_FEED_DOG              0x0040
#define	COM1_FEED_DOG              0x0080
#define CAN_FEED_DOG               0x0100
#define BACKSTAGE_FEED_DOG         0x0200
#define LOAD_FILE_FEED_DOG         0x0400
#define COMPARE_TIME_FEED_DOG      0x0800

#define FEED_DOG_EVT_FLAGS         (DISPLAY_FEED_DOG | AC_SAMPLE_FEED_DOG | DC_SAMPLE_FEED_DOG \
                                    | SWT_SAMPLE_FEED_DOG | KEY_FEED_DOG | FAULT_FEED_DOG \
									| BATT_FEED_DOG | COM1_FEED_DOG | CAN_FEED_DOG \
									| BACKSTAGE_FEED_DOG |LOAD_FILE_FEED_DOG | COMPARE_TIME_FEED_DOG )

#endif
