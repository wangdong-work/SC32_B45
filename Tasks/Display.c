/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Display.c
版    本：1.00
创建日期：2012-04-18
作    者：郭数理
功能描述：显示和按键处理相关的实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-04-18  1.00     创建
**************************************************************/

#include <rtl.h>
#include <string.h>
#include <stdio.h>

#include "../Drivers/Lcd.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Delay.h"
#include "../Drivers/Dataflash.h"

#include "CompareTime.h"
#include "PublicData.h"
#include "ShareDataStruct.h"
#include "Display.h"
#include "DisplayString.h"
#include "Type.h"
#include "FetchFlash.h"
#include "Fault.h"
#include "FaultId.h"
#include "Log.h"

#define MMI_REFRESH_INTERVAL     10         //刷屏间隔10个tick，100ms
#define MMI_RETURN_MAIN_WIN_TIME 180000000  //(3*60*1000*1000), 3分钟没有按键操作关背光返回主界面

#define MMI_ICON_UP_X          143     //"UP"图标的X坐标
#define MMI_ICON_UP_Y          2       //"UP"图标的Y坐标
#define MMI_ICON_DOWN_X        143     //"DOWN"图标的X坐标
#define MMI_ICON_DOWN_Y        145     //"DOWN"图标的X坐标

#define MMI_CELL_CNT           10      //电池单体电压页面显示的电池节数
#define MMI_INSU_CNT           5       //馈出支路绝缘页面显示的支路数
#define MMI_SWT_CNT            5       //馈出支路开关状态页面显示的支路数
#define MMI_HIS_FAULT_CNT      2       //历史故障页面显示的故障条数
#define MMI_CURR_FAULT_CNT     2       //当前故障页面显示的故障条数
#define MMI_EXCEPTION_CNT      2       //掉电末复归故障页面显示的故障条数
#define MMI_RECORD_CNT         3       //事件记录页面显示的记录条数
#define MMI_RECT_ON_OFF_CNT    10      //整流模块开关机页面显示的模块数量
#define MMI_SWT_ON_OFF_CNT     10      //电操开关页面显示的开关数量
#define MMI_FEEDER_BRANCH_CNT  2       //馈出支路信息页面显示的馈出支路数量

#define MMI_FAULT_NAME_WIDTH   120     //故障名称的宽度

#define MMI_AC_FEEDER_MODULE_ADDR    1       //交流屏馈线模块起始地址
#define MMI_FEEDER_PANEL_MODULE_ADDR 14      //馈线屏馈线模块起始地址
#define MMI_DCDC_FEEDER_MODULE_ADDR  112     //通信屏馈线模块起始地址
#define MMI_DCAC_FEEDER_MODULE_ADDR  114     //UPS屏馈线模块起始地址

#define MMI_USER_PASSWORD      11111   //用户默认密码
#define MMI_SUPER_PASSWORD     2051    //用户级超级密码
#define MMI_DEFEND_PASSWORD    2012    //维护级密码

static U16_T  m_u16_swt_index=0;                    //电操开关索引号

/* 与220V和110V系统相关的电压值 */
typedef struct
{
	F32_T  f32_batt_equ_volt;                 // 电池均充电压
	F32_T  f32_batt_flo_volt;                 // 电池浮充电压
	U16_T  u16_batt_total_end_volt;           // 电池放电终止电压
	F32_T  f32_batt_high_volt_limit;          // 电池组过压点
	F32_T  f32_batt_low_volt_limit;           // 电池组欠压点
	       
	F32_T  f32_rect_out_volt;                 // 充电模块输出电压
	F32_T  f32_rect_max_out_volt;             // 充电模块输出电压上限
	F32_T  f32_rect_min_out_volt;             // 充电模块输出电压下限
	F32_T  f32_rect_offline_out_volt;         // 充电模块默认输出电压
	       
	U16_T  u16_pb_high_volt;                  // 合母过压门限
	U16_T  u16_pb_low_volt;                   // 合母欠压门限
	U16_T  u16_cb_high_volt;                  // 控母过压门限
	U16_T  u16_cb_low_volt;                   // 控母欠压门限
	U16_T  u16_bus_high_volt;                 // 母线过压门限
	U16_T  u16_bus_low_volt;                  // 母线欠压门限
	U16_T  u16_cb_output_volt;                // 控母输出电压
}DC_VOLT_VALUE_T;

/* 220V系统电压默认值，110V系统除以2 */
static const DC_VOLT_VALUE_T m_t_dc_volt_def =
{
	245,      // 电池均充电压
	235,      // 电池浮充电压
	200,      // 电池放电终止电压
	264,      // 电池组过压点
	187,      // 电池组欠压点
	
	220,      // 充电模块输出电压
	286,      // 充电模块输出电压上限
	176,      // 充电模块输出电压下限
	220,      // 充电模块默认输出电压
	
	286,      // 合母过压门限
	186,      // 合母欠压门限
	235,      // 控母过压门限
	205,      // 控母欠压门限
	286,      // 母线过压门限
	186,      // 母线欠压门限
	220,      // 控母输出电压
};

/* 220V系统电压最小值，110V系统除以2 */
static const DC_VOLT_VALUE_T m_t_dc_volt_min =
{
	200000,   // 电池均充电压，浮点数乘以1000
	200000,   // 电池浮充电压，浮点数乘以1000
	190,      // 电池放电终止电压
	220000,   // 电池组过压点，浮点数乘以1000
	186000,   // 电池组欠压点，浮点数乘以1000
	
	176000,   // 充电模块输出电压，浮点数乘以1000
	220000,   // 充电模块输出电压上限，浮点数乘以1000
	176000,   // 充电模块输出电压下限，浮点数乘以1000
	176000,   // 充电模块默认输出电压，浮点数乘以1000
	
	220,      // 合母过压门限
	186,      // 合母欠压门限
	220,      // 控母过压门限
	198,      // 控母欠压门限
	220,      // 母线过压门限
	186,      // 母线欠压门限
	210,      // 控母输出电压
};

/* 220V系统电压最大值，110V系统除以2 */
static const DC_VOLT_VALUE_T m_t_dc_volt_max =
{
	300000,   // 电池均充电压，浮点数乘以1000
	300000,   // 电池浮充电压，浮点数乘以1000
	220,      // 电池放电终止电压
	320000,   // 电池组过压点，浮点数乘以1000
	220000,   // 电池组欠压点，浮点数乘以1000
	
	286000,   // 充电模块输出电压，浮点数乘以1000
	286000,   // 充电模块输出电压上限，浮点数乘以1000
	220000,   // 充电模块输出电压下限，浮点数乘以1000
	286000,   // 充电模块默认输出电压，浮点数乘以1000
	
	320,      // 合母过压门限
	220,      // 合母欠压门限
	242,      // 控母过压门限
	220,      // 控母欠压门限
	320,      // 母线过压门限
	220,      // 母线欠压门限
	230,      // 控母输出电压
};	

/* 记录当前显示的窗口ID，设置条目索引等信息 */
static MMI_WIN_RECORD_T m_t_win_record;


/* 记录设置菜单条目的数量 */
static U8_T m_u8_set_menu_item_cnt;

/* 备份系统电压等级 */
SYS_VOLT_LEVEL_E m_e_volt_level;

/* 备份馈线屏号和馈线模块号 */
U8_T m_u8_feeder_panel_ordianl;               //馈线屏号
U16_T m_u16_feeder_module_ordianl;            //馈线模块号

/* 按键变量定义 */
U16_T g_u16_key_cancel;
U16_T g_u16_key_enter;
U16_T g_u16_key_up;
U16_T g_u16_key_down;
U16_T g_u16_key_add;
U16_T g_u16_key_sub;

/* LCD方向备份 */
LCD_DIRECTION_E m_e_lcd_direction;

/* 语言备份 */
LANG_TYPE_E     m_e_lang;


