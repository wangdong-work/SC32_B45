/************************************************************
Copyright (C), 深圳英可瑞科技开发有限公司, 保留一切权利
文 件 名：BSInteranl.c
版    本：1.00
创建日期：2013-03-01
作    者：郭数理
功能描述：内部MODBUS规约实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2013-03-01  1.00     创建
***********************************************************/

#include <string.h>
#include <math.h>

#include "Type.h"
#include "PublicData.h"
#include "BSInternal.h"
#include "Backstage.h"
#include "Crc.h"
#include "Fault.h"
#include "FetchFlash.h"

#include "../Drivers/uart_device.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Delay.h"


//#define DEBUG

#ifdef DEBUG
	#define DPRINT(fmt, args...) u32_usb_debug_print(fmt, ##__VA_ARGS__)
#else
	#define DPRINT(fmt, args...)
#endif


#define MODBUS_BYTE_SIZE           8
#define MODBUS_MAXREG_NUM          125    /* 一次最多可读取的寄存器个数 */

#define BS_DC10_YC_DATA_CNT       20
#define BS_DC10_YC_AD_CNT         17
#define BS_DC10_YX_CNT            70
#define BS_DC10_YT_SET_CNT        43
#define BS_DC10_YT_ADJUST_CNT     33
#define BS_DC10_YC_DATA_OFFSET    0x0100
#define BS_DC10_YC_AD_OFFSET      0x0400
#define BS_DC10_YX_OFFSET         0x0500
#define BS_DC10_YT_SET_OFFSET     0x1000
#define BS_DC10_YT_ADJUST_OFFSET  0x1100
#define BS_DC10_ADDR_MASK         0xFF00
#define BS_DC10_INDEX_MASK        0x00FF

#define BS_DC10_AC_INPUT_MODE_INDEX 0x00
#define BS_DC10_AC_CTRL_MODE_INDEX  0x01
#define BS_DC10_V1_HIGH_VOLT_INDEX  0x05
#define BS_DC10_V1_LOW_VOLT_INDEX   0x06
#define BS_DC10_A2_OVER_CURR_INDEX  0x0C
#define BS_DC10_A3_OVER_CURR_INDEX  0x0D
#define BS_DC10_A1_SHUNT_VOLT_INDEX 0x0E
#define BS_DC10_A2_SHUNT_VOLT_INDEX 0x10
#define BS_DC10_A3_SHUNT_VOLT_INDEX 0x12
#define BS_DC10_DIODE_CHAIN_INDEX   0x21
#define BS_DC10_RELAY_START_INDEX   0x22


//遥测组装条目
typedef struct
{
	void   *pv_val;        //指向遥测数据指针
	U16_T  u16_type;       //pv_val指向的数据类型
	U16_T  u16_coeff;      //变比
}DC10_YC_ASSEMBLE_T;

//遥信组装条目
typedef struct
{
	void   *pv_val;        //指向遥测数据指针
	U16_T  u16_type;        //pv_val指向的数据类型
	U16_T  u16_mask;       //屏蔽码
}DC10_YX_ASSEMBLE_T;



static DC10_YC_ASSEMBLE_T m_t_yc_data_assemble[BS_DC10_YC_DATA_CNT] =
{
	//交流1路UV电压，寄存器地址0x100
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//交流1路VW电压，寄存器地址0x101
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//交流1路WU电压，寄存器地址0x102
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//交流2路UV电压，寄存器地址0x103
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//交流2路VW电压，寄存器地址0x104
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//交流2路WU电压，寄存器地址0x105
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//环境温度，寄存器地址0x106
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature),         //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V1电压值，控母电压，寄存器地址0x107
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt),             //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A1分流器电流，负载电流，寄存器地址0x108
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr),           //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V2电压值，电池电压，寄存器地址0x109
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt),           //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A2分流器电流，一组电池电流，寄存器地址0x10A
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]),        //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V3电压值，合母电压，寄存器地址0x10B
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt),             //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A3分流器电流，二组电池电流，寄存器地址0x10C
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]),        //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//传感器分路S1电流，监控无此数据，寄存器地址0x10D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//传感器分路S2电流，监控无此数据，寄存器地址0x10E
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//传感器分路S3电流，监控无此数据，寄存器地址0x10F
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},

	//电池组状态(0:浮充，1:均充，2:核容)，寄存器地址0x110
	{
		&(g_t_share_data.t_rt_data.t_batt.e_state[0]),                         //pv_val
		BS_TYPE_U8,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//电池组1容量，寄存器地址0x111
	{
		&(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity),    //pv_val
		BS_TYPE_F32,                                                        //u16_type
		10,                                                                 //u16_coeff
	},

	//电池组2容量，寄存器地址0x112
	{
		&(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity),    //pv_val
		BS_TYPE_F32,                                                        //u16_type
		10,                                                                 //u16_coeff
	},

	//母线负对地电压，寄存器地址0x113
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt),    //pv_val
		BS_TYPE_F32,                                                        //u16_type
		10,                                                                 //u16_coeff
	},
};