/* 界面结构体数组 */
static const MMI_WIN_T m_t_win_array[MMI_WIN_MAX_CNT] =
{
	/******************* 一段母线主界面结构体初始化 ******************************/
	{
        //u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MAIN_WINDOW,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_MAIN_WINDOW2,  0,             2,           10,
		
		//初始化icon结构数组
		{
			// u16_index   u8_x   u8_y
			{ ICON_CLOCK,  3,     3 }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X+4, MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x   u8_val_y  u8_val_type          u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME,  16,        3,         0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 0,                        3,         19,        0,             115,       19,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature) },
			{ 1,                        3,         32,        0,             115,       32,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt) },
			{ 2,                        3,         45,        0,             115,       45,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]) },
			{ 3,                        3,         58,        0,             115,       58,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity) },
			{ 4,                        3,         71,        0,             115,       71,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr) },
			{ 8,                        3,         132,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 14,                       3,         145,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL }, 
			{ 5,                        3,         84,        0,             115,       84,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt) },
			{ 7,                        3,         97,        0,             115,       97,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt) },
		}
	},	

	/******************* 主菜单界面结构体初始化 ****************************/
	{
		//u16_id               u16_id_father            u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_MAIN_WINDOW,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           12,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x   u8_val_y   u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 23,             3,         3,         0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 603,            3,         16,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 24,             3,         29,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 25,             3,         42,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 26,             3,         55,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 27,             3,         68,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 28,             3,         81,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 29,             3,         94,        0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 30,             3,         107,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 31,             3,         120,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 447,            3,         133,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 32,             3,         146,       0,             0,         0,         MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 直流系统信息查询菜单界面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           10,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 33,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 34,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 35,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 36,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 37,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 38,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 39,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 613,           3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 45,            3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 46,            3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 馈线柜信息查询菜单界面结构体初始化 ******************************/
	{
		//u16_id                          u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_RUN_INFO_MENU,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           5,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 40,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 41,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 42,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 43,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 44,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 通信系统信息查询菜单界面结构体初始化 ******************************/
	{
		//u16_id                        u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           3,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 49,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 50,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 51,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 逆变系统信息查询菜单界面结构体初始化 ******************************/
	{
		//u16_id                        u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  4,             0,           3,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 52,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 53,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 54,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 参数设置菜单界面结构体初始化 ******************************/
	{
		//u16_id              u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  9,             0,           12,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 55,             3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 591,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 56,             3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 57,             3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 58,             3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 59,             3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 60,             3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 61,             3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 63,             3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 64,             3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 65,             3,         133,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 517,            3,         146,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },	
		}
	},
	
	/******************* 直流参数设置菜单界面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           8,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 66,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 67,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 68,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 69,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 70,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 71,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 611,           3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 80,            3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 馈线柜参数设置菜单界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_SET_MENU,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           5,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 72,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 73,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 74,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 75,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 76,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 末配置此项提示界面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_CONFIG,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 81,             6,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 82,             6,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 三相交流信息显示页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_TRIPHASE_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           9,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 83,             3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 84,             3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv) },
			{ 87,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw) },
			{ 88,             9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu) },
			{ 85,             3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         94,        0,             110,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv) },
			{ 87,             9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw) },
			{ 88,             9,         120,       0,             110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu) },
		}
	},

	/******************* 单相交流信息显示页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_UNIPHASE_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           5,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 83,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 89,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 91,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv) },
			{ 90,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 91,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv) },
		}
	},

	/******************* 电池组运行数据页面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_TOTAL_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS2_CELL_INFO,  MMI_WIN_ID_BATT_GROUP_INFO,  2,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 103,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 93,             9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity) },
			{ 94,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt) },
			{ 95,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]) },
			{ 96,             9,         55,        100,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           &(g_t_share_data.t_rt_data.t_batt.e_state[0]) },
			{ 97,             9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16,     0,           0,           &(g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[0]) },
		}
	},
	
	/******************* 电池数据页面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_GROUP_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BATT_TOTAL_INFO,  MMI_WIN_ID_BMS_GROUP1_INFO,  2,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index   u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 104,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 93,             9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity) },
			{ 94,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt2_volt) },
			{ 95,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]) },
			{ 96,             9,         55,        100,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           &(g_t_share_data.t_rt_data.t_batt.e_state[1]) },
			{ 97,             9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16,     0,           0,           &(g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[1]) },
		}
	},

	/******************* 一组电池巡检数据页面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                  u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS_GROUP1_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BATT_GROUP_INFO,  MMI_WIN_ID_BMS1_CELL_INFO,  2,             2,           8,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 107,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 0,              9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_temperature1) },
			{ 109,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_cell_max_volt_id) },
			{ 112,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_max_cell_volt) },
			{ 110,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_cell_min_volt_id) },
			{ 112,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_min_cell_volt) },
		}
	},

	/******************* 一组单体电压数据页面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father                 u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS1_CELL_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS_GROUP1_INFO,  MMI_WIN_ID_BMS_GROUP2_INFO,  2,             2,           11,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 113,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             80,       16,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         29,        0,             80,       29,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        0,             80,       42,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         55,        0,             80,       55,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        0,             80,       68,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             80,       81,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        0,             80,       94,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         107,       0,             80,       107,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       0,             80,       120,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         133,       0,             80,       133,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL }
		}
	},
	
	/******************* 二组电池巡检数据页面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS_GROUP2_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS1_CELL_INFO,  MMI_WIN_ID_BMS2_CELL_INFO,  2,             2,           8,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 108,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 0,              9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_temperature1) },
			{ 109,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_cell_max_volt_id) },
			{ 112,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_max_cell_volt) },
			{ 110,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 111,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8,      0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_cell_min_volt_id) },
			{ 112,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_3P,  0,           0,           &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_min_cell_volt) },
		}
	},

	/******************* 二组单体电压数据页面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father                 u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BMS2_CELL_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_BMS_GROUP2_INFO,  MMI_WIN_ID_BATT_TOTAL_INFO,  2,             2,           11,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x             u8_y
			{ ICON_UP,   MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN, MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 114,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             80,       16,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         29,        0,             80,       29,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        0,             80,       42,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         55,        0,             80,       55,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        0,             80,       68,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             80,       81,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        0,             80,       94,       MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         107,       0,             80,       107,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       0,             80,       120,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         133,       0,             80,       133,      MMI_VAL_TYPE_F32_3P,  0,           0,           NULL }
		}
	},

	/******************* 合控母线绝缘状态查询页面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PB_CB_INSU_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           8,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 115,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 116,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 117,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 120,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         107,       0,             115,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
		}
	},
	
	/******************* 一段绝缘状态查询页面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BUS_INSU_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           7,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 115,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 118,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 120,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         81,        0,             115,      81,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL }
		}
	},

	/******************* AC/DC模块信息查询页面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father                 u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECT_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_RECT_INFO,  MMI_WIN_ID_RECT_INFO,  4,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 123,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 125,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 126,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 127,                            9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 128,                            9,         55,        130,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 129,                            9,         68,        132,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* 开关状态查询页面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father                 u16_id_prev              u16_id_next              u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SWITCH_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_SWITCH_INFO,  MMI_WIN_ID_SWITCH_INFO,  5,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         pv_val  u32_val_min  u32_val_max
			{ 135,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        141,           100,      29,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        141,           100,      55,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        141,           100,      81,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        141,           100,      107,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       141,           100,      131,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 }
		}
	},
	
	/******************* 馈电柜馈出支路信息查询页面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father                     u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_FEEDER_INFO,  MMI_WIN_ID_FEEDER_RUN_INFO_MENU,  MMI_WIN_ID_DC_FEEDER_INFO,  MMI_WIN_ID_DC_FEEDER_INFO,  1,             2,           11,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 143,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         42,        154,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         55,        154,           110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         68,        154,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         107,       151,           110,      107,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         120,       154,           110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         133,       154,           110,      133,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         146,       154,           110,      146,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* 通信模块信息查询页面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father                   u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_INFO,  MMI_WIN_ID_DCDC_RUN_INFO_MENU,  MMI_WIN_ID_DCDC_INFO,  MMI_WIN_ID_DCDC_INFO,  1,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 124,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 125,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 126,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 127,                            9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 128,                            9,         55,        130,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 129,                            9,         68,        132,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* 通信屏馈出支路信息查询页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father                   u16_id_prev                   u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_FEEDER_INFO,  MMI_WIN_ID_DCDC_RUN_INFO_MENU,  MMI_WIN_ID_DCDC_FEEDER_INFO,  MMI_WIN_ID_DCDC_FEEDER_INFO,  2,             2,           9,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 144,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         42,        151,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         55,        154,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         94,        151,           110,      94,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         107,       151,           110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         120,       154,           110,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* 逆变模块信息查询页面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father                   u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_INFO,  MMI_WIN_ID_DCAC_RUN_INFO_MENU,  MMI_WIN_ID_DCAC_INFO,  MMI_WIN_ID_DCAC_INFO,  1,             2,           11,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 157,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 125,                            9,         16,        0,             100,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 126,                            9,         29,        0,             100,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 158,                            9,         42,        0,             100,      42,       MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 159,                            9,         55,        0,             100,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 160,                            9,         68,        0,             100,      68,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 161,                            9,         81,        0,             100,      81,       MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 162,                            9,         94,        0,             100,      94,       MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 163,                            9,         107,       0,             100,      107,      MMI_VAL_TYPE_F32_2P,  0,           0,           NULL },
			{ 164,                            9,         120,       165,           100,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 168,                            9,         133,       169,           100,      133,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
	
	/******************* 逆变屏馈出支路信息查询页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father                   u16_id_prev                   u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_FEEDER_INFO,  MMI_WIN_ID_DCAC_RUN_INFO_MENU,  MMI_WIN_ID_DCAC_FEEDER_INFO,  MMI_WIN_ID_DCAC_FEEDER_INFO,  2,             2,           9,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index           u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 145,                     3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME, 9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                     9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                     9,         42,        151,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                     9,         55,        154,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME, 9,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                     9,         94,        151,           110,      94,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                     9,         107,       151,           110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                     9,         120,       154,           110,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},

	/******************* 当前故障界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father          u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_CURR_FAULT_INFO,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_CURR_FAULT_INFO,  MMI_WIN_ID_CURR_FAULT_INFO,  5,             2,           7,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 171,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 无当前故障提示界面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_CURR_FAULT,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  5,             0,           2,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 177,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 178,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 历史故障界面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father          u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_HIS_FAULT_INFO,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_HIS_FAULT_INFO,  MMI_WIN_ID_HIS_FAULT_INFO,  6,             2,           9,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 172,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 176,  3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 176,  3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 无历史故障提示界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_HIS_FAULT,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           1,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x   u8_y
			{ 0,          0,     0 }, 
			{ 0,          0,     0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type        u32_val_min  u32_val_max  pv_val  
			{ 179,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE, 0,           0,           NULL },
		}
	},

	/******************* 掉电末复归故障界面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father          u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_EXCEPTION_INFO,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_EXCEPTION_INFO,  MMI_WIN_ID_EXCEPTION_INFO,  7,             2,           7,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 173,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175,  3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 无掉电末复归故障提示界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_EXCEPTION,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           2,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x   u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 180,            10,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 181,            10,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 事件记录界面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father     u16_id_prev         u16_id_next         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECORD,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_RECORD,  MMI_WIN_ID_RECORD,  8,             2,           7,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                 u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 174, 3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,       3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175, 3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,       3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175, 3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,       3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 175, 3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 无事件记录界面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_RECORD,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  8,             0,           1,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type          u32_val_min  u32_val_max  pv_val  
			{ 182,            6,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 输入密码界面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INPUT_PASSWORD, MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  9,             0,           1,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x,  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 184,            3,         16,        0,             60,       29,       MMI_VAL_TYPE_U32_5BIT,  0,           99999,       &(m_t_win_record.u32_set_value) },
		}
	},

	/******************* 密码错误提示界面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father               u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PASSWORD_ERROR, MMI_WIN_ID_INPUT_PASSWORD,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           1,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x, u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 185,            6,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 参数设置超限提示界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PARAM_OUT_RANGE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           5,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }  
		},

		//初始化item结构数组					
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 186,                            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 187,                            9,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 188,                            9,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 189,  9,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 190,  9,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 直流系统设置界面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father            u16_id_prev                  u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_SYSTEM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_METER2_SET,  MMI_WIN_ID_AC_PARAM_SET,  1,             2,           12,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组					
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 191,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 615,            9,         16,        616,           110,      16,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num) },
			{ 192,            9,         29,        197,           110,      29,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level) },
			{ 193,            9,         42,        199,           110,      42,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb) },
			{ 195,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  210,         230,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt) },
			{ 194,            9,         68,        201,           110,      68,       MMI_VAL_TYPE_ENUM,      0,           4,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl) },
			{ 196,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8,        0,           4,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num) },  
			{  48,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8,        0,           4,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num) },
			{ 610,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U8_2BIT,   0,           11,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num) },
			{ 618,            3,         120,       0,             0,        120,      MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 619,            9,         133,       0,             110,      133,      MMI_VAL_TYPE_U8_3BIT,   14,          111,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add) },
		    { 620,            9,         146,       0,             110,      146,      MMI_VAL_TYPE_U8_3BIT,   14,          111,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add) },
		}
	},

	/******************* 交流参数配置界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC_SYSTEM_SET,  MMI_WIN_ID_BATT_PARAM_SET,  1,             2,           4,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 206,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 207,            9,         16,        210,           100,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase) },
			{ 208,            9,         29,        212,           100,      29,       MMI_VAL_TYPE_ENUM,  0,           2,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path) },
			{ 209,            9,         42,        215,           100,      42,       MMI_VAL_TYPE_ENUM,  0,           3,           &(g_t_share_data.t_sys_cfg.t_ctl.u16_ac) },
		}
	},

	/******************* 电池配置界面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father            u16_id_prev               u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_AC_PARAM_SET,  MMI_WIN_ID_DC_PARAM_SET,  1,             2,           3,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 219,           3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 220,           9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,       0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num) },
			{ 221,           9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_4BIT, 10,          3000,        &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10) },
		}
	},

	/******************* 直流参数配置界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_PARAM_SET,  MMI_WIN_ID_DC10_PARAM_SET,  1,             2,           9-2,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x             u8_y
			{ ICON_UP,    MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 222,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 223,            9,         16,        224,           110,      16,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt) },
			{ 226,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range) },
			{ 227,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt1_shunt_range) },
			{ 228,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt2_shunt_range) },
			{ 229,            3,         68,        0,             110,      68,       MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 230,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_4BIT,  10,          1000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_feeder_shunt_range) },
			{ 564,            3,         94,        0,             110,      94,       MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 565,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U8     ,   0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num) },
		}
	},

	/******************* 整流模块参数配置界面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father            u16_id_prev               u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECT_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC10_PARAM_SET,  MMI_WIN_ID_BATT_METER1_SET,  1,             2,           6,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 231,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 232,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8_2BIT,   1,          24,          &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num) },
			{ 233,            9,         29,        530,           110,      29,       MMI_VAL_TYPE_ENUM,      0,          7,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr) },
			{ 234,            9,         42,        375,           110,      42,       MMI_VAL_TYPE_ENUM,      0,          0,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_protocol) },
			{ 235,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_5W1P,  176000,     286000,      &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt) },   //浮点数*1000来处理
			{ 236,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_5W1P,  5000,       110000,      &(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_curr_percent) },   //浮点数*1000来处理
		}
	},

	/******************* 一组电池巡检配置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_METER1_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_RECT_PARAM_SET,  MMI_WIN_ID_BATT_METER2_SET,  1,             2,           8,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 244,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 246,            9,         16,        253,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           2,           &(g_t_share_data.t_sys_cfg.t_batt.e_bms_type) },
			{ 247,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8,       0,           5,           &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num) },
			{ 248,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[0]) },
			{ 249,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[1]) },
			{ 250,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[2]) },
			{ 251,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[3]) },
			{ 252,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[4]) },
		}
	},
	
	/******************* 二组电池巡检配置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                  u16_id_next                u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_METER2_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_METER1_SET,  MMI_WIN_ID_DC_SYSTEM_SET,  1,             2,           7,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 245,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 247,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,       0,           5,           &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num) },
			{ 248,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[0]) },
			{ 249,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[1]) },
			{ 250,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[2]) },
			{ 251,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[3]) },
			{ 252,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_3BIT,  1,           24,          &(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[4]) },
		}
	},
	
	/******************* 馈电柜馈线模块数量配置界面结构体初始化 ******************************/
	{
		//u16_id                                 u16_id_father                u16_id_prev                          u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET,  MMI_WIN_ID_FEEDER_SET_MENU,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  1,             2,           2,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 262,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           4,           NULL },
		}
	},
	
	/******************* 馈电柜馈线模块配置界面结构体初始化 ******************************/
	{
		//u16_id                             u16_id_father                u16_id_prev                              u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  MMI_WIN_ID_FEEDER_SET_MENU,  MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET,  1,             2,           8,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 264,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 268,                            9,         42,        260,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 269,                            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 270,                            9,         68,        260,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 271,                            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 272,                            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
		}
	},

	/******************* 交流门限设置界面结构体初始化 ******************************/
	{
		//u16_id                u16_id_father            u16_id_prev               u16_id_next             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_INSU_THR_SET,  MMI_WIN_ID_DC_THR_SET,  3,             2,           4,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 273,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 274,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U16_3BIT,  220,        530,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_high_volt) },
			{ 275,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  186,        380,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_low_volt) },
			{ 276,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  0,          380,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_lack_phase) },
		}
	},

	/******************* 直流门限设置界面结构体初始化 ******************************/
	{
		//u16_id                u16_id_father            u16_id_prev             u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_AC_THR_SET,  MMI_WIN_ID_BATT_THR_SET,  3,             2,           7,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 277,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 278,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U16_3BIT,  220,         320,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt) },
			{ 279,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  186,         220,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt) },
			{ 280,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  220,         242,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt) },
			{ 281,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  198,         220,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt) },
			{ 282,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16_3BIT,  220,         320,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt) },
			{ 283,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_3BIT,  186,         220,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt) },
		}
	},

	/******************* 电池门限值设置界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev             u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC_THR_SET,  MMI_WIN_ID_INSU_THR_SET,  3,             2,           8,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 284,           3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 285,           9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_5W1P,  220000,      320000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit) },   //浮点数*1000来处理
			{ 286,           9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_5W1P,  176000,      220000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit) },   //浮点数*1000来处理
			{ 287,           9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_5W2P,  2000,        15000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_cell_high_volt) },   //浮点数*1000来处理
			{ 288,           9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_5W2P,  1800,        12000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_cell_low_volt) },   //浮点数*1000来处理
			{ 289,           9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_5W2P,  2000,        15000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_tail_high_volt) },   //浮点数*1000来处理
			{ 290,           9,         81,        0,             110,      81,       MMI_VAL_TYPE_F32_5W2P,  1800,        12000,       &(g_t_share_data.t_sys_cfg.t_batt.f32_tail_low_volt) },   //浮点数*1000来处理
			{ 291,           9,         94,        0,             110,      94,       MMI_VAL_TYPE_F32_4W2P,  100,         1000,        &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit) },   //浮点数*1000来处理
		}
	},

	/******************* 绝缘门限报警设置界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev               u16_id_next             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INSU_THR_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_THR_SET,  MMI_WIN_ID_AC_THR_SET,  3,             2,           4,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 292,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 293,            9,         16,        0,             125,      16,       MMI_VAL_TYPE_U16_3BIT,  20,         100,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_volt_imbalance) },
			{ 294,            9,         29,        0,             125,      29,       MMI_VAL_TYPE_U16_2BIT,  5,          99,          &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_res_thr) },
			{ 515,            9,         42,        0,             125,      42,       MMI_VAL_TYPE_U16_2BIT,  1,          50,          &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_dc_bus_input_ac_thr) },
		}
	},

	/******************* 电池充电参数设置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                     u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_CHARGE_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_DISCHARGE_SET,  MMI_WIN_ID_BATT_TO_FLO_SET,  4,             2,           5,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 295,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 296,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_5W1P, 200000,      300000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt) },   //浮点数*1000来处理
			{ 297,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_5W1P, 200000,      300000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt) },   //浮点数*1000来处理
			{ 298,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_5W1P, 0,           500000,      &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt) },   //浮点数*1000来处理
			{ 299,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_F32_4W2P, 50,          400,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr) },   //浮点数*1000来处理
		}
	},

	/******************* 电池转浮充判据界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                  u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_TO_FLO_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_CHARGE_SET,  MMI_WIN_ID_BATT_TO_EQU_SET,  4,             2,           5,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 300,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 301,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_4W2P,  10,          30,          &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_flo_curr) },   //浮点数*1000来处理
			{ 302,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_U16_3BIT,  1,           360,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_curr_go_time) },
			{ 303,            60,        42,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 304,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_equ_time) },
		}
	},

	/******************* 电池转均充判据界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_TO_EQU_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_TO_FLO_SET,  MMI_WIN_ID_BATT_DISCHARGE_END_SET,  4,             2,           7,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 305,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 306,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_4W2P,  40,          100,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr) },   //浮点数*1000来处理
			{ 307,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_to_equ_dur_time) },
			{ 303,            60,        42,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 308,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_ac_fail_time) },
			{ 303,            60,        68,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 309,            9,         81,        0,             115,      81,       MMI_VAL_TYPE_U16_4BIT,  1,           9999,        &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_equ_cycle) },
		}
	},
	
	/******************* 电池核容终止设置界面结构体初始化 ******************************/
	{
		//u16_id                            u16_id_father            u16_id_prev                  u16_id_next                     u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_DISCHARGE_END_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_TO_EQU_SET,  MMI_WIN_ID_BATT_DISCHARGE_SET,  4,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 310,            3,        3,          0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 311,            9,        16,         0,             115,      16,       MMI_VAL_TYPE_U16_3BIT,  1,           999,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_dis_time) },
			{ 303,            60,       29,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 312,            9,        42,         0,             115,      42,       MMI_VAL_TYPE_U16_3BIT,  190,         220,         &(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt) },
			{ 303,            60,       55,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 313,            9,        68,         0,             115,      68,       MMI_VAL_TYPE_F32_4W1P,  1800,        12000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_cell_end_volt) },   //浮点数*1000来处理
		}
	},

	/******************* 电池放电曲线设置界面结构体初始化 ******************************/
	{
		//u16_id                        u16_id_father            u16_id_prev                         u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_DISCHARGE_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_DISCHARGE_END_SET,  MMI_WIN_ID_BATT_CHARGE_SET,  4,             2,           11,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 314,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 315,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_01c_dis_rate) },   //浮点数*1000来处理
			{ 316,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_02c_dis_rate) },   //浮点数*1000来处理
			{ 317,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_03c_dis_rate) },   //浮点数*1000来处理
			{ 318,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_04c_dis_rate) },   //浮点数*1000来处理
			{ 319,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_05c_dis_rate) },   //浮点数*1000来处理
			{ 320,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_06c_dis_rate) },   //浮点数*1000来处理
			{ 321,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_07c_dis_rate) },   //浮点数*1000来处理
			{ 322,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_08c_dis_rate) },   //浮点数*1000来处理
			{ 323,            9,         120,       0,             110,      120,      MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_09c_dis_rate) },   //浮点数*1000来处理
			{ 324,            9,         131,       0,             110,      131,      MMI_VAL_TYPE_F32_5W2P,  0,           10000,       &(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_10c_dis_rate) },   //浮点数*1000来处理
		}
	},
	
	/******************* 系统控制方式界面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father            u16_id_prev              u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SYSTEM_CTL_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_RECT_ON_OFF,  MMI_WIN_ID_BATT_CTL_SET,  5,             2,           2,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 325,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 128,            3,         29,        130,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0]) },
		}
	},

	/******************* 电池充电方式界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                 u16_id_next              u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_CTL_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_SYSTEM_CTL_SET,  MMI_WIN_ID_RECT_ON_OFF,  5,             2,           3,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 326,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 327,            3,         29,        100,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           2,           &(g_t_share_data.t_sys_cfg.t_ctl.u16_batt[0]) },
			{ 590,            3,         55,        100,           110,      55,       MMI_VAL_TYPE_ENUM,  0,           2,           &(g_t_share_data.t_sys_cfg.t_ctl.u16_batt[1]) },
		}
	},

	/******************* 整流模块开关机界面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father            u16_id_prev               u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECT_ON_OFF,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_BATT_CTL_SET,  MMI_WIN_ID_SYSTEM_CTL_SET,  5,             2,           11,
		                                                          
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 328,                            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         16,        132,           110,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         29,        132,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         42,        132,           110,      42,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         55,        132,           110,      55,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         68,        132,           110,      68,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         81,        132,           110,      81,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         94,        132,           110,      94,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         107,       132,           110,      107,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         120,       132,           110,      120,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         131,       132,           110,      131,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
		}
	},

	/******************* 通信模块参数配置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                         u16_id_next                             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCDC_FEEDER_MODULE_SET,  MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET,  3,             2,           6,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min u32_val_max  pv_val  
			{ 330,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,          0,           NULL },
			{ 232,            9,         16,        0,             100,      16,       MMI_VAL_TYPE_U8,        0,          8,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num) },
			{ 233,            9,         29,        540,           100,      29,       MMI_VAL_TYPE_ENUM,      0,          8,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr) },
			{ 234,            9,         42,        538,           100,      42,       MMI_VAL_TYPE_ENUM,      0,          1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.e_protocol) },
			{ 125,            9,         55,        0,             100,      55,       MMI_VAL_TYPE_F32_4W1P,  20000,      60000,       &(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt) },   //浮点数*1000来处理
			{ 332,            9,         68,        0,             100,      68,       MMI_VAL_TYPE_F32_5W1P,  5000,       110000,      &(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent) },   //浮点数*1000来处理
		}
	},
	
	/******************* 通信屏馈线模块数量配置界面结构体初始化 ******************************/
	{
		//u16_id                                u16_id_father         u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCDC_MODULE_SET,  MMI_WIN_ID_DCDC_FEEDER_MODULE_SET,  3,             2,           2,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 333,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num) },
		}
	},
	
	/******************* 通信屏馈线模块配置界面结构体初始化 ******************************/
	{
		//u16_id                            u16_id_father         u16_id_prev                             u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCDC_FEEDER_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_DCDC_MODULE_SET,  3,             2,           8,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 335,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             100,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].u8_alarm_feeder_num) },
			{ 268,                            9,         42,        260,           100,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].e_alarm_type) },
			{ 269,                            9,         55,        0,             100,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].u8_state_feeder_num) },
			{ 270,                            9,         68,        260,           100,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].e_state_type) },
			{ 272,                            9,         81,        0,             100,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].u8_curr_feeder_num) },
			{ 339,                            9,         94,        0,             100,      94,       MMI_VAL_TYPE_U16_4BIT, 10,          1000,        &(g_t_share_data.t_sys_cfg.t_dcdc_panel.u16_feeder_shunt_range) },
		}
	},
	
	/******************* 逆变模块参数配置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                         u16_id_next                             u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCAC_FEEDER_MODULE_SET,  MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET,  4,             2,           3,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min u32_val_max  pv_val  
			{ 331,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,          0,           NULL },
			{ 232,            9,         16,        0,             100,      16,       MMI_VAL_TYPE_U8,    0,          8,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num) },
			{ 234,            9,         29,        375,           100,      29,       MMI_VAL_TYPE_ENUM,  0,          0,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.e_protocol) },
		}
	},
	
	/******************* 逆变屏馈线模块数量配置界面结构体初始化 ******************************/
	{
		//u16_id                                u16_id_father         u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCAC_MODULE_SET,  MMI_WIN_ID_DCAC_FEEDER_MODULE_SET,  4,             2,           2,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 334,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           1,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num) },
		}
	},
	
	/******************* 逆变屏馈线模块配置界面结构体初始化 ******************************/
	{
		//u16_id                            u16_id_father         u16_id_prev                             u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DCAC_FEEDER_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_DCAC_MODULE_SET,  4,             2,           8,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 337,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             100,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].u8_alarm_feeder_num) },
			{ 268,                            9,         42,        260,           100,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].e_alarm_type) },
			{ 269,                            9,         55,        0,             100,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].u8_state_feeder_num) },
			{ 270,                            9,         68,        260,           100,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].e_state_type) },
			{ 272,                            9,         81,        0,             100,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].u8_curr_feeder_num) },
			{ 339,                            9,         94,        0,             100,      94,       MMI_VAL_TYPE_U16_4BIT, 10,          1000,        &(g_t_share_data.t_sys_cfg.t_dcac_panel.u16_feeder_shunt_range) },
		}
	},

	/******************* 干接点输出设定界面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father         u16_id_prev            u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RELAY_OUT_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_ALARM_SET,  MMI_WIN_ID_ALARM_SET,  5,             2,           12,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 340,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 341,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 342,            9,         29,        0,             130,      29,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output) },
			{ 343,            9,         42,        0,             130,      42,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output) },
			{ 344,            9,         55,        0,             130,      55,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output) },
			{ 345,            9,         68,        0,             130,      68,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output) },
			{ 346,            9,         81,        0,             130,      81,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output) },
			{ 347,            9,         94,        0,             130,      94,       MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output) },
			{ 348,            9,         107,       0,             130,      107,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output) },
			{ 349,            9,         120,       0,             130,      120,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output) },
			{ 350,            9,         133,       0,             130,      133,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output) },
			{ 351,            9,         146,       0,             130,      146,      MMI_VAL_TYPE_U8,    0,           6,           &(g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output) },
		}
	},

	/******************* 报警设定界面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father         u16_id_prev                u16_id_next                u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ALARM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_RELAY_OUT_SET,  MMI_WIN_ID_RELAY_OUT_SET,  5,             2,           5,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 352,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 353,            9,         16,        357,           115,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_buzzer_state) },
			{ 354,            9,         29,        359,           115,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_curr_imbalance_alm) },
			{ 355,            9,         42,        361,           115,      42,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_minor_fault_save) },
			{ 356,            9,         55,        361,           115,      55,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_general_fault_save) },
		}
	},

	/******************* 远程通讯设置界面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BACKSTAGE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           6,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0   }, 
			{ 0,         0,    0 }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 363,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 266,            9,         16,        0,             100,      16,       MMI_VAL_TYPE_U8_3BIT,  1,           255,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_local_addr) },
			{ 364,            9,         29,        367,           100,      29,       MMI_VAL_TYPE_ENUM,     0,           4,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_baud) },
			{ 365,            9,         42,        372,           100,      42,       MMI_VAL_TYPE_ENUM,     0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_parity) },
			{ 234,            9,         55,        375,           100,      55,       MMI_VAL_TYPE_ENUM,     0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_protrol) },
			{ 366,            9,         81,        379,           100,      81,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time) },
		}
	},

	/******************* RTC时间和密码设定界面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RTC_PASSWORD_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           8,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index    u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 381,              3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 382,              9,         16,        0,             100,      16,       MMI_VAL_TYPE_U16_2BIT,  0,           99,          &(m_t_win_record.t_time.year) },
			{ 383,              9,         29,        0,             100,      29,       MMI_VAL_TYPE_U8_2BIT,   1,           12,          &(m_t_win_record.t_time.month) },
			{ 384,              9,         42,        0,             100,      42,       MMI_VAL_TYPE_U8_2BIT,   1,           31,          &(m_t_win_record.t_time.day) },
			{ 385,              9,         55,        0,             100,      55,       MMI_VAL_TYPE_U8_2BIT,   0,           23,          &(m_t_win_record.t_time.hour) },
			{ 386,              9,         68,        0,             100,      68,       MMI_VAL_TYPE_U8_2BIT,   0,           59,          &(m_t_win_record.t_time.min) },
			{ 387,              9,         81,        0,             100,      81,       MMI_VAL_TYPE_U8_2BIT,   0,           59,          &(m_t_win_record.t_time.sec) },
			{ 388,              9,         107,       0,             100,      107,      MMI_VAL_TYPE_U32_5BIT,  0,           99999,       &(g_t_share_data.t_sys_cfg.t_sys_param.u32_password) },
		}
	},
	
	/******************* 历史告警记录清除界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_HIS_FAULT_CLEAR,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_EXCEPTION_CLEAR,  8,             2,           3,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }   
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 389,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 390,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 391,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 掉电末复归告警记录清除界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev                  u16_id_next               u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_EXCEPTION_CLEAR,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_HIS_FAULT_CLEAR,  MMI_WIN_ID_RECORD_CLEAR,  8,             2,           3,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }   
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 392,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 393,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 394,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 事件记录清除界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father         u16_id_prev                  u16_id_next                u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_RECORD_CLEAR,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_EXCEPTION_CLEAR,  MMI_WIN_ID_BATT_RESTORE,  8,             2,           3,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }   
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 395,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 396,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 397,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 电池容量恢复界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father         u16_id_prev               u16_id_next                    u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT_RESTORE,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_RECORD_CLEAR,  MMI_WIN_ID_PARAM_RESTORE,  8,             2,           3,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 398,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 399,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 400,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 设置参数恢复界面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father         u16_id_prev               u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_BATT_RESTORE,  MMI_WIN_ID_HIS_FAULT_CLEAR,  8,             2,           3,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 401,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 402,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 403,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 确认操作提示界面结构体初始化 ******************************/
	{
		//u16_id                       u16_id_father              u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_CONFIRM_OPERATION,  MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0  }, 
			{ 0,          0,    0  }   
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x   u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 404,            10,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 405,            10,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 提示操作已成功界面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father              u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_OPERATION_SCRESS,  MMI_WIN_ID_PARAM_RESTORE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0  }, 
			{ 0,          0,    0  }   
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x   u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 406,            10,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL  },
			{ 407,            10,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL  },
		}
	},

	/******************* 交流校准界面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father         u16_id_prev                    u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_BATT2_CURR_ADJUST,  MMI_WIN_ID_DC_VOLT_ADJUST,  9,             2,           7,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index    u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 408,              3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 409,              9,         16,        0,             105,      16,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv) },   //浮点数*1000来处理
			{ 410,              9,         29,        0,             105,      29,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw) },   //浮点数*1000来处理
			{ 411,              9,         42,        0,             105,      42,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu) },   //浮点数*1000来处理
			{ 412,              9,         55,        0,             105,      55,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv) },   //浮点数*1000来处理
			{ 413,              9,         68,        0,             105,      68,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw) },   //浮点数*1000来处理
			{ 414,              9,         81,        0,             105,      81,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu) },   //浮点数*1000来处理
		}
	},

	/******************* 直流电压校准界面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father         u16_id_prev            u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_VOLT_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_AC_ADJUST,  MMI_WIN_ID_LOAD_CURR_ADJUST,  9,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 415,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,        0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 1,              9,         29,        0,             120,      29,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_batt_volt) },   //浮点数*1000来处理
			{ 5,              9,         42,        0,             120,      42,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_pb_volt) },   //浮点数*1000来处理
			{ 7,              9,         55,        0,             120,      55,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_cb_volt) },   //浮点数*1000来处理
			{ 516,            9,         68,        0,             120,      68,       MMI_VAL_TYPE_F32_5W1P,  100,         999900,      &(g_t_share_data.t_dc_adjust_data.f32_bus_neg_to_gnd_volt) },   //浮点数*1000来处理
		}
	},

	/******************* 负载电流校准界面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father         u16_id_prev                 u16_id_next                    u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_LOAD_CURR_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_DC_VOLT_ADJUST,  MMI_WIN_ID_BATT1_CURR_ADJUST,  9,             2,           4,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 417,            3,         3,        0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,       0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 418,            9,         29,       0,              110,      29,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_load_curr_1) },   //浮点数*1000来处理
			{ 419,            9,         42,       0,              110,      42,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_load_curr_2) },   //浮点数*1000来处理
		}
	},

	/******************* 一组电池电流校准界面结构体初始化 ******************************/
	{
		//u16_id                       u16_id_father         u16_id_prev                   u16_id_next                    u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT1_CURR_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_LOAD_CURR_ADJUST,  MMI_WIN_ID_BATT2_CURR_ADJUST,  9,             2,           4,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 420,            3,         3,        0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,       0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 422,            9,         29,       0,              110,      29,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1) },   //浮点数*1000来处理
			{ 423,            9,         42,       0,              110,      42,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2) },   //浮点数*1000来处理
		}
	},

	/******************* 二组电池电流校准界面结构体初始化 ******************************/
	{
		//u16_id                       u16_id_father         u16_id_prev                    u16_id_next            u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BATT2_CURR_ADJUST,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_BATT1_CURR_ADJUST,  MMI_WIN_ID_AC_ADJUST,  9,             2,           4,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 421,            3,         3,        0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 416,            60,        16,       0,              0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 422,            9,         29,       0,              110,      29,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt2_curr_1) },   //浮点数*1000来处理
			{ 423,            9,         42,       0,              110,      42,       MMI_VAL_TYPE_F32_6W1P,  100,         9999900,     &(g_t_share_data.t_dc_adjust_data.f32_batt2_curr_2) },   //浮点数*1000来处理
		}
	},

	/******************* 校准正在进行提示页面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_DOING,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           1,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 424,            30,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 校准成功提示页面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_SCUESS,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 425,            10,         42,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 407,            10,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 校准失败提示页面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_FAIL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           2,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 426,            10,         42,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 407,            10,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 提示输入下一个电流值页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ADJUST_NEXT_CURR,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           3,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x   u8_name_y   u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 427,            10,         42,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 428,            10,         55,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 429,            10,         68,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 显示设置页面结构体初始化 ******************************/
	{
		//u16_id             u16_id_father         u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DISPLAY,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  10,             0,           3,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 430,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 431,            9,         16,        433,           110,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_lang) },
			{ 432,            9,         29,        435,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection) },
		}
	},

	/******************* 关于页面结构体初始化 ******************************/
	{
		//u16_id           u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ABOUT,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  11,             0,          12,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME,        5,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        5,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 437,  5,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 438,  5,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 439,  5,         133,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 529,  5,         146,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 末配置此项提示界面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_NO_FUNCTION,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  0,             0,           1,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 446,            6,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 维护指南页面结构体初始化 ******************************/
	{
		//u16_id               u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  10,             0,           8,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 448,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 449,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 450,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 451,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 452,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 453,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 454,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 455,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 开通页面结构体初始化 ******************************/
	{
		//u16_id           u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_START,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           7,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 456,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 457,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 458,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 459,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 460,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 461,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 462,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 通讯异常报警页面结构体初始化 ******************************/
	{
		//u16_id                u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_COMM_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  2,             0,           10,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 463,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 464,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 465,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 466,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 467,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 468,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 469,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 470,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 471,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 472,        3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 过欠压报警页面结构体初始化 ******************************/
	{
		//u16_id                u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_VOLT_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  3,             0,           9,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 473,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 474,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 475,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 476,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 477,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 478,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 479,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 480,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 481,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 绝缘报警页面结构体初始化 ******************************/
	{
		//u16_id                u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INSU_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  4,             0,           11,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 482,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 483,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 484,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 485,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 486,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 487,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 488,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 489,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 490,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 491,        3,         120,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 492,        3,         133,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 后台通讯异常页面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BACKSTAGE_FAULT,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  5,             0,           7,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 493,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 494,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 495,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 496,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 497,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 498,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 499,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 联系厂家页面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_CONTACT_VENDER,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  6,             0,           9,

		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 500,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 501,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 502,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 503,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 504,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 505,        3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 506,        3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 507,        3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 508,        3,         107,       0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 关于设备返修页面结构体初始化 ******************************/
	{
		//u16_id            u16_id_father          u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_REPAIR,  MMI_WIN_ID_GUIDELINE,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           5,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type  u32_val_min  u32_val_max  pv_val  
			{ 509,        3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 510,        3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 511,        3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 512,        3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 513,        3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},
	
	/******************* 手动限流设定界面结构体初始化 ******************************/
	{
		//u16_id                           u16_id_father         u16_id_prev                   u16_id_next                   u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MANUAL_LIMIT_CURR_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_ZERO_CURR_SET,  MMI_WIN_ID_COMM_OFFLINE_SET,  11,            2,           3,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 518,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 519,            9,         16,        130,           115,      16,       MMI_VAL_TYPE_ENUM,      0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_limit_curr) },
			{ 520,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_5W1P,  5000,        110000,      &(g_t_share_data.t_sys_cfg.t_sys_param.f32_manual_limit_curr) },
		}
	},
	
	/******************* 通讯报警设定界面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father         u16_id_prev                        u16_id_next                        u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_COMM_OFFLINE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_MANUAL_LIMIT_CURR_SET,  MMI_WIN_ID_INSU_MEAS_SET,             11,            2,           9,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index    u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 521,              3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 522,              9,         16,        0,             115,      16,       MMI_VAL_TYPE_U8_2BIT,   5,           60,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_fc10_offline_time) },
			{ 523,              9,         29,        0,             115,      29,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_bms_offline_cnt) },
			{ 524,              9,         42,        0,             115,      42,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_offline_cnt) },
			{ 525,              9,         55,        0,             115,      55,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dcdc_offline_cnt) },
			{ 526,              9,         68,        0,             115,      68,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_offline_cnt) },
			{ 608,              9,         81,        0,             115,      81,       MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt) },
			{ 609,              9,         94,        0,             115,      94,       MMI_VAL_TYPE_U8_2BIT,   1,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_offline_cnt) },
			{ 622,              9,         107,       0,             115,      107,      MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_ats_offline_cnt) },
//			{ 528,              9,         120,       0,             115,      120,      MMI_VAL_TYPE_U8_2BIT,   4,           99,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_acm_offline_cnt) },
		}
	},

	/******************* 直流母线绝缘测量配置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_INSU_MEAS_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_COMM_OFFLINE_SET,  MMI_WIN_ID_ZERO_CURR_SET,  11,             2,           9,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 549,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 550,            9,         16,        558,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           3,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_can_baud) },
			{ 551,            9,         29,        562,           110,      29,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_insu_meas_way) },
			{ 552,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U8_3BIT,  1,           120,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_res_switch_delay) },
			{ 553,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range) },
			{ 554,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U8_3BIT,  1,           180,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_bus_err_confirm) },
			{ 555,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_3BIT,  1,           180,         &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_period) },
			{ 556,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_2BIT,  0,           23,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_hour) },
			{ 557,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U8_2BIT,  0,           59,          &(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_min) },
		}
	},

	/******************* 直流电流零点修正配置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev                 u16_id_next                  u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ZERO_CURR_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_INSU_MEAS_SET,  MMI_WIN_ID_MANUAL_LIMIT_CURR_SET,  11,             2,           7,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ 566,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
//			{ 567,            9,         16,        573,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_load_sign) },
//			{ 568,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,   0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero) },
//			{ 569,            9,         42,        573,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat1_sign) },
//			{ 570,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_1P,   0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero) },
//			{ 571,            9,         68,        573,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat2_sign) },
//			{ 572,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_F32_1P,   0,           2,           &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero) },
			{ 567,            9,         16,        573,           110,      16,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_load_sign) },
			{ 568,            9,         16,        0,             118,      16,       MMI_VAL_TYPE_F32_3W1P, 0,           2000,        &(g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero) },
			{ 569,            9,         29,        573,           110,      29,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat1_sign) },
			{ 570,            9,         29,        0,             118,      29,       MMI_VAL_TYPE_F32_3W1P, 0,           2000,        &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero) },
			{ 571,            9,         42,        573,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           &(g_t_share_data.t_sys_cfg.t_sys_param.e_bat2_sign) },
			{ 572,            9,         42,        0,             118,      42,       MMI_VAL_TYPE_F32_3W1P, 0,           2000,        &(g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero) },
		}
	},

	/******************* 二段母线主界面结构体初始化 ******************************/
	{
        //u16_id                 u16_id_father     u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_MAIN_WINDOW2,  MMI_WIN_ID_NULL,  MMI_WIN_ID_MAIN_WINDOW,  MMI_WIN_ID_NULL,  0,             2,           10,
		
		//初始化icon结构数组
		{
			// u16_index   u8_x   u8_y
			{ ICON_CLOCK,  3,     3 }, 
			{ ICON_UP,   MMI_ICON_UP_X+4,   MMI_ICON_UP_Y   }
		},
			
		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x   u8_val_y  u8_val_type          u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME,  16,        3,         0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 0,                        3,         19,        0,             115,       19,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature) },
			{ 575,                      3,         32,        0,             115,       32,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt2_volt) },
			{ 576,                      3,         45,        0,             115,       45,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]) },
			{ 577,                      3,         58,        0,             115,       58,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity) },
			{ 578,                      3,         71,        0,             115,       71,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr) },
			{ 8,                        3,         132,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL },
			{ 14,                       3,         145,       0,             0,         0,        MMI_VAL_TYPE_NONE,   0,           0,            NULL }, 
			{ 579,                      3,         84,        0,             115,       84,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_volt) },
			{ 581,                      3,         97,        0,             115,       97,       MMI_VAL_TYPE_F32_1P, 0,           0,            &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_volt) },
		}
	},

	/******************* 直流DC10参数配置界面结构体初始化 ******************************/
	{
		//u16_id                  u16_id_father            u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC10_PARAM_SET,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_DC_PARAM_SET,  MMI_WIN_ID_RECT_PARAM_SET,  1,             2,           9,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x             u8_y
			{ ICON_UP,    MMI_ICON_UP_X,   MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X, MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type             u32_val_min  u32_val_max  pv_val  
			{ 582,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 583,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8     ,   0,           1,           &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u8_dc10_module_num) },
			{ 584,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt) },
			{ 585,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt) },
			{ 586,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  1,           500,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt) },
			{ 587,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range) },
			{ 588,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range) },
			{ 589,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range) },
			{ 195,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U16_3BIT,  (210/2),     230,         &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt) },			
		}
	},

	/******************* 逆变模块参数配置界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father         u16_id_prev           u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_AC_MODULE_SET,  MMI_WIN_ID_SET_MENU,    MMI_WIN_ID_ACS_FEEDER_MODULE_SET,      MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET,         1,             0,           9,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min u32_val_max  pv_val  
			{ 592,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,      0,           0,           NULL },
			{ 593,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8     ,   0,           1,           &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num) },
			{ 594,            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U16_3BIT,  220,         530,         &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_high_volt) },
			{ 595,            9,         42,        0,             110,      42,       MMI_VAL_TYPE_U16_3BIT,  186,         380,         &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_low_volt) },
			{ 596,            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U16_3BIT,  0,           380,         &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_lack_phase) },
			{ 597,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range) },
			{ 598,            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range) },
			{ 599,            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U16_4BIT,  25,          2000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range) },
			{ 339,            9,         107,       0,             110,      107,      MMI_VAL_TYPE_U16_4BIT,  10,          1000,        &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u16_feeder_shunt_range) },
		}
	},

	/******************* 交流系统馈线模块数量配置界面结构体初始化 ******************************/
	{
		//u16_id                                u16_id_father         u16_id_prev                  u16_id_next                         u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_AC_MODULE_SET,  MMI_WIN_ID_ACS_FEEDER_MODULE_SET,      1,             2,           2,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 600,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           3,           &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num) },
		}
	},

	/******************* 馈电柜馈线模块配置界面结构体初始化 ******************************/
	{
		//u16_id                             u16_id_father                u16_id_prev                              u16_id_next     u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_FEEDER_MODULE_SET,  MMI_WIN_ID_SET_MENU,  MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET,  MMI_WIN_ID_AC_MODULE_SET,  1,             2,           7,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 601,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 268,                            9,         42,        260,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 269,                            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 270,                            9,         68,        260,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 272,                            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
		}
	},

	/******************* 交流系统交流信息显示菜单页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_AC_RUN_INFO_MENU,  MMI_WIN_ID_MAIN_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,   1,             0,           3,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 604,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 605,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 606,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 交流柜三相交流信息显示页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_AC_RUN_INFO,  MMI_WIN_ID_ACS_AC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  1,             0,           11,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 83,             3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 84,             3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         29,        0,             110,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac1_uv_volt) },
			{ 87,             9,         42,        0,             110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac1_vw_volt) },
			{ 88,             9,         55,        0,             110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac1_wu_volt) },
			{ 621,            9,         68,        0,             110,      68,       MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_s1_curr) },
			{ 85,             3,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 86,             9,         107,       0,             110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac2_uv_volt) },
			{ 87,             9,         120,       0,             110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac2_vw_volt) },
			{ 88,             9,         133,       0,             110,      133,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_ac2_wu_volt) },
			{ 621,            9,         146,       0,             110,      146,      MMI_VAL_TYPE_F32_1P,  0,           0,           &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.f32_s3_curr) },
		}
	},

	/******************* 交流馈线支路信息显示页面结构体初始化 ******************************/
	{
		//u16_id                      u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ACS_FEEDER_INFO,  MMI_WIN_ID_ACS_AC_RUN_INFO_MENU,  MMI_WIN_ID_ACS_FEEDER_INFO,  MMI_WIN_ID_ACS_FEEDER_INFO,  2,             0,           9,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x  u8_y
			{ 0,          0,    0 }, 
			{ 0,          0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ 607,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         42,        151,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         55,        154,           110,      55,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                      9,         94,        151,           110,      94,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 149,                      9,         107,       151,           110,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                      9,         120,       154,           110,      120,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},

	/******************* 电操开关控制界面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father            u16_id_prev               u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_SWT_ON_OFF,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_SWT_ON_OFF,  MMI_WIN_ID_SWT_ON_OFF,         6,             2,           11,
		                                                          
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 612,                            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         16,        141,           110,      16,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         29,        141,           110,      29,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         42,        141,           110,      42,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         55,        141,           110,      55,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         68,        141,           110,      68,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         81,        141,           110,      81,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         94,        141,           110,      94,       MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         107,       141,           110,      107,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         120,       141,           110,      120,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 329,  9,         131,       141,           110,      131,      MMI_VAL_TYPE_ENUM,  0,           1,           NULL },
		}
	},

	/******************* 开关状态查询页面结构体初始化 ******************************/
	{
		//u16_id                 u16_id_father                 u16_id_prev              u16_id_next              u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_ECSWT_INFO,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_ECSWT_INFO,  MMI_WIN_ID_ECSWT_INFO,     7,             2,           6,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index            u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         pv_val  u32_val_min  u32_val_max
			{ 614,                      3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         16,        141,           100,      29,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         42,        141,           100,      55,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         68,        141,           100,      81,       MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         94,        141,           100,      107,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 },
			{ MMI_STR_ID_SPECIAL_NAME,  9,         120,       141,           100,      131,      MMI_VAL_TYPE_ENUM,  NULL,   0,           0 }
		}
	},

	/******************* 二段馈线柜参数设置菜单界面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father            u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_SET_MENU2,  MMI_WIN_ID_DC_SET_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  7,             0,           5,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 77,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 73,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 74,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 75,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 76,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 二段馈电柜馈线模块数量配置界面结构体初始化 ******************************/
	{
		//u16_id                                 u16_id_father                u16_id_prev                          u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2,  MMI_WIN_ID_FEEDER_SET_MENU2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  1,             2,           2,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 262,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 263,                            9,         16,        0,             110,      16,       MMI_VAL_TYPE_U8,    0,           4,           NULL },
		}
	},
	
	/******************* 二段馈电柜馈线模块配置界面结构体初始化 ******************************/
	{
		//u16_id                             u16_id_father                u16_id_prev                              u16_id_next                          u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  MMI_WIN_ID_FEEDER_SET_MENU2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2,  MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2,  1,             2,           8,
		
		//初始化icon结构数组
		{
			// u16_index  u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }  
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type            u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 264,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME | 266,  9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,     0,           0,           NULL },
			{ 267,                            9,         29,        0,             110,      29,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 268,                            9,         42,        260,           110,      42,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 269,                            9,         55,        0,             110,      55,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 270,                            9,         68,        260,           110,      68,       MMI_VAL_TYPE_ENUM,     0,           1,           NULL },
			{ 271,                            9,         81,        0,             110,      81,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
			{ 272,                            9,         94,        0,             110,      94,       MMI_VAL_TYPE_U8_2BIT,  0,           64,          NULL },
		}
	},

	/******************* 二段合控母线绝缘状态查询页面结构体初始化 ******************************/
	{
		//u16_id                     u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_PB_CB_INSU_INFO2,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  8,             0,           8,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{  78,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 116,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 117,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         55,        0,             115,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{  79,            3,         81,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         107,       0,             115,      107,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
		}
	},

	/******************* 二段绝缘状态查询页面结构体初始化 ******************************/
	{
		//u16_id                   u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_BUS_INSU_INFO2,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  8,             0,           7,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 }
		},

		//初始化item结构数组
		{ 
			//u16_name_index  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{  78,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 118,            9,         16,        0,             115,      16,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 119,            9,         29,        0,             115,      29,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 514,            9,         42,        0,             115,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{  79,            3,         68,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 121,            9,         81,        0,             115,      81,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 122,            9,         94,        0,             115,      94,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL }
		}
	},

	/******************* 二段馈线柜信息查询菜单界面结构体初始化 ******************************/
	{
		//u16_id                          u16_id_father                 u16_id_prev       u16_id_next       u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_FEEDER_RUN_INFO_MENU2,  MMI_WIN_ID_DC_RUN_INFO_MENU,  MMI_WIN_ID_NULL,  MMI_WIN_ID_NULL,  9,             0,           5,
		
		//初始化icon结构数组
		{
			//u16_index  u8_x  u8_y
			{ 0,         0,    0 }, 
			{ 0,         0,    0 } 
		},

		//初始化item结构数组
		{ 
			//u16_name_index u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type         u32_val_min  u32_val_max  pv_val  
			{ 47,            3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 41,            3,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 42,            3,         29,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 43,            3,         42,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
			{ 44,            3,         55,        0,             0,        0,        MMI_VAL_TYPE_NONE,  0,           0,           NULL },
		}
	},

	/******************* 二段馈电柜馈出支路信息查询页面结构体初始化 ******************************/
	{
		//u16_id                    u16_id_father                     u16_id_prev                 u16_id_next                 u8_sel_father  u8_icon_cnt  u8_item_cnt
		MMI_WIN_ID_DC_FEEDER_INFO2,  MMI_WIN_ID_FEEDER_RUN_INFO_MENU2,  MMI_WIN_ID_DC_FEEDER_INFO2,  MMI_WIN_ID_DC_FEEDER_INFO2,  1,             2,           11,
		
		//初始化icon结构数组
		{
			//u16_index   u8_x              u8_y
			{ ICON_UP,    MMI_ICON_UP_X,    MMI_ICON_UP_Y   }, 
			{ ICON_DOWN,  MMI_ICON_DOWN_X,  MMI_ICON_DOWN_Y }
		},

		//初始化item结构数组
		{ 
			//u16_name_index                  u8_name_x  u8_name_y  u16_val_index  u8_val_x  u8_val_y  u8_val_type           u32_val_min  u32_val_max  pv_val  
			{ MMI_STR_ID_SPECIAL_NAME | 143,  3,         3,         0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         16,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         29,        151,           110,      29,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         42,        154,           110,      42,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         55,        154,           110,      55,       MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         68,        154,           110,      68,       MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ MMI_STR_ID_SPECIAL_NAME,        9,         94,        0,             0,        0,        MMI_VAL_TYPE_NONE,    0,           0,           NULL },
			{ 147,                            9,         107,       151,           110,      107,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL },
			{ 148,                            9,         120,       154,           110,      120,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 149,                            9,         133,       154,           110,      133,      MMI_VAL_TYPE_F32_1P,  0,           0,           NULL },
			{ 150,                            9,         146,       154,           110,      146,      MMI_VAL_TYPE_ENUM,    0,           0,           NULL }
		}
	},
};

static STR_T s_disp_get_string_pointer(U16_T index)
{
	if (m_e_lang == CHINESE)
		return (g_s_string[index][0]);
	else
		return (g_s_string[index][1]);
}

static STR_T s_disp_get_swt_string_pointer(U16_T index)
{
	if (m_e_lang == CHINESE)
		return (g_s_ctrl_swt_name[index][0]);
	else
		return (g_s_ctrl_swt_name[index][1]);
}


/*************************************************************
函数名称: v_disp_volt_init_by_level		           				
函数功能: 根据电压等级设置相关的电压参数，并设置设置界面的最大值和最小值						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_volt_init_by_level(void)
{
	U32_T div;
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
		div = 1;
	else
		div = 2;
	
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt               = m_t_dc_volt_def.f32_batt_equ_volt / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt               = m_t_dc_volt_def.f32_batt_flo_volt / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt         = m_t_dc_volt_def.u16_batt_total_end_volt / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit        = m_t_dc_volt_def.f32_batt_high_volt_limit / div;
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit         = m_t_dc_volt_def.f32_batt_low_volt_limit / div;
	
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[0]      = m_t_dc_volt_def.f32_rect_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[1]      = m_t_dc_volt_def.f32_rect_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_max_out_volt     = m_t_dc_volt_def.f32_rect_max_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_min_out_volt     = m_t_dc_volt_def.f32_rect_min_out_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt = m_t_dc_volt_def.f32_rect_offline_out_volt / div;
	
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt       = m_t_dc_volt_def.u16_pb_high_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt        = m_t_dc_volt_def.u16_pb_low_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt       = m_t_dc_volt_def.u16_cb_high_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt        = m_t_dc_volt_def.u16_cb_low_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt      = m_t_dc_volt_def.u16_bus_high_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt       = m_t_dc_volt_def.u16_bus_low_volt / div;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt     = m_t_dc_volt_def.u16_cb_output_volt / div;

	//二段直流控母输出电压
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt   = m_t_dc_volt_def.u16_cb_output_volt / div; 
	
	os_mut_release(g_mut_share_data);	
}


#define B21_CELL_MAX 24
#define B3_CELL_MAX 56
#define B4_CELL_MAX 112

/*************************************************************
函数名称: v_disp_batt1_data_init		           				
函数功能: 根据电池巡检类型设置界面的最大值和最小值，必须在当前页面己经设置为一组电池巡检配置页面的情况下，才能调用					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_batt1_data_init(void)
{
	U32_T i, j;
	U8_T u8_cell_max;
	U8_T u8_bms_max;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B21)
	{
		u8_cell_max = B21_CELL_MAX;
		u8_bms_max = 5;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B3)
	{
		u8_cell_max = B3_CELL_MAX;
		u8_bms_max = 2;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B4)
	{
		u8_cell_max = B4_CELL_MAX;
		u8_bms_max = 1;
	}
	
	for (i=0; i<BATT_GROUP_MAX; i++)
	{
		for (j=0; j<BATT_METER_MAX; j++)
		{
			if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_num[j] > u8_cell_max)
				g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_num[j] = u8_cell_max;
		}
		
		if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_bms_num > u8_bms_max)
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_bms_num = u8_bms_max;
		
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num = 0;	
		for (j=0; j<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_bms_num; j++)
		{
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num +=
						g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_num[j];
		}
					
		if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num > BATT_CELL_MAX)
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[i].u8_cell_total_num = BATT_CELL_MAX;
	}
	
	win->t_item[2].u32_val_max = u8_bms_max;
	win->t_item[3].u32_val_max = u8_cell_max;
	win->t_item[4].u32_val_max = u8_cell_max;
	win->t_item[5].u32_val_max = u8_cell_max;
	win->t_item[6].u32_val_max = u8_cell_max;
	win->t_item[7].u32_val_max = u8_cell_max;
	
	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
		win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;

	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_batt2_data_init		           				
函数功能: 根据电池巡检类型设置界面的最大值和最小值，必须在当前页面己经设置为二组电池巡检配置页面的情况下，才能调用					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_batt2_data_init(void)
{
	U32_T i;
	U8_T u8_cell_max;
	U8_T u8_bms_max;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B21)
	{
		u8_cell_max = B21_CELL_MAX;
		u8_bms_max = 5;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B3)
	{
		u8_cell_max = B3_CELL_MAX;
		u8_bms_max = 2;
	}
	else if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B4)
	{
		u8_cell_max = B4_CELL_MAX;
		u8_bms_max = 1;
	}
	
	for (i=0; i<BATT_METER_MAX; i++)
	{
		if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i] > u8_cell_max)
			g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i] = u8_cell_max;
	}
		
	if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > u8_bms_max)
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num = u8_bms_max;

	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num = 0;	
	for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num; i++)
	{
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num +=
					g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i];
	}
				
	if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num > BATT_CELL_MAX)
		g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num = BATT_CELL_MAX;
		
	
	win->t_item[1].u32_val_max = u8_bms_max;
	win->t_item[2].u32_val_max = u8_cell_max;
	win->t_item[3].u32_val_max = u8_cell_max;
	win->t_item[4].u32_val_max = u8_cell_max;
	win->t_item[5].u32_val_max = u8_cell_max;
	win->t_item[6].u32_val_max = u8_cell_max;

	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: u8_disp_get_product_type		           				
函数功能: 得到产品的型号编码					
输入参数: 无       		   				
输出参数: 无
返回值  ：型号码, 0--SC12, 1--SC22, 2--SC32														   				
**************************************************************/
static U8_T u8_disp_get_product_type(void)
{
	U8_T u8_type;
	
	s32_flash_dataflash_read(DATAFLASH_MODEL_ADDR, &u8_type, 1);
	
	if ((u8_type == PRODUCT_TYPE_SC12)
		|| (u8_type == PRODUCT_TYPE_SC22)
		|| (u8_type == PRODUCT_TYPE_SC32))
	{
		return u8_type;
	}
	else
	{
		return PRODUCT_TYPE_SC12;
	}
}

/*************************************************************
函数名称: v_disp_share_data_restore		           				
函数功能: 恢复默认数据，将设置参数恢复到默认值						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_disp_cfg_data_restore(void)
{
	U32_T i, j;

	g_u8_product_type = u8_disp_get_product_type();
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	m_e_lang = g_t_share_data.t_sys_cfg.t_sys_param.e_lang;
	m_e_lcd_direction = g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection;
	
	memset(&(g_t_share_data.t_sys_cfg), 0, sizeof(g_t_share_data.t_sys_cfg));
	
	if (g_u8_product_type == PRODUCT_TYPE_SC12)
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = ONE_PATH;                     // 交流输入路数，无/1路，默认1路
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num = 0;                // 馈线屏面数，无馈线屏固定为0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num = 0;               // 馈电屏面数，无馈线屏固定为0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num = 1;                     // 电池组数，范围0~1，默认1		
		
		g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B码对时：固定为无
		g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;                   // 通信模块个数，固定为0
		g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;                   // UPS模块个数，固定为0
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 2;                     // 整流模块数量，范围1~3，默认2
		
		g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output = 4;                    //交流进线故障，范围0,4~6，默认4
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output = 5;                //直流母线故障，范围0,4~6，默认5
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output = 6;                  //电池异常故障，范围0,4~1，默认6
		g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output = 0;                  //绝缘下降故障，范围0,4~1，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output = 0;                  //充电模块故障，范围0,4~1，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output = 0;                  //电池熔断器故障，范围0,4~1，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output = 0;                  //通信及逆变电源故障，范围0,4~1，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output = 0;                //馈线支路故障，范围0,4~1，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output = 0;                    //均充指示，范围0,4~1，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output = 0;                    //核容指示，范围0,4~1，默认0
		
	}
	else if (g_u8_product_type == PRODUCT_TYPE_SC22)
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = TWO_PATH;                     // 交流输入路数，1路/2路/无，默认2路
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num = 1;                // 馈电屏面数，范围0~1，默认1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num = 1;               // 馈电屏面数，范围0~1，默认1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num = 1;                     // 电池组数，范围0~1，默认1
		
		g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B码对时：无/有，默认无
		g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;                   // 通信模块个数，范围0~4，默认0
		g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;                   // UPS模块个数，范围0~4，默认0
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 4;                     // 模块数量，范围1~8，默认4
		
		g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output = 1;                    //交流进线故障，范围0~6，默认1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output = 2;                //直流母线故障，范围0~6，默认2
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output = 3;                  //电池异常故障，范围0~6，默认3
		g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output = 4;                  //绝缘下降故障，范围0~6，默认4
		g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output = 0;                  //充电模块故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output = 0;                  //电池熔断器故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output = 0;                  //通信及逆变电源故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output = 0;                //馈线支路故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output = 5;                    //均充指示，范围0~6，默认5
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output = 6;                    //核容指示，范围0~6，默认6
	}
	else
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = TWO_PATH;                     // 交流输入路数，1路/2路/无，默认2路
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num = 1;                // 馈电屏面数，范围0~4，默认1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num = 1;               // 馈电屏面数，范围0~4，默认1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num = 2;                     // 电池组数，范围0~2，默认1
		
		g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B码对时：无/有，默认无
		g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;                   // 通信模块个数，范围0~8，默认0
		g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;                   // UPS模块个数，范围0~8，默认0
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 4;                     // 模块数量，范围1~24，默认4
		
		g_t_share_data.t_sys_cfg.t_sys_param.u8_ac_fault_output = 1;                    //交流进线故障，范围0~6，默认1
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_bus_fault_output = 2;                //直流母线故障，范围0~6，默认2
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_fault_output = 3;                  //电池异常故障，范围0~6，默认3
		g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_fault_output = 4;                  //绝缘下降故障，范围0~6，默认4
		g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_fault_output = 0;                  //充电模块故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_fuse_fault_output = 0;                  //电池熔断器故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_fault_output = 0;                  //通信及逆变电源故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_feeder_fault_output = 0;                //馈线支路故障，范围0~6，默认0
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output = 5;                    //均充指示，范围0~6，默认5
		g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output = 6;                    //核容指示，范围0~6，默认6
	}
	
	g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num = 9;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num = 0;                      //默认没有D21模块
	g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] = AUTO_MODE;                        // 电池管理方式 手动、自动
	g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[1] = AUTO_MODE;                        // 电池管理方式 手动、自动
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 = 65;                        // 电池标称容量，范围30~3000Ah，默认100Ah
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt = 245;                        // 电池均充电压，范围电池组欠压点~过压点，默认245V
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt = 235;                        // 电池浮充电压，范围电池组欠压点~过压点，默认235V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr = 0.1;                      // 电池充电限流点，范围0.05~充电过流点，默认0.1C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr = 0.08;		            // 电池转均充电流，范围0.04~充电限流点，默认0.08C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_to_equ_dur_time = 60;                  // 电流满足转均充条件的持续时间，范围30~999秒，默认60秒
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_equ_cycle = 4320;	    	            // 电池定时均充周期，范围1~9999小时，默认4320小时
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_ac_fail_time = 20;                     // 交流停电时长，范围1~999分钟，默认20分钟
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_cap = 0.8;                         // 电池容量低于，范围0.8~0.95C10，默认0.8C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_flo_curr = 0.02;			        // 电池转浮充电流，范围0.01~0.03C10，默认0.02C10
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_curr_go_time = 180;                    // 电池转浮充计时时间，范围1~360分钟，默认180分钟
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_equ_time = 720;	                // 电池均充保护时间，范围1~999分钟，默认720分钟
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt = 200;                  // 电池放电终止电压，范围190~220V，默认200V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_cell_end_volt = 1.8;                   // 电池放电终止单体电压，范围1.8~12V，默认1.8V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_dis_time = 600;                    // 电池放电终止时间，范围1~999分钟，默认600分钟
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt = 0;               // 温度补偿电压，范围0~500mV，默认0mV
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit = 264;                 // 电池组过压点，范围220~320V，默认264V
	//g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit = 187;                  // 电池组欠压点，范围186~220，默认187V
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit = 0.2;                 // 电池充电过流点，范围0.1~1.0C10，默认0.2
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_10c_dis_rate = 0.3;               // 电池1.0C放电率   [0~10.0], 默认0.3
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_09c_dis_rate = 0.4;               // 电池0.9C放电率   [0~10.0], 默认0.4
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_08c_dis_rate = 0.5;               // 电池0.8C放电率   [0~10.0], 默认0.5
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_07c_dis_rate = 0.7;               // 电池0.7C放电率   [0~10.0], 默认0.7
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_06c_dis_rate = 0.8;               // 电池0.6C放电率   [0~10.0], 默认0.8
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_05c_dis_rate = 0.9;               // 电池0.5C放电率   [0~10.0], 默认0.9
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_04c_dis_rate = 2.2;               // 电池0.4C放电率   [0~10.0], 默认2.2
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_03c_dis_rate = 3.3;               // 电池0.3C放电率   [0~10.0], 默认3.3
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_02c_dis_rate = 4;                 // 电池0.2C放电率   [0~10.0], 默认4
	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_01c_dis_rate = 10;                // 电池0.1C放电率   [0~10.0], 默认10
	
	g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level = VOLT_LEVEL_220;             // 系统类型，220/110V
	m_e_volt_level = g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level;
	g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add = 14;               // 馈电屏面数，范围14~111，默认14
	g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add = 30;               // 馈电屏面数，范围14~111，默认30
	g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num = TWO;			            // 直流母线段数，范围ONE/TWO，默认ONE
	g_t_share_data.t_sys_cfg.t_sys_param.e_lang = m_e_lang;                         // 语言，中文/英文，默认中文
	g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection = m_e_lcd_direction;       // 显示屏方向，水平/竖显，默认水平
	g_t_share_data.t_sys_cfg.t_sys_param.e_limit_curr = AUTO_LIMIT_CURR;            // 限流方式，自动/手动，默认手动
	g_t_share_data.t_sys_cfg.t_sys_param.f32_manual_limit_curr = 100;               // 手动限流点，5%~110%，默认100%
	g_t_share_data.t_sys_cfg.t_sys_param.u8_fc10_offline_time = 10;                 // 馈线模块通讯中断时间，范围5~60S，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_bms_offline_cnt = 10;                   // 电池巡检通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_offline_cnt = 10;                  // 整流模块通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_dcdc_offline_cnt = 10;                  // 通信模块通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_offline_cnt = 10;                  // 逆变模块通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_ats_offline_cnt = 10;                   // ATS模块通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_acm_offline_cnt = 10;                   // 多功能电表通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt = 10;                  // DC10模块通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_offline_cnt = 3;                   // RC10模块通讯中断命令个数，范围4~99个，默认10
	g_t_share_data.t_sys_cfg.t_sys_param.e_can_baud = CAN_BAUD_125K;		        // CAN通信速率，125K｜50K｜20K｜10K，默认125K
	g_t_share_data.t_sys_cfg.t_sys_param.e_insu_meas_way = INSU_PROJECT;            // 母线电阻测量方式，工程模式和调试模式，默认工程模式
    g_t_share_data.t_sys_cfg.t_sys_param.u8_res_switch_delay = 2;                   // 平衡及不平衡电桥投切延时，1~120可设，以秒为单位，默认2秒
    g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range = 10;                // 支路传感器量程，1~500可设，以mA为单位，默认10mA
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_bus_err_confirm = 3;               // 母线绝缘压差报警确认时间，1~180可设，以秒为单位，默认3秒
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_period = 24;                  // 绝缘定期测量周期时间，0~180可设，以小时为单位，默认24小时
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_hour = 8;                     // 强制启动绝缘测量的时间-时，0~23小时，默认8点
    g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_min = 0;                      // 强制启动绝缘测量的时间-分，0~59分钟，默认0分
	g_t_share_data.t_sys_cfg.t_sys_param.e_buzzer_state = BEEP;                     // 蜂鸣器告警设置：鸣叫/静音，默认鸣叫
	g_t_share_data.t_sys_cfg.t_sys_param.e_minor_fault_save = SAVE;                 // 次要告警保存设置：0-不保存，1-保存
	g_t_share_data.t_sys_cfg.t_sys_param.e_general_fault_save = NO_SAVE;            // 一般告警保存设置：0-不保存，1-保存
	g_t_share_data.t_sys_cfg.t_sys_param.e_curr_imbalance_alm = CLOSE;              // 电流不平衡报警：关闭/开启，默认关闭
	g_t_share_data.t_sys_cfg.t_sys_param.u32_password = MMI_USER_PASSWORD; 			// 用户操作密码设置，5位密码，默认为11111，超级密码为02051
	g_t_share_data.t_sys_cfg.t_sys_param.u8_local_addr = 1;                         // 本机地址，范围1~255，默认1
	g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_protrol = BS_MODBUS;           // 后台通信协议，MODBUS/CDT/IEC101/IEC102，默认MODBUS
	g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_baud = COM_BAUD_9600;          // 波特率，默认9600
	g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_parity = ODD_PARITY;           // 校验，默认奇检验
	//g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time = NO;                       // B码对时：无/有，默认无

	g_t_share_data.t_sys_cfg.t_sys_param.e_load_sign = POSITIVE;		            // 负载电流零点修正值符号，正｜负, 默认正
	g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero = 0.0;                       // 负载电流零点修正绝对值，范围[0~2.0], 默认0.0
    g_t_share_data.t_sys_cfg.t_sys_param.e_bat1_sign = POSITIVE;		            // 电池1电流零点修正值符号，正｜负, 默认正
	g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero = 0.0;                       // 电池1电流零点修正绝对值，范围[0~2.0], 默认0.0
    g_t_share_data.t_sys_cfg.t_sys_param.e_bat2_sign = POSITIVE;		            // 电池2电流零点修正值符号，正｜负, 默认正
	g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero = 0.0;                       // 电池2电流零点修正绝对值，范围[0~2.0], 默认0.0
		
	
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num = 4;                     // 模块数量，范围1~24，默认4
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_protocol = MODBUS;                 // 通信协议，协议固定为modbus，不可设
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr = CURR_10A;             // 额定电流，5A/7A/10A/20A/30A/35A/40A/50A可选，默认10A
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt = 220;                  // 输出电压，这个值界面不能设定，由电池管理任务修改
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[0] = 110;             // 限流百分比，这个值界面不能设定，由电池管理任务修改
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[1] = 110;             // 限流百分比，这个值界面不能设定，由电池管理任务修改
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_max_out_volt = 286;              // 输出电压上限，范围220~286V，默认286V，这个值界面没有相关的设定项，采用默认值
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_min_out_volt = 176;              // 输出电压下限，范围176~220V，默认176V，这个值界面没有相关的设定项，采用默认值
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt = 220;          // 默认输出电压，范围176~286V，默认220V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_curr_percent = 110;      // 默认输出限流比，范围5~110%，默认110%
  
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = ONE_PATH;                     // 交流输入路数，1路/2路/无，默认1路
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase = AC_3_PHASE;                  // 交流输入相数，三相/单相，默认三相
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_high_volt = 456;//253;                   // 交流过压点，范围220~530V，默认456V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_low_volt = 327;//187;                    // 交流欠压点，范围187~380V，默认327V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_lack_phase = 200;//100;                  // 交流缺相点，范围0~380V，默认200V，在单相情况下，此项无效
	
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb = HAVE;                      // 控母配置，无/有，默认有
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl = STEP_5_7V;    // 硅链控制，无/5级4伏/5级7伏/7级3伏/7级5伏，默认无
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_75MV;        // 分流器额定电压，75mv/50mv，默认75mv
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range = 50;            // 负载分流器量程，范围25~2000A，默认100A
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt1_shunt_range = 30;           // 电池1分流器量程，范围25~2000A，默认100A
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt2_shunt_range = 30;           // 电池2分流器量程，范围25~2000A，默认100A
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_feeder_shunt_range = 100;          // 支路电流传感器量程，范围10~1000A，默认100A

	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u8_dc10_module_num = 1;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_ac_meas_modle = 0x0200;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_ac_meas_num = 0x0000;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt = 75;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt = 75;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt = 75;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range = 50;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range = 30;
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range = 30;

	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num = 1;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_ac_meas_modle = 0x0200;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_ac_meas_num = 0x0000;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_high_volt = 456;//253;                   // 交流过压点，范围220~530V，默认456V
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_low_volt  = 327;//187;                    // 交流欠压点，范围187~380V，默认327V
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_lack_phase = 200;//100;                  // 交流缺相点，范围0~380V，默认200V，在单相情况下，此项无效
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range = 100;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range = 100;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range = 100;
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u16_feeder_shunt_range = 100;          // 支路电流传感器量程，范围10~1000A，默认100A 

	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num = 1;        //馈线监测模块个数，范围0~3，默认1
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num = 31;       //馈线支路数，范围0~64
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_state_feeder_num = 31;        //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_alarm_feeder_num = 0;        //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_curr_feeder_num = 0;         //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_feeder_num = 27;           // 该馈线模块的实际支路数
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_alarm_feeder_num = 0;     // 告警量支路数，范围0~64，默认21
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_state_feeder_num = 27;     // 状态量支路数，范围0~64，默认21
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_insu_feeder_num = 0;      // 绝缘量支路数，范围0~64，默认21
	g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[0].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
		
	for (j=1; j<ACS_FEEDER_MODULE_MAX; j++)
	{
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_feeder_num = 0;            // 该馈线模块的实际支路数
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_alarm_feeder_num = 0;      // 告警量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_state_feeder_num = 0;      // 状态量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_insu_feeder_num = 0;       // 绝缘量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
	}

	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt = 286;			    // 合母过压门限，范围220~320V，默认286V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt = 187;               // 合母欠压门限，范围187~220V，默认187V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt = 235;              // 控母过压门限，范围220~242V，默认235V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt = 205;               // 控母欠压门限，范围198~220V，默认205V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt = 286;             // 母线过压门限，范围220~320V，默认286V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt = 187;              // 母线欠压门限，范围186~220V，默认187V
	//g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt = 220;            // 控母输出电压（硅链输出电压），范围210~230V，默认220V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_volt_imbalance = 50;          // 绝缘电压不平衡，范围20~100V，默认50V
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_res_thr = 25;                 // 绝缘电阻门限，范围5~99K，默认25K
	g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_dc_bus_input_ac_thr = 10;          // 交流窜直流报警门限值，范围1~50V，默认10V

	g_t_share_data.t_sys_cfg.t_batt.e_bms_type = B21;                               // 电池仪种类，B21/B3/B4，默认B21
	g_t_share_data.t_sys_cfg.t_batt.f32_cell_high_volt = 12.05;                     // 单体过压，范围2~15V，默认12.05V
	g_t_share_data.t_sys_cfg.t_batt.f32_cell_low_volt = 10.08;                      // 单体欠压，范围1.8~12V，默认10.08V
	g_t_share_data.t_sys_cfg.t_batt.f32_tail_high_volt = 12.05;                     // 末端单体过压，范围2~15V，默认12.05V
	g_t_share_data.t_sys_cfg.t_batt.f32_tail_low_volt = 10.08;                      // 末端单体欠压，范围1.8~12V，默认10.08V
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num = 1;                 // 电池仪个数，范围0~5，默认0
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num = 0;          // 实际单体电池节数
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[0] = 18;            // u8_cell_num[0]的范围0~120，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[1] = 24;            // u8_cell_num[1]的范围0~54，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[2] = 24;            // u8_cell_num[2]的范围0~24，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[3] = 24;            // u8_cell_num[3]的范围0~24，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[4] = 24;            // u8_cell_num[4]的范围0~24，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num = 1;                 // 电池仪个数，范围0~5，默认0
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num = 0;          // 实际单体电池节数
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[0] = 18;            // u8_cell_num[0]的范围0~120，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[1] = 24;            // u8_cell_num[1]的范围0~54，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[2] = 24;            // u8_cell_num[2]的范围0~24，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[3] = 24;            // u8_cell_num[3]的范围0~24，默认24
	g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[4] = 24;            // u8_cell_num[4]的范围0~24，默认24
	
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num = 1;        //馈线监测模块个数，范围0~4，默认1
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_branch_num = 17;       //馈线支路数，范围0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_state_feeder_num = 17;        //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_insu_feeder_num = 17;         //绝缘量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_alarm_feeder_num = 0;        //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_curr_feeder_num = 0;         //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_feeder_num = 17;           // 该馈线模块的实际支路数
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_alarm_feeder_num = 0;     // 告警量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_state_feeder_num = 17;     // 状态量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_insu_feeder_num = 17;      // 绝缘量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[0].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
			
		for (j=1; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_feeder_num = 0;            // 该馈线模块的实际支路数
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // 告警量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_state_feeder_num = 0;      // 状态量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_insu_feeder_num = 0;       // 绝缘量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
		}
	}
	
	for (i=g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i<FEEDER_PANEL_MAX/2; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num = 0;        //馈线监测模块个数，范围0~4，默认1
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_branch_num = 0;        //馈线支路数，范围0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_state_feeder_num = 0;         //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_insu_feeder_num = 0;          //绝缘量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_alarm_feeder_num = 0;         //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_curr_feeder_num = 0;          //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
			
		for (j=0; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_feeder_num = 0;            // 该馈线模块的实际支路数
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // 告警量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_state_feeder_num = 0;      // 状态量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_insu_feeder_num = 0;       // 绝缘量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
		}
	}
	
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num = 1;        //馈线监测模块个数，范围0~4，默认1
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_branch_num = 17;       //馈线支路数，范围0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_state_feeder_num = 17;        //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_insu_feeder_num = 17;         //绝缘量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_alarm_feeder_num = 0;        //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_curr_feeder_num = 0;         //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_feeder_num = 17;           // 该馈线模块的实际支路数
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_alarm_feeder_num = 0;     // 告警量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_state_feeder_num = 17;     // 状态量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_insu_feeder_num = 17;      // 绝缘量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[0].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
			
		for (j=1; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_feeder_num = 0;            // 该馈线模块的实际支路数
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // 告警量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_state_feeder_num = 0;      // 状态量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_insu_feeder_num = 0;       // 绝缘量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
		}
	}
	
	for (i=g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i<FEEDER_PANEL_MAX/2; i++)
	{
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num = 0;        //馈线监测模块个数，范围0~4，默认1
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_branch_num = 0;        //馈线支路数，范围0~64
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_state_feeder_num = 0;         //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_insu_feeder_num = 0;          //绝缘量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_alarm_feeder_num = 0;         //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
		g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_curr_feeder_num = 0;          //告警量支路数，范围0~64，该值没有对应的设置项，是计算出来的
			
		for (j=0; j<FEEDER_PANEL_MODULE_MAX; j++)
		{
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_feeder_num = 0;            // 该馈线模块的实际支路数
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_alarm_feeder_num = 0;      // 告警量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_state_feeder_num = 0;      // 状态量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_insu_feeder_num = 0;       // 绝缘量支路数，范围0~64，默认21
			g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
		}
	}
	
	//g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num = 0;             //通信模块个数，范围0~8，默认0
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num = 0;           //馈线监测模块个数，范围0~1，默认0
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num = 0;           //馈线支路数，范围0~64
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_state_feeder_num = 0;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_alarm_feeder_num = 0;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_curr_feeder_num = 0;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_dcdc_panel.u16_feeder_shunt_range = 100;       // 支路电流传感器量程，范围10~1000A，默认100A
	g_t_share_data.t_sys_cfg.t_dcdc_panel.e_protocol = DCDC_MODBUS;           // 通信协议，协议为modbus/can，默认modbus
	g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr = DCDC_CURR_100A;       // 额定电流，5A/10A/20A/30A/40A/50A/60A/80A/100A可选，默认100A
	g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt = 48;                  // 输出电压，范围40~60，默认48
	g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent = 100;             // 限流百分比，范围10%~100%
	
	for (i=0; i<DCDC_FEEDER_MODULE_MAX; i++)
	{
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_feeder_num = 0;            // 该馈线模块的实际支路数
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_alarm_feeder_num = 0;      // 告警量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_state_feeder_num = 0;      // 状态量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_insu_feeder_num = 0;       // 绝缘量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[i].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
	}
	
	//g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num = 0;             //UPS模块个数，范围0~8，默认0
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num = 0;           //馈线监测模块个数，范围0~1，默认0
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num = 0;           //馈线支路数，范围0~64
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_state_feeder_num = 0;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_alarm_feeder_num = 0;            //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_dcac_panel.u8_curr_feeder_num = 0;             //状态量支路数，范围0~64，该值没有对应的设置项，是计算出来的
	g_t_share_data.t_sys_cfg.t_dcac_panel.u16_feeder_shunt_range = 100;       // 支路电流传感器量程，范围10~1000A，默认100A
	g_t_share_data.t_sys_cfg.t_dcac_panel.e_protocol = MODBUS;                // 通信协议，协议固定为modbus，不可设
	
	for (i=0; i<DCAC_FEEDER_MODULE_MAX; i++)
	{
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_feeder_num = 0;            // 该馈线模块的实际支路数
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_alarm_feeder_num = 0;      // 告警量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].e_alarm_type = NORMALLY_OPEN; // 告警量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_state_feeder_num = 0;      // 状态量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].e_state_type = NORMALLY_OPEN; // 状态量输入方式，常开｜常闭，默认常开
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_insu_feeder_num = 0;       // 绝缘量支路数，范围0~64，默认21
		g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[i].u8_curr_feeder_num = 0;       // 电流量支路数，范围0~64，默认0
	}
	
	v_disp_volt_init_by_level();    //根据电压等级设置相关的电压值

	g_t_share_data.t_sys_cfg.t_ctl.u16_batt[0] = 0;                                    //手动充电控制，0:浮充，1:均充，2:核容
	g_t_share_data.t_sys_cfg.t_ctl.u16_batt[1] = 0;                                    //手动充电控制，0:浮充，1:均充，2:核容
	g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;                                      // bit0~1:交流工作策略，0：一路优先，1：二路优先，2：固定一路，3：固定二路
														                            // bit2~15:保留
	for (i=0; i<RECT_CNT_MAX; i++)
		g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl[i] = 0;                        // 充电模块状态控制，1：关机，0：开机

	os_mut_release(g_mut_share_data);
	
	v_fetch_save_cfg_data();   //保存参数到DATAFLASH
}

/*************************************************************
函数名称: v_disp_adjust_data_restore	           				
函数功能: 校准系数初始化					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_adjust_data_restore(void)
{
	os_mut_wait(g_mut_share_data, 0xFFFF);

	memset(&g_t_share_data.t_coeff_data, 0, sizeof(g_t_share_data.t_coeff_data));

	//交流电压缺省斜率
	g_t_share_data.t_coeff_data.f32_ac1_uv_slope = 1.19;				
	g_t_share_data.t_coeff_data.f32_ac1_vw_slope = 1.19;
	g_t_share_data.t_coeff_data.f32_ac1_wu_slope = 1.19;
	
	g_t_share_data.t_coeff_data.f32_ac2_uv_slope = 1.19;				
	g_t_share_data.t_coeff_data.f32_ac2_vw_slope = 1.19;
	g_t_share_data.t_coeff_data.f32_ac2_wu_slope = 1.19;
	
	//直流电压缺省测量系数
	g_t_share_data.t_coeff_data.f32_v1_vol_slope = 0.1209;
	g_t_share_data.t_coeff_data.s16_v1_vol_zero  = 0;
	g_t_share_data.t_coeff_data.f32_v2_vol_slope = 0.1209;
	g_t_share_data.t_coeff_data.s16_v2_vol_zero	 = 0;
	g_t_share_data.t_coeff_data.f32_v3_vol_slope = 0.1209;
	g_t_share_data.t_coeff_data.s16_v3_vol_zero  = 0;

	g_t_share_data.t_coeff_data.s16_a1_fixed_zero = 2048;
	g_t_share_data.t_coeff_data.s16_a2_fixed_zero = 2048;
	g_t_share_data.t_coeff_data.s16_a3_fixed_zero = 2048;

	g_t_share_data.t_coeff_data.f32_a1_curr_slope = 0.0562;//0.0649;
	g_t_share_data.t_coeff_data.f32_a1_curr_zero  = -115;//133
	g_t_share_data.t_coeff_data.f32_a2_curr_slope = 0.0562;//0.0649;
	g_t_share_data.t_coeff_data.f32_a2_curr_zero  = -115;//133
	g_t_share_data.t_coeff_data.f32_a3_curr_slope = 0.0562;//0.0649;
	g_t_share_data.t_coeff_data.f32_a3_curr_zero  = -115;//133

	g_t_share_data.t_coeff_data.f32_ref_volt = 2.5;

	g_t_share_data.t_coeff_data.f32_neg_vol_slope = 0.1209;  // 负对地电压测量斜率
	g_t_share_data.t_coeff_data.s16_neg_vol_zero  = 0;       // 负对地电压测量零点

	os_mut_release(g_mut_share_data);

	v_fetch_save_adjust_coeff();
}

/*************************************************************
函数名称: v_disp_cfg_data_init		           				
函数功能: 配置数据初始化						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_disp_cfg_data_init(void)
{
	U8_T i, j;
	os_mut_init(g_mut_share_data);           //初始化互斥量

	os_mut_wait(g_mut_share_data, 0xFFFF);
	memset(&g_t_share_data, 0, sizeof(g_t_share_data));
	os_mut_release(g_mut_share_data);

	g_u8_product_type = u8_disp_get_product_type();

	if (s32_fetch_read_cfg_data() == -1)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		memset(&(g_t_share_data.t_sys_cfg), 0, sizeof(g_t_share_data.t_sys_cfg));
		os_mut_release(g_mut_share_data);

		v_disp_cfg_data_restore();
	}

	if (s32_fetch_read_adjust_coeff() == -1)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		memset(&(g_t_share_data.t_coeff_data), 0, sizeof(g_t_share_data.t_coeff_data));
		os_mut_release(g_mut_share_data);

		v_disp_adjust_data_restore();
	}
	
	v_fetch_read_swt_cfg_data();    //读取开关量配置
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	for (i=0; i<FEEDER_PANEL_MAX; i++)
	{
		for (j=0; j<FEEDER_BRANCH_MAX; j++)
		{
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_alarm = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_state = 1;//00本路无开关状态检测，01表示断开，02表示闭合
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_insu_state = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].u8_curr_state = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].u8_sensor_state[j] = 1;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].f32_res = 999.9;
			g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].f32_curr = 0.0;
		}
	}
		
	for (j=0; j<FEEDER_BRANCH_MAX; j++)
	{
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].u8_alarm = 1;
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].u8_state = 2;//00本路无开关状态检测，01表示断开，02表示闭合
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].u8_curr_state = 1;
		g_t_share_data.t_rt_data.t_dcdc_panel.u8_sensor_state[j] = 1;
		g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[j].f32_curr = 0.0;
	}
	
	for (j=0; j<FEEDER_BRANCH_MAX; j++)
	{
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].u8_alarm = 1;
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].u8_state = 2;//00本路无开关状态检测，01表示断开，02表示闭合
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].u8_curr_state = 1;
		g_t_share_data.t_rt_data.t_dcac_panel.u8_sensor_state[j] = 1;
		g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[j].f32_curr = 0.0;
	}

	g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = 0;
	for (i=0; i<RELAY_MAX; i++)
		g_t_share_data.t_sys_cfg.t_ctl.u8_relay[i] = 0;

	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_display_init		           				
函数功能: 显示初始化，初始化全局数据区，初始化页面结构的数据指针						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_display_init(void)
{
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	m_t_win_record.u16_curr_id = MMI_WIN_ID_MAIN_WINDOW;
	m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
	{
		m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
		m_t_win_record.t_curr_win.u8_icon_cnt = 1;
	}
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	m_e_volt_level = g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level;
	m_e_lang = g_t_share_data.t_sys_cfg.t_sys_param.e_lang;
	m_e_lcd_direction = g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection;
	
	if (m_e_lcd_direction == LCD_HORIZONTAL)   //根据显示方向初始化按键值
	{
		g_u16_key_cancel = KEY_K7;
		g_u16_key_enter = KEY_K6;
		g_u16_key_up = KEY_K4;
		g_u16_key_down = KEY_K5;
		g_u16_key_add = KEY_K3;
		g_u16_key_sub = KEY_K2;
	}
	else
	{
		g_u16_key_cancel = KEY_K7;
		g_u16_key_enter = KEY_K6;
		g_u16_key_up = KEY_K3;
		g_u16_key_down = KEY_K2;
		g_u16_key_add = KEY_K5;
		g_u16_key_sub = KEY_K4;
	}
	 
	os_mut_release(g_mut_share_data);

	v_relay_relay_operation(ENABLE_BACKLIGHT);
}


/*************************************************************
函数名称: s_disp_get_record_name_by_id		           				
函数功能: 通过事件ID查找事件的名称						
输入参数: u16_id -- 事件ID        		   				
输出参数: 无
返回值  ：名称字符串指针														   				
**************************************************************/
static char *s_disp_get_record_name_by_id(U16_T u16_id)
{
	U16_T u16_group = ((u16_id & LOG_ID_GROUP_MASK) >> LOG_ID_GROUP_OFFSET);
	U16_T u16_index = (u16_id & LOG_ID_INDEX_MASK);
	char *s_name = NULL;

	if (u16_group == LOG_ID_BATT_GROUP)
	{
		if (m_e_lang == CHINESE)
			s_name = g_s_batt_record[u16_index][0];
		else
			s_name = g_s_batt_record[u16_index][1];
	}

	return s_name;
}


/*************************************************************
函数名称: s_disp_get_fault_name_by_id		           				
函数功能: 通过故障ID查找故障的名称						
输入参数: u16_id -- 故障ID        		   				
输出参数: 无
返回值  ：名称字符串指针														   				
**************************************************************/
static char *s_disp_get_fault_name_by_id(U16_T u16_id)
{
	U16_T u16_group = ((u16_id & FAULT_ID_GROUP_MASK) >> FAULT_GROUP_OFFSET);
	U16_T u16_index = (u16_id & FAULT_ID_INDEX_MASK);
	U32_T u32_addr;
	
	switch (u16_group)
	{
		case FAULT_AC_GROUP:                  //交流
			u32_addr = DATAFLASH_AC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_DC_BUS_GROUP:              //直流母线
			u32_addr = DATAFLASH_DC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_BATT1_GROUP:               //1组电池异常
			u32_addr = DATAFLASH_BATT1_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
		
		case FAULT_BATT2_GROUP:               //2组电池异常
			u32_addr = DATAFLASH_BATT2_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_BMS_GROUP:                 //电池巡检故障
			u32_addr = DATAFLASH_BMS_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_RECT_GROUP:                //整流模块故障
			u32_addr = DATAFLASH_RECT_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCAC_GROUP:                //逆变模块故障
			u32_addr = DATAFLASH_DCAC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCDC_GROUP:                //通信模块故障
			u32_addr = DATAFLASH_DCDC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_FC_GROUP:                  //馈线模块故障
			u32_addr = DATAFLASH_FC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_RC_GROUP:                  //RC10模块故障
			u32_addr = DATAFLASH_RC_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_SWT_GROUP:                 //自带开关量故障
			u32_addr = DATAFLASH_SWT_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_DC_PANEL1_GROUP:           //分电屏01故障
			u32_addr = DATAFLASH_DC_PANEL1_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_DC_PANEL2_GROUP:           //分电屏02故障
			u32_addr = DATAFLASH_DC_PANEL2_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL3_GROUP:           //分电屏03故障
			u32_addr = DATAFLASH_DC_PANEL3_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL4_GROUP:           //分电屏04故障
			u32_addr = DATAFLASH_DC_PANEL4_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL5_GROUP:           //分电屏05故障
			u32_addr = DATAFLASH_DC_PANEL5_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL6_GROUP:           //分电屏06故障
			u32_addr = DATAFLASH_DC_PANEL6_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL7_GROUP:           //分电屏07故障
			u32_addr = DATAFLASH_DC_PANEL7_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DC_PANEL8_GROUP:           //分电屏08故障
			u32_addr = DATAFLASH_DC_PANEL8_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCDC_PANEL_GROUP:          //通信屏故障
			u32_addr = DATAFLASH_DCDC_PANEL_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		case FAULT_DCAC_PANEL_GROUP:          //UPS屏故障
			u32_addr = DATAFLASH_DCAC_PANEL_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;

		case FAULT_AC_PANEL_GROUP:          //交流屏故障
			u32_addr = DATAFLASH_AC_PANEL_GROUP_START_ADDR + u16_index * DATAFLASH_FAULT_NAME_ITEM_SIZE;
			break;
			
		default:
			return NULL;
	}
	
	if (m_e_lang == ENGLISH)
		u32_addr += (DATAFLASH_FAULT_NAME_ITEM_SIZE / 2);
	
	memset(m_t_win_record.u8_buffer, 0, sizeof(m_t_win_record.u8_buffer));
	s32_flash_dataflash_read(u32_addr, m_t_win_record.u8_buffer, DATAFLASH_FAULT_NAME_ITEM_SIZE/2);

	return (char *)(m_t_win_record.u8_buffer);
}

/*************************************************************
函数名称: s_disp_get_branch_name_by_id		           				
函数功能: 通过故障ID查找故障的名称						
输入参数: u8_panel  -- 馈线屏号
          u8_branch -- 屏内支路号
输出参数: 无
返回值  ：名称字符串指针														   				
**************************************************************/
static char *s_disp_get_branch_name_by_id(U8_T u8_panel, U8_T u8_branch)
{
	U32_T u32_addr = DATAFLASH_BRANCH_NAME_ADDR + (u8_panel*64 + u8_branch)*DATAFLASH_BRANCH_NAME_ITME_SIZE;
	
	if (m_e_lang == ENGLISH)
		u32_addr += (DATAFLASH_BRANCH_NAME_ITME_SIZE / 2);
	
	memset(m_t_win_record.u8_buffer, 0, sizeof(m_t_win_record.u8_buffer));
	s32_flash_dataflash_read(u32_addr, m_t_win_record.u8_buffer, DATAFLASH_BRANCH_NAME_ITME_SIZE/2);

	return (char *)(m_t_win_record.u8_buffer);
}

/*************************************************************
函数名称: s_disp_get_about_name_by_index		           				
函数功能: 通过故障ID查找故障的名称						
输入参数: u8_index -- 索引，范围0~7，共8条信息       		   				
输出参数: 无
返回值  ：名称字符串指针														   				
**************************************************************/
static char *s_disp_get_about_name_by_index(U8_T u8_index)
{
	U32_T u32_addr = DATAFLASH_ABOUT_ADDR + u8_index*DATAFLASH_ABOUT_ITEM_SIZE;
	
	if (m_e_lang == ENGLISH)
		u32_addr += (DATAFLASH_ABOUT_ITEM_SIZE / 2);
	
	memset(m_t_win_record.u8_buffer, 0, sizeof(m_t_win_record.u8_buffer));
	s32_flash_dataflash_read(u32_addr, m_t_win_record.u8_buffer, DATAFLASH_ABOUT_ITEM_SIZE/2);

	return (char *)(m_t_win_record.u8_buffer);
}