static DC10_YC_ASSEMBLE_T m_t_yc_ad_assemble[BS_DC10_YC_DATA_CNT] =
{
	//交流1路UV采样AD值，寄存器地址0x400
	{
		&(g_t_share_data.t_ad_data.u16_ac1_uv_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//交流1路VW采样AD值，寄存器地址0x401
	{
		&(g_t_share_data.t_ad_data.u16_ac1_vw_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//交流1路WU采样AD值，寄存器地址0x402
	{
		&(g_t_share_data.t_ad_data.u16_ac1_wu_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//交流2路UV采样AD值，寄存器地址0x403
	{
		&(g_t_share_data.t_ad_data.u16_ac2_uv_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//交流2路VW采样AD值，寄存器地址0x404
	{
		&(g_t_share_data.t_ad_data.u16_ac2_vw_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//交流2路WU采样AD值，寄存器地址0x405
	{
		&(g_t_share_data.t_ad_data.u16_ac2_wu_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//环境温度采样AD值，寄存器地址0x406
	{
		&(g_t_share_data.t_ad_data.u16_temp_ad),                             //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V1采样AD值，寄存器地址0x407
	{
		&(g_t_share_data.t_ad_data.u16_v1_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A1采样AD值，寄存器地址0x408
	{
		&(g_t_share_data.t_ad_data.u16_a1_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V2采样AD值，寄存器地址0x409
	{
		&(g_t_share_data.t_ad_data.u16_v2_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A2采样AD值，寄存器地址0x40A
	{
		&(g_t_share_data.t_ad_data.u16_a2_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V3采样AD值，寄存器地址0x40B
	{
		&(g_t_share_data.t_ad_data.u16_v3_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A3采样AD值，寄存器地址0x40C
	{
		&(g_t_share_data.t_ad_data.u16_a3_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S1采样AD值，监控无此数据，寄存器地址0x40D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S2采样AD值，监控无此数据，寄存器地址0x40E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S3采样AD值，监控无此数据，寄存器地址0x40F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//母线负对地电压采样AD值，监控无此数据，寄存器地址0x410
	{
		&(g_t_share_data.t_ad_data.u16_neg_v_ad),                            //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
};												

static DC10_YX_ASSEMBLE_T m_t_yx_assemble[BS_DC10_YX_CNT] =
{
	//交流一路过压，寄存器地址0x500
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0004,                                                              //u16_mask
	},
	
	//交流一路欠压，寄存器地址0x501
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0002,                                                              //u16_mask
	},
	
	//交流一路缺相，寄存器地址0x502
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0008,                                                              //u16_mask
	},
	
	//交流一路停电，寄存器地址0x503
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//交流二路过压，寄存器地址0x504
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0040,                                                              //u16_mask
	},
	
	//交流二路欠压，寄存器地址0x505
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0020,                                                              //u16_mask
	},
	
	//交流二路缺相，寄存器地址0x506
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0080,                                                              //u16_mask
	},
	
	//交流二路停电，寄存器地址0x507
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0010,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x508
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x509
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x50A
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x50B
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x50C
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x50D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x50E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x50F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//V1路电压过高，控母过压或者母线过压，寄存器地址0x510
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0014,                                                              //u16_mask
	},
	
	//V1路电压过低，控母欠压或者母线欠压，寄存器地址0x511
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0028,                                                              //u16_mask
	},
	
	//A1路电流过高，监控无此数据，寄存器地址0x512
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//A1路分流器异常，监控无此数据，寄存器地址0x513
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//V2路电压过高，电池过压，寄存器地址0x514
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0040,                                                              //u16_mask
	},
	
	//V2路电压过低，电池欠压，寄存器地址0x515
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0080,                                                              //u16_mask
	},
	
	//A2路电流过高，一组电池过流，寄存器地址0x516
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0200,                                                              //u16_mask
	},
	
	//A2路分流器异常，监控无此数据，寄存器地址0x517
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//V3路电压过高，合母过压，寄存器地址0x518
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//V3路电压过低，合母欠压，寄存器地址0x519
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0002,                                                              //u16_mask
	},
	
	//A3路电流过高，二组电池过流，寄存器地址0x51A
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0400,                                                              //u16_mask
	},
	
	//A3路分流器异常，监控无此数据，寄存器地址0x51B
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S1路电流过高，监控无此数据，寄存器地址0x51C
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S1路传感器异常，监控无此数据，寄存器地址0x51D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S2路电流过高，监控无此数据，寄存器地址0x51E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S2路传感器异常，监控无此数据，寄存器地址0x51F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S3路电流过高，监控无此数据，寄存器地址0x520
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S3路传感器异常，监控无此数据，寄存器地址0x521
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x522
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x523
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x524
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x525
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x526
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x527
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x528
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x529
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x52A
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x52B
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x52C
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x52D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x52E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//备用，寄存器地址0x52F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//开关01状态，寄存器地址0x530
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[0]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关02状态，寄存器地址0x531
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[1]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关03状态，寄存器地址0x532
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[2]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关04状态，寄存器地址0x533
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[3]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关05状态，寄存器地址0x534
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[4]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关06状态，寄存器地址0x535
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[5]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关07状态，寄存器地址0x536
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[6]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关08状态，寄存器地址0x537
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[7]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关09状态，寄存器地址0x538
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[8]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关10状态，寄存器地址0x539
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[9]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关11状态，寄存器地址0x53A
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[10]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关12状态，寄存器地址0x53B
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[11]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关13状态，寄存器地址0x53C
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[12]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关14状态，寄存器地址0x53D
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[13]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关15状态，寄存器地址0x53E
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[14]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关16状态，寄存器地址0x53F
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[15]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},

	//开关17状态，寄存器地址0x540
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[16]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关18状态，寄存器地址0x541
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[17]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关19状态，寄存器地址0x542
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[18]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//开关20状态，寄存器地址0x543
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[19]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},

	//一组电池1#巡检模块通信中断，寄存器地址0x544
	{
		&(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_comm_state[0]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},

	//1#馈屏1#馈线模块通信中断，寄存器地址0x545
	{
		&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder_module[0].u8_comm_state),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
};

static DC10_YC_ASSEMBLE_T m_t_yt_set_assemble[BS_DC10_YT_SET_CNT] =
{
	//交流输入方式，需特殊处理，寄存器地址0x1000
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//交流投切模式，需特殊处理，寄存器地址0x1001
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//交流电压过高门限，寄存器地址0x1002
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_high_volt),           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//交流电压过低门限，寄存器地址0x1003
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_low_volt),            //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//交流缺相门限，寄存器地址0x1004
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_lack_phase),          //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V1电压过高门限，控母过压或者母线过压（需特殊处理），寄存器地址0x1005
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt),        //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V1电压过低门限，控母欠压或者母线欠压（需特殊处理），寄存器地址0x1006
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt),        //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V2电压过高门限，电池过压，寄存器地址0x1007
	{
		&(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit),         //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V2电压过低门限，电池欠压，寄存器地址0x1008
	{
		&(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit),          //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V3电压过高门限，合母过压，寄存器地址0x1009
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt),        //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V3电压过低门限，合母欠压，寄存器地址0x100A
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt),         //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A1过流门限，监控无此数据，寄存器地址0x100B
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A2过流门限，电池过流门限值，需特殊处理，寄存器地址0x100C
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A3过流门限，电池过流门限值，需特殊处理，寄存器地址0x100D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A1分流器额定采样电压，需特殊处理，寄存器地址0x100E
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A1分流器额定量程，寄存器地址0x100F
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range),    //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A2分流器额定采样电压，需特殊处理，寄存器地址0x1010
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A2分流器额定量程，寄存器地址0x1011
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt1_shunt_range),    //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A3分流器额定采样电压，需特殊处理，寄存器地址0x1012
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A3分流器额定量程，寄存器地址0x1013
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt2_shunt_range),    //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S1过流门限，监控无此数据，寄存器地址0x1014
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//S2过流门限，监控无此数据，寄存器地址0x1015
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//S3过流门限，监控无此数据，寄存器地址0x1016
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//S1传感器量程，监控无此数据，寄存器地址0x1017
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S2传感器量程，监控无此数据，寄存器地址0x1018
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S3传感器量程，监控无此数据，寄存器地址0x1019
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//备用，监控无此数据，寄存器地址0x101A
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//备用，监控无此数据，寄存器地址0x101B
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//备用，监控无此数据，寄存器地址0x101C
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//备用，监控无此数据，寄存器地址0x101D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//备用，监控无此数据，寄存器地址0x101E
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//备用，监控无此数据，寄存器地址0x101F
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//硅链控制输出电压，寄存器地址0x1020
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt),      //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//硅链规格，寄存器地址0x1021
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl),       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//系统故障继电器控制，寄存器地址0x1022
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay),                      //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器01控制，寄存器地址0x1023
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[0]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器02控制，寄存器地址0x1024
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[1]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器03控制，寄存器地址0x1025
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[2]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器04控制，寄存器地址0x1026
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[3]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器05控制，寄存器地址0x1027
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[4]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器06控制，寄存器地址0x1028
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[5]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器07控制，监控无此数据，寄存器地址0x1029
	{
		NULL,                                                                //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//输出继电器08控制，监控无此数据，寄存器地址0x102A
	{
		NULL,                                                                //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
};

static DC10_YC_ASSEMBLE_T m_t_yt_adjust_assemble[BS_DC10_YT_ADJUST_CNT] =
{
	//交流1路UV斜率，寄存器地址0x1100
	{
		&(g_t_share_data.t_coeff_data.f32_ac1_uv_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//交流1路VW斜率，寄存器地址0x1101
	{
		&(g_t_share_data.t_coeff_data.f32_ac1_vw_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//交流1路WU斜率，寄存器地址0x1102
	{
		&(g_t_share_data.t_coeff_data.f32_ac1_wu_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//交流2路UV斜率，寄存器地址0x1103
	{
		&(g_t_share_data.t_coeff_data.f32_ac2_uv_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//交流2路VW斜率，寄存器地址0x1104
	{
		&(g_t_share_data.t_coeff_data.f32_ac2_vw_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//交流2路WU斜率，寄存器地址0x1105
	{
		&(g_t_share_data.t_coeff_data.f32_ac2_wu_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V1电压测量斜率，寄存器地址0x1106
	{
		&(g_t_share_data.t_coeff_data.f32_v1_vol_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V1电压测量零点，寄存器地址0x1107
	{
		&(g_t_share_data.t_coeff_data.s16_v1_vol_zero),                      //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V2电压测量斜率，寄存器地址0x1108
	{
		&(g_t_share_data.t_coeff_data.f32_v2_vol_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V2电压测量零点，寄存器地址0x1109
	{
		&(g_t_share_data.t_coeff_data.s16_v2_vol_zero),                      //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V3电压测量斜率，寄存器地址0x110A
	{
		&(g_t_share_data.t_coeff_data.f32_v3_vol_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V3电压测量零点，寄存器地址0x110B
	{
		&(g_t_share_data.t_coeff_data.s16_v3_vol_zero),                      //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A1电路固定零点，寄存器地址0x110C
	{
		&(g_t_share_data.t_coeff_data.s16_a1_fixed_zero),                    //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A2电路固定零点，寄存器地址0x110D
	{
		&(g_t_share_data.t_coeff_data.s16_a2_fixed_zero),                    //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A3电路固定零点，寄存器地址0x110E
	{
		&(g_t_share_data.t_coeff_data.s16_a3_fixed_zero),                    //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A1电流测量斜率，寄存器地址0x110F
	{
		&(g_t_share_data.t_coeff_data.f32_a1_curr_slope),                    //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//A1电流测量零点，寄存器地址0x1110
	{
		&(g_t_share_data.t_coeff_data.f32_a1_curr_zero),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//A2电流测量斜率，寄存器地址0x1111
	{
		&(g_t_share_data.t_coeff_data.f32_a2_curr_slope),                    //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//A2电流测量零点，寄存器地址0x1112
	{
		&(g_t_share_data.t_coeff_data.f32_a2_curr_zero),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//A3电流测量斜率，寄存器地址0x1113
	{
		&(g_t_share_data.t_coeff_data.f32_a3_curr_slope),                    //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//A3电流测量零点，寄存器地址0x1114
	{
		&(g_t_share_data.t_coeff_data.f32_a3_curr_zero),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//AD测量基准电压，寄存器地址0x1115
	{
		&(g_t_share_data.t_coeff_data.f32_ref_volt),                         //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1000,                                                                //u16_coeff
	},

	//S1电路固定零点，监控无此数据，寄存器地址0x1116
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S2电路固定零点，监控无此数据，寄存器地址0x1117
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S3电路固定零点，监控无此数据，寄存器地址0x1118
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S1电流测量斜率，监控无此数据，寄存器地址0x1119
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//S1电流测量零点，监控无此数据，寄存器地址0x111A
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//S2电流测量斜率，监控无此数据，寄存器地址0x111B
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//S2电流测量零点，监控无此数据，寄存器地址0x111C
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//S3电流测量斜率，监控无此数据，寄存器地址0x111D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//S3电流测量零点，监控无此数据，寄存器地址0x111E
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//母线负对地电压测量斜率，寄存器地址0x111F
	{
		&(g_t_share_data.t_coeff_data.f32_neg_vol_slope),                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//母线负对地电压测量零点，寄存器地址0x1120
	{
		&(g_t_share_data.t_coeff_data.s16_neg_vol_zero),                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
};


/*************************************************************
函数名称: v_internal_dc10_handle		           				
函数功能: DC10协议处理函数						
输入参数: u8_code         -- 功能码
          u16_reg         -- 寄存器地址
          u16_cnt_or_data -- 寄存器数量或者设置数据     		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_internal_dc10_handle(U8_T u8_code, U16_T u16_reg, U16_T u16_cnt_or_data)
{
	U8_T u8_reg_index, u8_change_flag;
	U16_T u16_cnt, u16_len, u16_crc, i;
	S16_T s16_data;
		
	u8_reg_index = (u16_reg & BS_DC10_INDEX_MASK);
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
		
	if (u8_code == MODBUS_FUNC_CODE_04)         //读遥测量
	{
		DC10_YC_ASSEMBLE_T *pt_yc_assemble = NULL;
		U16_T u16_yc_cnt;

		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YC_DATA_OFFSET) && (u8_reg_index < BS_DC10_YC_DATA_CNT))
		{
			pt_yc_assemble = m_t_yc_data_assemble;
			u16_yc_cnt = BS_DC10_YC_DATA_CNT;
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YC_AD_OFFSET) && (u8_reg_index < BS_DC10_YC_AD_CNT))
		{
			pt_yc_assemble = m_t_yc_ad_assemble;
			u16_yc_cnt = BS_DC10_YC_AD_CNT;
		}
		else
		{
			goto OUT;
		}

		if (u16_yc_cnt - u8_reg_index < u16_cnt_or_data)
			u16_cnt = u16_yc_cnt - u8_reg_index;
		else
			u16_cnt = u16_cnt_or_data;
			
		memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
		g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
		g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_04;
		g_u8_bs_send_buf[2] = (U8_T)(u16_cnt*2);
		
		for (i=0; i<u16_cnt; i++)
		{
			if (pt_yc_assemble[u8_reg_index+i].pv_val == NULL)
				s16_data = 0;
			else if (pt_yc_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
				s16_data = (S16_T)((*(U8_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			else if (pt_yc_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
				s16_data = (S16_T)((*(U16_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			else if (pt_yc_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
				s16_data = (S16_T)((*(S16_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			else
				s16_data = (S16_T)((*(F32_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			
			g_u8_bs_send_buf[3+i*2] = (U8_T)(s16_data>>8);
			g_u8_bs_send_buf[4+i*2] = (U8_T)s16_data;
		}
	
		u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 3+u16_cnt*2);
    	g_u8_bs_send_buf[3+u16_cnt*2] = (U8_T)u16_crc;
		g_u8_bs_send_buf[4+u16_cnt*2] = (U8_T)(u16_crc >> 8);
		
		u16_len = 5+u16_cnt*2;
	}
	else if (u8_code == MODBUS_FUNC_CODE_02)    //读遥信量
	{
		U16_T u16_yx_data[(BS_DC10_YX_CNT+15)/16];
		U16_T data;
		U32_T i;
		U8_T  word_index, bit_index, byte_cnt, rem_bit_cnt;
		
		if (((u16_reg & BS_DC10_ADDR_MASK) != BS_DC10_YX_OFFSET) ||
			(u8_reg_index >= BS_DC10_YX_CNT))
			goto OUT;
			
		memset(u16_yx_data, 0, sizeof(u16_yx_data));
		for (i=0; i<BS_DC10_YX_CNT; i++)
		{
			if (m_t_yx_assemble[i].pv_val == NULL)
				continue;

			if (m_t_yx_assemble[i].u16_type == BS_TYPE_U8)
			{
				if (((*(U8_T *)(m_t_yx_assemble[i].pv_val)) & (U8_T)(m_t_yx_assemble[i].u16_mask)) != 0)
					u16_yx_data[i/16] |= (1 << (i%16));
			}
			else if (m_t_yx_assemble[i].u16_type == BS_TYPE_U16)
			{
				if (((*(U16_T *)(m_t_yx_assemble[i].pv_val)) & m_t_yx_assemble[i].u16_mask) != 0)
					u16_yx_data[i/16] |= (1 << (i%16));
			}
		}

		if (BS_DC10_YX_CNT - u8_reg_index < u16_cnt_or_data)
			u16_cnt = BS_DC10_YX_CNT - u8_reg_index;
		else
			u16_cnt = u16_cnt_or_data;
			
		word_index = u8_reg_index / 16;
    	bit_index  = u8_reg_index % 16;
    	byte_cnt    = u16_cnt / 8;
		rem_bit_cnt = u16_cnt % 8;
		
    	if (rem_bit_cnt > 0)
			byte_cnt++;
		
		memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
		g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
		g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_02;
		g_u8_bs_send_buf[2] = byte_cnt;	
    	
		for (i=0; i<byte_cnt; i++)
		{
    	    data = u16_yx_data[word_index];
    	    g_u8_bs_send_buf[3+i] = (U8_T)(data>>bit_index);
    	
    	    if (bit_index > 8)  //不够8位数据部分提取
    	    {
    	        data = u16_yx_data[++word_index];
    	        g_u8_bs_send_buf[3+i] |= (U8_T)(data<<(16-bit_index));
    	    }
    	    else if (bit_index==8)
    	        ++word_index;
    	
    	    bit_index = ((bit_index+8)%16);
		}
		
		if (rem_bit_cnt > 0)
			g_u8_bs_send_buf[2+byte_cnt] &= ((1<<rem_bit_cnt) - 1);
			
		u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, byte_cnt+3);
		g_u8_bs_send_buf[byte_cnt+3] = (U8_T)u16_crc;
		g_u8_bs_send_buf[byte_cnt+4] = (U8_T)(u16_crc >> 8);
		
		u16_len = byte_cnt+5;
	}
	else if (u8_code == MODBUS_FUNC_CODE_03)    //读设置寄存器
	{
		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_SET_OFFSET) && (u8_reg_index < BS_DC10_YT_SET_CNT))
		{
			if (BS_DC10_YT_SET_CNT - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_SET_CNT - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_03;
			g_u8_bs_send_buf[2] = (U8_T)(u16_cnt*2);
		
			for (i=0; i<u16_cnt; i++)
			{
				if ((u8_reg_index+i) == BS_DC10_AC_INPUT_MODE_INDEX)
				{
					s16_data = ((g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path << 8) |
								g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase);
				}
				else if ((u8_reg_index+i) == BS_DC10_AC_CTRL_MODE_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac == 1)
						s16_data = 2;
					else if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac == 2)
						s16_data = 1;
					else
						s16_data = g_t_share_data.t_sys_cfg.t_ctl.u16_ac;
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_HIGH_VOLT_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt * 10);
					else
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt * 10);
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_LOW_VOLT_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt * 10);
					else
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt * 10);
				}
				else if (((u8_reg_index+i) == BS_DC10_A2_OVER_CURR_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A3_OVER_CURR_INDEX))
				{
					s16_data = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10;
				}
				else if (((u8_reg_index+i) == BS_DC10_A1_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A2_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A3_SHUNT_VOLT_INDEX))
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt == VOLT_75MV)
						s16_data = 75;
					else
						s16_data = 50;
				}
				else
				{
					if (m_t_yt_set_assemble[u8_reg_index+i].pv_val == NULL)
						s16_data = 0;
					else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
						s16_data = (S16_T)((*(U8_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
					else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
						s16_data = (S16_T)((*(U16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
					else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
						s16_data = (S16_T)((*(S16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
					else
						s16_data = (S16_T)((*(F32_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
				}
			
				g_u8_bs_send_buf[3+i*2] = (U8_T)(s16_data>>8);
				g_u8_bs_send_buf[4+i*2] = (U8_T)s16_data;
			}
	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 3+u16_cnt*2);
    		g_u8_bs_send_buf[3+u16_cnt*2] = (U8_T)u16_crc;
			g_u8_bs_send_buf[4+u16_cnt*2] = (U8_T)(u16_crc >> 8);
		
			u16_len = 5+u16_cnt*2;
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_ADJUST_OFFSET) && (u8_reg_index < BS_DC10_YT_ADJUST_CNT))
		{
			if (BS_DC10_YT_ADJUST_CNT - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_ADJUST_CNT - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_03;
			g_u8_bs_send_buf[2] = (U8_T)(u16_cnt*2);
		
			for (i=0; i<u16_cnt; i++)
			{
				if (m_t_yt_adjust_assemble[u8_reg_index+i].pv_val == NULL)
					s16_data = 0;
				else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
					s16_data = (S16_T)((*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
				else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
					s16_data = (S16_T)((*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
				else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
					s16_data = (S16_T)((*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
				else
					s16_data = (S16_T)((*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
			
				g_u8_bs_send_buf[3+i*2] = (U8_T)(s16_data>>8);
				g_u8_bs_send_buf[4+i*2] = (U8_T)s16_data;
			}
	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 3+u16_cnt*2);
    		g_u8_bs_send_buf[3+u16_cnt*2] = (U8_T)u16_crc;
			g_u8_bs_send_buf[4+u16_cnt*2] = (U8_T)(u16_crc >> 8);
		
			u16_len = 5+u16_cnt*2;
		}
		else
		{
			goto OUT;
		}
	}
	else if (u8_code == MODBUS_FUNC_CODE_06)    //设置单个数据
	{
		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_SET_OFFSET) && (u8_reg_index < BS_DC10_YT_SET_CNT))
		{
			u8_change_flag = 0;
			
			if (u8_reg_index == BS_DC10_AC_INPUT_MODE_INDEX)
			{
				if (((u16_cnt_or_data>>8) <= 2) && ((u16_cnt_or_data&0x00FF) <= 1))
				{
					if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path != (AC_INPUT_NUM_E)(u16_cnt_or_data>>8)) ||
						(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase != (AC_INPUT_TYPE_E)u16_cnt_or_data))
					{
						u8_change_flag = 1;
					}

					g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = (AC_INPUT_NUM_E)(u16_cnt_or_data>>8);
					g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase = (AC_INPUT_TYPE_E)u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_AC_CTRL_MODE_INDEX)
			{
				if (u16_cnt_or_data == 1)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 2)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 2;
				}
				else if (u16_cnt_or_data == 2)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 1)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 1;
				}
				else if (u16_cnt_or_data <= 3)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != u16_cnt_or_data)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_ctl.u16_ac = u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_V1_HIGH_VOLT_INDEX)
			{
				if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt = u16_cnt_or_data / 10;
				}
				else
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt = u16_cnt_or_data / 10;
				}
			}
			else if (u8_reg_index == BS_DC10_V1_LOW_VOLT_INDEX)
			{	
				if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt = u16_cnt_or_data / 10;
				}
				else
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt = u16_cnt_or_data / 10;
				}
			}
			else if ((u8_reg_index == BS_DC10_A2_OVER_CURR_INDEX) ||
					 (u8_reg_index == BS_DC10_A3_OVER_CURR_INDEX))
			{
				if (fabs(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit-((F32_T)((S16_T)u16_cnt_or_data)) / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10)) > 0.000001)
					u8_change_flag = 1;
				
				g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit = ((F32_T)((S16_T)u16_cnt_or_data)) / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10);
			}
			else if ((u8_reg_index == BS_DC10_A1_SHUNT_VOLT_INDEX) ||
					 (u8_reg_index == BS_DC10_A2_SHUNT_VOLT_INDEX) ||
					 (u8_reg_index == BS_DC10_A3_SHUNT_VOLT_INDEX))
			{
				if (u16_cnt_or_data == 75)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_75MV)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_75MV;
				}
				else if (u16_cnt_or_data == 50)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_50MV)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_50MV;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_DIODE_CHAIN_INDEX)
			{
				if (u16_cnt_or_data <= 4)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != (DIODE_CHAIN_E)u16_cnt_or_data)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl = (DIODE_CHAIN_E)u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_RELAY_START_INDEX)
			{
				if (u16_cnt_or_data == 0)
				{
					v_relay_relay_operation(SYS_FIAL_ON);
					g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = u16_cnt_or_data;
				}
				else if (u16_cnt_or_data == 1)
				{
					v_relay_relay_operation(SYS_FIAL_OFF);
					g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if ((u8_reg_index > BS_DC10_RELAY_START_INDEX) &&
					 (u8_reg_index < (BS_DC10_YT_SET_CNT - 2)))
			{
				if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != NO_DIODE_CHAIN)
					&& (((u8_reg_index-BS_DC10_RELAY_START_INDEX) == 4) || 
						((u8_reg_index-BS_DC10_RELAY_START_INDEX) == 5) ||
						((u8_reg_index-BS_DC10_RELAY_START_INDEX) == 6)))
				{
					goto OUT;
				}
        	
				if (u16_cnt_or_data == 0)
				{
					RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_OFF, ALARM_RELAY2_OFF, ALARM_RELAY3_OFF, ALARM_RELAY4_OFF,
												ALARM_RELAY5_OFF, ALARM_RELAY6_OFF };
	    	                   
					v_relay_relay_operation(e_cmd[u8_reg_index-BS_DC10_RELAY_START_INDEX-1]);
					g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index-BS_DC10_RELAY_START_INDEX-1] = u16_cnt_or_data;
				}
				else if (u16_cnt_or_data == 1)
				{
					RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_ON, ALARM_RELAY2_ON, ALARM_RELAY3_ON, ALARM_RELAY4_ON,
												ALARM_RELAY5_ON, ALARM_RELAY6_ON };
	    	                   
					v_relay_relay_operation(e_cmd[u8_reg_index-BS_DC10_RELAY_START_INDEX-1]);
					g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index-BS_DC10_RELAY_START_INDEX-1] = u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else
			{
				if (m_t_yt_set_assemble[u8_reg_index].pv_val != NULL)
				{
					if (m_t_yt_set_assemble[u8_reg_index].u16_type == BS_TYPE_U8)
					{
						if (*(U8_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) != (U8_T)(u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff))
							u8_change_flag = 1;
						
						*(U8_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = (U8_T)(u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff);
					}
					else if (m_t_yt_set_assemble[u8_reg_index].u16_type == BS_TYPE_U16)
					{
						if (*(U16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) != u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff)
							u8_change_flag = 1;
						
						*(U16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff;
					}
					else if (m_t_yt_set_assemble[u8_reg_index].u16_type == BS_TYPE_S16)
					{
						if (*(S16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) != (S16_T)u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff)
							u8_change_flag = 1;
						
						*(S16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = (S16_T)u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff;
					}
					else
					{
						if (fabs(*(F32_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) - ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_set_assemble[u8_reg_index].u16_coeff) > 0.000001)
							u8_change_flag = 1;
						
						*(F32_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_set_assemble[u8_reg_index].u16_coeff;
					}
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_cfg_data();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_06;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8;
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_ADJUST_OFFSET) && (u8_reg_index < BS_DC10_YT_ADJUST_CNT))
		{
			if (m_t_yt_adjust_assemble[u8_reg_index].pv_val != NULL)
			{
				if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_U8)
				{
					if (*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != (U8_T)(u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff))
						u8_change_flag = 1;
					
					*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = (U8_T)(u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff);
				}
				else if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_U16)
				{
					if (*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff)
						u8_change_flag = 1;
					
					*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
				}
				else if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_S16)
				{
					if (*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != (S16_T)u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff)
						u8_change_flag = 1;
					
					*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = (S16_T)u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
				}
				else
				{
					if (fabs(*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) - ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff) > 0.000001)
						u8_change_flag = 1;
					
					*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_adjust_coeff();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_06;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8;
		}
		else
		{
			goto OUT;
		}
	}
	else if (u8_code == MODBUS_FUNC_CODE_16)    //设置多个数据
	{
		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_SET_OFFSET) && (u8_reg_index < BS_DC10_YT_SET_CNT))
		{	
			if (BS_DC10_YT_SET_OFFSET - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_SET_OFFSET - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			u8_change_flag = 0;
			
			for (i=0; i<u16_cnt; i++)
			{
				s16_data = (S16_T)((g_u8_bs_recv_buf[7+i*2]<<8) + g_u8_bs_recv_buf[8+i*2]);
				
				if ((u8_reg_index+i) == BS_DC10_AC_INPUT_MODE_INDEX)
				{
					if (((s16_data>>8) <= 2) && ((s16_data&0x00FF) <= 1))
					{
						if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path != (AC_INPUT_NUM_E)(s16_data>>8)) ||
							(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase != (AC_INPUT_TYPE_E)s16_data))
						{
							u8_change_flag = 1;
						}
							
						g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = (AC_INPUT_NUM_E)(s16_data>>8);
						g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase = (AC_INPUT_TYPE_E)s16_data;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_AC_CTRL_MODE_INDEX)
				{
					if (s16_data == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 2)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 2;
					}
					else if (s16_data == 2)
					{
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 1)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 1;
					}
					else if (s16_data <= 3)
					{
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != s16_data)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_ctl.u16_ac = s16_data;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_HIGH_VOLT_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt = s16_data / 10;
					}
					else
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt = s16_data / 10;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_LOW_VOLT_INDEX)
				{	
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt = s16_data / 10;
					}
					else
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt = s16_data / 10;
					}
				}
				else if (((u8_reg_index+i) == BS_DC10_A2_OVER_CURR_INDEX) ||
					 	 ((u8_reg_index+i) == BS_DC10_A3_OVER_CURR_INDEX))
				{
					if (fabs(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit - (F32_T)s16_data / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10)) > 0.0001)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit = (F32_T)s16_data / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10);
				}
				else if (((u8_reg_index+i) == BS_DC10_A1_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A2_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A3_SHUNT_VOLT_INDEX))
				{
					if (s16_data == 75)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_75MV)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_75MV;
					}
					else if (s16_data == 50)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_50MV)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_50MV;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_DIODE_CHAIN_INDEX)
				{
					if (s16_data <= 4)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != (DIODE_CHAIN_E)s16_data)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl = (DIODE_CHAIN_E)s16_data;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_RELAY_START_INDEX)
				{
					if (s16_data == 0)
					{
						v_relay_relay_operation(SYS_FIAL_ON);
						g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = s16_data;
					}
					else if (s16_data == 1)
					{
						v_relay_relay_operation(SYS_FIAL_OFF);
						g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = s16_data;
					}
				}
				else if (((u8_reg_index+i) > BS_DC10_RELAY_START_INDEX) &&
						 ((u8_reg_index+i) < (BS_DC10_YT_SET_CNT - 2)))
				{
					if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != NO_DIODE_CHAIN)
					&& (((u8_reg_index+i-BS_DC10_RELAY_START_INDEX) == 4) || 
						((u8_reg_index+i-BS_DC10_RELAY_START_INDEX) == 5) ||
						((u8_reg_index+i-BS_DC10_RELAY_START_INDEX) == 6)))
					{
						continue;
					}
        	
					if (s16_data == 0)
					{
						RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_OFF, ALARM_RELAY2_OFF, ALARM_RELAY3_OFF, ALARM_RELAY4_OFF,
												ALARM_RELAY5_OFF, ALARM_RELAY6_OFF };
	    	                   
						v_relay_relay_operation(e_cmd[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1]);
						g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1] = s16_data;
					}
					else if (s16_data == 1)
					{
						RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_ON, ALARM_RELAY2_ON, ALARM_RELAY3_ON, ALARM_RELAY4_ON,
												ALARM_RELAY5_ON, ALARM_RELAY6_ON };
	    	                   
						v_relay_relay_operation(e_cmd[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1]);
						g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1] = s16_data;
					}
				}
				else
				{
					if (m_t_yt_set_assemble[u8_reg_index+i].pv_val != NULL)
					{
						if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
						{
							if (*(U8_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) != (U8_T)(s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff))
								u8_change_flag = 1;
							
							*(U8_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) = (U8_T)(s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
						}
						else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
						{
							if (*(U16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) != s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff)
								u8_change_flag = 1;
							
							*(U16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) = s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff;
						}
						else if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_S16)
						{
							if (*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != s16_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff)
								u8_change_flag = 1;
					
							*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = s16_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
						}
						else
						{
							if (fabs(*(F32_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) - ((F32_T)s16_data) / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff) > 0.000001)
								u8_change_flag = 1;
							
							*(F32_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) = ((F32_T)s16_data) / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff;
						}
					}
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_cfg_data();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_16;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8; 
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_ADJUST_OFFSET) && (u8_reg_index < BS_DC10_YT_ADJUST_CNT))
		{
			if (BS_DC10_YT_ADJUST_CNT - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_ADJUST_CNT - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			u8_change_flag = 0;
			
			for (i=0; i<u16_cnt; i++)
			{
				s16_data = (S16_T)((g_u8_bs_recv_buf[7+i*2]<<8) + g_u8_bs_recv_buf[8+i*2]);

				if (m_t_yt_adjust_assemble[u8_reg_index+i].pv_val != NULL)
				{
					if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
					{
						if (*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) != (U8_T)(s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff))
							u8_change_flag = 1;
						
						*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = (U8_T)(s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
					}
					else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
					{
						if (*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) != s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff)
							u8_change_flag = 1;
						
						*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff;
					}
					else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
					{
						if (*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) != s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff)
							u8_change_flag = 1;
						
						*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff;
					}
					else
					{
						if (fabs(*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) - ((F32_T)s16_data) / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff) > 0.000001)
							u8_change_flag = 1;
						
						*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = ((F32_T)s16_data) / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff;
					}
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_adjust_coeff();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_16;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8; 
		}
		else
		{
			goto OUT;
		}
	}
	else
	{
		goto OUT;
	}
	
	os_mut_release(g_mut_share_data);
	
	v_bs_send_data(g_u8_bs_send_buf, u16_len);
	return;

OUT:
	os_mut_release(g_mut_share_data);	
}

/*************************************************************
函数名称: v_internal_run		           				
函数功能: 内部modubs协议运行函数						
输入参数: 无     		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_internal_run(void)
{
	U32_T i, h;
	S32_T cnt = 0;
	U16_T u16_reg_addr, u16_reg_cnt_or_data;
	U8_T  u8_func_code;

	cnt = Uart_recv(BS_COM_PORT_NUM, g_u8_bs_recv_buf+g_u8_bs_recv_len, BS_RECV_BUF_SIZE-g_u8_bs_recv_len);
	if (cnt == -1)
		return;
			
	g_u8_bs_recv_len += cnt;	
	h = 0;
	
	//查找头尾标志以便取得一完整的数据包
	while (g_u8_bs_recv_len >= MODBUS_BYTE_SIZE)
	{
		/*地址不相符*/
		if(g_u8_bs_recv_buf[h] != (DC10_START_ADDR + g_u8_addr - 1))		/*地址不相符，头指针前移一个位置*/
		{
			h++;
			g_u8_bs_recv_len--;
			continue;
		}
		
		//功能码不相符，头指针前移一个位置
		if ((g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_02) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_04) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_03) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_06) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_16))
		{
			h++;
			g_u8_bs_recv_len--;
			continue;
		}		
	
		//比较校验码
		if (g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_16)
		{
			if (u16_crc_calculate_crc(&g_u8_bs_recv_buf[h], MODBUS_BYTE_SIZE) != 0)
			{
				h++;
				g_u8_bs_recv_len--;
				continue;
			}
		}
		else
		{
			//只有DC10有16功能码，16功能码需要加一些限制条件，地址高位为0x10、字节数等于寄存器个数乘以2、字节数小于86
			if (((g_u8_bs_recv_buf[h+2] != 0x10) && (g_u8_bs_recv_buf[h+2] != 0x11)) ||
				(((g_u8_bs_recv_buf[h+4]<<8)+g_u8_bs_recv_buf[h+5])*2 != g_u8_bs_recv_buf[h+6]) ||
				(g_u8_bs_recv_buf[h+6] > 86))
			{
				h++;
				g_u8_bs_recv_len--;
				continue;
			}
				
			if ((g_u8_bs_recv_buf[h+6]+9) > g_u8_bs_recv_len)
			{
				if (h > 0)
				{
					for (i=0; i<g_u8_bs_recv_len; i++)
						g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
				}
					
				break;
			}
			else if (u16_crc_calculate_crc(&g_u8_bs_recv_buf[h], g_u8_bs_recv_buf[h+6]+9) != 0)
			{
				h++;
				g_u8_bs_recv_len--;
				continue;
			}
		}
			
		if (h > 0)
		{
			for (i=0; i<g_u8_bs_recv_len; i++)
				g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
			
			h = 0;
		}

		u8_func_code = g_u8_bs_recv_buf[1];
		u16_reg_addr = (g_u8_bs_recv_buf[2]<<8) + g_u8_bs_recv_buf[3];
		u16_reg_cnt_or_data = (g_u8_bs_recv_buf[4]<<8) + g_u8_bs_recv_buf[5];
		DPRINT("modbus, u8_func_code=%x, u16_reg_addr=%x, cnt=%x\n", u8_func_code, u16_reg_addr, u16_reg_cnt);
		
		//设为校准协议时调用	
		v_internal_dc10_handle(u8_func_code, u16_reg_addr, u16_reg_cnt_or_data);
					
		if (u8_func_code != MODBUS_FUNC_CODE_16)
		{
			h += MODBUS_BYTE_SIZE;
			g_u8_bs_recv_len -= MODBUS_BYTE_SIZE;
		}
		else
		{
			h += (g_u8_bs_recv_buf[6]+9);
			g_u8_bs_recv_len -= (g_u8_bs_recv_buf[6]+9);
		}
			
		if (h > 0)
		{
			for (i=0; i<g_u8_bs_recv_len; i++)
				g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
				
			h = 0;
		}
	}

	if (h > 0)
	{
		for (i=0; i<g_u8_bs_recv_len; i++)
			g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
	}
}