/*************************************************************
函数名称: s_disp_get_swt_name_by_index		           				
函数功能: 通过开关索引查找开关的名称						
输入参数: u8_index -- 索引      		   				
输出参数: 无
返回值  ：名称字符串指针														   				
**************************************************************/
static char *s_disp_get_swt_name_by_index(U8_T u8_index)
{
	if (m_e_lang == CHINESE)
		return (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[u8_index].s_ch_name);
	else
		return (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[u8_index].s_en_name);
}

/*************************************************************
函数名称: s_disp_get_product_type_name		           				
函数功能: 得到产品的型号名称					
输入参数: 无       		   				
输出参数: 无
返回值  ：名称字符串指针														   				
**************************************************************/
static char *s_disp_get_product_type_name(void)
{
	if (g_u8_product_type == PRODUCT_TYPE_SC12)
	{
		if (m_e_lang == CHINESE)
			return (g_s_string[443][0]);
		else
			return (g_s_string[443][1]);	
	}
		
	else if (g_u8_product_type == PRODUCT_TYPE_SC22)
	{
		if (m_e_lang == CHINESE)
			return (g_s_string[444][0]);
		else
			return (g_s_string[444][1]);
	}
		
	else
	{
		if (m_e_lang == CHINESE)
			return (g_s_string[445][0]);
		else
			return (g_s_string[445][1]);
	}
}

/*************************************************************
函数名称: s32_disp_judge_record		           				
函数功能: 通过事件ID判断事件ID是否正确						
输入参数: u16_id -- 事件ID        		   				
输出参数: 无
返回值  ：0：正确，-1：不正确														   				
**************************************************************/
static S32_T s32_disp_judge_record(LOG_DATA_T *pt_record)
{
	U16_T u16_group = ((pt_record->u16_log_id & LOG_ID_GROUP_MASK) >> LOG_ID_GROUP_OFFSET);
	U16_T u16_index = (pt_record->u16_log_id & LOG_ID_INDEX_MASK);

	if (u16_group == LOG_ID_BATT_GROUP)
	{
		if (u16_index < LOG_ID_BATT_NUM * 2)
			return 0;
	}

	return -1;
}

/*************************************************************
函数名称: s32_disp_judge_no_resume_fault		           				
函数功能: 判断掉电末复归故障是否正确						
输入参数: t_fault -- 指向要判断的掉电末复归故障        		   				
输出参数: 无
返回值  ：0--故障正常，-1--故障异常														   				
**************************************************************/
static S32_T s32_disp_judge_no_resume_fault(const CURR_FAULT_DATA_T *pt_fault)
{
	U16_T u16_group = ((pt_fault->u16_id & FAULT_ID_GROUP_MASK) >> FAULT_GROUP_OFFSET);
	U16_T u16_index = (pt_fault->u16_id & FAULT_ID_INDEX_MASK);
	
	switch (u16_group)
	{
		case FAULT_AC_GROUP:                  //交流
			if (u16_index >= FAULT_AC_NUM)
				return -1;
			break;

		case FAULT_DC_BUS_GROUP:              //直流母线
			if (u16_index >= FAULT_DC_NUM)
				return -1;
			break;

		case FAULT_BATT1_GROUP:               //1组电池异常
			if (u16_index >= FAULT_BATT1_NUM)
				return -1;
			break;
			
		case FAULT_BATT2_GROUP:               //2组电池异常
			if (u16_index >= FAULT_BATT2_NUM)
				return -1;
			break;

		case FAULT_BMS_GROUP:                 //电池巡检故障
			if (u16_index >= FAULT_BMS_NUM)
				return -1;
			break;

		case FAULT_RECT_GROUP:                //整流模块故障
			if (u16_index >= FAULT_RECT_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_GROUP:                //通信模块故障
			if (u16_index >= FAULT_DCDC_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_GROUP:                //逆变模块故障
			if (u16_index >= FAULT_DCAC_NUM)
				return -1;
			break;
			
		case FAULT_FC_GROUP:                  //馈线模块故障
			if (u16_index >= FAULT_FC_COMM_FIAL_NUM)
				return -1;
			break;

		case FAULT_RC_GROUP:                  //RC10模块故障
			if (u16_index >= FAULT_RC_COMM_FIAL_NUM)
				return -1;
			break;
			
		case FAULT_SWT_GROUP:                 //自带开关量故障
			if (u16_index >= FAULT_SWT_NUM)
				return -1;
			break;

		case FAULT_DC_PANEL1_GROUP:           //分电屏01故障
		case FAULT_DC_PANEL2_GROUP:           //分电屏02故障
		case FAULT_DC_PANEL3_GROUP:           //分电屏03故障
		case FAULT_DC_PANEL4_GROUP:           //分电屏04故障
		case FAULT_DC_PANEL5_GROUP:           //分电屏05故障
		case FAULT_DC_PANEL6_GROUP:           //分电屏06故障
		case FAULT_DC_PANEL7_GROUP:           //分电屏07故障
		case FAULT_DC_PANEL8_GROUP:           //分电屏08故障
			if (u16_index >= FAULT_DC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_PANEL_GROUP:          //通信屏故障
			if (u16_index >= FAULT_DCDC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_PANEL_GROUP:          //UPS屏故障
			if (u16_index >= FAULT_DCAC_PANEL_NUM)
				return -1;
			break;

		case FAULT_AC_PANEL_GROUP:          //交流屏故障
			if (u16_index >= FAULT_AC_PANEL_NUM)
				return -1;
			break;
						
		default:
			return -1;
	}

	if ((pt_fault->u8_occur_year > 99) || (pt_fault->u8_occur_mon < 1) || (pt_fault->u8_occur_mon > 12)
		|| (pt_fault->u8_occur_day < 1) || (pt_fault->u8_occur_day > 31) || (pt_fault->u8_occur_hour > 23)
		|| (pt_fault->u8_occur_min > 59) && (pt_fault->u8_occur_sec > 59))
	{
		return -1;
	}

	return 0;
}


/*************************************************************
函数名称: s32_disp_judge_his_fault		           				
函数功能: 判断历史故障是否正确						
输入参数: t_fault -- 指向要判断的历史故障        		   				
输出参数: 无
返回值  ：0--故障正常，-1--故障异常														   				
**************************************************************/
static S32_T s32_disp_judge_his_fault(const HIS_FAULT_DATA_T *pt_fault)
{
	U16_T u16_group = ((pt_fault->u16_id & FAULT_ID_GROUP_MASK) >> FAULT_GROUP_OFFSET);
	U16_T u16_index = (pt_fault->u16_id & FAULT_ID_INDEX_MASK);
	
	switch (u16_group)
	{
		case FAULT_AC_GROUP:                  //交流
			if (u16_index >= FAULT_AC_NUM)
				return -1;
			break;

		case FAULT_DC_BUS_GROUP:              //直流母线
			if (u16_index >= FAULT_DC_NUM)
				return -1;
			break;

		case FAULT_BATT1_GROUP:               //1组电池异常
			if (u16_index >= FAULT_BATT1_NUM)
				return -1;
			break;
			
		case FAULT_BATT2_GROUP:               //2组电池异常
			if (u16_index >= FAULT_BATT2_NUM)
				return -1;
			break;

		case FAULT_BMS_GROUP:                 //电池巡检故障
			if (u16_index >= FAULT_BMS_NUM)
				return -1;
			break;

		case FAULT_RECT_GROUP:                //整流模块故障
			if (u16_index >= FAULT_RECT_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_GROUP:                //通信模块故障
			if (u16_index >= FAULT_DCDC_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_GROUP:                //逆变模块故障
			if (u16_index >= FAULT_DCAC_NUM)
				return -1;
			break;
			
		case FAULT_FC_GROUP:                  //馈线模块故障
			if (u16_index >= FAULT_FC_COMM_FIAL_NUM)
				return -1;
			break;

		case FAULT_RC_GROUP:                  //RC10模块故障
			if (u16_index >= FAULT_RC_COMM_FIAL_NUM)
				return -1;
			break;
			
		case FAULT_SWT_GROUP:                 //自带开关量故障
			if (u16_index >= FAULT_SWT_NUM)
				return -1;
			break;

		case FAULT_DC_PANEL1_GROUP:           //分电屏01故障
		case FAULT_DC_PANEL2_GROUP:           //分电屏02故障
		case FAULT_DC_PANEL3_GROUP:           //分电屏03故障
		case FAULT_DC_PANEL4_GROUP:           //分电屏04故障
		case FAULT_DC_PANEL5_GROUP:           //分电屏05故障
		case FAULT_DC_PANEL6_GROUP:           //分电屏06故障
		case FAULT_DC_PANEL7_GROUP:           //分电屏07故障
		case FAULT_DC_PANEL8_GROUP:           //分电屏08故障
			if (u16_index >= FAULT_DC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCDC_PANEL_GROUP:          //通信屏故障
			if (u16_index >= FAULT_DCDC_PANEL_NUM)
				return -1;
			break;
			
		case FAULT_DCAC_PANEL_GROUP:          //UPS屏故障
			if (u16_index >= FAULT_DCAC_PANEL_NUM)
				return -1;
			break;

		case FAULT_AC_PANEL_GROUP:          //交流屏故障
			if (u16_index >= FAULT_AC_PANEL_NUM)
				return -1;
			break;
			
		default:
			return -1;
	}

	if ((pt_fault->u8_occur_year > 99) || (pt_fault->u8_occur_mon < 1) || (pt_fault->u8_occur_mon > 12)
		|| (pt_fault->u8_occur_day < 1) || (pt_fault->u8_occur_day > 31) || (pt_fault->u8_occur_hour > 23)
		|| (pt_fault->u8_occur_min > 59) || (pt_fault->u8_occur_sec > 59))
	{
		return -1;
	}

	if ((pt_fault->u8_resume_year > 99) || (pt_fault->u8_resume_mon < 1) || (pt_fault->u8_resume_mon > 12)
		|| (pt_fault->u8_resume_day < 1) || (pt_fault->u8_resume_day > 31) || (pt_fault->u8_resume_hour > 23)
		|| (pt_fault->u8_resume_min > 59) || (pt_fault->u8_resume_sec > 59))
	{
		return -1;
	}

	return 0;
}

/*************************************************************
函数名称: v_disp_his_falut_page		           				
函数功能: 显示历史故障页，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_mut_fetch_his_fault						
输入参数: 无       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_his_falut_page(void)
{
	U16_T i, num;
	HIS_FAULT_DATA_T t_fault;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	char *s1, *s2;
	
	num = u16_fault_read_his_fault_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d", 
					s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);

	num -=m_t_win_record.u_ordinal.u16_his_fault;
	
	for (i=0; i<MMI_HIS_FAULT_CNT && i<num; i++)
	{
		v_fault_read_his_fault(&t_fault, m_t_win_record.u_ordinal.u16_his_fault+i);
	
		if (s32_disp_judge_his_fault(&t_fault) == 0)				
		{
			s1 = s_disp_get_fault_name_by_id(t_fault.u16_id);
			s2 = s_lcd_lcd_cut_string(s1, MMI_FAULT_NAME_WIDTH);
			 
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s1);
			if (s2 != NULL)
				snprintf((char *)(m_t_win_record.u8_special_name[i*4+2]), sizeof(m_t_win_record.u8_special_name[i*4+2]),
						"    %s", s2);
			else
				memset(m_t_win_record.u8_special_name[i*4+2], 0, sizeof(m_t_win_record.u8_special_name[i*4+2]));

			snprintf((char *)(m_t_win_record.u8_special_name[i*4+3]), sizeof(m_t_win_record.u8_special_name[i*4+3]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*4+3].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_fault.u8_occur_year, t_fault.u8_occur_mon, t_fault.u8_occur_day, t_fault.u8_occur_hour,
					t_fault.u8_occur_min, t_fault.u8_occur_sec);
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+4]), sizeof(m_t_win_record.u8_special_name[i*4+4]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*4+4].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_fault.u8_resume_year, t_fault.u8_resume_mon, t_fault.u8_resume_day, t_fault.u8_resume_hour,
					t_fault.u8_resume_min, t_fault.u8_resume_sec);
		}
		else
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s_disp_get_string_pointer(183));
			memset(m_t_win_record.u8_special_name[i*4+2], 0, sizeof(m_t_win_record.u8_special_name[i*4+2]));
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+3]), sizeof(m_t_win_record.u8_special_name[i*4+3]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*4+3].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
			snprintf((char *)(m_t_win_record.u8_special_name[i*4+4]), sizeof(m_t_win_record.u8_special_name[i*4+4]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*4+4].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
		}
	}

	win->u8_item_cnt = i*4+1;
}

/*************************************************************
函数名称: v_disp_his_fault_first_page		           				
函数功能: 显示历史故障第一页，调用此函数前需要获取互斥量g_mut_fetch_his_fault						
输入参数: 无       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_his_fault_first_page(void)
{
	U16_T num;
	
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	
	v_fault_reset_his_fault_state();
	num  = u16_fault_read_his_fault_cnt();
	if (num > 0)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_INFO;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		if (num > MMI_HIS_FAULT_CNT)
			m_t_win_record.t_curr_win.u8_icon_cnt = 2;
		else
			m_t_win_record.t_curr_win.u8_icon_cnt = 0;

		v_disp_his_falut_page();
	}
	else
	{
		m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_HIS_FAULT; // 无历史故障记录
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
函数名称: v_disp_no_resume_fault_page		           				
函数功能: 掉电末复归故障显示处理函数，将显示数据打印到缓冲区中						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_no_resume_fault_page(void)
{
	U16_T i, num;
	CURR_FAULT_DATA_T t_fault;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	char *s1, *s2;
	
	num = u16_fault_read_no_resume_fault_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d", 
					s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);

	num -= m_t_win_record.u_ordinal.u16_exception;
	
	for (i=0; (i<MMI_EXCEPTION_CNT && i<num); i++)
	{
		v_fault_read_no_resume_fault(&t_fault, m_t_win_record.u_ordinal.u16_exception+i);

		if (s32_disp_judge_no_resume_fault(&t_fault) == 0)
		{
			s1 = s_disp_get_fault_name_by_id(t_fault.u16_id);
			s2 = s_lcd_lcd_cut_string(s1, MMI_FAULT_NAME_WIDTH);
			 
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+1]), sizeof(m_t_win_record.u8_special_name[i*3+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s1);
			if (s2 != NULL)
				snprintf((char *)(m_t_win_record.u8_special_name[i*3+2]), sizeof(m_t_win_record.u8_special_name[i*3+2]),
						"    %s", s2);
			else
				memset(m_t_win_record.u8_special_name[i*3+2], 0, sizeof(m_t_win_record.u8_special_name[i*3+2]));

			snprintf((char *)(m_t_win_record.u8_special_name[i*3+3]), sizeof(m_t_win_record.u8_special_name[i*3+3]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*3+3].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_fault.u8_occur_year, t_fault.u8_occur_mon, t_fault.u8_occur_day, t_fault.u8_occur_hour,
					t_fault.u8_occur_min, t_fault.u8_occur_sec);
		}
		else
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+1]), sizeof(m_t_win_record.u8_special_name[i*3+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s_disp_get_string_pointer(183));
			memset(m_t_win_record.u8_special_name[i*3+2], 0, sizeof(m_t_win_record.u8_special_name[i*3+2]));
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+3]), sizeof(m_t_win_record.u8_special_name[i*3+3]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*3+3].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
		}
	}

	win->u8_item_cnt = i*3+1;
}

/*************************************************************
函数名称: v_disp_curr_falut_page		           				
函数功能: 显示当前故障页，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_mut_fault_data						
输入参数: 无       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_curr_falut_page(void)
{
	U16_T i, num;
	CURR_FAULT_DATA_T t_fault;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	char *s1, *s2;
	
	num  = u16_fault_read_curr_fault_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d", 
				s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);
				
	num -= m_t_win_record.u_ordinal.u16_curr_fault;

	for (i=0; (i<MMI_CURR_FAULT_CNT && i<num); i++)
	{
		v_fault_read_curr_fault(&t_fault, m_t_win_record.u_ordinal.u16_curr_fault+i);
		
		s1 = s_disp_get_fault_name_by_id(t_fault.u16_id);
		s2 = s_lcd_lcd_cut_string(s1, MMI_FAULT_NAME_WIDTH);
		 
		snprintf((char *)(m_t_win_record.u8_special_name[i*3+1]), sizeof(m_t_win_record.u8_special_name[i*3+1]),
				"%03d:%s", m_t_win_record.u_ordinal.u16_his_fault+i+1, s1);
		if (s2 != NULL)
			snprintf((char *)(m_t_win_record.u8_special_name[i*3+2]), sizeof(m_t_win_record.u8_special_name[i*3+2]),
					"    %s", s2);
		else
			memset(m_t_win_record.u8_special_name[i*3+2], 0, sizeof(m_t_win_record.u8_special_name[i*3+2]));

		snprintf((char *)(m_t_win_record.u8_special_name[i*3+3]), sizeof(m_t_win_record.u8_special_name[i*3+3]),
				"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*3+3].u16_name_index & MMI_STR_ID_INDEX_MASK),
				t_fault.u8_occur_year, t_fault.u8_occur_mon, t_fault.u8_occur_day, t_fault.u8_occur_hour,
				t_fault.u8_occur_min, t_fault.u8_occur_sec);
	}

	win->u8_item_cnt = i*3+1;
}

/*************************************************************
函数名称: v_disp_curr_falut_first_page		           				
函数功能: 显示当前故障第一页，调用此函数前需要获取互斥量g_mut_fault_data						
输入参数: 无       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_curr_falut_first_page(void)
{
	U16_T num;
	
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	
	v_fault_reset_curr_fault_state();
	num  = u16_fault_read_curr_fault_cnt();
	if (num > 0)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CURR_FAULT_INFO;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		if (num > MMI_CURR_FAULT_CNT)
			m_t_win_record.t_curr_win.u8_icon_cnt = 2;
		else
			m_t_win_record.t_curr_win.u8_icon_cnt = 0;

		v_disp_curr_falut_page();
	}
	else
	{
		m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CURR_FAULT;   // 无当前故障
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
函数名称: v_disp_record_page		           				
函数功能: 事件记录显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_mut_log						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_record_page(void)
{
	U16_T i, num;
	LOG_DATA_T t_log_data;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = u16_log_read_log_cnt();
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]),
				"%s%d", s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK), num);
				
	num -= m_t_win_record.u_ordinal.u16_record;

	for (i=0; (i<MMI_RECORD_CNT && i<num); i++)
	{
		v_log_read_log_record(&t_log_data, m_t_win_record.u_ordinal.u16_record+i);

		if (s32_disp_judge_record(&t_log_data) == 0)
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+1]), sizeof(m_t_win_record.u8_special_name[i*2+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_record+i+1, s_disp_get_record_name_by_id(t_log_data.u16_log_id));
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+2]), sizeof(m_t_win_record.u8_special_name[i*2+2]),
					"%s%02d-%02d-%02d %02d:%02d:%02d", s_disp_get_string_pointer(win->t_item[i*2+2].u16_name_index & MMI_STR_ID_INDEX_MASK),
					t_log_data.u8_occur_year, t_log_data.u8_occur_mon, t_log_data.u8_occur_day, t_log_data.u8_occur_hour,
					t_log_data.u8_occur_min, t_log_data.u8_occur_sec);
		}
		else
		{
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+1]), sizeof(m_t_win_record.u8_special_name[i*2+1]),
					"%03d:%s", m_t_win_record.u_ordinal.u16_record+i+1, s_disp_get_string_pointer(183));
			snprintf((char *)(m_t_win_record.u8_special_name[i*2+2]), sizeof(m_t_win_record.u8_special_name[i*2+2]),
					"%s%s", s_disp_get_string_pointer(win->t_item[i*2+2].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_string_pointer(151));
		}
	}

	win->u8_item_cnt = i*2+1;
}

/*************************************************************
函数名称: v_disp_record_first_page		           				
函数功能: 显示事件记录第一页，调用此函数前需要获取互斥量g_mut_log						
输入参数: 无       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_record_first_page(void)
{
	U32_T num;
	
	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	
	num = u16_log_read_log_cnt();
	if (num > 0)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		if (num > MMI_RECORD_CNT)
			m_t_win_record.t_curr_win.u8_icon_cnt = 2;
		else
			m_t_win_record.t_curr_win.u8_icon_cnt = 0;
			
		v_disp_record_page();
	}
	else
	{
		m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_RECORD;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
函数名称: v_disp_feeder_branch_info		           				
函数功能: 馈出支路信息显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_feeder_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T panel = m_u8_feeder_panel_ordianl;
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
								panel+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
	
	num = g_t_share_data.t_sys_cfg.t_feeder_panel[panel].u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*5+1]), sizeof(m_t_win_record.u8_special_name[i*5+1]), "%s", 
								s_disp_get_branch_name_by_id(panel, branch+i));
								
		win->t_item[i*5+2].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].u8_state);
		
		m_t_win_record.u32_set_value = 0;

		if (g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].u8_insu_state == 0)
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+3].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].f32_res);
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+4].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+4].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*5+5].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[panel].u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*5+1;
}

/*************************************************************
函数名称: v_disp_feeder_branch_info		           				
函数功能: 馈出支路信息显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_feeder_branch_info2(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T panel = m_u8_feeder_panel_ordianl;
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
								panel+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
	
	num = g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel].u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*5+1]), sizeof(m_t_win_record.u8_special_name[i*5+1]), "%s", 
								s_disp_get_branch_name_by_id(4+panel, branch+i));
								
		win->t_item[i*5+2].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].u8_state);
		
		m_t_win_record.u32_set_value = 0;

		if (g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].u8_insu_state == 0)
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+3].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].f32_res);
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_ENUM;
			win->t_item[i*5+4].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*5+4].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*5+4].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*5+5].pv_val = &(g_t_share_data.t_rt_data.t_feeder_panel[4+panel].u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*5+1;
}

/*************************************************************
函数名称: v_disp_rect_info		           				
函数功能: 整流模块信息显示处理函数，将显示数据打印到缓冲区中						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_rect_info(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%02d%s", 
			m_t_win_record.u_ordinal.u16_rect_info+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));

	win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].f32_out_volt);
	win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].f32_out_curr);
	win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].f32_curr_percent);
	win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].b_ctl_mode);
	win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].e_module_state);
}

/*************************************************************
函数名称: v_disp_dcdc_module_info		           				
函数功能: 通信模块信息显示处理函数，将显示数据打印到缓冲区中						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_dcdc_module_info(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%02d%s", 
			m_t_win_record.u_ordinal.u16_dcdc_module+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));

	win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].f32_out_volt);
	win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].f32_out_curr);
	win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].f32_curr_percent);
	win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].b_ctl_mode);
	win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].e_module_state);
}

/*************************************************************
函数名称: v_disp_dcac_module_info		           				
函数功能: UPS模块信息显示处理函数，将显示数据打印到缓冲区中						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_dcac_module_info(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%02d%s", 
			m_t_win_record.u_ordinal.u16_dcac_module+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));

	win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_volt);
	win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_curr);
	win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_freq);
	win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_inverter_volt);
	win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_bypass_input_volt);
	win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_active_power);
	win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_apparen_power);
	win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].f32_out_power_factor);
	win->t_item[9].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].e_module_state);
	win->t_item[10].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].b_alarm_state);
}

/*************************************************************
函数名称: v_disp_dcdc_branch_info		           				
函数功能: 通信屏支路信息显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_dcdc_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]), "%s", 
								s_disp_get_branch_name_by_id(8, branch+i));
								
		win->t_item[i*4+2].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[branch+i].u8_state);
		
		if (g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			m_t_win_record.u32_set_value = 0;
			win->t_item[i*4+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*4+3].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*4+4].pv_val = &(g_t_share_data.t_rt_data.t_dcdc_panel.u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*4+1;
}

/*************************************************************
函数名称: v_disp_dcac_branch_info		           				
函数功能: UPS屏支路信息显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_dcac_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]), "%s", 
								s_disp_get_branch_name_by_id(9, branch+i));
								
		win->t_item[i*4+2].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[branch+i].u8_state);
		
		if (g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			m_t_win_record.u32_set_value = 0;
			win->t_item[i*4+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*4+3].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*4+4].pv_val = &(g_t_share_data.t_rt_data.t_dcac_panel.u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*4+1;
}

/*************************************************************
函数名称: v_disp_ac_branch_info		           				
函数功能: 通信屏支路信息显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_ac_branch_info(void)
{
	U32_T i, num;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U32_T branch = m_t_win_record.u_ordinal.u16_feeder_branch;
	
	num = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num - branch;
				
	for (i=0; (i<num && i<MMI_FEEDER_BRANCH_CNT); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i*4+1]), sizeof(m_t_win_record.u8_special_name[i*4+1]), "%s", 
								s_disp_get_branch_name_by_id(10, branch+i));
								
		win->t_item[i*4+2].pv_val = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[branch+i].u8_state);
		
		if (g_t_share_data.t_rt_data.t_ac_panel.t_feeder[branch+i].u8_curr_state == 0)
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_ENUM;
			m_t_win_record.u32_set_value = 0;
			win->t_item[i*4+3].pv_val = &(m_t_win_record.u32_set_value);
		}
		else
		{
			win->t_item[i*4+3].u8_val_type = MMI_VAL_TYPE_F32_1P;
			win->t_item[i*4+3].pv_val = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[branch+i].f32_curr);
		}
		
		win->t_item[i*4+4].pv_val = &(g_t_share_data.t_rt_data.t_ac_panel.u8_sensor_state[branch+i]);
	}
	
	win->u8_item_cnt = i*4+1;
}

/*************************************************************
函数名称: v_disp_cell1_info		           				
函数功能: 一组单体电池信息显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_cell1_info(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num - m_t_win_record.u_ordinal.u16_cell;

	for (i=0; (i<MMI_CELL_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%03d:", m_t_win_record.u_ordinal.u16_cell+i+1);
		win->t_item[i+1].pv_val = &(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_cell_volt[m_t_win_record.u_ordinal.u16_cell+i]);
	}

	win->u8_item_cnt = i + 1;
}

/*************************************************************
函数名称: v_disp_cell2_info		           				
函数功能: 二组单体电池信息显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_cell2_info(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num - m_t_win_record.u_ordinal.u16_cell;

	for (i=0; (i<MMI_CELL_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%03d:", m_t_win_record.u_ordinal.u16_cell+i+1);
		win->t_item[i+1].pv_val = &(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_cell_volt[m_t_win_record.u_ordinal.u16_cell+i]);
	}

	win->u8_item_cnt = i + 1;
}

/*************************************************************
函数名称: v_disp_special_swt_info		           				
函数功能: 根据开关量的配置，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_special_swt_info(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt - m_t_win_record.u_ordinal.u16_swt;

	for (i=0; (i<MMI_SWT_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%s",
						s_disp_get_swt_name_by_index(i+m_t_win_record.u_ordinal.u16_swt));
		win->t_item[i+1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[i+m_t_win_record.u_ordinal.u16_swt]);
	}

	win->u8_item_cnt = i+1;
	
	if (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt > MMI_SWT_CNT)
		win->u8_icon_cnt = 2;
	else
		win->u8_icon_cnt = 0;
}

/*************************************************************
函数名称: v_disp_ec_swt_info		           				
函数功能: 根据电操开关量的配置，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_ec_swt_info(void)
{
	U16_T num, i, index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = u16_public_get_ctrl_swt_num() - m_t_win_record.u_ordinal.u16_swt;
	index = m_u16_swt_index;

	for (i=0; (i<MMI_SWT_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%s",
					s_disp_get_swt_string_pointer(index));
		win->t_item[i+1].pv_val = (g_t_swt_sheet[index].p_u8_swt_state);

		index++;
		while ( (!g_t_swt_sheet[index].u8_swt_valid) && 
		        (index < FACT_SWT_CTRL_MAX) )
		{
			index++;
		}
	}

	win->u8_item_cnt = i+1;
	
	if (u16_public_get_ctrl_swt_num() > MMI_SWT_CNT)
		win->u8_icon_cnt = 2;
	else
		win->u8_icon_cnt = 0;
}

/*************************************************************
函数名称: v_disp_rect_on_off		           				
函数功能: 整流模块开关机显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_rect_on_off(void)
{
	U16_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num - m_t_win_record.u_ordinal.u16_rect_on_off;

	for (i=0; (i<MMI_RECT_ON_OFF_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%02d%s",
					m_t_win_record.u_ordinal.u16_rect_on_off+i+1,
					s_disp_get_string_pointer(win->t_item[i+1].u16_name_index & MMI_STR_ID_INDEX_MASK));
		win->t_item[i+1].pv_val = &(g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl[m_t_win_record.u_ordinal.u16_rect_on_off+i]);
	}

	win->u8_item_cnt = i+1;
}

/*************************************************************
函数名称: v_disp_swt_on_off		           				
函数功能: 电操开关显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_swt_on_off(void)
{
	U16_T num, i, index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	num = u16_public_get_ctrl_swt_num() - m_t_win_record.u_ordinal.u16_swt_on_off;
	index = m_u16_swt_index;

	for (i=0; (i<MMI_SWT_ON_OFF_CNT && i<num); i++)
	{
		snprintf((char *)(m_t_win_record.u8_special_name[i+1]), sizeof(m_t_win_record.u8_special_name[i+1]), "%s",
					s_disp_get_swt_string_pointer(index));
		win->t_item[i+1].pv_val = (g_t_swt_sheet[index].p_u8_swt_ctrl);

		index++;
		while ( (!g_t_swt_sheet[index].u8_swt_valid) && 
		        (index < FACT_SWT_CTRL_MAX) )
		{
			index++;
		}
	}

	win->u8_item_cnt = i+1;
}

/*************************************************************
函数名称: v_disp_feeder_panel_module_num_set		           				
函数功能: 馈电屏馈线模块数量设置显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_feeder_panel_module_num_set(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
				m_u8_feeder_panel_ordianl+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
			
	win->t_item[1].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num);
	
	if (g_u8_product_type == PRODUCT_TYPE_SC22)
		m_t_win_record.t_curr_win.t_item[1].u32_val_max = 1;
}

/*************************************************************
函数名称: v_disp_feeder_panel_module_num_set2		           				
函数功能: 二段馈电屏馈线模块数量设置显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_feeder_panel_module_num_set2(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s", 
				m_u8_feeder_panel_ordianl+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
			
	win->t_item[1].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num);
	
	if (g_u8_product_type == PRODUCT_TYPE_SC22)
		m_t_win_record.t_curr_win.t_item[1].u32_val_max = 1;
}

/*************************************************************
函数名称: v_disp_acfeeder_panel_module_num_set		           				
函数功能: 馈电屏馈线模块数量设置显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_acfeeder_panel_module_num_set(void)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s", 
				s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK));
			
	win->t_item[1].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num);
	
	if (g_u8_product_type == PRODUCT_TYPE_SC22)
		m_t_win_record.t_curr_win.t_item[1].u32_val_max = 1;
}

/*************************************************************
函数名称: v_disp_feeder_panel_module_set		           				
函数功能: 馈电屏馈线模块设置显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_feeder_panel_module_set(void)
{
	U16_T panel_index, module_index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	panel_index = m_u8_feeder_panel_ordianl;
	module_index = m_t_win_record.u_ordinal.u16_feeder_module;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s%d%s", 
					panel_index+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
					module_index+1, s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
	snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
					s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK),
					MMI_FEEDER_PANEL_MODULE_ADDR+ panel_index * 4 + module_index);
			
	win->t_item[2].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_alarm_feeder_num);
	win->t_item[3].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].e_alarm_type);
	win->t_item[4].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_state_feeder_num);
	win->t_item[5].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].e_state_type);
	win->t_item[6].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_insu_feeder_num);
	win->t_item[7].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[panel_index].t_feeder_module[module_index].u8_curr_feeder_num);
}

/*************************************************************
函数名称: v_disp_feeder_panel_module_set2		           				
函数功能: 馈电屏馈线模块设置显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_feeder_panel_module_set2(void)
{
	U16_T panel_index, module_index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	panel_index = m_u8_feeder_panel_ordianl;
	module_index = m_t_win_record.u_ordinal.u16_feeder_module;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%d%s%d%s", 
					panel_index+1, s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
					module_index+1, s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
	snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
					s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK),
					MMI_FEEDER_PANEL_MODULE_ADDR+ (4+panel_index) * 4 + module_index);
			
	win->t_item[2].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_alarm_feeder_num);
	win->t_item[3].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].e_alarm_type);
	win->t_item[4].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_state_feeder_num);
	win->t_item[5].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].e_state_type);
	win->t_item[6].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_insu_feeder_num);
	win->t_item[7].pv_val = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+panel_index].t_feeder_module[module_index].u8_curr_feeder_num);
}

/*************************************************************
函数名称: v_disp_acfeeder_panel_module_set		           				
函数功能: 馈电屏馈线模块设置显示处理函数，将显示数据打印到缓冲区中，调用此函数前需要获取互斥量g_t_share_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_acfeeder_panel_module_set(void)
{
	U16_T  module_index;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	module_index = m_t_win_record.u_ordinal.u16_feeder_module;
	
	snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s%d%s", 
					s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
					module_index+1, s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
	snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
					s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK),
					MMI_AC_FEEDER_MODULE_ADDR + module_index);
			
	win->t_item[2].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].u8_alarm_feeder_num);
	win->t_item[3].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].e_alarm_type);
	win->t_item[4].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].u8_state_feeder_num);
	win->t_item[5].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].e_state_type);
	win->t_item[6].pv_val = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[module_index].u8_curr_feeder_num);
}

/*************************************************************
函数名称: v_disp_display_update		           				
函数功能: 显示页面更新，根据当前的页面ID刷新页面						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_display_update(void)
{
	MMI_ITEM_T *t_item;
	STR_T str;
	char buf[25];
	U32_T i, j;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);

	if ( (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW) ||
	     (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2) )            // 主界面刷新
	{
		if (u8_ctime_get_time(&(m_t_win_record.t_time)) == TIME_IRIGB)	      // 刷新主界面时间
			win->t_icon[0].u16_index = ICON_SIGNAL;
		else
			win->t_icon[0].u16_index = ICON_CLOCK;
		snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%4d-%d-%d %d:%02d:%02d",
					m_t_win_record.t_time.year, m_t_win_record.t_time.month, m_t_win_record.t_time.day,
					m_t_win_record.t_time.hour, m_t_win_record.t_time.min, m_t_win_record.t_time.sec);
		
		//根据是否有合控母，显示不同的电压名称
		if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
		{
			if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW)
			{
				win->t_item[8].u16_name_index = 5;
				win->u8_item_cnt = 10;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt);
			}
			else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2)
			{
				win->t_item[8].u16_name_index = 579;
				win->u8_item_cnt = 10;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_volt);
			}	
		}
		else
		{
			if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW)
			{
				win->t_item[8].u16_name_index = 6;
				win->u8_item_cnt = 9;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt);
			}
			else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2)
			{
				win->t_item[8].u16_name_index = 580;
				win->u8_item_cnt = 9;
				win->t_item[8].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_volt);
			}
		}
		
		
		//根据电池当前状态，设置电池条目的ID
		if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
		{
			if (g_t_share_data.t_rt_data.t_batt.e_state[0] == DIS)
				win->t_item[6].u16_name_index = 10; 
			else if ((g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0] < -1.0)
					|| (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] < -1.0))
				win->t_item[6].u16_name_index = 11;				
			else if (g_t_share_data.t_rt_data.t_batt.e_state[0] == EQU)
				win->t_item[6].u16_name_index = 9;	
			else
				win->t_item[6].u16_name_index = 8;
		}
		else
		{
			if (g_t_share_data.t_rt_data.t_batt.e_state[0] == DIS)
				win->t_item[6].u16_name_index = 440; 
			else if ((g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0] < -1.0)
					|| (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] < -1.0))
				win->t_item[6].u16_name_index = 441;				
			else if (g_t_share_data.t_rt_data.t_batt.e_state[0] == EQU)
				win->t_item[6].u16_name_index = 13;	
			else
				win->t_item[6].u16_name_index = 12;
		}

		//查询当前做障，根据故障条目的ID
		switch (u8_fault_get_fault_class())
		{
			case AC_FAULT_CLASS:                      //交流故障
				win->t_item[7].u16_name_index = 15;
				break;

			case DC_FAULT_CLASS:                      //直流母线故障
				win->t_item[7].u16_name_index = 16;
				break;

			case BATT_FAULT_CLASS:                    //电池故障
				win->t_item[7].u16_name_index = 17;
				break;

			case INSU_FAULT_CLASS:                    //绝缘故障
				win->t_item[7].u16_name_index = 18;
				break;

			case RECT_FAULT_CLASS:                    //充电模块故障
				win->t_item[7].u16_name_index = 19;
				break;

			case FUSE_FAULT_CLASS:                    //熔断器故障
				win->t_item[7].u16_name_index = 20;
				break;

			case DCAC_FAULT_CLASS:                    //UPS模块故障
				win->t_item[7].u16_name_index = 21;
				break;

			case FEEDER_FAULT_CLASS:                  //馈线支路故障
				win->t_item[7].u16_name_index = 22;
				break;
				
			case SYSTEM_NORMAL:                       //系统正常 
			default:
				win->t_item[7].u16_name_index = 14;
				break;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TOTAL_INFO)
	{
		//根据电池当前状态，设置最后一个条目的ID
		if (g_t_share_data.t_rt_data.t_batt.e_state[0] == DIS)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 99; 			
		else if (g_t_share_data.t_rt_data.t_batt.e_state[0] == EQU)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 98;	
		else
			win->t_item[win->u8_item_cnt-1].u16_name_index = 97;
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_GROUP_INFO)
	{
		//根据电池当前状态，设置最后一个条目的ID
		if (g_t_share_data.t_rt_data.t_batt.e_state[1] == DIS)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 99; 			
		else if (g_t_share_data.t_rt_data.t_batt.e_state[1] == EQU)
			win->t_item[win->u8_item_cnt-1].u16_name_index = 98;	
		else
			win->t_item[win->u8_item_cnt-1].u16_name_index = 97;
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP1_INFO)
	{
		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num; i++)  // 如果所有的电池巡检通信中断，则反显
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_comm_state[i] == 0)
			{
				break;
			}
		}

		if (i < g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num)
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP2_INFO)
	{
		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num; i++)  // 如果所有的电池巡检通信中断，则反显
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_comm_state[i] == 0)
			{
				break;
			}
		}

		if (i < g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num)
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS1_CELL_INFO)                // 单体电池电压界面刷新
	{
		U32_T ordianl, end_num, start_num = 0;

		for (j=0; j<win->u8_item_cnt-1; j++)                                          //先正常显示所有的条目
		{
			m_t_win_record.u8_item_inverse[j+1] = MMI_NORMAL_DISP;
		}

		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num; i++)  // 如果电池巡检通信中断，则它所检测的单体电池电压反显
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_comm_state[i] != 0)
			{
				end_num = start_num + g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[i];

				for (j=0; j<win->u8_item_cnt-1; j++)
				{
					ordianl = m_t_win_record.u_ordinal.u16_cell + j;
					if ((ordianl >= start_num) && (ordianl < end_num))
					{
						m_t_win_record.u8_item_inverse[j+1] = MMI_INVERSE_DISP;
					}
				}
			}

			start_num += g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_num[i];
		}

		for (i=0; i<win->u8_item_cnt-1; i++)                                   // 如果单体电压异常，则反显对应的条目
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_cell_state[m_t_win_record.u_ordinal.u16_cell+i] != 0)
			{
				m_t_win_record.u8_item_inverse[i+1] = MMI_INVERSE_DISP;
			}
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS2_CELL_INFO)                // 单体电池电压界面刷新
	{
		U32_T ordianl, end_num, start_num = 0;

		for (j=0; j<win->u8_item_cnt-1; j++)                                          //先正常显示所有的条目
		{
			m_t_win_record.u8_item_inverse[j+1] = MMI_NORMAL_DISP;
		}

		for (i=0; i<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num; i++)  // 如果电池巡检通信中断，则它所检测的单体电池电压反显
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_comm_state[i] != 0)
			{
				end_num = start_num + g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i];

				for (j=0; j<win->u8_item_cnt-1; j++)
				{
					ordianl = m_t_win_record.u_ordinal.u16_cell + j;
					if ((ordianl >= start_num) && (ordianl < end_num))
					{
						m_t_win_record.u8_item_inverse[j+1] = MMI_INVERSE_DISP;
					}
				}
			}

			start_num += g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_num[i];
		}

		for (i=0; i<win->u8_item_cnt-1; i++)                                   // 如果单体电压异常，则反显对应的条目
		{
			if (g_t_share_data.t_rt_data.t_batt.t_batt_group[1].u8_cell_state[m_t_win_record.u_ordinal.u16_cell+i] != 0)
			{
				m_t_win_record.u8_item_inverse[i+1] = MMI_INVERSE_DISP;
			}
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_INFO)
	{
		if (g_t_share_data.t_rt_data.t_dc_panel.t_rect[m_t_win_record.u_ordinal.u16_rect_info].u8_comm_state != 0)    // 整流模块通信中断，则反显模块相关的实时数据
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO)          // 馈出支路查询页面，如果馈线模块通信中断，则反显相关的数据
	{
		DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl]);
		
		v_disp_feeder_branch_info();

		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
		
		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch+1 >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch+1 < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO2)          // 馈出支路查询页面，如果馈线模块通信中断，则反显相关的数据
	{
		DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl]);
		
		v_disp_feeder_branch_info2();

		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[4+m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
		
		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch+1 >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch+1 < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_feeder_panel[4+m_u8_feeder_panel_ordianl].t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_INFO)
	{
		if (g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[m_t_win_record.u_ordinal.u16_dcdc_module].u8_comm_state != 0)    // DCDC模块通信中断，则反显模块相关的实时数据
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_INFO)
	{
		v_disp_dcdc_branch_info();

		if (g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[0].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_INFO)
	{
		if (g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[m_t_win_record.u_ordinal.u16_dcac_module].u8_comm_state != 0)    // DCDC模块通信中断，则反显模块相关的实时数据
		{
			m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[1] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[10] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_INFO)
	{
		v_disp_dcac_branch_info();

		if (g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[0].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
			
			m_t_win_record.u8_item_inverse[6] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
		}
	}
	else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_INFO)
	{
		AC_FEEDER_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed);

		v_disp_ac_branch_info();

		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[2] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[3] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[4] = MMI_NORMAL_DISP;
		}
		
		for (i=0; i<pt_feeder_panel->u8_feeder_module_num-1; i++)
		{
			if ((m_t_win_record.u_ordinal.u16_feeder_branch+1 >= pt_feeder_panel->u8_feeder_start_num[i])
				&& (m_t_win_record.u_ordinal.u16_feeder_branch+1 < pt_feeder_panel->u8_feeder_start_num[i+1]))
			{
				break;
			}
		}
		
		if (g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[i].u8_comm_state != 0)
		{
			m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
		}
		else
		{
			m_t_win_record.u8_item_inverse[7] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[8] = MMI_NORMAL_DISP;
			m_t_win_record.u8_item_inverse[9] = MMI_NORMAL_DISP;
		}
	}
	else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_CURR_FAULT_INFO)
			|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_NO_CURR_FAULT))
	{
		os_mut_release(g_mut_share_data);            //解锁，避免同时持有两个锁，造成死锁
		os_mut_wait(g_mut_fault_data, 0xFFFF);       //读写当前故障需要持有互斥量
		if (u8_fault_read_curr_fault_state() == FAULT_CHANGE)
		{
			v_disp_curr_falut_first_page();
			win = &(m_t_win_record.t_curr_win);
		}
		os_mut_release(g_mut_fault_data);            //解锁当前故障互斥量
		os_mut_wait(g_mut_share_data, 0xFFFF);       //加锁共享数据区互斥量
	}
	else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_HIS_FAULT_INFO)
			|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_NO_HIS_FAULT))
	{
		os_mut_release(g_mut_share_data);            //解锁，避免同时持有两个锁，造成死锁
		os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);  //读写历史故障需要持有互斥量
		if (u8_fault_read_his_fault_state() == FAULT_CHANGE)
		{
			v_disp_his_fault_first_page();
			win = &(m_t_win_record.t_curr_win);
		}
		os_mut_release(g_mut_fetch_his_fault);       //解锁历史故障互斥量
		os_mut_wait(g_mut_share_data, 0xFFFF);       //加锁共享数据区互斥量
	}
	else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RECORD)
			|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_NO_RECORD))
	{
		os_mut_release(g_mut_share_data);            //解锁，避免同时持有两个锁，造成死锁
		os_mut_wait(g_mut_log, 0xFFFF);              //读写事件记录需要持有互斥量
		if (u8_log_read_log_state() == LOG_CHANGE)
		{
			v_disp_record_first_page();
			win = &(m_t_win_record.t_curr_win);
		}
		os_mut_release(g_mut_log);                   //释放互斥量
		os_mut_wait(g_mut_share_data, 0xFFFF);       //加锁共享数据区互斥量
	}

	v_lcd_lcd_clear();
	v_lcd_lcd_draw_rectangle(0, 0, 160, 160, 1);                           // 画屏幕周的矩形
	if( (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW) ||
	    (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_WINDOW2) )              // 如果是主窗口，则多画4条横线
	{
		v_lcd_lcd_draw_hline(0, 15, 160, 1);
		v_lcd_lcd_draw_hline(0, 17, 160, 1);
		v_lcd_lcd_draw_hline(0, 128, 160, 1);
		v_lcd_lcd_draw_hline(0, 130, 160, 1);
	}

	for (i=0; i<win->u8_icon_cnt; i++)                                     // 画图标
	{
		v_lcd_lcd_draw_icon(win->t_icon[i].u8_x, win->t_icon[i].u8_y, win->t_icon[i].u16_index);
	}

	for (i=0; i<win->u8_item_cnt; i++)                                     // 循环画各个条目
	{
		t_item = &(win->t_item[i]);

		memset(buf, 0, sizeof(buf));
		if ((t_item->u16_name_index & MMI_STR_ID_TYPE_MASK) == MMI_STR_ID_NORMAL_NAME)
		{
			str = s_disp_get_string_pointer(t_item->u16_name_index);                   // 普通字符串，通过g_s_string获取字符串的内容
		}
		else
		{
			str = (char *)(m_t_win_record.u8_special_name[i]);             // 特殊字符串，通过m_t_win_record.u8_special_name获取内容
		}

		snprintf(buf, sizeof(buf), "%s", str);       //为防止数组越界，将memcpy改为snprintf
		//memcpy(buf, str, strlen(str));

		if ((t_item->u8_val_type==MMI_VAL_TYPE_NONE) &&                             // 反显名称的前提上条目没有数值
			(((m_t_win_record.u8_sel_index==i) && (m_t_win_record.u8_sel_index!=0)) // 选择条目不等于0，主要对应于菜单界面
				|| (m_t_win_record.u8_item_inverse[i]==MMI_INVERSE_DISP)))          // 条目反显，主要用于标题、异常条目的反显
		{
			v_lcd_lcd_draw_string(t_item->u8_name_x, t_item->u8_name_y, buf, 0, strlen(buf));
		}
		else
		{
			v_lcd_lcd_draw_string(t_item->u8_name_x, t_item->u8_name_y, buf, 0, 0);
		}
			
		
		if (t_item->u8_val_type != MMI_VAL_TYPE_NONE)
		{
			memset(buf, 0, sizeof(buf));

			/* 对于设置和非设置状态的数值显示分不同情况处理，主要原因是不能直接用浮点数进行设置，如果直接用浮点数进行设置，
			   由于精度方面的问题，会导致数据不准确，所以将浮点数扩大1000倍转换成整数处理，在显示的时候再除以1000转换成浮点数 */
			if (((m_t_win_record.u8_win_status==MMI_WIN_SET) && (i==m_t_win_record.u8_sel_index))
					|| ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD))
			{
				switch (t_item->u8_val_type)
				{
					case MMI_VAL_TYPE_ENUM:
						str = s_disp_get_string_pointer(t_item->u16_val_index+(U8_T)(m_t_win_record.u32_set_value));
						memcpy(buf, str, strlen(str));
						break;
	
					case MMI_VAL_TYPE_U8:
						snprintf(buf, sizeof(buf), "%d", (U8_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U8_2BIT:
						snprintf(buf, sizeof(buf), "%02d", (U8_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U8_3BIT:
						snprintf(buf, sizeof(buf), "%03d", m_t_win_record.u32_set_value); //设置显示3位的时候可能超出255，所以不能强制转换成U8_T
						break;


					case MMI_VAL_TYPE_U16:
						snprintf(buf, sizeof(buf), "%d", (U16_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U16_2BIT:
						snprintf(buf, sizeof(buf), "%02d", (U16_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U16_3BIT:
						snprintf(buf, sizeof(buf), "%03d", (U16_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U16_4BIT:
						snprintf(buf, sizeof(buf), "%04d", (U16_T)(m_t_win_record.u32_set_value));
						break;

				

					case MMI_VAL_TYPE_U32:
						snprintf(buf, sizeof(buf), "%d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_2BIT:
						snprintf(buf, sizeof(buf), "%02d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_3BIT:
						snprintf(buf, sizeof(buf), "%03d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_4BIT:
						snprintf(buf, sizeof(buf), "%04d", (U32_T)(m_t_win_record.u32_set_value));
						break;
					case MMI_VAL_TYPE_U32_5BIT:
						snprintf(buf, sizeof(buf), "%05d", (U32_T)(m_t_win_record.u32_set_value));
						break;


					case MMI_VAL_TYPE_F32_1P:
						snprintf(buf, sizeof(buf), "%.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_3W1P:
						snprintf(buf, sizeof(buf), "%03.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_4W1P:
						snprintf(buf, sizeof(buf), "%04.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_5W1P:
						snprintf(buf, sizeof(buf), "%05.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_6W1P:
						snprintf(buf, sizeof(buf), "%06.1f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;


					case MMI_VAL_TYPE_F32_2P:
						snprintf(buf, sizeof(buf), "%.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_4W2P:
						snprintf(buf, sizeof(buf), "%04.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_5W2P:
						snprintf(buf, sizeof(buf), "%05.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_6W2P:
						snprintf(buf, sizeof(buf), "%06.2f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;


					case MMI_VAL_TYPE_F32_3P:
						snprintf(buf, sizeof(buf), "%.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_5W3P:
						snprintf(buf, sizeof(buf), "%05.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_6W3P:
						snprintf(buf, sizeof(buf), "%06.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;
					case MMI_VAL_TYPE_F32_7W3P:
						snprintf(buf, sizeof(buf), "%07.3f", ((F32_T)m_t_win_record.u32_set_value)/1000.0);
						break;

					default:
						break;
				}  // end of switch (t_item->u8_val_type)
			}
			else
			{
				switch (t_item->u8_val_type)
				{
					case MMI_VAL_TYPE_ENUM:
						str = s_disp_get_string_pointer(t_item->u16_val_index+*((U8_T *)(t_item->pv_val)));
						memcpy(buf, str, strlen(str));
						break;
	
					case MMI_VAL_TYPE_U8:
						snprintf(buf, sizeof(buf), "%d", *((U8_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U8_2BIT:
						snprintf(buf, sizeof(buf), "%02d", *((U8_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U8_3BIT:
						snprintf(buf, sizeof(buf), "%03d", *((U8_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_U16:
						snprintf(buf, sizeof(buf), "%d", *((U16_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U16_2BIT:
						snprintf(buf, sizeof(buf), "%02d", *((U16_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U16_3BIT:
						snprintf(buf, sizeof(buf), "%03d", *((U16_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U16_4BIT:
						snprintf(buf, sizeof(buf), "%04d", *((U16_T *)(t_item->pv_val)));
						break;

				

					case MMI_VAL_TYPE_U32:
						snprintf(buf, sizeof(buf), "%d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_2BIT:
						snprintf(buf, sizeof(buf), "%02d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_3BIT:
						snprintf(buf, sizeof(buf), "%03d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_4BIT:
						snprintf(buf, sizeof(buf), "%04d", *((U32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_U32_5BIT:
						snprintf(buf, sizeof(buf), "%05d", *((U32_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_F32_1P:
						snprintf(buf, sizeof(buf), "%.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_3W1P:
						snprintf(buf, sizeof(buf), "%03.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_4W1P:
						snprintf(buf, sizeof(buf), "%04.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_5W1P:
						snprintf(buf, sizeof(buf), "%05.1f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_6W1P:
						snprintf(buf, sizeof(buf), "%06.1f", *((F32_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_F32_2P:
						snprintf(buf, sizeof(buf), "%.2f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_4W2P:
						snprintf(buf, sizeof(buf), "%04.2f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_5W2P:
						snprintf(buf, sizeof(buf), "%05.2f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_6W2P:
						snprintf(buf, sizeof(buf), "%06.2f", *((F32_T *)(t_item->pv_val)));
						break;


					case MMI_VAL_TYPE_F32_3P:
						snprintf(buf, sizeof(buf), "%.3f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_5W3P:
						snprintf(buf, sizeof(buf), "%05.3f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_6W3P:
						snprintf(buf, sizeof(buf), "%06.3f", *((F32_T *)(t_item->pv_val)));
						break;
					case MMI_VAL_TYPE_F32_7W3P:
						snprintf(buf, sizeof(buf), "%07.3f", *((F32_T *)(t_item->pv_val)));
						break;

					default:
						break;
				}  // end of switch (t_item->u8_val_type)
			}  /* enf of if (((m_t_win_record.u8_win_status==MMI_WIN_SET) && (i==m_t_win_record.u8_sel_index))
					          || ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD)) */

		
			if (((m_t_win_record.u8_win_status == MMI_WIN_SET) && (m_t_win_record.u8_sel_index==i)) //设置条目需闪烁显示当前设置的数据位
					|| ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD))
			{
				if (m_t_win_record.u8_set_blink == MMI_INVERSE_DISP)   // 反显当前设置的数据位
				{
					if (t_item->u8_val_type == MMI_VAL_TYPE_ENUM)      // 枚举类型数据全部反显
					{
						v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, strlen(buf));
					}
					else if ((t_item->u8_val_type & 0xF0) != 0x30)     // 整形数据反显当前设置的数据位
					{
						v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, m_t_win_record.u8_bit_index, 1);
					}
					else
					{
						if (                                           // 浮点数据反显时跳过小数点位
								(
									(m_t_win_record.u8_bit_index >= 1)
									&&
									(
										(t_item->u8_val_type == MMI_VAL_TYPE_F32_3W1P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_4W2P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_5W3P)
									)
								)
								||
								(
									(m_t_win_record.u8_bit_index >= 2)
									&&
									(
										(t_item->u8_val_type == MMI_VAL_TYPE_F32_4W1P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_5W2P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_6W3P)
									)
								)
								||
								(
									(m_t_win_record.u8_bit_index >= 3)
									&&
									(
										(t_item->u8_val_type == MMI_VAL_TYPE_F32_5W1P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_6W2P)
										|| (t_item->u8_val_type == MMI_VAL_TYPE_F32_7W3P)
									)
								)
								||
								(
									(m_t_win_record.u8_bit_index >= 4) && (t_item->u8_val_type == MMI_VAL_TYPE_F32_6W1P)
								)
							)
						{
							v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, m_t_win_record.u8_bit_index+1, 1);
						}
						else
						{
							v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, m_t_win_record.u8_bit_index, 1);
						}
					}
				}
				else
				{
					v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, 0);            // 不反显当前设置的数据位
				}  // end of if (m_t_win_record.u8_set_blink == MMI_INVERSE_DISP)
			}
			else if (((m_t_win_record.u8_sel_index==i)&&(m_t_win_record.u8_sel_index!=0))
						|| (m_t_win_record.u8_item_inverse[i] == MMI_INVERSE_DISP))

			{
				v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, strlen(buf));
			}
			else
			{
				v_lcd_lcd_draw_string(t_item->u8_val_x, t_item->u8_val_y, buf, 0, 0);
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
	
	if (m_e_lcd_direction == LCD_HORIZONTAL)
		v_lcd_lcd_flush(LCD_ROTATE_0);
	else
		v_lcd_lcd_flush(LCD_ROTATE_90);
}

/*************************************************************
函数名称: v_disp_cancel_key_handler		           				
函数功能: CANCEL按键处理函数，由于很多页的CANCEL键处理是一致的，所以提出来封装成一个函数						
输入参数: 无      		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_cancel_key_handler(void)
{
	U16_T id_back;
	U8_T index_back;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	id_back = win->u16_id_father;
	index_back = win->u8_sel_father;

	memset(&m_t_win_record, 0, sizeof(m_t_win_record));
	m_t_win_record.u16_curr_id = id_back;
	m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

	if ((id_back>>8) == MMI_WIN_TYPE_MENU)
	{
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u8_sel_index = index_back;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);		
		if (id_back == MMI_WIN_ID_SET_MENU)
			win->u8_item_cnt = m_u8_set_menu_item_cnt;
		else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
	    else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
		else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
		else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU2)
			win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
		else if (id_back == MMI_WIN_ID_DC_RUN_INFO_MENU)        // 运行信息查询菜单
		{
			if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
			{
				win->u8_item_cnt = 8;
			}
		}
		os_mut_release(g_mut_share_data);
	}
	else if ((id_back>>8) == MMI_WIN_TYPE_MAIN_WIN)
	{
		m_t_win_record.u8_sel_index = index_back;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		if (id_back == MMI_WIN_ID_MAIN_WINDOW)
		{
			if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
			{
				win->u16_id_next = MMI_WIN_ID_NULL;
				win->u8_icon_cnt = 1;
			}
		}
		os_mut_release(g_mut_share_data);
	}
}


/*************************************************************
函数名称: v_disp_menu_key_handler		           				
函数功能: 菜单页面按键处理函数						
输入参数: u16_key_val -- 按键值       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_menu_key_handler(U16_T u16_key_val)
{
	U8_T index_back;
	U32_T i, num = 0;
	U8_T feeder_flag = 0;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (u16_key_val == g_u16_key_enter)                                              // "ENTER"键处理
	{
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_MENU)  // 主菜单
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:                                          //交流系统信息查询
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 1;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_AC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 1;
						}
					}
					break;

				case 2:                                          // 直流运行信息查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_RUN_INFO_MENU;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
					{
						m_t_win_record.t_curr_win.u8_item_cnt = 8;
					}
					break;
					
				case 3:                                          //通信系统信息查询
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 3;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 3;
						}
					}
					break;
					
				case 4:                                          //UPS系统信息查询
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 4;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_MAIN_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 4;
						}
					}
					break;
					
				case 5:                                          // 当前故障查询
					os_mut_release(g_mut_share_data);            // 释放共享数据区互斥量，避免同时加持两个锁，造成死锁
					os_mut_wait(g_mut_fault_data, 0xFFFF);       // 读写当前故障需要持有互斥量
					v_disp_curr_falut_first_page();
					os_mut_release(g_mut_fault_data);            // 释放当前故障互斥量
					os_mut_wait(g_mut_share_data, 0xFFFF);       // 加持共享数据区互斥量
					break;


				case 6:                                          // 历史故障查询
					os_mut_release(g_mut_share_data);            // 释放共享数据区互斥量，避免同时加持两个锁，造成死锁
					os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);  // 读写历史故障需要持有互斥量
					v_disp_his_fault_first_page();
					os_mut_release(g_mut_fetch_his_fault);       // 释放历史故障互斥量
					os_mut_wait(g_mut_share_data, 0xFFFF);       // 加持共享数据区互斥量
					break;

				case 7:                                          // 掉电末复归故障查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));

					num  = u16_fault_read_no_resume_fault_cnt();
					if (num > 0)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_EXCEPTION_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

						if (num > MMI_EXCEPTION_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
							
						v_disp_no_resume_fault_page();
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_EXCEPTION;   // 无掉电末复归故障
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					}
					break;

				case 8:                                          //事件记录
					os_mut_release(g_mut_share_data);            //释放共享数据区互斥量，避免同时加持两个锁，造成死锁
					os_mut_wait(g_mut_log, 0xFFFF);              //读写事件记录需要持有互斥量
					u8_log_read_log_state();
					v_disp_record_first_page();
					os_mut_release(g_mut_log);                   //释放互斥量
					os_mut_wait(g_mut_share_data, 0xFFFF);       //加持共享数据区互斥量
					break;


				case 9:                                         // 参数设置
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_INPUT_PASSWORD;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
					
				case 10:                                         // 维护指南
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_GUIDELINE;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					break;


				case 11:                                        // 关于
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ABOUT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);
					
					//获取制造商、软硬件版本号等相关信息，填充record.u8_special_name
					for (i=0; i<8; i++)
					{
						snprintf((char *)(m_t_win_record.u8_special_name[i]), sizeof(m_t_win_record.u8_special_name[i]),
							"%s", s_disp_get_about_name_by_index(i));
					}

					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-4]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-4]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-4].u16_name_index & MMI_STR_ID_INDEX_MASK), s_disp_get_product_type_name());
					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-3]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-3]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-3].u16_name_index & MMI_STR_ID_INDEX_MASK), HW_VERSION);
					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-2]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-2]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-2].u16_name_index & MMI_STR_ID_INDEX_MASK), SW_VERSION);
					snprintf((char *)(m_t_win_record.u8_special_name[win->u8_item_cnt-1]), sizeof(m_t_win_record.u8_special_name[win->u8_item_cnt-1]),
							"%s:  %s", s_disp_get_string_pointer(win->t_item[win->u8_item_cnt-1].u16_name_index & MMI_STR_ID_INDEX_MASK), TOOL_VERSION);
					break;				

				default:
					break;
			}  //end of switch (m_t_win_record.u8_sel_index)
		}  //end of if (m_t_win_record.u16_curr_id == MMI_WIN_ID_MAIN_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_RUN_INFO_MENU)        // 运行信息查询菜单
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:                         // 交流信息显示页面
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;

					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase == AC_3_PHASE) // 三相
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_TRIPHASE_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							m_t_win_record.t_curr_win.u8_item_cnt = 5;
					}
					else                                                             // 单相
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_UNIPHASE_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[3] = MMI_INVERSE_DISP;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							m_t_win_record.t_curr_win.u8_item_cnt = 3;
					}
					break;

				case 2:                        // 电池状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)	            // 配置有电池组，则显示电池查询页面
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_TOTAL_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						win = &(m_t_win_record.t_curr_win);
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
						{
							if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
								win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
							else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
								win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
							else
								win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;

							win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
						}
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
						{
							if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
								win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
							else
								win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;

							if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
								win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
							else
								win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
						}
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;

				case 3:                        // 绝缘状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
					{
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num > 0)
						{
							feeder_flag = 1;
							break;
						}
					}
					
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_PB_CB_INSU_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
							
						win = &(m_t_win_record.t_curr_win);
						if (feeder_flag == 1)
						{
							win->u8_item_cnt = 8;     // 配置有馈线模块，则显示绝缘对地电压和对地电阻
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_pos_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_volt);
							win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_to_gnd_ac_volt);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);	
							win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
						else
						{
							win->u8_item_cnt = 8;//4;     //末配置馈线模块，则只显示绝缘对地电压
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_cb_pos_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt);
							//以下为2014.10.28新增显示
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);	
							win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_BUS_INSU_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
							
						win = &(m_t_win_record.t_curr_win);
						if (feeder_flag == 1)
						{
							win->u8_item_cnt = 7;     // 配置有馈线模块，则显示绝缘对地电压和对地电阻
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_to_gnd_ac_volt);
							win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
						else
						{
							win->u8_item_cnt = 7;//3;     //末配置馈线模块，则只显示绝缘对地电压
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_cb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt);
							//以下为2014.10.28新增显示
							win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
					}
					break;

				case 4:                        // 整流模块信息
					num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_RECT_INFO;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);

					if (num > 1)
					{
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_RECT_INFO;               // 有前一页
						win->u16_id_next = MMI_WIN_ID_RECT_INFO;               // 有下一页
					}
					else
					{
						win->u8_icon_cnt = 0;
						win->u16_id_prev = MMI_WIN_ID_NULL;                    // 没有前一页
						win->u16_id_next = MMI_WIN_ID_NULL;                    // 没有下一页
					}
					
					v_disp_rect_info();
					break;
					
				case 5:                        // 开关状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt > 0)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_SWITCH_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						v_disp_special_swt_info();
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 5;
					}
					break;
				
				case 6:                        // 一段馈线柜信息查询
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 6;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_RUN_INFO_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 6;
						}
					}
					break;

				case 7:                        // 电操开关状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_u16_swt_index = 0;
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num > 0)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_ECSWT_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						v_disp_ec_swt_info();
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 7;
					}
					break;

				case 8:                        // 二段绝缘状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i++)
					{
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num > 0)
						{
							feeder_flag = 1;
							break;
						}
					}
					
					if (feeder_flag == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_PB_CB_INSU_INFO2;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
								
							win = &(m_t_win_record.t_curr_win);
						
							win->u8_item_cnt = 8;     // 配置有馈线模块，则显示绝缘对地电压和对地电阻
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_pos_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_neg_to_gnd_volt);
							win->t_item[4].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_to_gnd_ac_volt);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_pos_to_gnd_res);	
							win->t_item[7].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_neg_to_gnd_res);
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_BUS_INSU_INFO;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_item_inverse[4] = MMI_INVERSE_DISP;
								
							win = &(m_t_win_record.t_curr_win);
							win->u8_item_cnt = 7;     // 配置有馈线模块，则显示绝缘对地电压和对地电阻
							win->t_item[1].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_pos_to_gnd_volt);
							win->t_item[2].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_volt);
							win->t_item[3].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_to_gnd_ac_volt);
							win->t_item[5].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res);
							win->t_item[6].pv_val = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res);
						}
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 8;
					}
					break;

				case 9:                        // 二段馈线柜信息查询
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 9;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_RUN_INFO_MENU2;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_RUN_INFO_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 9;
						}
					}
					break;

				default:
					break;
			}  //end of switch (m_t_win_record.u8_sel_index)
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SET_MENU)
		{	
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    // 交流系统配置
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 1;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_MODULE_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;

						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
						}
						else
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
						}
					}
					break;

				case 2:    // 直流系统配置
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_SET_MENU;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					break;

				case 3:    // 通信模块参数配置
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					else
					{	
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_MODULE_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
						
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
						}
						else
						{	
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
						}
					}
					break;

				case 4:    // UPS模块参数配置
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 3;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_MODULE_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
						
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
						}
						else
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
						}
					}
					break;

				case 5:    // 告警设定
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_RELAY_OUT_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 2;
					break;

				case 6:    // 后台通讯
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_BACKSTAGE_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u8_item_cnt = 5;
					break;

				case 7:    // 时间密码
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_RTC_PASSWORD_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					u8_ctime_get_time(&m_t_win_record.t_time);
					m_t_win_record.t_time.year -= 2000;
					m_t_win_record.u8_sel_index = 1;
					break;
					
				case 8:    // 参数恢复
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_CLEAR;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;

				case 9:    // 采样校准
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_ADJUST;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						win->u8_item_cnt = 4;
						win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
					}

					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu*10))/10.0;
					break;
					
				case 10:
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DISPLAY;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 11:
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_MANUAL_LIMIT_CURR_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				default:
					break;
			}  //end of switch (m_t_win_record.u8_sel_index)
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SET_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
		{
			index_back = m_t_win_record.u8_sel_index;
			
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_branch_num > 0)
			{
				m_u8_feeder_panel_ordianl = index_back - 1;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_FEEDER_INFO;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
				
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				else
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
				v_disp_feeder_branch_info();	
			}
			else
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_FEEDER_RUN_INFO_MENU;
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
		{
			index_back = m_t_win_record.u8_sel_index;
			
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_branch_num > 0)
			{
				m_u8_feeder_panel_ordianl = index_back - 1;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_FEEDER_INFO2;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
				
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				else
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
				v_disp_feeder_branch_info2();	
			}
			else
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_FEEDER_RUN_INFO_MENU2;
				m_t_win_record.t_curr_win.u8_sel_father = index_back;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_RUN_INFO_MENU)
		{	
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    //DC/DC模块信息
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_INFO;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

					if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 1)
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_DCDC_INFO;               // 有前一页
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_DCDC_INFO;               // 有下一页
					}
					else
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 0;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // 没有前一页
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // 没有下一页
					}
					
					v_disp_dcdc_module_info();
					break;
					
				case 2:    //通信开关状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					
					if ((g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
						&& (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > 0))
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_FEEDER_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
						v_disp_dcdc_branch_info();	
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DCDC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_RUN_INFO_MENU)
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    //UPS模块信息查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_INFO;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

					if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num > 1)
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_DCAC_INFO;               // 有前一页
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_DCAC_INFO;               // 有下一页
					}
					else
					{
						m_t_win_record.t_curr_win.u8_icon_cnt = 0;
						m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // 没有前一页
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // 没有下一页
					}
					
					v_disp_dcac_module_info();
					break;
					
				case 2:    //UPS系统开关状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					
					if ((g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
						&& (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > 0))
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_FEEDER_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
						v_disp_dcac_branch_info();	
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DCAC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_AC_RUN_INFO_MENU)
		{	
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:    //交流系统母线信息
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[6] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_AC_RUN_INFO; 					
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];					
					break;
					
				case 2:    //交流开关状态查询
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					
					if ((g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
						&& (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > 0))
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_FEEDER_INFO;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
						else
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					
						v_disp_ac_branch_info();	
					}
					else
					{
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_ACS_AC_RUN_INFO_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_AC_RUN_INFO_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SET_MENU)
		{
			U16_T u16_ctrl_swt_num;
			U8_T  u8_rc10_num;
				
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:     //直流系统配置
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_SYSTEM_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u8_item_cnt = 5;
					else if (g_u8_product_type == PRODUCT_TYPE_SC22)
						win->t_item[5].u32_val_max = 1;

					if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
					{
						win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
						win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
					}
					else
					{
						win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
						win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
					}
					
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
						win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
						win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
					break;
					
				case 2:    //馈线柜配置
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 2;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_SET_MENU;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 2;
						}
					}
					break;
					
				case 3:   //门限值设置
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_THR_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					break;
					
				case 4:   //电池充放电参数设置
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_CHARGE_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					
					//均浮充电压的上限为电池过压值，下限为电池欠压值
					win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
					win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
					win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					
					//充电限流点的上限为充电过流点
					win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
					break;
					
				case 5:   //系统控制
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)     //配置有电池组，显示系统控制页面
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_SYSTEM_CTL_SET;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;

						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						else
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
					}
					else            //末配置电池组，显示模块开关机控制页面
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_RECT_ON_OFF;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;

						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num > MMI_RECT_ON_OFF_CNT)
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						}
						else
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
						}
						
						v_disp_rect_on_off();
					}
					break;

				case 6:   //电操开关控制
					u16_ctrl_swt_num = u16_public_get_ctrl_swt_num();
					u8_rc10_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num;					
					if ((u16_ctrl_swt_num > 0) && (u8_rc10_num > 0))     //配置有RC10，显示电操开关控制页面
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = MMI_WIN_ID_SWT_ON_OFF;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						m_u16_swt_index = 0;

						if (u16_ctrl_swt_num > MMI_SWT_ON_OFF_CNT)
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 2;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_SWT_ON_OFF;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_SWT_ON_OFF;
						}
						else
						{
							m_t_win_record.t_curr_win.u8_icon_cnt = 0;
							m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;
							m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
						}
						
						v_disp_swt_on_off();
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 6;
					}
					break;

				case 7:    //二段馈线柜配置
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_FUNCTION;        // 提示没有配置
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
						m_t_win_record.t_curr_win.u8_sel_father = 7;
					}
					else
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num > 0)
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_SET_MENU2;
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
							m_t_win_record.u8_sel_index = 1;
							m_t_win_record.t_curr_win.u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
						}
						else
						{
							m_t_win_record.u16_curr_id = MMI_WIN_ID_NO_CONFIG;        // 提示没有配置馈线屏
							m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
							m_t_win_record.t_curr_win.u16_id_father = MMI_WIN_ID_DC_SET_MENU;
							m_t_win_record.t_curr_win.u8_sel_father = 7;
						}
					}
					break;
					
				default:
					break;
			}
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SET_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU)
		{
			index_back = m_t_win_record.u8_sel_index;
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.t_curr_win.u8_sel_father = index_back;
			m_u8_feeder_panel_ordianl = index_back - 1;
			m_t_win_record.u8_sel_index = 1;

			if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_module_num > 0)
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有前一页
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有下一页
			}
			else
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 0;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // 有前一页
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // 有下一页
			}
			
			v_disp_feeder_panel_module_num_set();
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU2)
		{
			index_back = m_t_win_record.u8_sel_index;
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.t_curr_win.u8_sel_father = index_back;
			m_u8_feeder_panel_ordianl = index_back - 1;
			m_t_win_record.u8_sel_index = 1;

			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_module_num > 0)
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 2;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有前一页
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有下一页
			}
			else
			{
				m_t_win_record.t_curr_win.u8_icon_cnt = 0;
				m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // 有前一页
				m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // 有下一页
			}
			
			v_disp_feeder_panel_module_num_set2();
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_SET_MENU2)
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_GUIDELINE)
		{
			switch (m_t_win_record.u8_sel_index)
			{
				case 1:     //开通
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_START;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 2:     //通讯异常报警
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_COMM_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 3:     //过欠压报警
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_VOLT_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 4:     //绝缘报警
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_INSU_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 5:     //后台通讯异常
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_BACKSTAGE_FAULT;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 6:     //联系厂家
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_CONTACT_VENDER;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				case 7:     //关于设备返修
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_REPAIR;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					break;
					
				default:
					break;
			}			
		}  //end of else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_GUIDELINE)
	}

	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u8_sel_index > 1)
		{
			m_t_win_record.u8_sel_index--;
		}
		else
		{
			m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
		}
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (m_t_win_record.u8_sel_index < win->u8_item_cnt-1)
		{
			m_t_win_record.u8_sel_index++;
		}
		else
		{
			m_t_win_record.u8_sel_index = 1;
		}
	}
	
	os_mut_release(g_mut_share_data);
}


/*************************************************************
函数名称: v_disp_run_info_key_handler		           				
函数功能: 运行信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_run_info_key_handler(U16_T u16_key_val)
{
	U16_T id_back, id_old;
	U8_T index_back;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (u16_key_val == g_u16_key_cancel)
	{
		id_old = m_t_win_record.u16_curr_id;
		id_back = win->u16_id_father;
		index_back = win->u8_sel_father;

		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = id_back;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;

		if ((id_back>>8) == MMI_WIN_TYPE_MENU)                  //如果返回到菜单页面，选中相应的条目
		{
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_sel_index = index_back;
			
			if (id_back == MMI_WIN_ID_SET_MENU)
				win->u8_item_cnt = m_u8_set_menu_item_cnt;
			else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_FEEDER_RUN_INFO_MENU2)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_FEEDER_SET_MENU2)
				win->u8_item_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num + 1;
			else if (id_back == MMI_WIN_ID_DC_RUN_INFO_MENU)        // 运行信息查询菜单
			{
				if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
				{
					win->u8_item_cnt = 8;
				}
			}
		}
		else if ((id_back == MMI_WIN_ID_AC_ADJUST)
			|| (id_back == MMI_WIN_ID_DC_VOLT_ADJUST)
			|| (id_back == MMI_WIN_ID_LOAD_CURR_ADJUST)
			|| (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)
			|| (id_back == MMI_WIN_ID_BATT2_CURR_ADJUST))          //当前页面是校准成功失败提示页面，返回校准页面
		{
			if (id_back != MMI_WIN_ID_AC_ADJUST)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
			}
			
			if (g_u8_product_type == PRODUCT_TYPE_SC12)
			{
				if (id_back == MMI_WIN_ID_AC_ADJUST)
				{
					win->u8_item_cnt = 4;
					win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
				}
				else if (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)
				{
					win->u16_id_next = MMI_WIN_ID_AC_ADJUST;
				}
			}
			
			m_t_win_record.u8_sel_index = index_back;
			if (id_old == MMI_WIN_ID_PARAM_OUT_RANGE)
			{
				m_t_win_record.u8_win_status = MMI_WIN_SET;
				m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*10)) * 100;
				m_t_win_record.pv_back_item_val = win->t_item[index_back].pv_val;     //保存数据指针的值
				win->t_item[index_back].pv_val = &(m_t_win_record.u32_set_value);     //数据指针指向临时的数据区
			}
		}
		else if ((id_back >>8) == MMI_WIN_TYPE_SET)             //当前页面是设置数据超出范围提示页面，返回到设置页面
		{
			m_t_win_record.u8_sel_index = index_back;
			m_t_win_record.u8_win_status = MMI_WIN_SET;

			if (id_back == MMI_WIN_ID_RTC_PASSWORD_SET)
			{ 
				u8_ctime_get_time(&m_t_win_record.t_time);
				m_t_win_record.t_time.year -= 2000;
			}
			else if (id_back == MMI_WIN_ID_DC_SYSTEM_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->u8_item_cnt = 5;
				else if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[5].u32_val_max = 1;
					
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
				}
				else
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
				}
				
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
				else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
				else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
					win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
			}
			else if (id_back == MMI_WIN_ID_BATT_PARAM_SET)
			{
				if (g_u8_product_type != PRODUCT_TYPE_SC32)
					win->t_item[1].u32_val_max = 1;
			}
			if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
			{
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
			}
			else if (id_back == MMI_WIN_ID_DC_PARAM_SET)
			{
				m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
				//m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
			}
			else if (id_back == MMI_WIN_ID_RECT_PARAM_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->t_item[1].u32_val_max = 3;
				else if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[1].u32_val_max = 8;
					
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt;
				}
				else
				{
					win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt / 2;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt / 2;
				}
				
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
					win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;
			}
			else if (id_back == MMI_WIN_ID_BATT_METER1_SET)
			{
				v_disp_batt1_data_init();
			}
			else if (id_back == MMI_WIN_ID_BATT_METER2_SET)
			{
				v_disp_batt2_data_init();
			}
			else if (id_back == MMI_WIN_ID_DC_THR_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt;
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt;
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt ;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt ;
					win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt;
					win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt;
					win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt;
					win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt;
				}
				else
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt / 2;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt / 2;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt / 2;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt / 2;
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt / 2;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt / 2;
					win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt / 2;
					win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt / 2;
					win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt / 2;
					win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt / 2;
					win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt / 2;
					win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt / 2;
				}
			}
			else if (id_back == MMI_WIN_ID_BATT_THR_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit;
				}
				else
				{
					win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit / 2;
					win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit / 2;
					win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit / 2;
					win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit / 2;
				}
			}
			else if (id_back == MMI_WIN_ID_INSU_THR_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->u8_item_cnt = 3;
				else
					win->u8_item_cnt = 4;
			}
			else if (id_back == MMI_WIN_ID_BATT_CHARGE_SET)
			{
				//均浮充电压的上限为电池过压值，下限为电池欠压值
				win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
				win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
				win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
				win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
				
				//充电限流点的上限为充电过流点
				win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
			}
			else if (id_back == MMI_WIN_ID_BATT_TO_EQU_SET)  //电池转均充电流的最大值应该是电池的充电限流点
			{
				win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * 1000);
			}
			else if (id_back == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
				{
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt;
				}
				else
				{
					win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt / 2;
					win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt / 2;
				}
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)
			{
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;

				if (g_t_share_data.t_sys_cfg.t_feeder_panel[index_back-1].u8_feeder_module_num > 0)
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有前一页
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有下一页
				}
				else
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // 有前一页
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // 有下一页
				}
			
				v_disp_feeder_panel_module_num_set();
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
				m_t_win_record.u_ordinal.u16_feeder_module = m_u16_feeder_module_ordianl;
				
				v_disp_feeder_panel_module_set();
					
				win->u8_icon_cnt = 2;
				if (m_u16_feeder_module_ordianl > 0)
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
				else
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;	
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)
			{
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;

				if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+index_back-1].u8_feeder_module_num > 0)
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 2;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有前一页
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有下一页
				}
				else
				{
					m_t_win_record.t_curr_win.u8_icon_cnt = 0;
					m_t_win_record.t_curr_win.u16_id_prev = MMI_WIN_ID_NULL;                    // 有前一页
					m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;                    // 有下一页
				}
			
				v_disp_feeder_panel_module_num_set2();
			}
			else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
				m_t_win_record.u_ordinal.u16_feeder_module = m_u16_feeder_module_ordianl;
				
				v_disp_feeder_panel_module_set2();
					
				win->u8_icon_cnt = 2;
				if (m_u16_feeder_module_ordianl > 0)
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
				else
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
				else
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;	
			}
			else if (id_back == MMI_WIN_ID_DCDC_MODULE_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[1].u32_val_max = 4;
					
				if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
					win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
			}
			else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
					win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
				else
					win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;
			}
			else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				
				snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
						s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
						s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
				snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
						s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCDC_FEEDER_MODULE_ADDR);
			}
			else if (id_back == MMI_WIN_ID_DCAC_MODULE_SET)
			{
				if (g_u8_product_type == PRODUCT_TYPE_SC22)
					win->t_item[1].u32_val_max = 4;
				
				if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
					win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
			}
			else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
			{
				if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
					win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
				else
					win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;
			}
			else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET)
			{
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				
				snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
						s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
						s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
				snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
						s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCAC_FEEDER_MODULE_ADDR);
			}

			else if (id_back == MMI_WIN_ID_AC_MODULE_SET)
			{			
				if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
			}
			else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
			{
					win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;
			}
			else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u16_curr_id = id_back;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

				m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num - 1;
				v_disp_acfeeder_panel_module_set();

				m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
				win->u8_icon_cnt = 2;
				win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
				if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 1)
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
				else
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
			}

			if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0)
			{
				m_t_win_record.u32_set_value = *((U8_T *)(win->t_item[index_back].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x10)
			{
				m_t_win_record.u32_set_value = *((U16_T *)(win->t_item[index_back].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x20)
			{
				m_t_win_record.u32_set_value = *((U32_T *)(win->t_item[index_back].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x30)
			{
				//把浮点数乘以1000转换为整数处理
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W1P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*10)) * 100;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W2P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*100)) * 10;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_7W3P))
				{
					m_t_win_record.u32_set_value = (U32_T)(*((F32_T *)(win->t_item[index_back].pv_val))*1000);
				}
			}

			m_t_win_record.pv_back_item_val = win->t_item[index_back].pv_val;     //保存数据指针的值
			win->t_item[index_back].pv_val = &(m_t_win_record.u32_set_value);     //数据指针指向临时的数据区
		}
	}  //end of if (u16_key_val == g_u16_key_cancel)

	else if (u16_key_val == g_u16_key_up)
	{
		id_back = win->u16_id_prev;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			
			switch (id_back)
			{
				case MMI_WIN_ID_BATT_TOTAL_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;

						win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;

						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					break;
					
				case MMI_WIN_ID_BATT_GROUP_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{						
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP2_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{				
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}					
					break;

				case MMI_WIN_ID_BMS_GROUP1_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS_GROUP2_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
						win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS1_CELL_INFO:               // 一组单体电池电压界面，多页界面
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u_ordinal.u16_cell = ((g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num - 1)
														/ MMI_CELL_CNT) * MMI_CELL_CNT; 

					v_disp_cell1_info();
					break;
					
				case MMI_WIN_ID_BMS2_CELL_INFO:               // 二组单体电池电压界面，多页界面
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u_ordinal.u16_cell = ((g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num - 1)
														/ MMI_CELL_CNT) * MMI_CELL_CNT; 

					v_disp_cell2_info();
					break;

				default:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP1_INFO)
					{
						m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					}
					
					break;
			}  //end of switch (id_back>>8)
		}  //end of if (id_back != MMI_WIN_ID_NULL)
	}  //end of else if (u16_key_val == g_u16_key_up)

	else if (u16_key_val == g_u16_key_down)
	{
		id_back = win->u16_id_next;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			
			switch (id_back)
			{
				case MMI_WIN_ID_BATT_TOTAL_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					win = &(m_t_win_record.t_curr_win);
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;

						win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
						else
							win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;

						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}

					break;
					
				case MMI_WIN_ID_BATT_GROUP_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
					{						
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP2_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
					{							
						if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
							win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_TOTAL_INFO;
					}
					break;

				case MMI_WIN_ID_BMS_GROUP1_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS_GROUP2_INFO:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						
					if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
						win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
					else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
						win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
					else
						win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
					break;
					
				case MMI_WIN_ID_BMS1_CELL_INFO:               // 一组单体电池电压界面，多页界面
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					v_disp_cell1_info();
					break;
					
				case MMI_WIN_ID_BMS2_CELL_INFO:               // 一组单体电池电压界面，多页界面
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					v_disp_cell2_info();
					break;

				default:
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BMS_GROUP1_INFO)
					{
						m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
					}
					
					break;
			}  //end of switch (id_back>>8)
		}
	}  //end of else if (u16_key_val == g_u16_key_down)
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_cell1_volt_key_handler		           				
函数功能: 一组单体电池电压页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_cell1_volt_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_cell_total_num;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if ((m_t_win_record.u_ordinal.u16_cell+MMI_CELL_CNT) < num)
		{
			m_t_win_record.u_ordinal.u16_cell += MMI_CELL_CNT;
			v_disp_cell1_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			if ((g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
				&& (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num > 0))
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_BMS_GROUP2_INFO;
				m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;

				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
			}
			else
			{
				m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_TOTAL_INFO;

				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
				{
					win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
				}
				else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
				{
					win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
				}
			}
			
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_cell >= MMI_CELL_CNT)
		{
			m_t_win_record.u_ordinal.u16_cell -= MMI_CELL_CNT;
			v_disp_cell1_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BMS_GROUP1_INFO;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
			if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
			{
				win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
			}
			else
			{
				win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_cell2_volt_key_handler		           				
函数功能: 二组单体电池电压页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_cell2_volt_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_cell_total_num;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if ((m_t_win_record.u_ordinal.u16_cell+MMI_CELL_CNT) < num)
		{
			m_t_win_record.u_ordinal.u16_cell += MMI_CELL_CNT;
			v_disp_cell2_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_TOTAL_INFO;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			
			win->u16_id_prev = MMI_WIN_ID_BMS2_CELL_INFO;
			if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
				win->u16_id_next = MMI_WIN_ID_BATT_GROUP_INFO;
			else
				win->u16_id_next = MMI_WIN_ID_BMS_GROUP1_INFO;
				
		}
	}


	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_cell >= MMI_CELL_CNT)
		{
			m_t_win_record.u_ordinal.u16_cell -= MMI_CELL_CNT;
			v_disp_cell2_info();
		}
		else
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BMS_GROUP2_INFO;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[2] = MMI_INVERSE_DISP;
			m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
			
			if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num > 0)
				win->u16_id_prev = MMI_WIN_ID_BMS1_CELL_INFO;
			else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
				win->u16_id_prev = MMI_WIN_ID_BATT_GROUP_INFO;
			else
				win->u16_id_prev = MMI_WIN_ID_BATT_TOTAL_INFO;
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_rect_info_key_handler		           				
函数功能: 整流模块信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static v_disp_rect_info_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
	os_mut_release(g_mut_share_data);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		m_t_win_record.u_ordinal.u16_rect_info += 1;
		m_t_win_record.u_ordinal.u16_rect_info %= num;
		v_disp_rect_info();	
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_rect_info > 0)
		{
			m_t_win_record.u_ordinal.u16_rect_info -= 1;
		}
		else
		{
			m_t_win_record.u_ordinal.u16_rect_info = num - 1;
		}

		v_disp_rect_info();
	}
}

/*************************************************************
函数名称: v_disp_swt_info_key_handler		           				
函数功能: 开关信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_swt_info_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (num > MMI_SWT_CNT)
		{
			if ((m_t_win_record.u_ordinal.u16_swt+MMI_SWT_CNT) < num)
				m_t_win_record.u_ordinal.u16_swt += MMI_SWT_CNT;
			else
				m_t_win_record.u_ordinal.u16_swt = 0;
	
			v_disp_special_swt_info();
		}
	}


	else if (u16_key_val == g_u16_key_up)
	{
		if (num > MMI_SWT_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_swt >= MMI_SWT_CNT)
				m_t_win_record.u_ordinal.u16_swt -= MMI_SWT_CNT;
			else
				m_t_win_record.u_ordinal.u16_swt = ((num-1) / MMI_SWT_CNT) * MMI_SWT_CNT;

			v_disp_special_swt_info();
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_ecswt_info_key_handler		           				
函数功能: 电操开关信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_ecswt_info_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = u16_public_get_ctrl_swt_num();

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (num > MMI_SWT_CNT)
		{
			if ((m_t_win_record.u_ordinal.u16_swt + MMI_SWT_CNT) < num)
			{
				m_t_win_record.u_ordinal.u16_swt += MMI_SWT_CNT;
				u16_public_next_mv_swt_index(&m_u16_swt_index, MMI_SWT_CNT);
			}
			else
			{
				m_t_win_record.u_ordinal.u16_swt = 0;
				m_u16_swt_index = u16_public_get_first_swt_index();
			}			
			v_disp_ec_swt_info();
		}
	}


	else if (u16_key_val == g_u16_key_up)
	{
		if (num > MMI_SWT_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_swt >= MMI_SWT_CNT)
			{
				m_t_win_record.u_ordinal.u16_swt -= MMI_SWT_CNT;
				u16_public_pre_mv_swt_index(&m_u16_swt_index, MMI_SWT_CNT);
			}
			else
			{
				m_t_win_record.u_ordinal.u16_swt = ((num-1) / MMI_SWT_CNT) * MMI_SWT_CNT;
				m_u16_swt_index = u16_public_get_swt_index_from_no(m_t_win_record.u_ordinal.u16_swt + 1);
			}

			v_disp_ec_swt_info();
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_curr_fault_key_handler		           				
函数功能: 当前故障作息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_curr_fault_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		os_mut_wait(g_mut_fault_data, 0xFFFF);       //读写当前故障需要持有互斥量

		if (u8_fault_read_curr_fault_state() == FAULT_CHANGE)
		{
			v_disp_curr_falut_first_page();
		}
		else
		{
			num  = u16_fault_read_curr_fault_cnt();
			
			if (num > MMI_CURR_FAULT_CNT)
			{
				m_t_win_record.u_ordinal.u16_curr_fault += MMI_CURR_FAULT_CNT;
				if (m_t_win_record.u_ordinal.u16_curr_fault >= num)
					m_t_win_record.u_ordinal.u16_curr_fault = 0;

				v_disp_curr_falut_page();
			}
		}

		os_mut_release(g_mut_fault_data);
	}

	else if (u16_key_val == g_u16_key_up)
	{
		os_mut_wait(g_mut_fault_data, 0xFFFF);       //读写当前故障需要持有互斥量

		if (u8_fault_read_curr_fault_state() == FAULT_CHANGE)
		{
			v_disp_curr_falut_first_page();
		}
		else
		{
			num  = u16_fault_read_curr_fault_cnt();
			if (num > MMI_CURR_FAULT_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_curr_fault >= MMI_CURR_FAULT_CNT)
				{
					m_t_win_record.u_ordinal.u16_curr_fault -= MMI_CURR_FAULT_CNT;
				}
				else
				{
					m_t_win_record.u_ordinal.u16_curr_fault = ((num-1) / MMI_CURR_FAULT_CNT) * MMI_CURR_FAULT_CNT;
				}

				v_disp_curr_falut_page();
			}
		}
		
		os_mut_release(g_mut_fault_data);
	}
}


/*************************************************************
函数名称: v_disp_his_fault_key_handler		           				
函数功能: 历史故障作息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_his_fault_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);                 //读写历史故障需要持有互斥量

		if (u8_fault_read_his_fault_state() == FAULT_CHANGE)
		{
			v_disp_his_fault_first_page();
		}
		else
		{
			num  = u16_fault_read_his_fault_cnt();
			if (num > MMI_HIS_FAULT_CNT)
			{
				m_t_win_record.u_ordinal.u16_his_fault += MMI_HIS_FAULT_CNT;
				if (m_t_win_record.u_ordinal.u16_his_fault >= num)
					m_t_win_record.u_ordinal.u16_his_fault = 0;
				
				v_disp_his_falut_page();
			}
		}

		os_mut_release(g_mut_fetch_his_fault);
	}

	else if (u16_key_val == g_u16_key_up)
	{
		os_mut_wait(g_mut_fetch_his_fault, 0xFFFF);                 //读写历史故障需要持有互斥量

		if (u8_fault_read_his_fault_state() == FAULT_CHANGE)
		{
			v_disp_his_fault_first_page();
		}
		else
		{
			num  = u16_fault_read_his_fault_cnt();
			if (num > MMI_HIS_FAULT_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_curr_fault >= MMI_HIS_FAULT_CNT)
					m_t_win_record.u_ordinal.u16_curr_fault -= MMI_HIS_FAULT_CNT;
				else
					m_t_win_record.u_ordinal.u16_curr_fault = ((num-1) / MMI_HIS_FAULT_CNT) * MMI_HIS_FAULT_CNT;

				v_disp_his_falut_page();
			}
		}

		os_mut_release(g_mut_fetch_his_fault);
	}
}

/*************************************************************
函数名称: v_disp_excption_fault_key_handler		           				
函数功能: 掉电末复归故障作息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_excption_fault_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		num  = u16_fault_read_no_resume_fault_cnt();

		if (num > MMI_EXCEPTION_CNT)
		{
			m_t_win_record.u_ordinal.u16_exception += MMI_EXCEPTION_CNT;
			
			if (m_t_win_record.u_ordinal.u16_exception >= num)
				m_t_win_record.u_ordinal.u16_exception = 0;

			v_disp_no_resume_fault_page();
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		num  = u16_fault_read_no_resume_fault_cnt();

		if (num > MMI_EXCEPTION_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_exception >= MMI_EXCEPTION_CNT)
				m_t_win_record.u_ordinal.u16_exception -= MMI_EXCEPTION_CNT;
			else
				m_t_win_record.u_ordinal.u16_exception = ((num-1) / MMI_EXCEPTION_CNT) * MMI_EXCEPTION_CNT;

			v_disp_no_resume_fault_page();
		}
	}
}

/*************************************************************
函数名称: v_disp_record_key_handler		           				
函数功能: 事件记录页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_record_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		os_mut_wait(g_mut_log, 0xFFFF);                 //读写事件记录需要持有互斥量

		if (u8_log_read_log_state() == LOG_CHANGE)
		{
			v_disp_record_first_page();
		}
		else
		{
			num = u16_log_read_log_cnt();
			if (num > MMI_RECORD_CNT)
			{
				m_t_win_record.u_ordinal.u16_record += MMI_RECORD_CNT;
				if (m_t_win_record.u_ordinal.u16_record >= num)
					m_t_win_record.u_ordinal.u16_record = 0;

				v_disp_record_page();
			}
		}	
		os_mut_release(g_mut_log);                      //释放互斥量
	}

	else if (u16_key_val == g_u16_key_up)
	{
		os_mut_wait(g_mut_log, 0xFFFF);                 //读写事件记录需要持有互斥量

		if (u8_log_read_log_state() == LOG_CHANGE)
		{
			v_disp_record_first_page();
		}
		else
		{
			num = u16_log_read_log_cnt();
			if (num > MMI_RECORD_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_record >= MMI_RECORD_CNT)
					m_t_win_record.u_ordinal.u16_record -= MMI_RECORD_CNT;
				else
					m_t_win_record.u_ordinal.u16_record = ((num-1) / MMI_RECORD_CNT) * MMI_RECORD_CNT;
			
				v_disp_record_page();
			}
		}
		os_mut_release(g_mut_log);                      //释放互斥量
	}
}

/*************************************************************
函数名称: v_disp_dc_feeder_key_handler		           				
函数功能: 馈出支路信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_dc_feeder_key_handler(U16_T u16_key_val)
{
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
				if (m_t_win_record.u_ordinal.u16_feeder_branch
					>= g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 0;
				}
				v_disp_feeder_branch_info();	
			}
		}
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO2)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
				if (m_t_win_record.u_ordinal.u16_feeder_branch
					>= g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 0;
				}
				v_disp_feeder_branch_info2();	
			}
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
				}
				else
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 
						((g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
							* MMI_FEEDER_BRANCH_CNT;
				}
	
				v_disp_feeder_branch_info();	
			}
		}
		else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_FEEDER_INFO2)
		{
			if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
			{
				if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
				{
					m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
				}
				else
				{
					m_t_win_record.u_ordinal.u16_feeder_branch = 
						((g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
							* MMI_FEEDER_BRANCH_CNT;
				}
	
				v_disp_feeder_branch_info2();	
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_dcdc_module_key_handler		           				
函数功能: 通信模块信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static v_disp_dcdc_module_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	os_mut_release(g_mut_share_data);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		m_t_win_record.u_ordinal.u16_dcdc_module += 1;
		m_t_win_record.u_ordinal.u16_dcdc_module %= num;
		v_disp_dcdc_module_info();	
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_dcdc_module > 0)
		{
			m_t_win_record.u_ordinal.u16_dcdc_module -= 1;
		}
		else
		{
			m_t_win_record.u_ordinal.u16_dcdc_module = num - 1;
		}

		v_disp_dcdc_module_info();
	}
}

/*************************************************************
函数名称: v_disp_dcdc_feeder_key_handler		           				
函数功能: 通信屏支路信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_dcdc_feeder_key_handler(U16_T u16_key_val)
{
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
			if (m_t_win_record.u_ordinal.u16_feeder_branch
				>= g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 0;
			}
			v_disp_dcdc_branch_info();	
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
			}
			else
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 
					((g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
						* MMI_FEEDER_BRANCH_CNT;
			}

			v_disp_dcdc_branch_info();	
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_dcac_module_key_handler		           				
函数功能: UPS模块信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static v_disp_dcac_module_key_handler(U16_T u16_key_val)
{
	U32_T num = 0;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num;

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		m_t_win_record.u_ordinal.u16_dcac_module += 1;
		m_t_win_record.u_ordinal.u16_dcac_module %= num;
		v_disp_dcac_module_info();	
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (m_t_win_record.u_ordinal.u16_dcac_module > 0)
		{
			m_t_win_record.u_ordinal.u16_dcac_module -= 1;
		}
		else
		{
			m_t_win_record.u_ordinal.u16_dcac_module = num - 1;
		}

		v_disp_dcac_module_info();
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_dcac_feeder_key_handler		           				
函数功能: UPS屏支路信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_dcac_feeder_key_handler(U16_T u16_key_val)
{	
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
			if (m_t_win_record.u_ordinal.u16_feeder_branch
				>= g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 0;
			}
			v_disp_dcac_branch_info();	
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
			}
			else
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 
					((g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
						* MMI_FEEDER_BRANCH_CNT;
			}

			v_disp_dcac_branch_info();	
		}
	}
	
	os_mut_release(g_mut_share_data);
}


/*************************************************************
函数名称: v_disp_ac_feeder_key_handler		           				
函数功能: 交流屏支路信息页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_ac_feeder_key_handler(U16_T u16_key_val)
{	
	os_mut_wait(g_mut_share_data, 0xFFFF);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_down)
	{
		if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			m_t_win_record.u_ordinal.u16_feeder_branch += MMI_FEEDER_BRANCH_CNT;
			if (m_t_win_record.u_ordinal.u16_feeder_branch
				>= g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 0;
			}
			v_disp_ac_branch_info();	
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num > MMI_FEEDER_BRANCH_CNT)
		{
			if (m_t_win_record.u_ordinal.u16_feeder_branch >= MMI_FEEDER_BRANCH_CNT)
			{
				m_t_win_record.u_ordinal.u16_feeder_branch -= MMI_FEEDER_BRANCH_CNT;
			}
			else
			{
				m_t_win_record.u_ordinal.u16_feeder_branch = 
					((g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_branch_num-1)/MMI_FEEDER_BRANCH_CNT)
						* MMI_FEEDER_BRANCH_CNT;
			}

			v_disp_ac_branch_info();	
		}
	}
	
	os_mut_release(g_mut_share_data);
}


#define MMI_1000000_BIT   0   //百万位
#define MMI_100000_BIT    1   //十万位
#define MMI_10000_BIT     2   //万位
#define MMI_1000_BIT      3	  //千位
#define MMI_100_BIT       4   //百位
#define MMI_10_BIT        5   //十位
#define MMI_1_BIT         6   //个位


/*************************************************************
函数名称: u8_disp_get_integer_bit		           				
函数功能: 提取u8_flag指定的数据位						
输入参数: u32_data -- 要提取的数据
          u8_flag  -- 指向相应的数据位       		   				
输出参数: 无
返回值  ：u8_flag指定位的数值														   				
**************************************************************/
static U8_T u8_disp_get_integer_bit(U32_T u32_data, U8_T u8_flag)
{
	U8_T bit = 0;

	switch (u8_flag)
	{
		case MMI_1000000_BIT:
			bit = (u32_data%10000000)/1000000;
			break;

		case MMI_100000_BIT:
			bit = (u32_data%1000000)/100000;
			break;

		case MMI_10000_BIT:
			bit = (u32_data%100000)/10000;
			break;

		case MMI_1000_BIT:
			bit = (u32_data%10000)/1000;
			break;

		case MMI_100_BIT:
			bit = (u32_data%1000)/100;
			break;

		case MMI_10_BIT:
			bit = (u32_data%100)/10;
			break;

		case MMI_1_BIT:
			bit = (u32_data%10);
			break;
		default:
			break;
	}

	return bit;
}

/*************************************************************
函数名称: v_disp_data_bit_add		           				
函数功能: 如果u8_flag对应的数据位小于朋友，则相应数据位加1；如果u8_flag对应的数据位等于9，则相应的数据位减9						
输入参数: u8_flag -- 指向相应的数据位       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_data_bit_add(U8_T u8_flag)
{
	U32_T val_tab[7] = { 1000000, 100000, 10000, 1000, 100, 10, 1};

	if (u8_disp_get_integer_bit(m_t_win_record.u32_set_value, u8_flag) < 9)
	{
		m_t_win_record.u32_set_value += val_tab[u8_flag];
	}
	else
	{
		m_t_win_record.u32_set_value -= (val_tab[u8_flag] * 9);
	}
}

/*************************************************************
函数名称: v_disp_data_bit_sub		           				
函数功能: 如果u8_flag对应的数据位大于0，则相应数据位减1；如果u8_flag对应的数据位等于0，则相应的数据位加9						
输入参数: u8_flag -- 指向相应的数据位       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_data_bit_sub(U8_T u8_flag)
{
	U32_T val_tab[7] = { 1000000, 100000, 10000, 1000, 100, 10, 1};

	if (u8_disp_get_integer_bit(m_t_win_record.u32_set_value, u8_flag) > 0)
	{
		m_t_win_record.u32_set_value -= val_tab[u8_flag];
	}
	else
	{
		m_t_win_record.u32_set_value += (val_tab[u8_flag] * 9);
	}
}

/*************************************************************
函数名称: v_disp_bit_index_add		           				
函数功能: 如果位索引是最大值，则变为最小值；如果不是最大值，则设置位索引加1						
输入参数: u8_bit_max -- 最大位数        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_bit_index_add(U8_T bit_max)
{
	if (m_t_win_record.u8_bit_index < (bit_max-1))
	{
		m_t_win_record.u8_bit_index++;
	}
	else
	{
		m_t_win_record.u8_bit_index = 0;
	}
}

/*************************************************************
函数名称: v_disp_bit_index_sub		           				
函数功能: 如果位索引是最小值，则变为最大值；如果不是最小值，则设置位索引减1					
输入参数: u8_bit_max -- 最大位数        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_bit_index_sub(U8_T u8_bit_max)
{
	if (m_t_win_record.u8_bit_index > 0)
	{
		m_t_win_record.u8_bit_index--;
	}
	else
	{
		m_t_win_record.u8_bit_index = u8_bit_max - 1;
	}
}

/*************************************************************
函数名称: v_disp_password_key_handler		           				
函数功能: 输入密码页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_password_key_handler(U16_T u16_key_val)
{
	U32_T password;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}

	else if (u16_key_val == g_u16_key_enter)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		password = g_t_share_data.t_sys_cfg.t_sys_param.u32_password;
		os_mut_release(g_mut_share_data);
		
		if ((m_t_win_record.u32_set_value == password)                                              //用户级密码正确，显示设置菜单页面
			|| (m_t_win_record.u32_set_value == MMI_SUPER_PASSWORD))
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_SET_MENU;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_sel_index = 1;

			win->u8_item_cnt = 8;                                                                  //用户级设置菜单有7项
			m_u8_set_menu_item_cnt = win->u8_item_cnt;
		}
		else if (m_t_win_record.u32_set_value == MMI_DEFEND_PASSWORD)                               //维护级密码正确，显示设置菜单页面
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_SET_MENU;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
			m_t_win_record.u8_sel_index = 1;

			win->u8_item_cnt = 12;                                                                   //维护级设置菜单有10项
			m_u8_set_menu_item_cnt = win->u8_item_cnt;
		}
		else                                                                                        //密码错误，显示密码错误提示页面
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_PASSWORD_ERROR;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}

	else if (u16_key_val == g_u16_key_up)
	{
		v_disp_bit_index_sub(5);
	}

	else if (u16_key_val == g_u16_key_down)
	{
		v_disp_bit_index_add(5);
	}

	else if (u16_key_val == g_u16_key_add)
	{
		if (m_t_win_record.u8_bit_index == 0)	       // 万位     
		{
			v_disp_data_bit_add(MMI_10000_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 1)	    // 千位     
		{
			v_disp_data_bit_add(MMI_1000_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 2)    // 百位     
		{
			v_disp_data_bit_add(MMI_100_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 3)	   // 十位     
		{
			v_disp_data_bit_add(MMI_10_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 4)    // 个位
		{
			v_disp_data_bit_add(MMI_1_BIT);
		}
	}

	else if (u16_key_val == g_u16_key_sub)
	{
		if (m_t_win_record.u8_bit_index == 0)	       // 万位     
		{
			v_disp_data_bit_sub(MMI_10000_BIT);
		}                   
		else if (m_t_win_record.u8_bit_index == 1)	   // 千位     
		{
			v_disp_data_bit_sub(MMI_1000_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 2)    // 百位     
		{
			v_disp_data_bit_sub(MMI_100_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 3)	   // 十位     
		{
			v_disp_data_bit_sub(MMI_10_BIT);
		}
		else if (m_t_win_record.u8_bit_index == 4)    // 个位
		{
			v_disp_data_bit_sub(MMI_1_BIT);
		}
	}
}

/*************************************************************
函数名称: v_disp_set_key_handler		           				
函数功能: 设置页面按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_set_key_handler(U16_T u16_key_val)
{
	U16_T id_back;
	U8_T index_back;
	U32_T num, i;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if (m_t_win_record.u8_win_status == MMI_WIN_NORMAL)     // 查看状态
	{
		if (u16_key_val == g_u16_key_enter)                                 // 进入设置状态
		{
			m_t_win_record.u8_win_status = MMI_WIN_SET;
			m_t_win_record.u8_bit_index = 0;

			if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0)
			{
				m_t_win_record.u32_set_value = *((U8_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x10)
			{
				m_t_win_record.u32_set_value = *((U16_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x20)
			{
				m_t_win_record.u32_set_value = *((U32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val));
			}
			else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x30)
			{
				//把浮点数乘以1000转换为整数处理
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W1P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W1P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val))*10)) * 100;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_4W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W2P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W2P))
				{
					m_t_win_record.u32_set_value = ((U32_T)(*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val))*100)) * 10;
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_5W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_6W3P)
					|| (win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_F32_7W3P))
				{
					m_t_win_record.u32_set_value = (U32_T)(*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val))*1000);
				}
			}

			m_t_win_record.pv_back_item_val = win->t_item[m_t_win_record.u8_sel_index].pv_val;
			win->t_item[m_t_win_record.u8_sel_index].pv_val = &(m_t_win_record.u32_set_value);
		}

		else if (u16_key_val == g_u16_key_cancel)
		{
			v_disp_cancel_key_handler();
		}

		else if (u16_key_val == g_u16_key_up)
		{
			if (m_t_win_record.u8_sel_index <= 1)
			{
				id_back = win->u16_id_prev;

				if (id_back == MMI_WIN_ID_BATT_CTL_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];						
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num < 2)
					{
						m_t_win_record.t_curr_win.u8_item_cnt = 2;
						m_t_win_record.u8_sel_index = 1;
					}
					else
					{
						m_t_win_record.u8_sel_index = 2;
					}
				}
				else if (id_back == MMI_WIN_ID_RECT_ON_OFF)
				{
					num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;

					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u_ordinal.u16_rect_on_off = ((num - 1) / MMI_RECT_ON_OFF_CNT) * MMI_RECT_ON_OFF_CNT;

						v_disp_rect_on_off();
						m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						
						win->u8_icon_cnt = 2;
						win->u16_id_next = MMI_WIN_ID_SYSTEM_CTL_SET;
						if (num > MMI_RECT_ON_OFF_CNT)
						{
							win->u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;
						}
						else
						{
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
								win->u16_id_prev = MMI_WIN_ID_SYSTEM_CTL_SET;
							else
								win->u16_id_prev = MMI_WIN_ID_BATT_CTL_SET;
						}
					}
					else  // else of if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
					{
						if (m_t_win_record.u_ordinal.u16_rect_on_off == 0)
							m_t_win_record.u_ordinal.u16_rect_on_off = ((num - 1) / MMI_RECT_ON_OFF_CNT) * MMI_RECT_ON_OFF_CNT;
						else
							m_t_win_record.u_ordinal.u16_rect_on_off -= MMI_RECT_ON_OFF_CNT;

						v_disp_rect_on_off();
						m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						
						win->u8_icon_cnt = 2;
						win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						win->u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;

						if (m_t_win_record.u_ordinal.u16_rect_on_off == 0)
						{
							if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)
							{
								if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
									win->u16_id_prev = MMI_WIN_ID_SYSTEM_CTL_SET;
								else
									win->u16_id_prev = MMI_WIN_ID_BATT_CTL_SET;
							}
						}
					}  // end of if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
				}  //end of if (id_back == MMI_WIN_ID_RECT_ON_OFF)
				else if (id_back == MMI_WIN_ID_SWT_ON_OFF)
				{
					num = u16_public_get_ctrl_swt_num();

					/*if (m_t_win_record.u_ordinal.u16_swt_on_off == 0)
					{
						m_t_win_record.u_ordinal.u16_swt_on_off = ((num - 1) / MMI_SWT_ON_OFF_CNT) * MMI_SWT_ON_OFF_CNT;
						m_u16_swt_index = u16_public_get_swt_index_from_no(m_t_win_record.u_ordinal.u16_swt_on_off + 1);
					}
					else
					{
						m_t_win_record.u_ordinal.u16_swt_on_off -= MMI_SWT_ON_OFF_CNT;
						u16_public_pre_mv_swt_index(&m_u16_swt_index, MMI_SWT_ON_OFF_CNT);
					}*/
					if (m_t_win_record.u_ordinal.u16_swt_on_off >= MMI_SWT_ON_OFF_CNT)
				  {
						m_t_win_record.u_ordinal.u16_swt_on_off -= MMI_SWT_ON_OFF_CNT;
						u16_public_pre_mv_swt_index(&m_u16_swt_index, MMI_SWT_ON_OFF_CNT);
					}
					else
					{
						m_t_win_record.u_ordinal.u16_swt_on_off = ((num - 1) / MMI_SWT_ON_OFF_CNT) * MMI_SWT_ON_OFF_CNT;
						m_u16_swt_index = u16_public_get_swt_index_from_no(m_t_win_record.u_ordinal.u16_swt_on_off + 1);
					}

					v_disp_swt_on_off();
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_SWT_ON_OFF;
					win->u16_id_prev = MMI_WIN_ID_SWT_ON_OFF;					
				}  //end of if (id_back == MMI_WIN_ID_SWT_ON_OFF)
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
					m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num - 1;
					v_disp_feeder_panel_module_set();
					
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
					if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
					m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num - 1;
					v_disp_feeder_panel_module_set2();
					
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
					if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
				}
				else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCDC_FEEDER_MODULE_ADDR);
					
				}
				else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCAC_FEEDER_MODULE_ADDR);
				}
				else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

					m_t_win_record.u_ordinal.u16_feeder_module = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num - 1;
					v_disp_acfeeder_panel_module_set();

					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 1)
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;					
				}
				else if (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)      //二组电池电流校准页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u16_id_next = MMI_WIN_ID_AC_ADJUST;
						
					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_BATT2_CURR_ADJUST)      //二组电池电流校准页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;

					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_RELAY_OUT_SET)        //干结点输出设定页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
				} 
				else if (id_back != MMI_WIN_ID_NULL)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = m_t_win_array[id_back & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					{
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
						//m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
						{
							win->t_item[2].u32_val_max = 1;
							win->u8_item_cnt = 3;
							m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						}
						else
						{
							if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path == ONE_PATH)
							{
								win->t_item[3].u32_val_max = 1;
								if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac >= 2)
									g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;
							}
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_PARAM_SET)
					{
						if (g_u8_product_type != PRODUCT_TYPE_SC32)
							win->t_item[1].u32_val_max = 1;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_CHARGE_SET)   //电池充电参数设置页面
					{
						//均浮充电压的上限为电池过压值，下限为电池欠压值
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
						win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					
						//充电限流点的上限为充电过流点
						win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)  //电池转均充电流的最大值应该是电池的充电限流点
					{
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER1_SET)
					{
						v_disp_batt1_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER2_SET)
					{
						v_disp_batt2_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt;
						}
						else
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->t_item[1].u32_val_max = 3;
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 8;
					
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt ;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt ;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt / 2;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt / 2;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt / 2;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt / 2;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt / 2;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt / 2;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_INSU_THR_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->u8_item_cnt = 3;
						else
							win->u8_item_cnt = 4;
							
						m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
						{
							win->u8_item_cnt = 5;
							m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
						}
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
						{
							win->t_item[5].u32_val_max = 1;
						}
							
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SYSTEM_CTL_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
							win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
							
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
					{
						win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;  // 有下一页
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
								
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
					{
						win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;  // 有下一页
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
								
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
					{
						win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;  // 有下一页
					}
				}
				else
				{
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
				}
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_VOLT_ADJUST) && (m_t_win_record.u8_sel_index == 2))
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_AC_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
				{
					win->u8_item_cnt = 4;
					win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
				}
						
				m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;

				g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw*10))/10.0;
				g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_LOAD_CURR_ADJUST) && (m_t_win_record.u8_sel_index == 2))        //负载电流校准页面
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DC_VOLT_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_array[MMI_WIN_ID_DC_VOLT_ADJUST & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;

				g_t_share_data.t_dc_adjust_data.f32_batt_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_pb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_cb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_bus_neg_to_gnd_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT1_CURR_ADJUST) && (m_t_win_record.u8_sel_index == 2))       //一组电池电流校准页面
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_LOAD_CURR_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_array[MMI_WIN_ID_LOAD_CURR_ADJUST & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;

				g_t_share_data.t_dc_adjust_data.f32_load_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_load_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT2_CURR_ADJUST) && (m_t_win_record.u8_sel_index == 2))       //二组电池电流校准页面
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT1_CURR_ADJUST;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_array[MMI_WIN_ID_BATT1_CURR_ADJUST & MMI_WIN_INDEX_MASK].u8_item_cnt - 1;
				
				if (g_u8_product_type == PRODUCT_TYPE_SC12)
					win->u16_id_next = MMI_WIN_ID_AC_ADJUST;

				g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
				g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RELAY_OUT_SET) && (m_t_win_record.u8_sel_index == 2))           //干结点输出设定页面
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_ALARM_SET;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u8_sel_index = m_t_win_record.t_curr_win.u8_item_cnt - 1;
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET) && (m_t_win_record.u8_sel_index == 2)) //馈线柜馈电模块设定页面
			{
				if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
				{
					m_t_win_record.u_ordinal.u16_feeder_module--;

					v_disp_feeder_panel_module_set();
					
					win->u8_icon_cnt = 2;
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
					if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
				}
				else
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有前一页
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有下一页

					v_disp_feeder_panel_module_num_set();
				}
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2) && (m_t_win_record.u8_sel_index == 2)) //馈线柜馈电模块设定页面
			{
				if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
				{
					m_t_win_record.u_ordinal.u16_feeder_module--;

					v_disp_feeder_panel_module_set2();
					
					win->u8_icon_cnt = 2;
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
					if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
					else
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
				}
				else
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有前一页
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有下一页

					v_disp_feeder_panel_module_num_set2();
				}
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET) && (m_t_win_record.u8_sel_index == 2))  //通信开关配置页面
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_sel_index = 1;

				win->u8_icon_cnt = 2;
				win->u16_id_prev = MMI_WIN_ID_DCDC_MODULE_SET;         // 有前一页
				win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;  // 有下一页
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET) && (m_t_win_record.u8_sel_index == 2))  //UPS开关配置页面
			{
				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				m_t_win_record.u16_curr_id = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_sel_index = 1;

				win->u8_icon_cnt = 2;
				win->u16_id_prev = MMI_WIN_ID_DCAC_MODULE_SET;         // 有前一页
				win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;  // 有下一页
			}
			else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_SET) && (m_t_win_record.u8_sel_index == 2))  //交流系统开关配置页面
			{
				if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
				{
					m_t_win_record.u_ordinal.u16_feeder_module--;

					v_disp_acfeeder_panel_module_set();
					
					win->u8_icon_cnt = 2;
					m_t_win_record.u8_sel_index = win->u8_item_cnt - 1;
					win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
					if (m_t_win_record.u_ordinal.u16_feeder_module > 0)
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
					else
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
				}
				else
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;  // 有前一页
					win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;  // 有下一页

					v_disp_acfeeder_panel_module_num_set();
				}
			}
			else  // else of if (m_t_win_record.u8_sel_index <= 1)
			{
				m_t_win_record.u8_sel_index--;

				//特殊情况：电池转浮充判据、电池转均充判据、电池核容设置，这些界面会出现条目没有数值的情况，在移动光标时需跳过此条目
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_NONE)
				   && ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_FLO_SET)
				      || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)))
				{
					m_t_win_record.u8_sel_index--;
				}

				/*//特殊情况：整流模块参数配置界面，通讯协议固定为MODBUS，不可设定，所以需要跳过此条目
				if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET) && (m_t_win_record.u8_sel_index == 3))
				{
					m_t_win_record.u8_sel_index--;
				}*/
			}  // end of if (m_t_win_record.u8_sel_index <= 1)
		}

		else if (u16_key_val == g_u16_key_down)
		{
			if (m_t_win_record.u8_sel_index >= win->u8_item_cnt-1)
			{
				id_back = win->u16_id_next;

				if (id_back == MMI_WIN_ID_BATT_CTL_SET)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_sel_index = 1;
					if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num < 2)
					{
						m_t_win_record.t_curr_win.u8_item_cnt = 2;
					}
				}
				else if (id_back == MMI_WIN_ID_RECT_ON_OFF)      // 整流模块开关机控制页面
				{
					num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;

					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_RECT_ON_OFF)
					{
						U16_T id_curr_back = m_t_win_record.u16_curr_id;

						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 1;
						win->u16_id_prev = id_curr_back;

						v_disp_rect_on_off();
					}
					else
					{
						m_t_win_record.u8_sel_index = 1;
						m_t_win_record.u_ordinal.u16_rect_on_off += MMI_RECT_ON_OFF_CNT;
						if (m_t_win_record.u_ordinal.u16_rect_on_off >= num)
							m_t_win_record.u_ordinal.u16_rect_on_off = 0;

						v_disp_rect_on_off();
						win->u16_id_prev = MMI_WIN_ID_RECT_ON_OFF;
					}

					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;

					if (m_t_win_record.u_ordinal.u16_rect_on_off + MMI_RECT_ON_OFF_CNT >= num)
					{
					   	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num > 0)
							win->u16_id_next = MMI_WIN_ID_SYSTEM_CTL_SET;
					}
				}  // end of if (id_back == MMI_WIN_ID_RECT_ON_OFF)
				else if (id_back == MMI_WIN_ID_SWT_ON_OFF)      // 电操开关控制页面
				{
					num = u16_public_get_ctrl_swt_num(); 
										
					m_t_win_record.u8_sel_index = 1;
					m_t_win_record.u_ordinal.u16_swt_on_off += MMI_SWT_ON_OFF_CNT;
					u16_public_next_mv_swt_index(&m_u16_swt_index, MMI_SWT_ON_OFF_CNT);
					if (m_t_win_record.u_ordinal.u16_swt_on_off >= num)
					{
						m_t_win_record.u_ordinal.u16_swt_on_off = 0;
						m_u16_swt_index = u16_public_get_first_swt_index();
					}

					v_disp_swt_on_off();
					win->u16_id_prev = MMI_WIN_ID_SWT_ON_OFF;

					win->u8_icon_cnt = 2;
					win->u16_id_next = MMI_WIN_ID_SWT_ON_OFF;
				}  // end of if (id_back == MMI_WIN_ID_SWT_ON_OFF)
				else if (id_back == MMI_WIN_ID_AC_ADJUST)       //交流校准页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
					{
						win->u8_item_cnt = 4;
						win->u16_id_prev = MMI_WIN_ID_BATT1_CURR_ADJUST;
					}

					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw*10))/10.0;
					g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_DC_VOLT_ADJUST)       //直流电压校准页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;

					g_t_share_data.t_dc_adjust_data.f32_batt_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_pb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_cb_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_bus_neg_to_gnd_volt = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_LOAD_CURR_ADJUST)       //负载电流校准页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;

					g_t_share_data.t_dc_adjust_data.f32_load_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_load_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_BATT1_CURR_ADJUST)       //电池电流校准页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
					
					if (g_u8_product_type == PRODUCT_TYPE_SC12)
						win->u16_id_next = MMI_WIN_ID_AC_ADJUST;

					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_BATT2_CURR_ADJUST)       //充电机电流校准页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;

					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_1 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
					g_t_share_data.t_dc_adjust_data.f32_batt2_curr_2 = ((U32_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]*10))/10.0;
				}
				else if (id_back == MMI_WIN_ID_RELAY_OUT_SET)        //干结点输出设定页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)   //馈电柜馈线模块数量配置页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有前一页
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;  // 有下一页

					v_disp_feeder_panel_module_num_set();
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)       //馈电柜馈线模块配置设定页面
				{	
					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
						m_t_win_record.u8_sel_index = 2;
						v_disp_feeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;	
					}
					else
					{
						m_t_win_record.u_ordinal.u16_feeder_module++;
						m_t_win_record.u8_sel_index = 2;

						v_disp_feeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						if ((m_t_win_record.u_ordinal.u16_feeder_module+1) < g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl].u8_feeder_module_num)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET;
					}
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)   //馈电柜馈线模块数量配置页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl+1;
					m_t_win_record.u8_sel_index = 1;

					win->u8_icon_cnt = 2;
					win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有前一页
					win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;  // 有下一页

					v_disp_feeder_panel_module_num_set2();
				}
				else if (id_back == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)       //馈电柜馈线模块配置设定页面
				{	
					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
					{
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.t_curr_win.u8_sel_father = m_u8_feeder_panel_ordianl + 1;
						m_t_win_record.u8_sel_index = 2;
						v_disp_feeder_panel_module_set2();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
						if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num > 1)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;	
					}
					else
					{
						m_t_win_record.u_ordinal.u16_feeder_module++;
						m_t_win_record.u8_sel_index = 2;

						v_disp_feeder_panel_module_set2();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						if ((m_t_win_record.u_ordinal.u16_feeder_module+1) < g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl].u8_feeder_module_num)
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						else
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2;
					}
				}
				else if (id_back == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET)     //通信屏馈线模块数量配置页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCDC_FEEDER_MODULE_ADDR);
				}
				else if (id_back == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET)     //UPS屏馈线模块数量配置页面
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 2;
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
							s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_DCAC_FEEDER_MODULE_ADDR);
				}
				else if (id_back == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)     //交流屏馈线模块数量配置页面
				{
					if (m_t_win_record.u16_curr_id != MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
					{	//多个馈线模块配置中第一个配置
						memset(&m_t_win_record, 0, sizeof(m_t_win_record));
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
						m_t_win_record.u16_curr_id = id_back;
						m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
						m_t_win_record.u8_sel_index = 2;
						v_disp_acfeeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 1)
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;//MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;	
					}
					else
					{   //多个馈线模块配置中非第一个配置
						m_t_win_record.u_ordinal.u16_feeder_module++;
						m_t_win_record.u8_sel_index = 2;

						v_disp_acfeeder_panel_module_set();
					
						win->u8_icon_cnt = 2;
						win->u16_id_prev = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						if ((m_t_win_record.u_ordinal.u16_feeder_module+1) < g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num)
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;//MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET;
					}

//					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
//					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
//					m_t_win_record.u8_item_inverse[1] = MMI_INVERSE_DISP;
//					m_t_win_record.u8_sel_index = 2;
//					m_t_win_record.u16_curr_id = id_back;
//					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
//					
//					snprintf((char *)(m_t_win_record.u8_special_name[0]), sizeof(m_t_win_record.u8_special_name[0]), "%s1%s", 
//							 s_disp_get_string_pointer(win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK),
//							 s_disp_get_string_pointer((win->t_item[0].u16_name_index & MMI_STR_ID_INDEX_MASK)+1));
//					snprintf((char *)(m_t_win_record.u8_special_name[1]), sizeof(m_t_win_record.u8_special_name[1]), "(%s%d)", 
//							s_disp_get_string_pointer(win->t_item[1].u16_name_index & MMI_STR_ID_INDEX_MASK), MMI_AC_FEEDER_MODULE_ADDR);
				}
				else if (id_back != MMI_WIN_ID_NULL)
				{
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = id_back;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
					m_t_win_record.u8_sel_index = 1;
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
						m_t_win_record.u8_item_inverse[9] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					{
						m_t_win_record.u8_item_inverse[5] = MMI_INVERSE_DISP;
						//m_t_win_record.u8_item_inverse[7] = MMI_INVERSE_DISP;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
						{
							win->t_item[2].u32_val_max = 1;
							win->u8_item_cnt = 3;
						}
						else
						{
							if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path == ONE_PATH)
							{
								win->t_item[3].u32_val_max = 1;
								if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac >= 2)
									g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;
							}
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_PARAM_SET)
					{
						if (g_u8_product_type != PRODUCT_TYPE_SC32)
							win->t_item[1].u32_val_max = 1;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_CHARGE_SET)   //电池充电参数设置页面
					{
						//均浮充电压的上限为电池过压值，下限为电池欠压值
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[1].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
						win->t_item[2].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit * 1000);
						win->t_item[2].u32_val_min = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit * 1000);
					
						//充电限流点的上限为充电过流点
						win->t_item[4].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)  //电池转均充电流的最大值应该是电池的充电限流点
					{
						win->t_item[1].u32_val_max = (U32_T)(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * 1000);
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER1_SET)
					{
						v_disp_batt1_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER2_SET)
					{
						v_disp_batt2_data_init();
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt;
						}
						else
						{
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_batt_total_end_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_batt_total_end_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->t_item[1].u32_val_max = 3;
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 8;
					
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.f32_rect_offline_out_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.f32_rect_offline_out_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_next = MMI_WIN_ID_DC_SYSTEM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt ;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt ;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.u16_pb_high_volt / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.u16_pb_high_volt / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.u16_pb_low_volt / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.u16_pb_low_volt / 2;
							win->t_item[3].u32_val_min = m_t_dc_volt_min.u16_cb_high_volt / 2;
							win->t_item[3].u32_val_max = m_t_dc_volt_max.u16_cb_high_volt / 2;
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_low_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_low_volt / 2;
							win->t_item[5].u32_val_min = m_t_dc_volt_min.u16_bus_high_volt / 2;
							win->t_item[5].u32_val_max = m_t_dc_volt_max.u16_bus_high_volt / 2;
							win->t_item[6].u32_val_min = m_t_dc_volt_min.u16_bus_low_volt / 2;
							win->t_item[6].u32_val_max = m_t_dc_volt_max.u16_bus_low_volt / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_THR_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit;
						}
						else
						{
							win->t_item[1].u32_val_min = m_t_dc_volt_min.f32_batt_high_volt_limit / 2;
							win->t_item[1].u32_val_max = m_t_dc_volt_max.f32_batt_high_volt_limit / 2;
							win->t_item[2].u32_val_min = m_t_dc_volt_min.f32_batt_low_volt_limit / 2;
							win->t_item[2].u32_val_max = m_t_dc_volt_max.f32_batt_low_volt_limit / 2;
						}
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_INSU_THR_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->u8_item_cnt = 3;
						else
							win->u8_item_cnt = 4;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC12)
							win->u8_item_cnt = 5;
						else if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[5].u32_val_max = 1;
					
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
						}
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 2)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER2_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 1)
							win->u16_id_prev = MMI_WIN_ID_BATT_METER1_SET;
						else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
							win->u16_id_prev = MMI_WIN_ID_RECT_PARAM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SYSTEM_CTL_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
							win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
						else
							win->u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
								
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
					{
						if (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0)
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_MODULE_SET)
					{
						if (g_u8_product_type == PRODUCT_TYPE_SC22)
							win->t_item[1].u32_val_max = 4;
							
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
						else
							win->u16_id_prev = MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET;
					}
					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
					{  		
						if (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0)
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;
					}

					else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
					{  		
						if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num > 0)
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;
						else
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;
					}
						
				} // end of else if (id_back != MMI_WIN_ID_NULL)
				else
				{
					m_t_win_record.u8_sel_index = 1;
				}
			}
			else  // else of if (m_t_win_record.u8_sel_index >= win->u8_item_cnt-1)
			{
				m_t_win_record.u8_sel_index++;

				//特殊情况：电池转浮充判据、电池转均充判据、电池核容设置，这些界面会出会出现条目没有数值的情况，在移动光标时需跳过此条目
				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type == MMI_VAL_TYPE_NONE)
				   && ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_FLO_SET)
				      || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_TO_EQU_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_DISCHARGE_END_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_PARAM_SET)
					  || (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)))
				{
					m_t_win_record.u8_sel_index++;
				}

				/*//特殊情况：整流模块参数配置界面，通讯协议固定为MODBUS，不可设定，所以需要跳过此条目
				if ((win->u16_curr_id == MMI_WIN_ID_RECT_PARAM_SET) && (m_t_win_record.u8_sel_index == 3))
				{
					m_t_win_record.u8_sel_index++;
				}*/
			}  //end of if (m_t_win_record.u8_sel_index >= win->u8_item_cnt-1)
		}
	}
	else                                                    // 设置状态
	{
		if (u16_key_val == g_u16_key_cancel)
		{
			m_t_win_record.u8_win_status = MMI_WIN_NORMAL;
			m_t_win_record.u8_bit_index = 0;
			win->t_item[m_t_win_record.u8_sel_index].pv_val = m_t_win_record.pv_back_item_val;
		}

		else if (u16_key_val == g_u16_key_enter)
		{
			if (m_t_win_record.u32_set_value >= win->t_item[m_t_win_record.u8_sel_index].u32_val_min &&
				m_t_win_record.u32_set_value <= win->t_item[m_t_win_record.u8_sel_index].u32_val_max)
			{
				m_t_win_record.u8_win_status = MMI_WIN_NORMAL;
				m_t_win_record.u8_bit_index = 0;

				win->t_item[m_t_win_record.u8_sel_index].pv_val = m_t_win_record.pv_back_item_val;

				if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0)
				{
					*((U8_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = (U8_T)(m_t_win_record.u32_set_value);
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x10)
				{
					*((U16_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = (U16_T)(m_t_win_record.u32_set_value);
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x20)
				{
					*((U32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = (U32_T)(m_t_win_record.u32_set_value);
				}
				else if ((win->t_item[m_t_win_record.u8_sel_index].u8_val_type & 0xF0) == 0x30)
				{
					*((F32_T *)(win->t_item[m_t_win_record.u8_sel_index].pv_val)) = ((F32_T)(m_t_win_record.u32_set_value) / 1000.0);
				}

				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_SYSTEM_CTL_SET)
				{
					if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0] == AUTO_MODE)
					{
						win->u16_id_next = MMI_WIN_ID_RECT_ON_OFF;
					}
					else
					{
						win->u16_id_next = MMI_WIN_ID_BATT_CTL_SET;
						g_t_share_data.t_sys_cfg.t_ctl.u16_batt[0] = (U16_T)(g_t_share_data.t_rt_data.t_batt.e_state[0]);
						g_t_share_data.t_sys_cfg.t_ctl.u16_batt[1] = (U16_T)(g_t_share_data.t_rt_data.t_batt.e_state[1]);
					}
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_THR_SET)
				{
					if ((m_t_win_record.u8_sel_index == 1) || (m_t_win_record.u8_sel_index == 2))
					{
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit)
						{
							//如果均充电压大于/小于电池过压/欠压值，则均充电压等于过压/欠压值
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit;
							else if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit;

							//如果浮充充电压大于/小于电池过压/欠压值，则浮充电压等于过压/欠压值
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit;
							else if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit;
						}
					}
					else if ((m_t_win_record.u8_sel_index == (win->u8_item_cnt - 1)))  //电池充电限流点的最大值应该是电池的充电过流点
					{
						//如果充电限流点大于充电过流点，则把充电限流点的值改为充电过流点
						if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit)
						{
							g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit;

							//如果转均充电流大于充电限流点，则把转均充电流的值改为充电限流点
							if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr)
								g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_CHARGE_SET)    //电池转均充电流的最大值应该是电池的充电限流点
					&& (m_t_win_record.u8_sel_index == (win->u8_item_cnt-1)))
				{
					//如果转均充电流大于充电限流点，则把转均充电流的值改为充电限流点
					if (g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr)
						g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr;
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_RTC_PASSWORD_SET)
					&& (m_t_win_record.u8_sel_index >= 1) && (m_t_win_record.u8_sel_index <= 6))
				{
					m_t_win_record.t_time.year += 2000; // RTC时间设置的时候不显示世纪，但改写RTC时间时需要完整的年份，所以改写RTC的时候加2000，改写完成减2000
					v_rtc_rtc_set_time(&m_t_win_record.t_time);
					m_t_win_record.t_time.year -= 2000;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_SYSTEM_SET)
				{
					if ((m_t_win_record.u8_sel_index == 1) && (m_e_volt_level != g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level))
					{
						m_e_volt_level = g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level;
						v_disp_volt_init_by_level();
						
						if (g_t_share_data.t_sys_cfg.t_sys_param.e_volt_level == VOLT_LEVEL_220)
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt;
						}
						else
						{
							win->t_item[4].u32_val_min = m_t_dc_volt_min.u16_cb_output_volt / 2;
							win->t_item[4].u32_val_max = m_t_dc_volt_max.u16_cb_output_volt / 2;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_PARAM_SET)
					&& (m_t_win_record.u8_sel_index == 2) && (g_u8_product_type != PRODUCT_TYPE_SC12))
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path == ONE_PATH)
					{
						win->t_item[3].u32_val_max = 1;
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac >= 2)
							g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 0;
					}
					else
					{
						win->t_item[3].u32_val_max = 3;
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET))
				{
					DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[m_u8_feeder_panel_ordianl]);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_feeder_panel->t_feeder_module[m_t_win_record.u_ordinal.u16_feeder_module]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_insu_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_insu_feeder_num > u8_max_num)
						pt_feeder_module->u8_insu_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_insu_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_insu_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//计算各支路数量
					pt_feeder_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						if (pt_feeder_panel->u8_feeder_branch_num + pt_feeder_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_feeder_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_feeder_panel->u8_feeder_branch_num;
							pt_feeder_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_feeder_panel->u8_feeder_branch_num += pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
					}
					
					pt_feeder_panel->u8_alarm_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_state_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_insu_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_curr_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					
					/*for (i=pt_feeder_panel->u8_feeder_module_num; i<FEEDER_PANEL_MODULE_MAX; i++)
					{
						memset(&(pt_feeder_panel->t_feeder_module[i]), 0, sizeof(FEEDER_MODULE_CFG_T));
					}*/
						
					pt_feeder_panel->u8_feeder_start_num[0] = 0;
					for (i=1; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						pt_feeder_panel->u8_feeder_start_num[i] = pt_feeder_panel->u8_feeder_start_num[i-1]
																	+ pt_feeder_panel->t_feeder_module[i-1].u8_feeder_num;
						
						if (pt_feeder_panel->u8_feeder_start_num[i] > FEEDER_BRANCH_MAX)
							pt_feeder_panel->u8_feeder_start_num[i] = FEEDER_BRANCH_MAX;
					}
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET)
					{		
						if (pt_feeder_panel->u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET;
						}
						else
						{
							win->u8_icon_cnt = 0;
							win->u16_id_prev = MMI_WIN_ID_NULL;
							win->u16_id_next = MMI_WIN_ID_NULL;
						}
					}	
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2))
				{
					DC_FEEDER_PANEL_CFG_T *pt_feeder_panel = &(g_t_share_data.t_sys_cfg.t_feeder_panel[4+m_u8_feeder_panel_ordianl]);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_feeder_panel->t_feeder_module[m_t_win_record.u_ordinal.u16_feeder_module]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_insu_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_insu_feeder_num > u8_max_num)
						pt_feeder_module->u8_insu_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_insu_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_insu_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//计算各支路数量
					pt_feeder_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						if (pt_feeder_panel->u8_feeder_branch_num + pt_feeder_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_feeder_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_feeder_panel->u8_feeder_branch_num;
							pt_feeder_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_state_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_insu_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num > pt_feeder_panel->t_feeder_module[i].u8_feeder_num)
								pt_feeder_panel->t_feeder_module[i].u8_curr_feeder_num = pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_feeder_panel->u8_feeder_branch_num += pt_feeder_panel->t_feeder_module[i].u8_feeder_num;
						}
					}
					
					pt_feeder_panel->u8_alarm_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_state_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_insu_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					pt_feeder_panel->u8_curr_feeder_num = pt_feeder_panel->u8_feeder_branch_num;
					
					/*for (i=pt_feeder_panel->u8_feeder_module_num; i<FEEDER_PANEL_MODULE_MAX; i++)
					{
						memset(&(pt_feeder_panel->t_feeder_module[i]), 0, sizeof(FEEDER_MODULE_CFG_T));
					}*/
						
					pt_feeder_panel->u8_feeder_start_num[0] = 0;
					for (i=1; i<pt_feeder_panel->u8_feeder_module_num; i++)
					{
						pt_feeder_panel->u8_feeder_start_num[i] = pt_feeder_panel->u8_feeder_start_num[i-1]
																	+ pt_feeder_panel->t_feeder_module[i-1].u8_feeder_num;
						
						if (pt_feeder_panel->u8_feeder_start_num[i] > FEEDER_BRANCH_MAX)
							pt_feeder_panel->u8_feeder_start_num[i] = FEEDER_BRANCH_MAX;
					}
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_NUM_SET2)
					{		
						if (pt_feeder_panel->u8_feeder_module_num > 0)
						{
							win->u8_icon_cnt = 2;
							win->u16_id_prev = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
							win->u16_id_next = MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2;
						}
						else
						{
							win->u8_icon_cnt = 0;
							win->u16_id_prev = MMI_WIN_ID_NULL;
							win->u16_id_next = MMI_WIN_ID_NULL;
						}
					}	
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_SET))
				{
					DCDC_PANEL_CFG_T *pt_dcdc_panel = &(g_t_share_data.t_sys_cfg.t_dcdc_panel);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_dcdc_panel->t_feeder_module[0]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//计算各支路数量
					pt_dcdc_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_dcdc_panel->u8_feeder_module_num; i++)
					{
						if (pt_dcdc_panel->u8_feeder_branch_num + pt_dcdc_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_dcdc_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_dcdc_panel->u8_feeder_branch_num;
							pt_dcdc_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_dcdc_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_dcdc_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcdc_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcdc_panel->t_feeder_module[i].u8_state_feeder_num > pt_dcdc_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcdc_panel->t_feeder_module[i].u8_state_feeder_num = pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcdc_panel->t_feeder_module[i].u8_curr_feeder_num > pt_dcdc_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcdc_panel->t_feeder_module[i].u8_curr_feeder_num = pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_dcdc_panel->u8_feeder_branch_num += pt_dcdc_panel->t_feeder_module[i].u8_feeder_num;
						}

					}
					
					pt_dcdc_panel->u8_state_feeder_num = pt_dcdc_panel->u8_feeder_branch_num;
					pt_dcdc_panel->u8_alarm_feeder_num = pt_dcdc_panel->u8_feeder_branch_num;
					pt_dcdc_panel->u8_curr_feeder_num = pt_dcdc_panel->u8_feeder_branch_num;
					
						
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCDC_FEEDER_MODULE_NUM_SET)
					{
						if (pt_dcdc_panel->u8_feeder_module_num > 0)
						{
							win->u16_id_prev = MMI_WIN_ID_DCDC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_FEEDER_MODULE_SET;
							
						}
						else
						{
							win->u16_id_prev = MMI_WIN_ID_DCDC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCDC_MODULE_SET;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_SET))
				{
					DCAC_PANEL_CFG_T *pt_dcac_panel = &(g_t_share_data.t_sys_cfg.t_dcac_panel);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_dcac_panel->t_feeder_module[0]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//计算各支路数量
					pt_dcac_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_dcac_panel->u8_feeder_module_num; i++)
					{
						if (pt_dcac_panel->u8_feeder_branch_num + pt_dcac_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_dcac_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_dcac_panel->u8_feeder_branch_num;
							pt_dcac_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_dcac_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_dcac_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcac_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcac_panel->t_feeder_module[i].u8_state_feeder_num > pt_dcac_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcac_panel->t_feeder_module[i].u8_state_feeder_num = pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_dcac_panel->t_feeder_module[i].u8_curr_feeder_num > pt_dcac_panel->t_feeder_module[i].u8_feeder_num)
								pt_dcac_panel->t_feeder_module[i].u8_curr_feeder_num = pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_dcac_panel->u8_feeder_branch_num += pt_dcac_panel->t_feeder_module[i].u8_feeder_num;
						}

					}
					
					pt_dcac_panel->u8_state_feeder_num = pt_dcac_panel->u8_feeder_branch_num;
					pt_dcac_panel->u8_alarm_feeder_num = pt_dcac_panel->u8_feeder_branch_num;
					pt_dcac_panel->u8_curr_feeder_num = pt_dcac_panel->u8_feeder_branch_num;
					
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DCAC_FEEDER_MODULE_NUM_SET)
					{
						if (pt_dcac_panel->u8_feeder_module_num > 0)
						{
							win->u16_id_prev = MMI_WIN_ID_DCAC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_FEEDER_MODULE_SET;
							
						}
						else
						{
							win->u16_id_prev = MMI_WIN_ID_DCAC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_DCAC_MODULE_SET;
						}
					}
				}
				else if ((m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
						|| (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_SET))
				{
					AC_FEEDER_CFG_T *pt_ac_panel = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed);
					FEEDER_MODULE_CFG_T *pt_feeder_module = &(pt_ac_panel->t_feeder_module[m_t_win_record.u_ordinal.u16_feeder_module]);
					U8_T u8_max_num;
					
					i = 0;
					if (pt_feeder_module->u8_alarm_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_state_feeder_num > 0)
						i++;
					if (pt_feeder_module->u8_curr_feeder_num > 0)
						i++;
					u8_max_num = FEEDER_BRANCH_MAX / i;
					if (pt_feeder_module->u8_alarm_feeder_num > u8_max_num)
						pt_feeder_module->u8_alarm_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_state_feeder_num > u8_max_num)
						pt_feeder_module->u8_state_feeder_num = u8_max_num;
					if (pt_feeder_module->u8_curr_feeder_num > u8_max_num)
						pt_feeder_module->u8_curr_feeder_num = u8_max_num;
						
					pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_alarm_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_state_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_state_feeder_num;
					if (pt_feeder_module->u8_feeder_num < pt_feeder_module->u8_curr_feeder_num)
						pt_feeder_module->u8_feeder_num = pt_feeder_module->u8_curr_feeder_num;
					
					//计算各支路数量
					pt_ac_panel->u8_feeder_branch_num = 0;
					for (i=0; i<pt_ac_panel->u8_feeder_module_num; i++)
					{
						if (pt_ac_panel->u8_feeder_branch_num + pt_ac_panel->t_feeder_module[i].u8_feeder_num > FEEDER_BRANCH_MAX)
						{
							pt_ac_panel->t_feeder_module[i].u8_feeder_num = FEEDER_BRANCH_MAX - pt_ac_panel->u8_feeder_branch_num;
							pt_ac_panel->u8_feeder_branch_num = FEEDER_BRANCH_MAX;
							
							if (pt_ac_panel->t_feeder_module[i].u8_alarm_feeder_num > pt_ac_panel->t_feeder_module[i].u8_feeder_num)
								pt_ac_panel->t_feeder_module[i].u8_alarm_feeder_num = pt_ac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_ac_panel->t_feeder_module[i].u8_state_feeder_num > pt_ac_panel->t_feeder_module[i].u8_feeder_num)
								pt_ac_panel->t_feeder_module[i].u8_state_feeder_num = pt_ac_panel->t_feeder_module[i].u8_feeder_num;
							if (pt_ac_panel->t_feeder_module[i].u8_curr_feeder_num > pt_ac_panel->t_feeder_module[i].u8_feeder_num)
								pt_ac_panel->t_feeder_module[i].u8_curr_feeder_num = pt_ac_panel->t_feeder_module[i].u8_feeder_num;
						}
						else
						{
							pt_ac_panel->u8_feeder_branch_num += pt_ac_panel->t_feeder_module[i].u8_feeder_num;
						}

					}
					
					pt_ac_panel->u8_state_feeder_num = pt_ac_panel->u8_feeder_branch_num;
					pt_ac_panel->u8_alarm_feeder_num = pt_ac_panel->u8_feeder_branch_num;
					pt_ac_panel->u8_curr_feeder_num = pt_ac_panel->u8_feeder_branch_num;
					
					
					if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_NUM_SET)
					{
						if (pt_ac_panel->u8_feeder_module_num > 0)
						{
							win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_ACS_FEEDER_MODULE_SET;							
						}
						else
						{
							win->u16_id_prev = MMI_WIN_ID_AC_MODULE_SET;
							win->u16_id_next = MMI_WIN_ID_AC_MODULE_SET;
						}
					}
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER1_SET)
				{
					v_disp_batt1_data_init();
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT_METER2_SET)
				{
					BATT_CFG_T *pt_batt_cfg = &(g_t_share_data.t_sys_cfg.t_batt);
					
					pt_batt_cfg->t_batt_group[1].u8_cell_total_num = 0;
					for (i=0; i<pt_batt_cfg->t_batt_group[1].u8_bms_num; i++)
						pt_batt_cfg->t_batt_group[1].u8_cell_total_num += pt_batt_cfg->t_batt_group[1].u8_cell_num[i];
					
					if (pt_batt_cfg->t_batt_group[1].u8_cell_total_num > BATT_CELL_MAX)
						pt_batt_cfg->t_batt_group[1].u8_cell_total_num = BATT_CELL_MAX;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_AC_ADJUST)      //交流校准页面
				{
					os_evt_set(1 << (m_t_win_record.u8_sel_index - 1), g_tid_ac_sample);
					index_back = m_t_win_record.u8_sel_index;
				
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //提示校准正在进行
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_AC_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DC_VOLT_ADJUST)  //直流电压校准页面
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_BATT_VOLT, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_PB_VOLT, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 4)
						os_evt_set(DC_ADJUST_CB_VOLT, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 5)
						os_evt_set(DC_ADJUST_BUS_NEG_TO_END_VOLT, g_tid_dc_sample);

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //提示校准正在进行
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_DC_VOLT_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_LOAD_CURR_ADJUST)    //负载电流校准页面
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_LOAD_CURR_1, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_LOAD_CURR_2, g_tid_dc_sample);

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //提示校准正在进行
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_LOAD_CURR_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT1_CURR_ADJUST)   //一组电池电流校准页面
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_BATT1_CURR_1, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_BATT1_CURR_2, g_tid_dc_sample);

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //提示校准正在进行
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_BATT1_CURR_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_BATT2_CURR_ADJUST)   //二组电池电流校准页面
				{
					index_back = m_t_win_record.u8_sel_index;
					if (m_t_win_record.u8_sel_index == 2)
						os_evt_set(DC_ADJUST_BATT2_CURR_1, g_tid_dc_sample);
					else if (m_t_win_record.u8_sel_index == 3)
						os_evt_set(DC_ADJUST_BATT2_CURR_2, g_tid_dc_sample);
					
					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_DOING;  //提示校准正在进行
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					
					win->u16_id_father = MMI_WIN_ID_BATT2_CURR_ADJUST;
					win->u8_sel_father = index_back;
				}
				else if (m_t_win_record.u16_curr_id == MMI_WIN_ID_DISPLAY)   // 显示设置界面
				{
					if (m_t_win_record.u8_sel_index == 1)
					{
						m_e_lang = g_t_share_data.t_sys_cfg.t_sys_param.e_lang;
					}
					else if (m_t_win_record.u8_sel_index == 2)
					{
						m_e_lcd_direction = g_t_share_data.t_sys_cfg.t_sys_param.e_lcd_driection;
						
						if (m_e_lcd_direction == LCD_HORIZONTAL)   //根据显示方向初始化按键值
						{
							g_u16_key_cancel = KEY_K7;
							g_u16_key_enter = KEY_K6;
							g_u16_key_up = KEY_K4;
							g_u16_key_down = KEY_K5;
							g_u16_key_add = KEY_K3;
							g_u16_key_sub = KEY_K2;
						}
						else
						{
							g_u16_key_cancel = KEY_K7;
							g_u16_key_enter = KEY_K6;
							g_u16_key_up = KEY_K3;
							g_u16_key_down = KEY_K2;
							g_u16_key_add = KEY_K5;
							g_u16_key_sub = KEY_K4;
						}
					}
				}

				v_fetch_save_cfg_data();                                   //保存配置数据到dataflash                                  
			}
			else              //设置数据范围超限，提示数据超出范围
			{
				U32_T u32_val_min, u32_val_max;
				U8_T u8_val_type;
				
				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET)
					m_u16_feeder_module_ordianl = m_t_win_record.u_ordinal.u16_feeder_module;

				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_FEEDER_PANEL_MODULE_SET2)
					m_u16_feeder_module_ordianl = m_t_win_record.u_ordinal.u16_feeder_module;

				if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ACS_FEEDER_MODULE_SET)
					m_u16_feeder_module_ordianl = m_t_win_record.u_ordinal.u16_feeder_module;
				
				id_back = m_t_win_record.u16_curr_id;
				index_back = m_t_win_record.u8_sel_index;
				win->t_item[index_back].pv_val = m_t_win_record.pv_back_item_val;
				u32_val_min = win->t_item[index_back].u32_val_min;
				u32_val_max = win->t_item[index_back].u32_val_max;
				u8_val_type = win->t_item[index_back].u8_val_type;

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_OUT_RANGE;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
				
				if ((u8_val_type == MMI_VAL_TYPE_F32_1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_3W1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_4W1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W1P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_6W1P))	    //分类打印，1位小数
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%.1f", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_max/1000.0);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]),
							"%s%.1f", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_min/1000.0);
				}
				else if ((u8_val_type == MMI_VAL_TYPE_F32_2P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_4W2P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W2P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_6W2P))	    //分类打印，2位小数
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%.2f", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_max/1000.0);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]), 
							"%s%.2f", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_min/1000.0);
				}
				else if ((u8_val_type == MMI_VAL_TYPE_F32_3P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W3P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_6W3P)
					|| (u8_val_type == MMI_VAL_TYPE_F32_5W3P))	    //分类打印，3位小数
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%.3f", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_max/1000.0);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]),
							"%s%.3f", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), (F32_T)u32_val_min/1000.0);
				}
				else
				{ 
					snprintf((char *)(m_t_win_record.u8_special_name[3]), sizeof(m_t_win_record.u8_special_name[3]),
							"%s%d", s_disp_get_string_pointer(win->t_item[3].u16_name_index & MMI_STR_ID_INDEX_MASK), u32_val_max);
					snprintf((char *)(m_t_win_record.u8_special_name[4]), sizeof(m_t_win_record.u8_special_name[4]),
							"%s%d", s_disp_get_string_pointer(win->t_item[4].u16_name_index & MMI_STR_ID_INDEX_MASK), u32_val_min);
				}

				win = &(m_t_win_record.t_curr_win);
				win->u16_id_father = id_back;
				win->u8_sel_father = index_back;
			}
		}

		else if (u16_key_val == g_u16_key_up)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_U8_2BIT: 
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:
				case MMI_VAL_TYPE_F32_3W1P: //以上4种情况都有2个数据位，合并处理
					v_disp_bit_index_sub(2);
					break;

				case MMI_VAL_TYPE_U8_3BIT: 
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:
				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_4W2P: //以上5种情况都有3个数据位，合并处理
					v_disp_bit_index_sub(3);
					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:
				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_5W2P:
				case MMI_VAL_TYPE_F32_5W3P: //以上5种情况都有4个数据位，合并处理
					v_disp_bit_index_sub(4);
					break;

				case MMI_VAL_TYPE_U32_5BIT:
				case MMI_VAL_TYPE_F32_6W1P:
				case MMI_VAL_TYPE_F32_6W2P:
				case MMI_VAL_TYPE_F32_6W3P: //以上3种情况都有5个数据位，合并处理
					v_disp_bit_index_sub(5);
					break;

				case MMI_VAL_TYPE_F32_7W3P: //6个数据位
					v_disp_bit_index_sub(6);
					break;
				
				default:
					break;
			}
		}

		else if (u16_key_val == g_u16_key_down)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_U8_2BIT: 
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:
				case MMI_VAL_TYPE_F32_3W1P: //以上4种情况都有2个数据位，合并处理
					v_disp_bit_index_add(2);
					break;

				case MMI_VAL_TYPE_U8_3BIT:
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:
				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_4W2P: //以上5种情况都有3个数据位，合并处理
					v_disp_bit_index_add(3);
					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:
				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_5W2P:
				case MMI_VAL_TYPE_F32_5W3P: //以上5种情况都有4个数据位，合并处理
					v_disp_bit_index_add(4);
					break;

				case MMI_VAL_TYPE_U32_5BIT:
				case MMI_VAL_TYPE_F32_6W1P:
				case MMI_VAL_TYPE_F32_6W2P:
				case MMI_VAL_TYPE_F32_6W3P: //以上3种情况都有5个数据位，合并处理
					v_disp_bit_index_add(5);
					break;

				case MMI_VAL_TYPE_F32_7W3P: //6个数据位
					v_disp_bit_index_add(6);
					break;
				
				default:
					break;
			}
		}

		else if (u16_key_val == g_u16_key_add)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_ENUM:
					m_t_win_record.u32_set_value += 1;
					if (m_t_win_record.u32_set_value > win->t_item[m_t_win_record.u8_sel_index].u32_val_max)
					{
						m_t_win_record.u32_set_value = win->t_item[m_t_win_record.u8_sel_index].u32_val_min;
					}
					break;

				case MMI_VAL_TYPE_U8:
				case MMI_VAL_TYPE_U16:
				case MMI_VAL_TYPE_U32:	                      // 以上3种情况只有1个数据位，合并处理
					if (m_t_win_record.u32_set_value < 9)
					{
						m_t_win_record.u32_set_value += 1;
					}
					else
					{
						m_t_win_record.u32_set_value = 0;
					}	
					break;

				case MMI_VAL_TYPE_U8_2BIT:
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:                   // 以上3种情况有2个数据位，合并处理	       
					if (m_t_win_record.u8_bit_index == 0)	      // 十位     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)   // 个位
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U8_3BIT:
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:	                  // 以上3种情况有3个数据位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	      // 百位     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	   // 十位     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // 个位
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:	                   // 以上2种情况有4个数据位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	       // 千位     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // 百位     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // 十位     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // 个位
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_U32_5BIT:	                   // 5个数据位的整数，主要用于密码设置
					if (m_t_win_record.u8_bit_index == 0)	       // 万位     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // 千位     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // 百位     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // 十位     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // 个位
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;
					
				case MMI_VAL_TYPE_F32_3W1P:
				case MMI_VAL_TYPE_F32_4W2P:         // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
				case MMI_VAL_TYPE_F32_5W3P:         // 以上3种情况有1个整数位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	  // 个位*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // 十分位*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // 百分位*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // 千分位*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_5W2P:             // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
				case MMI_VAL_TYPE_F32_6W3P:             // 以上3种情况有2个整数位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	       // 十位*1000     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // 个位*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // 十分位*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // 百分位*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // 千分位*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_6W2P:             // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
				case MMI_VAL_TYPE_F32_7W3P:             // 以上3种情况有3个整数位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	       // 百位*1000     
					{
						v_disp_data_bit_add(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // 十位*1000     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // 个位*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // 十分位*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)	   // 百分位*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)    // 千分位*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_6W1P:             // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
					if (m_t_win_record.u8_bit_index == 0)	       // 千位*1000     
					{
						v_disp_data_bit_add(MMI_1000000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	       // 百位*1000     
					{
						v_disp_data_bit_add(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // 十位*1000     
					{
						v_disp_data_bit_add(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	    // 个位*1000     
					{
						v_disp_data_bit_add(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // 十分位*1000     
					{
						v_disp_data_bit_add(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)	   // 百分位*1000     
					{
						v_disp_data_bit_add(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 6)    // 千分位*1000
					{
						v_disp_data_bit_add(MMI_1_BIT);
					}
					break;

				default:
					break;
			}
		}																																																				

		else if (u16_key_val == g_u16_key_sub)
		{
			switch (win->t_item[m_t_win_record.u8_sel_index].u8_val_type)
			{
				case MMI_VAL_TYPE_ENUM:
					if (m_t_win_record.u32_set_value > win->t_item[m_t_win_record.u8_sel_index].u32_val_min)
					{
						m_t_win_record.u32_set_value -= 1;
					}
					else
					{
						m_t_win_record.u32_set_value = win->t_item[m_t_win_record.u8_sel_index].u32_val_max;
					}
					break;

				case MMI_VAL_TYPE_U8:
				case MMI_VAL_TYPE_U16:
				case MMI_VAL_TYPE_U32:	                      // 以上3种情况只有1个数据位，合并处理
					if (m_t_win_record.u32_set_value > 0)
					{
						m_t_win_record.u32_set_value -= 1;
					}
					else
					{
						m_t_win_record.u32_set_value = 9;
					}	
					break;

				case MMI_VAL_TYPE_U8_2BIT:
				case MMI_VAL_TYPE_U16_2BIT:
				case MMI_VAL_TYPE_U32_2BIT:                   // 以上3种情况有2个数据位，合并处理	       
					if (m_t_win_record.u8_bit_index == 0)	      // 十位     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)   // 个位
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U8_3BIT:
				case MMI_VAL_TYPE_U16_3BIT:
				case MMI_VAL_TYPE_U32_3BIT:	                  // 以上3种情况有3个数据位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	      // 百位     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	   // 十位     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // 个位
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}

					break;

				case MMI_VAL_TYPE_U16_4BIT:
				case MMI_VAL_TYPE_U32_4BIT:	                   // 以上2种情况有4个数据位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	       // 千位     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // 百位     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // 十位     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // 个位
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_U32_5BIT:                    // 5个数据位的整数，主要用于密码设置
					if (m_t_win_record.u8_bit_index == 0)	       // 万位     
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}                   
					else if (m_t_win_record.u8_bit_index == 1)	   // 千位     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // 百位     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // 十位     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // 个位
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;
					
				case MMI_VAL_TYPE_F32_3W1P:
				case MMI_VAL_TYPE_F32_4W2P:			   // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
				case MMI_VAL_TYPE_F32_5W3P:              // 以上3种情况有1个整数位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	       // 个位*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)    // 十分位*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	   // 百分位*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // 千分位*1000
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_4W1P:
				case MMI_VAL_TYPE_F32_5W2P:			   // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
				case MMI_VAL_TYPE_F32_6W3P:              // 以上3种情况有2个整数位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	       // 十位*1000     
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // 个位*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)    // 十分位*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	   // 百分位*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // 千分位*1000
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_5W1P:
				case MMI_VAL_TYPE_F32_6W2P:			   // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
				case MMI_VAL_TYPE_F32_7W3P:              // 以上3种情况有3个整数位，合并处理
					if (m_t_win_record.u8_bit_index == 0)	       // 百位*1000     
					{
						v_disp_data_bit_sub(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)	    // 十位*1000    
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // 个位*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)    // 十分位*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)	   // 百分位*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)    // 千分位*1000
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				case MMI_VAL_TYPE_F32_6W1P:			   // 由于直接用浮点处理会有精度损失，造成数据不对，浮点数*1000转化为整形进行处理
					if (m_t_win_record.u8_bit_index == 0)	       // 千位*1000     
					{
						v_disp_data_bit_sub(MMI_1000000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 1)     // 百位*1000     
					{
						v_disp_data_bit_sub(MMI_100000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 2)	    // 十位*1000    
					{
						v_disp_data_bit_sub(MMI_10000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 3)	    // 个位*1000     
					{
						v_disp_data_bit_sub(MMI_1000_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 4)    // 十分位*1000     
					{
						v_disp_data_bit_sub(MMI_100_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 5)	   // 百分位*1000     
					{
						v_disp_data_bit_sub(MMI_10_BIT);
					}
					else if (m_t_win_record.u8_bit_index == 6)    // 千分位*1000
					{
						v_disp_data_bit_sub(MMI_1_BIT);
					}
					break;

				default:
					break;
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_disp_main_win_key_handler		           				
函数功能: 主窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_main_win_key_handler(U16_T u16_key_val)
{
	U16_T id_back;
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_MAIN_MENU;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u8_sel_index = 1;
	}
	else if (u16_key_val == g_u16_key_up)
	{
		id_back = win->u16_id_prev;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));			
			m_t_win_record.u16_curr_id = id_back;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
	else if (u16_key_val == g_u16_key_down)
	{
		id_back = win->u16_id_next;
		if (id_back != MMI_WIN_ID_NULL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));			
			m_t_win_record.u16_curr_id = id_back;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
}

/*************************************************************
函数名称: v_disp_about_key_handler		           				
函数功能: 关于窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_about_key_handler(U16_T u16_key_val)
{
	if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
}

/*************************************************************
函数名称: v_disp_restore_param_key_handler		           				
函数功能: 恢复默认参数窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_restore_param_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];

		win->u16_id_father = MMI_WIN_ID_PARAM_RESTORE;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		U32_T num;

		os_mut_wait(g_mut_share_data, 0xFFFF);
		num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		os_mut_release(g_mut_share_data);
				
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;

		if (num > 0)
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_RESTORE;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
		else
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD_CLEAR;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
}

/*************************************************************
函数名称: v_disp_restore_batt_cap_key_handler		           				
函数功能: 恢复电池容量窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_restore_batt_cap_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_BATT_RESTORE;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_RESTORE;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
函数名称: v_disp_clear_record_key_handler		           				
函数功能: 清除电池充放电记录窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_clear_record_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	
	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_RECORD_CLEAR;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		U32_T num;

		os_mut_wait(g_mut_share_data, 0xFFFF);
		num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		os_mut_release(g_mut_share_data);

		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;

		if (num > 0)
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_BATT_RESTORE;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
		else
		{
			m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_RESTORE;
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		}
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_EXCEPTION_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}

/*************************************************************
函数名称: v_disp_clear_his_record_key_handler		           				
函数功能: 清除历史记录窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_clear_his_fault_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_HIS_FAULT_CLEAR;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_EXCEPTION_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_PARAM_RESTORE;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}


/*************************************************************
函数名称: v_disp_clear_no_resume_falut_key_handler		           				
函数功能: 清除历史记录窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_clear_no_resume_falut_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);

	if (u16_key_val == g_u16_key_enter)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = MMI_WIN_ID_CONFIRM_OPERATION;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		win->u16_id_father = MMI_WIN_ID_EXCEPTION_CLEAR;
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		v_disp_cancel_key_handler();
	}
	else if (u16_key_val == g_u16_key_down)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_RECORD_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
	else if (u16_key_val == g_u16_key_up)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
		m_t_win_record.u16_curr_id = MMI_WIN_ID_HIS_FAULT_CLEAR;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
	}
}


/*************************************************************
函数名称: v_disp_confirm_key_handler		           				
函数功能: 确认操作提示窗口按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_confirm_key_handler(U16_T u16_key_val)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U16_T u16_id_father = win->u16_id_father;

	if (u16_key_val == g_u16_key_enter)
	{
		switch (u16_id_father)
		{
			case MMI_WIN_ID_PARAM_RESTORE:
				v_disp_cfg_data_restore();      //执行恢复默认参数操作

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_PARAM_RESTORE;
				break;

			case MMI_WIN_ID_HIS_FAULT_CLEAR:
				v_fault_clear_his_fault();      //执行清除历史告警记录操作

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_HIS_FAULT_CLEAR;
				break;

			case MMI_WIN_ID_EXCEPTION_CLEAR:
				v_fault_clear_no_resume_fault();      //执行清除掉电末复归告警记录操作

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_EXCEPTION_CLEAR;
				break;

			case MMI_WIN_ID_BATT_RESTORE:
				os_evt_set(BATT_CAPACITY_RESTORE, g_tid_batt);   //发送电池容量恢复事件标志给电池管理任务

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_BATT_RESTORE;
				break;

			case MMI_WIN_ID_RECORD_CLEAR:
				v_log_clear_record();            //执行清除事件记录操作

				memset(&m_t_win_record, 0, sizeof(m_t_win_record));
				m_t_win_record.u16_curr_id = MMI_WIN_ID_OPERATION_SCRESS;
				m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
				win->u16_id_father = MMI_WIN_ID_RECORD_CLEAR;
				break;

			default:
				break;
		}
	}
	else if (u16_key_val == g_u16_key_cancel)
	{
		memset(&m_t_win_record, 0, sizeof(m_t_win_record));
		m_t_win_record.u16_curr_id = u16_id_father;
		m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
		m_t_win_record.u8_item_inverse[0] = MMI_INVERSE_DISP;
	}
}


/*************************************************************
函数名称: v_disp_key_handler		           				
函数功能: 按键处理函数						
输入参数: u16_key_val -- 按键值        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_key_handler(U16_T u16_key_val)
{
	switch (m_t_win_record.u16_curr_id >> 8)             //根据页面类型，分类处理
	{
		case MMI_WIN_TYPE_MAIN_WIN:
			v_disp_main_win_key_handler(u16_key_val);
			break;
		
		case MMI_WIN_TYPE_MENU:
			v_disp_menu_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_PASSWORD:
			v_disp_password_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_SET:
			v_disp_set_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_ABOUT:
			v_disp_about_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_PARAM_RESTORE:
			v_disp_restore_param_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_BATT_CAP_RESTORE:
			v_disp_restore_batt_cap_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_RECORD_CLEAR:
			v_disp_clear_record_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_HIS_FAULT_CLEAR:
			v_disp_clear_his_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_EXCEPTION_CLEAR:
			v_disp_clear_no_resume_falut_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_RUN_INFO:
			v_disp_run_info_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_CELL1_VOLT:
			v_disp_cell1_volt_key_handler(u16_key_val);
			break;
		
		case MMI_WIN_TYPE_CELL2_VOLT:
			v_disp_cell2_volt_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_RECT:
			v_disp_rect_info_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_SWITCH:
			v_disp_swt_info_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_EC_SWITCH:
			v_disp_ecswt_info_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_CURR_FAULT:
			v_disp_curr_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_HIS_FAULT:
			v_disp_his_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_EXCEPTION:
			v_disp_excption_fault_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_BATT_RECORD:
			v_disp_record_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_CONFIRM:
			v_disp_confirm_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DC_FEEDER:
			v_disp_dc_feeder_key_handler(u16_key_val);
			break;
		
		case MMI_WIN_TYPE_DCDC_MODULE:
			v_disp_dcdc_module_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DCAC_MODULE:
			v_disp_dcac_module_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DCDC_FEEDER:
			v_disp_dcdc_feeder_key_handler(u16_key_val);
			break;
			
		case MMI_WIN_TYPE_DCAC_FEEDER:
			v_disp_dcac_feeder_key_handler(u16_key_val);
			break;

		case MMI_WIN_TYPE_AC_FEEDER:
			v_disp_ac_feeder_key_handler(u16_key_val);
			break;
			
		default:
			break;
	}
}

/*************************************************************
函数名称: v_disp_adjust_donig_handler		           				
函数功能: 校准正在进行页面事件处理						
输入参数: u16_flags -- 校准成功或者失败标志        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_disp_adjust_donig_handler(U16_T u16_flag)
{
	MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
	U16_T u16_id_father = win->u16_id_father;
	U8_T  u8_sel_index = win->u8_sel_father;

	if (u16_id_father == MMI_WIN_ID_AC_ADJUST)
	{
		if (u16_flag & AC_ADJUST_SCUESS)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_SCUESS;  //提示校准成功
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = MMI_WIN_ID_AC_ADJUST;
			win->u8_sel_father = u8_sel_index;
		}
		else if (u16_flag & AC_ADJUST_FAIL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_FAIL;   //提示校准失败
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = MMI_WIN_ID_AC_ADJUST;
			win->u8_sel_father = u8_sel_index;
		}
	}
	else if ((u16_id_father == MMI_WIN_ID_DC_VOLT_ADJUST)
	         || (u16_id_father == MMI_WIN_ID_LOAD_CURR_ADJUST)
			 || (u16_id_father == MMI_WIN_ID_BATT1_CURR_ADJUST)
			 || (u16_id_father == MMI_WIN_ID_BATT2_CURR_ADJUST)) 
	{
		if (u16_flag & DC_ADJUST_SCUESS)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_SCUESS;  //提示校准成功
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = u16_id_father;
			win->u8_sel_father = u8_sel_index;
		}
		else if (u16_flag & DC_ADJUST_FAIL)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_FAIL;  //提示校准失败
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = u16_id_father;
			win->u8_sel_father = u8_sel_index;
		}
		else if (u16_flag & DC_ADJUST_FIRST_CURR_COMPLETE)
		{
			memset(&m_t_win_record, 0, sizeof(m_t_win_record));
			m_t_win_record.u16_curr_id = MMI_WIN_ID_ADJUST_NEXT_CURR;  //提示校准下一个电流点
			m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
			win->u16_id_father = u16_id_father;
			win->u8_sel_father = u8_sel_index;
		}
	}
}

/*************************************************************
函数名称: v_disp_display_task		           				
函数功能: 显示和按键处理任务，该任务可通过RTX函数os_tsk_create来启动						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
__task void v_disp_display_task(void)
{
	U16_T flags;
	U32_T blink_cnt = 0;
	U32_T u32_time;
	BOOL_T b_backlight = 1;
	BOOL_T b_buzzer = 0;

	v_disp_display_init();

	while (1)
	{
		os_evt_set(DISPLAY_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
		
		//校准正在进行提示页面，读取校准成功或者失败标志
		if (m_t_win_record.u16_curr_id == MMI_WIN_ID_ADJUST_DOING)
		{
			if (os_evt_wait_or(ADJUST_FLAGS, 0) == OS_R_EVT)
			{ 
				flags = os_evt_get();
				v_disp_adjust_donig_handler(flags);
			}
		}

		//读取故障蜂鸣器事件标志
		if (os_evt_wait_or(BUZZER_EVT_FLAGS, 0) == OS_R_EVT)
		{
			flags = os_evt_get();

			if (flags & FAULT_BUZZER_BEEP)
			{
				b_buzzer = 1;
			}
			else if (flags & FAULT_BUZZER_QUEIT)
			{
				b_buzzer = 0;
			}

			if (flags & FAULT_OCCURE)
			{
				u32_time = u32_delay_get_timer_val();        //告警发生，如果背光是关闭的，则打开背光
				if (b_backlight == 0)
				{
					v_relay_relay_operation(ENABLE_BACKLIGHT);
					b_backlight = 1;
				}
			}
		}
		
		//等待按键事件标志		 
		if (os_evt_wait_or(KEY_EVT_FLAGS, MMI_REFRESH_INTERVAL) == OS_R_EVT)
		{ 
			flags = os_evt_get();

			if ((b_buzzer == 0) && (b_backlight == 1))
			{
				if (flags & g_u16_key_cancel)	
					v_disp_key_handler(g_u16_key_cancel);

				if (flags & g_u16_key_enter)
					v_disp_key_handler(g_u16_key_enter);

				if (flags & g_u16_key_down)
					v_disp_key_handler(g_u16_key_down);

				if (flags & g_u16_key_up)
					v_disp_key_handler(g_u16_key_up);

				if (flags & g_u16_key_sub)
					v_disp_key_handler(g_u16_key_sub);

				if (flags & g_u16_key_add)
					v_disp_key_handler(g_u16_key_add);
			}

			if (b_buzzer == 1)                                //如果蜂鸣器正在鸣叫，关闭蜂鸣器
			{
				v_relay_relay_operation(BEEP_OFF);
				b_buzzer = 0;
			}

			if (b_backlight == 0)                            //如果是黑屏状态，则打开背光
			{
				v_relay_relay_operation(ENABLE_BACKLIGHT);
				b_backlight = 1;
				u32_time = u32_delay_get_timer_val();
			}

			u32_time = u32_delay_get_timer_val();
		}
		else
		{
			if ((b_backlight == 1)                  //长时间没有按键按下，返回关背光主界面
				&& (u32_delay_time_elapse(u32_time, u32_delay_get_timer_val()) >= MMI_RETURN_MAIN_WIN_TIME))
			{
				if (m_t_win_record.u16_curr_id != MMI_WIN_ID_MAIN_WINDOW)
				{
					if (m_t_win_record.u8_win_status == MMI_WIN_SET)       //如果是设置状态，设置条目的数据指针需要恢复
					{
						MMI_WIN_T *win = &(m_t_win_record.t_curr_win);
						win->t_item[m_t_win_record.u8_sel_index].pv_val = m_t_win_record.pv_back_item_val;
					}

					memset(&m_t_win_record, 0, sizeof(m_t_win_record));
					m_t_win_record.u16_curr_id = MMI_WIN_ID_MAIN_WINDOW;
					m_t_win_record.t_curr_win = m_t_win_array[m_t_win_record.u16_curr_id & MMI_WIN_INDEX_MASK];
					if(g_t_share_data.t_sys_cfg.t_sys_param.e_bus_seg_num == ONE)
					{
						m_t_win_record.t_curr_win.u16_id_next = MMI_WIN_ID_NULL;
						m_t_win_record.t_curr_win.u8_icon_cnt = 1;
					}
				}

				v_relay_relay_operation(DISABLE_BACKLIGHT);
				b_backlight = 0;

				if (b_buzzer == 1)                                   //如果蜂鸣器正在鸣叫，关闭蜂鸣器
				{
					v_relay_relay_operation(BEEP_OFF);
					b_buzzer = 0;
				}
			}
		}

		if ((m_t_win_record.u8_win_status == MMI_WIN_SET)
			|| ((m_t_win_record.u16_curr_id>>8) == MMI_WIN_TYPE_PASSWORD))
		{
			if ((++blink_cnt) % 3 == 0)
			{
				blink_cnt = 0;
				if (m_t_win_record.u8_set_blink == MMI_NORMAL_DISP)
				{
			 		m_t_win_record.u8_set_blink = MMI_INVERSE_DISP;
				}
				else
				{
			 		m_t_win_record.u8_set_blink = MMI_NORMAL_DISP;
				}
			}
		} 

		v_disp_display_update();
	}
}
