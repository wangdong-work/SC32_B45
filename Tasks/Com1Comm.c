/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Com1Comm.c
版    本：1.00
创建日期：2012-08-01
作    者：郭数理
功能描述：下级模块的通信及数据处理任务实现


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-08-01  1.00     创建
**************************************************************/


#include <rtl.h>
#include <string.h>
#include <math.h>

#include "Type.h"
#include "PublicData.h"
#include "Com1Comm.h"
#include "Crc.h"
#include "FaultId.h"

#include "../Drivers/uart_device.h"
#include "../Drivers/Delay.h"


//#define COM1_DEBUG

#ifdef COM1_DEBUG
	#define DEBUG(fmt, ...) u32_usb_debug_print(fmt, ##__VA_ARGS__)
#else
	#define DEBUG(fmt, ...)
#endif

#define COM1_RC10_CTRL_SPACE_TM     (2*OSC_SECOND)   //2s电操控制命令间隔时间
#define COM1_SWT_SYNC_UPDATE_TM     (30*OSC_SECOND)  //10s电操控制命令间隔时间

#define COM1_RECV_WAIT_TICK         5      //每次等待5个TICK，也就是50ms
#define COM1_RECV_TIMEOUT_CNT       20     //超时计数最大值为20，超时值为1s

#define COM1_COMM_WAIT_TICK         10     //如果串口缓冲区写不进去数据，等待100MS后重试
#define COM1_BROADCAST_WAIT_TICK    30     //发送广播命令等待300MS，然后发送下一条命令

//#define COM1_FAIL_MAX_NUM           10     //连续10次通信不上，报通信故障
#define COM1_COM_PORT_NUM           1      //串口端口号

#define COM1_RX_BUF_SIZE            256    //缓冲区的大小
#define COM1_TX_BUF_SIZE            56     //缓冲区的大小

#define COM1_GROUP1_RECT			0		//第一组充电模块
#define COM1_GROUP2_RECT			1		//第二组充电模块

/* 发送和接收缓冲区定义 */
static U8_T m_u8_com1_rx_buf[COM1_RX_BUF_SIZE];
static U8_T m_u8_com1_tx_buf[COM1_TX_BUF_SIZE];



/* 命令定义 */
typedef struct
{
	U8_T u8_func_code;          //功能码，03--查询数据，06--设置数据
	U16_T u16_reg_addr;         //寄存器地址，高字节先发
	U16_T u16_data_or_num;      //功能码为03时，表示查询的寄存器个数
	                            //功能码为06时，表示设置的数据
}MODULE_CMD_T;

/* 模块数据记录 */
typedef struct
{
	U8_T u8_module_index;       //模块索引
	U8_T u8_cmd_index;          //命令索引
	U8_T u8_com1_fail_cnt;      //通信失败记数，失败次数超过10次报通信中断故障
}MODULE_RECORD_T;



/******************************* 电池巡检相关定义开始 ***************************/

typedef struct
{
	U8_T u8_total_index;        //在两组所有电池巡检中的索引
	U8_T u8_start_addr;         //电池组内电池巡检起始地址
	U8_T u8_group_index;        //电池组索引
	U8_T u8_module_index;       //模块索引
	U8_T u8_cmd_index;          //命令索引
	U8_T u8_com1_fail_cnt;      //通信失败记数，失败次数超过10次报通信中断故障
}BMS_RECORD_T;
	
#define BMS_B21_MAX_CELL_NUM              24     //B21测量的最多电池节数
#define BMS_START_ADDR                    0x70   //电池巡检起始地址
#define BMS_READ_CMD                      0      //查询数据命令索引
#define BMS_CMD_CNT                       1      //电池巡检命令的个数


static U8_T m_u8_bms_offline_cnt;          // 电池巡检通讯中断命令个数

static MODULE_CMD_T m_t_bms_cmd[BMS_CMD_CNT] =
{
	{ 3, 0, 114 },   //查询电池数据，114是最大寄存器数据量，可按实际配置，相应的减小
};

static BMS_RECORD_T m_t_bms_group1_record[BATT_METER_MAX] =
{
	{ 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 1, 0, 0 },
	{ 2, 0, 0, 2, 0, 0 },
	{ 3, 0, 0, 3, 0, 0 },
	{ 4, 0, 0, 4, 0, 0 },
};

static BMS_RECORD_T m_t_bms_group2_record[BATT_METER_MAX] =
{
	{ 5, 5, 1, 0, 0, 0 },
	{ 6, 5, 1, 1, 0, 0 },
	{ 7, 5, 1, 2, 0, 0 },
	{ 8, 5, 1, 3, 0, 0 },
	{ 9, 5, 1, 4, 0, 0 }
};
/******************************* 电池巡检相关定义结束 ***************************/


/******************************* 整流模块相关定义开始 ***************************/
#define RECT_MODULE_START_ADDR             0x00   //整流模块起始地址
#define RECT_MODULE_BROADCAST_ADDR         0xFF   //一组整流模块广播地址
                                           
#define RECT_MODULE_READ_CMD               0      //整流模块查询数据命令索引
#define RECT_MODULE_SET_STATE_CMD          1      //整流模块设置模块状态命令索引
#define RECT_MODULE_NORMAL_CMD_CNT         2      //整流模块命令的个数

#define RECT_MODULE_SET_VOLT_CMD           0      //整流模块调压命令索引，广播命令
#define RECT_MODULE_SET_CURR_CMD           1      //整流模块限流命令索引，广播命令
#define RECT_MODULE_SET_HIGH_VOLT_CMD      2      //整流模块设置模块输出电压上限，广播命令
#define RECT_MODULE_SET_LOW_VOLT_CMD       3      //整流模块设置模块输出电压下限，广播命令
#define RECT_MODULE_SET_DEF_VOLT_CMD       4      //整流模块设置默认输出电压命令索引，广播命令       
#define RECT_MODULE_BROADCAST_CMD_CNT      5      //整流模块命令的个数

#define RECT_MODULE_OVER_VOLT_PROTECT_MASK 0x8000 //整流模块过压保护状态屏蔽码
#define RECT_MODULE_FAULT_MASK             0x041C //整流模块故障状态屏蔽码，包含输出欠压状态、软件过压状态位、模块故障状态、保护状态位
#define RECT_MODULE_VOER_TEMPERATURE_MASK  0x0800 //整流模块过温状态屏蔽码
#define RECT_MODULE_AC_EXCEPTION_MASK      0x3000 //整流模块交流异常屏蔽码
#define RECT_MODULE_EXCEPTION_MASK         (RECT_MODULE_OVER_VOLT_PROTECT_MASK | RECT_MODULE_FAULT_MASK \
											| RECT_MODULE_VOER_TEMPERATURE_MASK | RECT_MODULE_AC_EXCEPTION_MASK)


static U8_T m_u8_rect_offline_cnt;         // 整流模块通讯中断命令个数

static MODULE_CMD_T m_t_rect_module_normal_cmd[RECT_MODULE_NORMAL_CMD_CNT] = 
{
	{ 3, 0, 6    },              //查询数据命令
	{ 6, 5, 0    },              //设置模块开关状态量命令
};

static MODULE_CMD_T m_t_rect_module_broadcast_cmd[RECT_MODULE_BROADCAST_CMD_CNT] =
{
	{ 6, 0, 2200 },              //设置模块输出电压，广播命令，默认220V，系数为0.1，220/0.1=2200
	{ 6, 2, 1100 },              //设置模块限流点，广播命令，范围5%~110%，默认110%，系数为0.1，110/0.1=1100
	{ 6, 3, 2860 },              //设置模块输出电压上限，广播命令，默认286V，系数为0.1，286/0.1=2860
	{ 6, 4, 1870 },              //设置模块输出电压下限，广播命令，默认187V，系数为0.1，187/0.1=1870
	{ 6, 6, 2200 },              //设置默认输出电压命令，广播命令，默认220V，系数为0.1，220/0.1=2200
};

static MODULE_RECORD_T m_t_rect_module_record[RECT_CNT_MAX] =
{
	{ 0,  0, 0 },
	{ 1,  0, 0 },
	{ 2,  0, 0 },
	{ 3,  0, 0 },
	{ 4,  0, 0 },
	{ 5,  0, 0 },
	{ 6,  0, 0 },
	{ 7,  0, 0 },
	{ 8,  0, 0 },
	{ 9,  0, 0 },
	{ 10, 0, 0 },
	{ 11, 0, 0 },
	{ 12, 0, 0 },
	{ 13, 0, 0 },
	{ 14, 0, 0 },
	{ 15, 0, 0 },
	{ 16, 0, 0 },
	{ 17, 0, 0 },
	{ 18, 0, 0 },
	{ 19, 0, 0 },
	{ 20, 0, 0 },
	{ 21, 0, 0 },
	{ 22, 0, 0 },
	{ 23, 0, 0 },
};

//备份整流模块的一些公共参数
static F32_T  m_f32_rect_out_volt[2];              //输出电压
static F32_T  m_f32_rect_curr_percent[2];          //限流百分比
static F32_T  m_u16_pb_high_volt[2];               //输出电压上限
static F32_T  m_u16_pb_low_volt[2];                //输出电压下限
static F32_T  m_f32_rect_offline_out_volt[2];      //默认输出电压
static U16_T  m_u16_rect_ctrl[RECT_CNT_MAX];    //充电模块状态控制，1：关机，0：开机

/******************************* 整流模块相关定义结束 ***************************/


/******************************* 通信模块相关定义开始 ***************************/
#define DCDC_MODULE_START_ADDR             0x90   //通信模块起始地址
#define DCDC_MODULE_BROADCAST_ADDR         0xF2   //一组通信模块广播地址
                                           
#define DCDC_MODULE_READ_CMD               0      //通信模块查询数据命令索引
#define DCDC_MODULE_NORMAL_CMD_CNT         1      //通信模块命令的个数

#define DCDC_MODULE_SET_VOLT_CMD           0      //通信模块调压命令索引，广播命令
#define DCDC_MODULE_SET_CURR_CMD           1      //通信模块限流命令索引，广播命令
#define DCDC_MODULE_SET_DEF_VOLT_CMD       2      //通信模块设置默认输出电压命令索引，广播命令       
#define DCDC_MODULE_BROADCAST_CMD_CNT      3      //通信模块命令的个数


static U8_T m_u8_dcdc_offline_cnt;         // 通信模块通讯中断命令个数

static MODULE_CMD_T m_t_dcdc_module_normal_cmd[DCDC_MODULE_NORMAL_CMD_CNT] = 
{
	{ 3, 0, 6    },              //查询数据命令
};

static MODULE_CMD_T m_t_dcdc_module_broadcast_cmd[DCDC_MODULE_BROADCAST_CMD_CNT] =
{
	{ 6, 0, 480  },              //设置模块输出电压，广播命令，默认48V，系数为0.1，48/0.1=480
	{ 6, 2, 1000 },              //设置模块限流点，广播命令，范围5%~110%，默认10%，系数为0.1，100/0.1=1000
	{ 6, 6, 480 },               //设置默认输出电压命令，广播命令，默认48V，系数为0.1，48/0.1=480
};

static MODULE_RECORD_T m_t_dcdc_module_record[DCDC_MODULE_MAX] =
{
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 0 },
	{ 7, 0, 0 },
};

//备份通信模块的一些公共参数
static F32_T  m_f32_dcdc_out_volt;              //输出电压
static F32_T  m_f32_dcdc_curr_percent;          //限流百分比

/******************************* 通信模块相关定义结束 ***************************/


/******************************* 逆变模块相关定义开始 ***************************/
#define DCAC_MODULE_START_ADDR             0x80   //逆变模块起始地址
#define DCAC_MODULE_BROADCAST_ADDR         0xF1   //一组逆变模块广播地址
                                           
#define DCAC_MODULE_READ_CMD               0      //逆变模块查询数据命令索引
#define DCAC_MODULE_NORMAL_CMD_CNT         1      //逆变模块命令的个数

#define DCAC_MODULE_FAULT_MASK             0x0004
#define DCAC_MODULE_OVERLOAD_MASK          0x0008
#define DCAC_MODULE_VOER_TEMPERATURE_MASK  0x0010
#define DCAC_MODULE_BATT_UNDERVOLT_MASK    0x0020
#define DCAC_MODULE_BYPASS_EXCEPTION_MASK  0x0040
#define DCAC_MODULE_EXCEPTION_MASK         (DCAC_MODULE_FAULT_MASK | DCAC_MODULE_OVERLOAD_MASK \
											| DCAC_MODULE_VOER_TEMPERATURE_MASK | DCAC_MODULE_BATT_UNDERVOLT_MASK \
											| DCAC_MODULE_BYPASS_EXCEPTION_MASK)


static U8_T m_u8_dcac_offline_cnt;         // 逆变模块通讯中断命令个数

static MODULE_CMD_T m_t_dcac_module_normal_cmd[DCAC_MODULE_NORMAL_CMD_CNT] = 
{
	{ 3, 0, 16 },              //查询数据命令
};

static MODULE_RECORD_T m_t_dcac_module_record[DCAC_MODULE_MAX] =
{
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 0 },
	{ 7, 0, 0 },
};
/******************************* 逆变模块相关定义结束 ***************************/


/******************************* D21模块相关定义开始 ***************************/
//#define D21_MODULE_START_ADDR             0x40   //DC10模块起始地址
//                                           
//#define D21_MODULE_READ_CMD               0      //DC10模块查询数据命令索引
//#define D21_MODULE_NORMAL_CMD_CNT         1      //DC10模块命令的个数	
//
//static U8_T m_u8_d21_offline_cnt = 10;         // DC10模块通讯中断命令计数
//
//static MODULE_CMD_T m_t_d21_module_normal_cmd[D21_MODULE_NORMAL_CMD_CNT] = 
//{
//	{ 3, 4, 5 },              //查询数据命令
//};
//
//static MODULE_RECORD_T m_t_d21_module_record;
/******************************* D21模块相关定义结束 ***************************/


/******************************* DC10模块相关定义开始 ***************************/
/* DC10模块数据记录 */
typedef struct
{
	U8_T u8_cmd_index;          //命令索引
	U8_T u8_com1_fail_cnt;      //通信失败记数，失败次数超过10次报通信中断故障
}DC10_RECORD_T;

#define DC10_MODULE_START_ADDR             0x40   //DC10模块起始地址
                                           
#define DC10_MODULE_READ_YC_CMD            0      //DC10模块查询数据命令索引
#define DC10_MODULE_READ_YX_CMD            1      //DC10模块查询数据命令索引
#define DC10_MODULE_WRITE_YK_CMD           2      //DC10模块遥控命令索引
#define DC10_MODULE_NORMAL_CMD_CNT         3      //DC10模块命令的个数

#define DC10_MODULE_FUSE_MASK              0x0080
#define DC10_MODULE_EXCEPTION_MASK         (DC10_MODULE_FUSE_MASK)

static U8_T m_u8_dc10_offline_cnt;         // DC10模块通讯中断命令个数

static MODULE_CMD_T m_t_dc10_module_normal_cmd[DC10_MODULE_NORMAL_CMD_CNT] = 
{
	{ 0x04, 0x100, 16 },              //遥测查询数据命令
	{ 0x02, 0x500, 64 },              //遥信查询数据命令
	{ 0x06, 0x1000, 0 },              //当前下发的遥控数据命令
};

static 	MODULE_CMD_T m_t_dc10_yk_cmd[] = 
{
	{ 0x06, 0x100E,  75 },         //A1分流器额定采样电压mV，遥控数据命令
	{ 0x06, 0x100F, 100 },         //A1分流器额定量程，遥控数据命令
	{ 0x06, 0x1010,  75 },         //A2分流器额定采样电压mV，遥控数据命令
	{ 0x06, 0x1011, 100 },         //A2分流器额定量程，遥控数据命令
	{ 0x06, 0x1012,  75 },         //A3分流器额定采样电压mV，遥控数据命令
	{ 0x06, 0x1013, 100 },         //A3分流器额定量程，遥控数据命令
	{ 0x06, 0x1017, 100 },         //S1传感器量程，遥控数据命令
	{ 0x06, 0x1018, 100 },         //S2传感器量程，遥控数据命令
	{ 0x06, 0x1019, 100 },         //S3传感器量程，遥控数据命令
	{ 0x06, 0x1020, 2200 },        //硅链控制输出电压，遥控数据命令
	{ 0x06, 0x1021,   0 },         //硅链规格，遥控数据命令
	{ 0x06, 0x1022,   0 },         //系统故障，遥控数据命令
	{ 0x06, 0x1023,   0 },         //故障输出干结点1，遥控数据命令
	{ 0x06, 0x1024,   0 },         //故障输出干结点2，遥控数据命令
	{ 0x06, 0x1025,   0 },         //故障输出干结点3，遥控数据命令
	{ 0x06, 0x1026,   0 },         //故障输出干结点4，遥控数据命令
	{ 0x06, 0x1027,   0 },         //故障输出干结点5，遥控数据命令
	{ 0x06, 0x1028,   0 },         //故障输出干结点6，遥控数据命令
	{ 0x06, 0x1029,   0 },         //故障输出干结点7，遥控数据命令
	{ 0x06, 0x102A,   0 },         //故障输出干结点8，遥控数据命令
};
#define DC10_YK_CMD_NUM		(sizeof(m_t_dc10_yk_cmd)/sizeof(MODULE_CMD_T))
static U8_T  m_u8_dc10_yk_idx = 0;
static DC10_RECORD_T m_t_dc10_module_record = { 0,  0 };


/******************************* AC10模块相关定义结束 ***************************/


/******************************* AC10模块相关定义开始 ***************************/
/* AC10模块数据记录 */
typedef struct
{
	U8_T u8_cmd_index;          //命令索引
	U8_T u8_com1_fail_cnt;      //通信失败记数，失败次数超过10次报通信中断故障
}AC10_RECORD_T;

#define AC10_MODULE_START_ADDR             0x41   //AC10模块起始地址
                                           
#define AC10_MODULE_READ_YC_CMD            0      //AC10模块查询数据命令索引
#define AC10_MODULE_READ_YX_CMD            1      //AC10模块查询数据命令索引
#define AC10_MODULE_WRITE_YK_CMD           2      //AC10模块遥控命令索引
#define AC10_MODULE_NORMAL_CMD_CNT         3      //AC10模块命令的个数

#define AC10_MODULE_FUSE_MASK              0x0080
#define AC10_MODULE_EXCEPTION_MASK         (AC10_MODULE_FUSE_MASK)

static U8_T m_u8_ac10_offline_cnt;         // AC10模块通讯中断命令个数

static MODULE_CMD_T m_t_ac10_module_normal_cmd[AC10_MODULE_NORMAL_CMD_CNT] = 
{
	{ 0x04, 0x100, 16 },              //遥测查询数据命令
	{ 0x02, 0x500, 64 },              //遥信查询数据命令
	{ 0x06, 0x1000, 0 },              //当前下发的遥控数据命令
};

static 	MODULE_CMD_T m_t_ac10_yk_cmd[] = 
{
	{ 0x06, 0x1000, 0x0200 },      //交流输入方式(默认：两路三相)，遥控数据命令
	{ 0x06, 0x1017, 100 },         //S1传感器量程，遥控数据命令
	{ 0x06, 0x1018, 100 },         //S2传感器量程，遥控数据命令
	{ 0x06, 0x1019, 100 },         //S3传感器量程，遥控数据命令
};
#define AC10_YK_CMD_NUM		(sizeof(m_t_ac10_yk_cmd)/sizeof(MODULE_CMD_T))
static U8_T  m_u8_ac10_yk_idx = 0;
static AC10_RECORD_T m_t_ac10_module_record = { 0,  0 };
U32_T	m_u32_ac_ov_start1;     //一路交流过压
U32_T	m_u32_ac_ov_start2;		//二路交流过压

/******************************* AC10模块相关定义结束 ***************************/

/******************************* RC10模块相关定义开始 ***************************/
#define RC10_MODULE_START_ADDR             0xC0   //RC10模块起始地址
                                           
#define RC10_MODULE_WRITE_CMD              0      //RC10模块查询数据命令索引
#define RC10_MODULE_NORMAL_CMD_CNT         1      //RC10模块命令的个数      

static U8_T m_u8_rc10_offline_cnt;         // 通信模块通讯中断命令个数

static MODULE_CMD_T m_t_rc10_module_normal_cmd[RC10_MODULE_NORMAL_CMD_CNT] = 
{
	{ 16, 0x0100, 16   },              //写数据命令
};

static U8_T m_u8_rc10_write_data[RC10_NODE_MAX] = { 0 };

static MODULE_RECORD_T m_t_rc10_module_record[RC10_MODULE_MAX] =
{
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 2, 0, 0 },
	{ 3, 0, 0 },
	{ 4, 0, 0 },
	{ 5, 0, 0 },
	{ 6, 0, 0 },
	{ 7, 0, 0 },
	{ 8, 0, 0 },
	{ 9, 0, 0 },
	{10, 0, 0 },
	{11, 0, 0 },
	{12, 0, 0 },
	{13, 0, 0 },
	{14, 0, 0 },
	{15, 0, 0 },
};

static U32_T m_u32_rc10_module_ctrl_tm[RC10_MODULE_MAX] = {0};
//static U32_T m_u32_rc10_module_retry[RC10_MODULE_MAX] = {0};
static U8_T  m_u8_rc10_swt_bak[FACT_SWT_CTRL_MAX] = {0};

static U32_T m_u32_switch_sync_time = 0;

/******************************* RC10模块相关定义结束 ***************************/


/******************************* 电池巡检相关函数定义开始 ***************************/

/*************************************************************
函数名称: v_com1_bms_send_cmd		           				
函数功能: 电池巡检发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_bms_send_cmd(BMS_RECORD_T *pt_record)
{
	U16_T crc, num;

#ifdef BIC_DEBUG
	U32_T i;
#endif

	if (pt_record->u8_cmd_index != BMS_READ_CMD)
		return;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type == B21)
		num = BMS_B21_MAX_CELL_NUM + 1;        //B21温度数据据放在单体电压后面
	else
		num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_num[pt_record->u8_module_index] + 2;
	os_mut_release(g_mut_share_data);
		
	m_u8_com1_tx_buf[0] = BMS_START_ADDR + pt_record->u8_start_addr + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_bms_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_bms_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_bms_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (num >> 8);
	m_u8_com1_tx_buf[5] = num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef BIC_DEBUG
	DEBUG("BMS send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_RECV_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
函数名称: u16_com1_create_cell_fault_id		           				
函数功能: 根据电池节号计算故障ID						
输入参数: u8_group_index -- 电池组号
          u8_batt_index -- 单体电池号
          u8_fault_type -- 故障类型，0：过压，1：欠压     		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_create_cell_fault_id(U8_T u8_group_index, U8_T u8_batt_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	if (u8_group_index == 0)
		u16_fault_id = (FAULT_BATT1_GROUP<<FAULT_GROUP_OFFSET);
	else
		u16_fault_id = (FAULT_BATT2_GROUP<<FAULT_GROUP_OFFSET);

	if (u8_fault_type == 0)
	{
		u16_fault_id |= (FAULT_CELL_OVER_VOLT_BASE_NUM + u8_batt_index);
	}
	else
	{
		u16_fault_id |= (FAULT_CELL_UNDER_VOLT_BASE_NUM + u8_batt_index);
	}

	return u16_fault_id;
}

/*************************************************************
函数名称: u16_com1_create_bms_fault_id		           				
函数功能: 根据电池序号计算故障ID						
输入参数: u8_module_index -- 电池巡检在所有两组电池巡检中的序号   		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_create_bms_fault_id(U8_T u8_module_index)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_BMS_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= u8_module_index;

	return u16_fault_id;
}

/*************************************************************
函数名称: v_com1_bms_comm_fault_alm		           				
函数功能: 电池巡检通信故障告警						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_bms_comm_fault_alm(BMS_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_bms_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_bms_offline_cnt)
		{
			//报通信故障
			v_fauid_send_fault_id_occur(u16_com1_create_bms_fault_id(pt_record->u8_total_index));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_batt.t_batt_group[pt_record->u8_group_index].u8_comm_state[pt_record->u8_module_index] = 1;
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
函数名称: v_com1_bms_unpacket		           				
函数功能: 电池巡检解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_bms_unpacket(BMS_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case BMS_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timout < 30)

			if (crc_scuess == 1)
			{
				BATT_GROUP_RT_DATA_T *batt_group = &(g_t_share_data.t_rt_data.t_batt.t_batt_group[pt_record->u8_group_index]);
				U8_T batt_num, bms_num;
				F32_T f32_cell_over_volt, f32_cell_under_volt;

#ifdef BIC_DEBUG
				DEBUG("BMS recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				i += 3;

				if (pt_record->u8_module_index == 0)      //如果是第一个电池巡检，则更新电池组温度
				{
					if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type != B21)
					{
						if ((m_u8_com1_rx_buf[i] & 0x80) != 0)
							batt_group->f32_temperature1 = -((((m_u8_com1_rx_buf[i]<<8) + m_u8_com1_rx_buf[i+1]) & 0x7FFF) / 10.0);
						else
							batt_group->f32_temperature1 = ((m_u8_com1_rx_buf[i]<<8) + m_u8_com1_rx_buf[i+1]) / 10.0;
							
						if ((m_u8_com1_rx_buf[i+2] & 0x80) != 0)
							batt_group->f32_temperature2 = -((((m_u8_com1_rx_buf[i+2]<<8) + m_u8_com1_rx_buf[i+3]) &0x7FFF) / 10.0);
						else
							batt_group->f32_temperature2 = ((m_u8_com1_rx_buf[i+2]<<8) + m_u8_com1_rx_buf[i+3]) / 10.0;
					}
					else
					{
						if ((m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i] & 0x80) != 0)
							batt_group->f32_temperature1 = -((((m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i]<<8)
															+ m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i+1]) & 0x7FFF) / 10.0);
						else
							batt_group->f32_temperature1 = ((m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i]<<8)
															+ m_u8_com1_rx_buf[BMS_B21_MAX_CELL_NUM*2+i+1]) / 10.0;
					}
				}

				if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type != B21)   //B3和B4前两个寄存器是温度数据，所以向后移动两个寄存器
				{
					i += 4;
				}

				batt_num = 0;
				for (j=0; j<pt_record->u8_module_index; j++)
				{
					batt_num += g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_num[j];
				}
				
				bms_num = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_num[pt_record->u8_module_index];

				for (j=0; j<bms_num; j++)
				{
					if (batt_num+j > BATT_CELL_MAX-1)    //防止数组越界
						break;

					if (g_t_share_data.t_sys_cfg.t_batt.e_bms_type != B21)
						batt_group->f32_cell_volt[batt_num+j] = ((m_u8_com1_rx_buf[i+j*2]<<8) + m_u8_com1_rx_buf[i+j*2+1]) / 1000.0;
					else
						batt_group->f32_cell_volt[batt_num+j] = ((m_u8_com1_rx_buf[i+j*2]<<8) + m_u8_com1_rx_buf[i+j*2+1]) * 20.0 / 65536.0;

					//判断单体电池是否过欠压，并发送故障ID
					if ((pt_record->u8_module_index ==
						(g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_bms_num-1)) && (j == (bms_num-1)))
					{
						f32_cell_over_volt = g_t_share_data.t_sys_cfg.t_batt.f32_tail_high_volt;
						f32_cell_under_volt = g_t_share_data.t_sys_cfg.t_batt.f32_tail_low_volt;
					}
					else
					{
						f32_cell_over_volt = g_t_share_data.t_sys_cfg.t_batt.f32_cell_high_volt;
						f32_cell_under_volt = g_t_share_data.t_sys_cfg.t_batt.f32_cell_low_volt;
					}

					if (batt_group->f32_cell_volt[batt_num+j] <= f32_cell_under_volt)   //欠压
					{
						if (batt_group->u8_cell_state[batt_num+j] == 0)
						{
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1));  //欠压故障发生
							batt_group->u8_cell_state[batt_num+j] = 2;
						}
						else if (batt_group->u8_cell_state[batt_num+j] == 1)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0)); //过压故障恢复
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1));  //欠压故障发生
							batt_group->u8_cell_state[batt_num+j] = 2;
						}
					}
					else if (batt_group->f32_cell_volt[batt_num+j] >= f32_cell_over_volt)  //过压
					{
						if (batt_group->u8_cell_state[batt_num+j] == 0)
						{
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0));  //过压故障发生
							batt_group->u8_cell_state[batt_num+j] = 1;
						}
						else if (batt_group->u8_cell_state[batt_num+j] == 2)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1)); //欠压故障恢复
							v_fauid_send_fault_id_occur(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0));  //过压故障发生
							batt_group->u8_cell_state[batt_num+j] = 1;
						}
					}
					else
					{
						if (batt_group->u8_cell_state[batt_num+j] == 1)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 0)); //过压故障恢复
							batt_group->u8_cell_state[batt_num+j] = 0;
						}
						else if (batt_group->u8_cell_state[batt_num+j] == 2)
						{
							v_fauid_send_fault_id_resume(u16_com1_create_cell_fault_id(pt_record->u8_group_index, batt_num+j, 1)); //欠压故障恢复
							batt_group->u8_cell_state[batt_num+j] = 0;
						}
					}
				}

				//判断单体电池最高最低电压
				batt_group->f32_min_cell_volt = batt_group->f32_cell_volt[0];
				batt_group->u8_cell_min_volt_id = 1;
				batt_group->f32_max_cell_volt = batt_group->f32_cell_volt[0];
				batt_group->u8_cell_max_volt_id = 1;

				for (j=1; j<g_t_share_data.t_sys_cfg.t_batt.t_batt_group[pt_record->u8_group_index].u8_cell_total_num; j++)
				{
					if (batt_group->f32_cell_volt[j] < batt_group->f32_min_cell_volt)
					{
						batt_group->f32_min_cell_volt = batt_group->f32_cell_volt[j];
						batt_group->u8_cell_min_volt_id = j + 1;
					}
					else if (batt_group->f32_cell_volt[j] > batt_group->f32_max_cell_volt)
					{
						batt_group->f32_max_cell_volt = batt_group->f32_cell_volt[j];
						batt_group->u8_cell_max_volt_id = j + 1;
					}
				}
				batt_group->f32_cell_min_volt_id = batt_group->u8_cell_min_volt_id;
				batt_group->f32_cell_max_volt_id = batt_group->u8_cell_max_volt_id;

				batt_group->u8_comm_state[pt_record->u8_module_index] = 0;   //清除通信中断标志
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_bms_offline_cnt)    //复归通信故障
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_create_bms_fault_id(pt_record->u8_total_index));
				}
				pt_record->u8_com1_fail_cnt = 0;                             //清除通信中断计数器
			}
			else
			{
				v_com1_bms_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
函数名称: v_com1_bms_comm_handle		           				
函数功能: 电池巡检通信及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_bms_comm_handle(BMS_RECORD_T *pt_record)
{
	v_com1_bms_send_cmd(pt_record);
	v_com1_bms_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= BMS_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
}

/******************************* 电池巡检相关函数定义结束 ***************************/


/******************************* 整流模块相关函数定义开始 ***************************/

/*************************************************************
函数名称: v_com1_rect_module_send_cmd		           				
函数功能: 整流模块发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rect_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, data_or_num;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	switch (pt_record->u8_cmd_index)
	{

		case RECT_MODULE_READ_CMD:
			data_or_num = m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case RECT_MODULE_SET_STATE_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);
			data_or_num = g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl[pt_record->u8_module_index];
			os_mut_release(g_mut_share_data);
			break;

		default:
			break;
	}

	m_u8_com1_tx_buf[0] = RECT_MODULE_START_ADDR + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_rect_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}


/*************************************************************
函数名称: v_com1_rect_module_broadcast_send		           				
函数功能: 整流模块发送广播命令函数						
输入参数: u8_cmd_index -- 广播命令索引       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rect_module_broadcast_send(U8_T no, U8_T u8_cmd_index, U16_T set_data)
{
	U16_T crc;

	//一组模块广播控制命令发送
	m_u8_com1_tx_buf[0] = RECT_MODULE_BROADCAST_ADDR - no;    //广播地址
	m_u8_com1_tx_buf[1] = m_t_rect_module_broadcast_cmd[u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_rect_module_broadcast_cmd[u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_rect_module_broadcast_cmd[u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (set_data >> 8);
	m_u8_com1_tx_buf[5] = set_data;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
函数名称: v_com1_rect_module_send_broadcast_cmd		           				
函数功能: 整流模块发送广播命令函数						
输入参数: u8_cmd_index -- 广播命令索引       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rect_module_send_broadcast_cmd(U8_T no, U8_T u8_cmd_index)
{
	U16_T set_data;	

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);
	switch (u8_cmd_index)
	{
		case RECT_MODULE_SET_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[no] * 10);
			break;

		case RECT_MODULE_SET_CURR_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[no] * 10);
			break;

		case RECT_MODULE_SET_HIGH_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt * 10);
			break;

		case RECT_MODULE_SET_LOW_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt * 10);
			break;
			
		case RECT_MODULE_SET_DEF_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt * 10);
			break;

		default:
			break;
	}
	os_mut_release(g_mut_share_data);
	
	if (no == 0)
	{
		//一组模块广播控制命令发送
		v_com1_rect_module_broadcast_send(COM1_GROUP1_RECT, u8_cmd_index, set_data);
		if (u8_cmd_index == RECT_MODULE_SET_CURR_CMD)
			os_evt_set(RECT_SET_CURR_SCUESS1, g_tid_batt);      //发送限流命令完成，发送事件标志给电池管理任务
	}

	else
	{	
		//二组模块广播控制命令发送
		v_com1_rect_module_broadcast_send(COM1_GROUP2_RECT, u8_cmd_index, set_data);
		if (u8_cmd_index == RECT_MODULE_SET_CURR_CMD)
			os_evt_set(RECT_SET_CURR_SCUESS2, g_tid_batt);      //发送限流命令完成，发送事件标志给电池管理任务
	}
	
	os_dly_wait(COM1_BROADCAST_WAIT_TICK);   //等待300ms，让模块处理命令，然后才能发送下一个命令

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);                                 //设置喂狗事件标志
}

/*************************************************************
函数名称: u16_com1_rect_create_fault_id		           				
函数功能: 根据整流模块索引和故障类型计算故障ID						
输入参数: u8_module_index -- 模块索引
          u8_fault_type   -- 故障类型    		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_rect_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_RECT_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (FAULT_RECT_CNT * u8_module_index + u8_fault_type);

	return u16_fault_id;
}

/*************************************************************
函数名称: v_com1_rect_module_send_fault_id		           				
函数功能: 整流模块发送故障ID函数						
输入参数: u16_state_old -- 旧的模块状态值
          u16_state_new -- 新的模块状态值       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rect_module_send_fault_id(U8_T u8_module_index, U16_T u16_state_old, U16_T u16_state_new)
{
	U16_T u16_fault_id;

	if (((u16_state_old & RECT_MODULE_OVER_VOLT_PROTECT_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_OVER_VOLT_PROTECT_MASK) != 0))   //过压保护故障发生
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_OVER_VOLT_PROTECT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_OVER_VOLT_PROTECT_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_OVER_VOLT_PROTECT_MASK) == 0))   //过压保护故障复归
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_OVER_VOLT_PROTECT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & RECT_MODULE_FAULT_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_FAULT_MASK) != 0))        //模块故障发生
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_FAULT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_FAULT_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_FAULT_MASK) == 0))        //模块故障复归
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_FAULT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & RECT_MODULE_VOER_TEMPERATURE_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_VOER_TEMPERATURE_MASK) != 0))    //过温故障发生
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_VOER_TEMPERATURE);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_VOER_TEMPERATURE_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_VOER_TEMPERATURE_MASK) == 0))    //过温故障复归
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_VOER_TEMPERATURE);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & RECT_MODULE_AC_EXCEPTION_MASK) == 0)
		&& ((u16_state_new & RECT_MODULE_AC_EXCEPTION_MASK) != 0))        //交流异常发生
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_AC_EXCEPTION);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & RECT_MODULE_AC_EXCEPTION_MASK) != 0)
		&& ((u16_state_new & RECT_MODULE_AC_EXCEPTION_MASK) == 0))        //交流异常复归
	{
		u16_fault_id = u16_com1_rect_create_fault_id(u8_module_index, FAULT_RECT_AC_EXCEPTION);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
}

/*************************************************************
函数名称: v_com1_rect_module_comm_fault_alm		           				
函数功能: 整流模块通信故障告警						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rect_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_rect_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_rect_offline_cnt)
		{
			//报通信故障
			v_fauid_send_fault_id_occur(u16_com1_rect_create_fault_id(pt_record->u8_module_index, FAULT_RECT_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dc_panel.t_rect[pt_record->u8_module_index].u8_comm_state = 1;   //设置通信中断标志
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
函数名称: v_com1_rect_module_unpacket		           				
函数功能: 整流模块解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rect_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case RECT_MODULE_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				RECT_RT_DATA_T *rect = &(g_t_share_data.t_rt_data.t_dc_panel.t_rect[pt_record->u8_module_index]);
				U16_T u16_state_old, u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				rect->f32_out_volt = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]) / 10.0;
				rect->f32_out_curr = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]) / 10.0;
				rect->f32_curr_percent = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]) / 10.0;
				rect->f32_max_out_volt = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]) / 10.0;
				rect->f32_min_out_volt = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]) / 10.0;
				u16_state_old = rect->u16_state;
				u16_state_new = (m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14];
				rect->u16_state = u16_state_new;
				rect->b_ctl_mode = ((u16_state_new & 0x0002) ? 1 : 0);
				rect->e_module_state = ((u16_state_new & 0x0001) ? SHUT_DOWN : START_UP);
				if ((u16_state_new & RECT_MODULE_EXCEPTION_MASK) != 0)
					rect->e_module_state = EXCEPTION;
				rect->u8_comm_state = 0;                                                             //清除通信中断标志
				os_mut_release(g_mut_share_data);

				v_com1_rect_module_send_fault_id(pt_record->u8_module_index, u16_state_old, u16_state_new);     //发送告警ID

				if (pt_record->u8_com1_fail_cnt >= m_u8_rect_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_rect_create_fault_id(pt_record->u8_module_index, FAULT_RECT_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_rect_module_comm_fault_alm(pt_record);
			}

			break;

		case RECT_MODULE_SET_STATE_CMD:
			rx_len = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)  //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < 8)
				{
					continue;
				}
				else
				{
					for (i=0; i<(rx_len-8+1); i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == m_u8_com1_tx_buf[2])
							&& (m_u8_com1_rx_buf[i+3] == m_u8_com1_tx_buf[3])
							&& (m_u8_com1_rx_buf[i+4] == m_u8_com1_tx_buf[4])
							&& (m_u8_com1_rx_buf[i+5] == m_u8_com1_tx_buf[5])
							&& (m_u8_com1_rx_buf[i+6] == m_u8_com1_tx_buf[6])
							&& (m_u8_com1_rx_buf[i+7] == m_u8_com1_tx_buf[7]))
						{
							if (pt_record->u8_com1_fail_cnt >= m_u8_rect_offline_cnt)
							{
								//复归通信故障报警
								v_fauid_send_fault_id_resume(u16_com1_rect_create_fault_id(pt_record->u8_module_index, FAULT_RECT_COMM_FAIL));

								os_mut_wait(g_mut_share_data, 0xFFFF);
								g_t_share_data.t_rt_data.t_dc_panel.t_rect[pt_record->u8_module_index].u8_comm_state = 0;   //清除通信中断标志
								os_mut_release(g_mut_share_data);
							}

							pt_record->u8_com1_fail_cnt = 0;

#ifdef COM1_DEBUG
							DEBUG("recv data: ");
							for (j=0; j<8; j++)
							{
								DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
							}
							DEBUG("\r\n");
#endif

							break;
						}
					}

					if (i<(rx_len-8+1))
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}
			}

			if (timeout >= COM1_RECV_TIMEOUT_CNT)
			{
				v_com1_rect_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
函数名称: v_com1_rect_module_comm_handle		           				
函数功能: 整流模块通信及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rect_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	v_com1_rect_module_send_cmd(pt_record);
	v_com1_rect_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= RECT_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
}

/******************************* 整流模块相关函数定义结束 ***************************/



/******************************* 通信模块相关函数定义开始 ***************************/

/*************************************************************
函数名称: v_com1_dcdc_module_send_cmd		           				
函数功能: 通信模块发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcdc_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, data_or_num;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	switch (pt_record->u8_cmd_index)
	{

		case DCDC_MODULE_READ_CMD:
			data_or_num = m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = DCDC_MODULE_START_ADDR + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dcdc_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
函数名称: v_com1_dcdc_module_send_broadcast_cmd		           				
函数功能: 通信模块发送广播命令函数						
输入参数: u8_cmd_index -- 广播命令索引       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcdc_module_send_broadcast_cmd(U8_T u8_cmd_index)
{
	U16_T crc, set_data;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);
	switch (u8_cmd_index)
	{
		case DCDC_MODULE_SET_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt * 10);
			break;

		case DCDC_MODULE_SET_CURR_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent * 10);
			break;

		case DCDC_MODULE_SET_DEF_VOLT_CMD:
			set_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt * 10);
			break;

		default:
			break;
	}
	os_mut_release(g_mut_share_data);
	

	m_u8_com1_tx_buf[0] = DCDC_MODULE_BROADCAST_ADDR;    //广播地址
	m_u8_com1_tx_buf[1] = m_t_dcdc_module_broadcast_cmd[u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dcdc_module_broadcast_cmd[u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dcdc_module_broadcast_cmd[u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (set_data >> 8);
	m_u8_com1_tx_buf[5] = set_data;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send broadcast cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
	
	os_dly_wait(COM1_BROADCAST_WAIT_TICK);   //等待300ms，让模块处理命令，然后才能发送下一个命令

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);                                 //设置喂狗事件标志
}

/*************************************************************
函数名称: u16_com1_dcdc_create_fault_id		           				
函数功能: 根据通信模块索引和故障类型计算故障ID						
输入参数: u8_module_index -- 模块索引
          u8_fault_type   -- 故障类型    		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_dcdc_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DCDC_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (FAULT_DCDC_CNT * u8_module_index + u8_fault_type);

	return u16_fault_id;
}

/*************************************************************
函数名称: v_com1_dcdc_module_send_fault_id		           				
函数功能: 通信模块发送故障ID函数						
输入参数: u16_state_old -- 旧的模块状态值
          u16_state_new -- 新的模块状态值       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_com1_dcdc_module_send_fault_id(U8_T u8_module_index, U16_T u16_state_old, U16_T u16_state_new)
{
	U16_T u16_fault_id;
	
	if (((u16_state_old & DCDC_MODULE_PROTECT_MASK) == 0)
		&& ((u16_state_new & DCDC_MODULE_PROTECT_MASK) != 0))     //保护故障发生
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_PROTECT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCDC_MODULE_PROTECT_MASK) != 0)
		&& ((u16_state_new & DCDC_MODULE_PROTECT_MASK) == 0))      //保护故障复归
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_PROTECT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & DCDC_MODULE_FAULT_MASK) == 0)
		&& ((u16_state_new & DCDC_MODULE_FAULT_MASK) != 0))        //模块故障发生
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_FAULT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCDC_MODULE_FAULT_MASK) != 0)
		&& ((u16_state_new & DCDC_MODULE_FAULT_MASK) == 0))        //模块故障复归
	{
		u16_fault_id = u16_com1_dcdc_create_fault_id(u8_module_index, FAULT_DCDC_FAULT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
}

/*************************************************************
函数名称: v_com1_dcdc_module_comm_fault_alm		           				
函数功能: 通信模块通信故障告警						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcdc_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_dcdc_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_dcdc_offline_cnt)
		{
			//报通信故障
			v_fauid_send_fault_id_occur(u16_com1_dcdc_create_fault_id(pt_record->u8_module_index, FAULT_DCDC_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[pt_record->u8_module_index].u8_comm_state = 1;   //设置通信中断标志
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
函数名称: v_com1_dcdc_module_unpacket		           				
函数功能: 通信模块解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcdc_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case DCDC_MODULE_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DCDC_MODULE_RT_DATA_T *dcdc = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[pt_record->u8_module_index]);
				U16_T u16_state_old, u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				dcdc->f32_out_volt = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]) / 10.0;
				dcdc->f32_out_curr = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]) / 10.0;
				dcdc->f32_curr_percent = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]) / 10.0;
				dcdc->f32_max_out_volt = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]) / 10.0;
				dcdc->f32_min_out_volt = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]) / 10.0;
				u16_state_old = dcdc->u16_state;
				u16_state_new = (m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14];
				dcdc->u16_state = u16_state_new;
				dcdc->b_ctl_mode = ((u16_state_new & 0x0002) ? 1 : 0);
				dcdc->e_module_state = ((u16_state_new & 0x0001) ? SHUT_DOWN : START_UP);
				if ((u16_state_new & DCDC_MODULE_EXCEPTION_MASK) != 0)
					dcdc->e_module_state = EXCEPTION;
				dcdc->u8_comm_state = 0;                                                             //清除通信中断标志
				
				//计算通信母线电压、电流
				g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_curr = 0;
				g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt = 0;
				for (j=0; j<g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num; j++)
				{
					g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_curr += 
								g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[j].f32_out_curr;
					
					if (g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt
							< g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[j].f32_out_volt)
					{
						g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt = 
							g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[j].f32_out_volt;
					}
				}
				
				
				os_mut_release(g_mut_share_data);

				v_com1_dcdc_module_send_fault_id(pt_record->u8_module_index, u16_state_old, u16_state_new);     //发送告警ID

				if (pt_record->u8_com1_fail_cnt >= m_u8_dcdc_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_dcdc_create_fault_id(pt_record->u8_module_index, FAULT_DCDC_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dcdc_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
函数名称: v_com1_dcdc_module_comm_handle		           				
函数功能: 通信模块通信及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcdc_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	v_com1_dcdc_module_send_cmd(pt_record);
	v_com1_dcdc_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= DCDC_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
}

/******************************* 通信模块相关函数定义结束 ***************************/


/******************************* 逆变模块相关函数定义开始 ***************************/

/*************************************************************
函数名称: v_com1_dcac_module_send_cmd		           				
函数功能: 逆变模块发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcac_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, data_or_num;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	switch (pt_record->u8_cmd_index)
	{
		case DCAC_MODULE_READ_CMD:
			data_or_num = m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = DCAC_MODULE_START_ADDR + pt_record->u8_module_index;
	m_u8_com1_tx_buf[1] = m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dcac_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
函数名称: u16_com1_dcac_create_fault_id		           				
函数功能: 根据逆变模块索引和故障类型计算故障ID						
输入参数: u8_module_index -- 模块索引
          u8_fault_type   -- 故障类型    		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_dcac_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DCAC_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (FAULT_DCAC_CNT * u8_module_index + u8_fault_type);

	return u16_fault_id;
}

/*************************************************************
函数名称: v_com1_dcac_module_send_fault_id		           				
函数功能: 逆变模块发送故障ID函数						
输入参数: u16_state_old -- 旧的模块状态值
          u16_state_new -- 新的模块状态值       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcac_module_send_fault_id(U8_T u8_module_index, U16_T u16_state_old, U16_T u16_state_new)
{
	U16_T u16_fault_id;
											
	if (((u16_state_old & DCAC_MODULE_FAULT_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_FAULT_MASK) != 0))                   //模块故障发生
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_FAULT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_FAULT_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_FAULT_MASK) == 0))                   //模块故障复归
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_FAULT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}

	if (((u16_state_old & DCAC_MODULE_OVERLOAD_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_OVERLOAD_MASK) != 0))                //模块过载发生
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOERLOAD);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_OVERLOAD_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_OVERLOAD_MASK) == 0))                //模块过载复归
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOERLOAD);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
	
	if (((u16_state_old & DCAC_MODULE_VOER_TEMPERATURE_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_VOER_TEMPERATURE_MASK) != 0))        //模块过温发生
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOER_TEMPERATURE);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_VOER_TEMPERATURE_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_VOER_TEMPERATURE_MASK) == 0))        //模块过温复归
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_VOER_TEMPERATURE);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
	
	if (((u16_state_old & DCAC_MODULE_BATT_UNDERVOLT_MASK) == 0)              //电池欠压故障
		&& ((u16_state_new & DCAC_MODULE_BATT_UNDERVOLT_MASK) != 0))          //电池欠压发生
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BATT_UNDERVOLT);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_BATT_UNDERVOLT_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_BATT_UNDERVOLT_MASK) == 0))          //电池欠压复归
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BATT_UNDERVOLT);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
	
	if (((u16_state_old & DCAC_MODULE_BYPASS_EXCEPTION_MASK) == 0)
		&& ((u16_state_new & DCAC_MODULE_BYPASS_EXCEPTION_MASK) != 0))        //旁路异常发生
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BYPASS_EXCEPTION);
		v_fauid_send_fault_id_occur(u16_fault_id);
	}
	else if (((u16_state_old & DCAC_MODULE_BYPASS_EXCEPTION_MASK) != 0)
		&& ((u16_state_new & DCAC_MODULE_BYPASS_EXCEPTION_MASK) == 0))        //旁路异常复归
	{
		u16_fault_id = u16_com1_dcac_create_fault_id(u8_module_index, FAULT_DCAC_BYPASS_EXCEPTION);
		v_fauid_send_fault_id_resume(u16_fault_id);
	}
}

/*************************************************************
函数名称: v_com1_dcac_module_comm_fault_alm		           				
函数功能: 逆变模块通信故障告警						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcac_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_dcac_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_dcac_offline_cnt)
		{
			//报通信故障
			v_fauid_send_fault_id_occur(u16_com1_dcac_create_fault_id(pt_record->u8_module_index, FAULT_DCAC_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[pt_record->u8_module_index].u8_comm_state = 1;   //设置通信中断标志
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
函数名称: v_com1_dcac_module_unpacket		           				
函数功能: 逆变模块解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcac_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case DCAC_MODULE_READ_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DCAC_MODULE_RT_DATA_T *dcac = &(g_t_share_data.t_rt_data.t_dcac_panel.t_dcac_module[pt_record->u8_module_index]);
				U16_T u16_state_old, u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				dcac->f32_out_volt = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]) / 10.0;
				dcac->f32_out_curr = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]) / 10.0;
				dcac->f32_out_freq = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]) / 100.0;
				dcac->f32_out_power_factor = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]) / 100.0;
				dcac->f32_inverter_volt = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]) / 10.0;
				dcac->f32_bypass_input_volt = ((m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14]) / 10.0;
				dcac->f32_bypass_input_freq = ((m_u8_com1_rx_buf[i+15]<<8) + m_u8_com1_rx_buf[i+16]) / 100.0;
				dcac->f32_batt_input_volt = ((m_u8_com1_rx_buf[i+17]<<8) + m_u8_com1_rx_buf[i+18]) / 10.0;
				dcac->f32_active_power = ((m_u8_com1_rx_buf[i+19]<<8) + m_u8_com1_rx_buf[i+20]) / 100.0;
				dcac->f32_apparen_power = ((m_u8_com1_rx_buf[i+21]<<8) + m_u8_com1_rx_buf[i+22]) / 100.0;
				dcac->f32_load_ratio = ((m_u8_com1_rx_buf[i+23]<<8) + m_u8_com1_rx_buf[i+24]) / 10.0;
				dcac->f32_temperature = (((m_u8_com1_rx_buf[i+25]&0x7F)<<8) + m_u8_com1_rx_buf[i+26]) / 10.0;
				if ((m_u8_com1_rx_buf[i+25] & 0x80) != 0)
					dcac->f32_temperature = -dcac->f32_temperature;
				dcac->f32_outage_capacity_ratio = ((m_u8_com1_rx_buf[i+27]<<8) + m_u8_com1_rx_buf[i+28]) / 10.0;
				dcac->f32_bypass_high_volt = ((m_u8_com1_rx_buf[i+29]<<8) + m_u8_com1_rx_buf[i+30]) / 10.0;
				dcac->f32_bypass_low_volt = ((m_u8_com1_rx_buf[i+31]<<8) + m_u8_com1_rx_buf[i+32]) / 10.0;
				
				u16_state_old = dcac->u16_state;
				u16_state_new = (m_u8_com1_rx_buf[i+33]<<8) + m_u8_com1_rx_buf[i+34];
				if ((u16_state_new & DCAC_MODULE_BATT_UNDERVOLT_MASK) != 0)
					u16_state_new &= ~DCAC_MODULE_BATT_UNDERVOLT_MASK;
				else
					u16_state_new |= DCAC_MODULE_BATT_UNDERVOLT_MASK;
				dcac->u16_state = u16_state_new;
				if (u16_state_new & 0x0001)
				{
					dcac->e_module_state = SHUT;
				}
				else
				{
					if (u16_state_new & 0x0080)
						dcac->e_module_state = INVERT;
					else
						dcac->e_module_state = BYPASS;
				}
								
				if ((u16_state_new & DCAC_MODULE_EXCEPTION_MASK) != 0)
					dcac->b_alarm_state = 1;
				else
					dcac->b_alarm_state = 0;
					
				dcac->u8_comm_state = 0; 
				                                                            //清除通信中断标志
				os_mut_release(g_mut_share_data);

				v_com1_dcac_module_send_fault_id(pt_record->u8_module_index, u16_state_old, u16_state_new);     //发送告警ID

				if (pt_record->u8_com1_fail_cnt >= m_u8_dcac_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_dcac_create_fault_id(pt_record->u8_module_index, FAULT_DCAC_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dcac_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
函数名称: v_com1_dcac_module_comm_handle		           				
函数功能: 逆变模块通信及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dcac_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	v_com1_dcac_module_send_cmd(pt_record);
	v_com1_dcac_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= DCAC_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
}

/******************************* 逆变模块相关函数定义结束 ***************************/



/******************************* D21模块相关函数定义开始 ***************************/

/*************************************************************
函数名称: v_com1_d21_module_send_cmd		           				
函数功能: D21模块发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
//static void v_com1_d21_module_send_cmd(MODULE_RECORD_T *pt_record)
//{
//	U16_T crc, data_or_num;
//
//#ifdef COM1_DEBUG
//	U32_T i;
//#endif
//
//	switch (pt_record->u8_cmd_index)
//	{
//		case D21_MODULE_READ_CMD:
//			data_or_num = m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
//			break;
//
//		default:
//			return;
//	}
//
//	m_u8_com1_tx_buf[0] = D21_MODULE_START_ADDR + pt_record->u8_module_index;
//	m_u8_com1_tx_buf[1] = m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
//	m_u8_com1_tx_buf[2] = (m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
//	m_u8_com1_tx_buf[3] = m_t_d21_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
//	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
//	m_u8_com1_tx_buf[5] = data_or_num;
//	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
//	m_u8_com1_tx_buf[6] = (U8_T)crc;
//	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);
//
//#ifdef COM1_DEBUG
//	DEBUG("send cmd: ");
//	for (i=0; i<8; i++)
//	{
//		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
//	}
//	DEBUG("\r\n");
//#endif
//
//	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
//	{
//		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
//		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
//	}
//}

/*************************************************************
函数名称: v_com1_d21_module_unpacket		           				
函数功能: D21模块解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
//static void v_com1_d21_module_unpacket(MODULE_RECORD_T *pt_record)
//{
//	U16_T byte_len, rx_len, timeout = 0;
//	U32_T i, j, crc_scuess;
//	U16_T id, u16_data,u16_load_shunt_range;
//	DC_RT_DATA_T *d21 = NULL;
//
//	os_mut_wait(g_mut_share_data, 0xFFFF);
//	d21 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc);
//	u16_load_shunt_range = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range;
//	os_mut_release(g_mut_share_data);
//
//	switch (pt_record->u8_cmd_index)
//	{
//		case D21_MODULE_READ_CMD:
//			rx_len = 0;
//			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节
//
//			crc_scuess = 0;
//			timeout = 0;
//
//			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
//			{
//				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
//				os_dly_wait(COM1_RECV_WAIT_TICK);
//				timeout++;
//
//				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
//				if (rx_len < byte_len)
//				{
//					continue;
//				}
//				else
//				{
//					for (i=0; i<rx_len-2; i++)
//					{
//						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
//							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
//							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
//						{
//							if (rx_len - i < byte_len)
//							{
//								break;
//							}
//							else
//							{
//								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
//								{
//									continue;
//								}
//								else
//								{
//									crc_scuess = 1;
//									break;
//								}
//							}
//						}
//					}
//
//					if (crc_scuess == 1)
//					{
//						break;
//					}
//					else
//					{
//						for (j=0; j<(rx_len-i); j++)
//							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];
//
//						rx_len -= i;
//					}
//				}  // end of if (rx_len < byte_len)
//			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)
//
//			if (crc_scuess == 1)
//			{
//#ifdef COM1_DEBUG
//				DEBUG("recv data: ");
//				for (j=0; j<byte_len; j++)
//				{
//					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
//				}
//				DEBUG("\r\n");
//#endif
//
//				os_mut_wait(g_mut_share_data, 0xFFFF);
//				u16_data = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]);
//				d21->f32_pb_volt = (F32_T)u16_data / 10.0;
//				u16_data = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]);
//				d21->f32_cb_volt = (F32_T)u16_data / 10.0;
//				u16_data = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]);
//				//D21采样上的负载电流传感器量程为50A，所以在此进行换算
//				d21->f32_load_curr = (F32_T)u16_data * u16_load_shunt_range / 500.0;
//					
//				d21->u16_state &= ~0x0100;				   //清除通信中断标志
//				os_mut_release(g_mut_share_data); 				
//
//				if (pt_record->u8_com1_fail_cnt >= m_u8_d21_offline_cnt)
//				{
//					//复归通信故障报警
//					id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC10_COMM_FAIL;
//					v_fauid_send_fault_id_resume(id);
//				}
//				pt_record->u8_com1_fail_cnt = 0;
//			}
//			else
//			{
//				if (pt_record->u8_com1_fail_cnt < m_u8_d21_offline_cnt)
//				{
//					pt_record->u8_com1_fail_cnt++;
//					if (pt_record->u8_com1_fail_cnt >= m_u8_d21_offline_cnt)
//					{
//						d21->u16_state |= 0x0100;
//						id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC10_COMM_FAIL;
//						v_fauid_send_fault_id_occur(id);
//					}
//				}
//			}
//
//			break;
//					
//		default:
//			break;
//	}
//}

/*************************************************************
函数名称: v_com1_d21_module_comm_handle		           				
函数功能: D21模块通信及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
//static void v_com1_d21_module_comm_handle(MODULE_RECORD_T *pt_record)
//{ 
//	v_com1_d21_module_send_cmd(pt_record);
//	v_com1_d21_module_unpacket(pt_record);
//
//	pt_record->u8_cmd_index++;
//	pt_record->u8_cmd_index %= D21_MODULE_NORMAL_CMD_CNT;
//
//	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
//}

/******************************* DC10模块相关函数定义结束 ***************************/

/******************************* DC10模块相关函数定义开始 ***************************/

/*************************************************************
函数名称: v_com1_dc10_module_send_cmd		           				
函数功能: DC10模块发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dc10_module_send_cmd(DC10_RECORD_T *pt_record)
{
	U16_T crc, data_or_num, u16_cfg_changed = 0;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);

	//配置改变及时下发处理
	if(m_t_dc10_yk_cmd[0].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt)
	{
		m_t_dc10_yk_cmd[0].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a1_rated_volt;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[0].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[0].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[0].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[1].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range)
	{
		m_t_dc10_yk_cmd[1].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[1].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[1].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[1].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[2].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt)
	{
		m_t_dc10_yk_cmd[2].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a2_rated_volt;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[2].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[2].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[2].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[3].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range)
	{
		m_t_dc10_yk_cmd[3].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[3].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[3].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[3].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[4].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt)
	{
		m_t_dc10_yk_cmd[4].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_shunt_a3_rated_volt;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[4].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[4].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[4].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[5].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range)
	{
		m_t_dc10_yk_cmd[5].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[5].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[5].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[5].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[6].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range)
	{
		m_t_dc10_yk_cmd[6].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a1_s1_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[6].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[6].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[6].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[7].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range)
	{
		m_t_dc10_yk_cmd[7].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a2_s2_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[7].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[7].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[7].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[8].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range)
	{
		m_t_dc10_yk_cmd[8].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_a3_s3_shunt_range;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[8].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[8].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[8].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[9].u16_data_or_num != (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt * 10))
	{
		m_t_dc10_yk_cmd[9].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u16_cb_output_volt * 10;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[9].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[9].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[9].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[10].u16_data_or_num != g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl)
	{
		m_t_dc10_yk_cmd[10].u16_data_or_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl;
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[10].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[10].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[10].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[11].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[0])
	{
		m_t_dc10_yk_cmd[11].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[0];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[11].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[11].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[11].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[12].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[1])
	{
		m_t_dc10_yk_cmd[12].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[1];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[12].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[12].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[12].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}	
	else if(m_t_dc10_yk_cmd[13].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[2])
	{
		m_t_dc10_yk_cmd[13].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[2];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[13].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[13].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[13].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[14].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[3])
	{
		m_t_dc10_yk_cmd[14].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[3];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[14].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[14].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[14].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}	
	else if(m_t_dc10_yk_cmd[15].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[4])
	{
		m_t_dc10_yk_cmd[15].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[4];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[15].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[15].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[15].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[16].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[5])
	{
		m_t_dc10_yk_cmd[16].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[5];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[16].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[16].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[16].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_dc10_yk_cmd[17].u16_data_or_num != g_t_share_data.t_rt_data.t_u8_dc10_fault_out[6])
	{
		m_t_dc10_yk_cmd[17].u16_data_or_num = g_t_share_data.t_rt_data.t_u8_dc10_fault_out[6];
		pt_record->u8_cmd_index = DC10_MODULE_WRITE_YK_CMD;
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[17].u8_func_code;	
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[17].u16_reg_addr;				
		m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[17].u16_data_or_num;
		data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}	
	
	os_mut_release(g_mut_share_data);


	switch (pt_record->u8_cmd_index)
	{
		case DC10_MODULE_READ_YC_CMD:
			data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case DC10_MODULE_READ_YX_CMD:
			data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case DC10_MODULE_WRITE_YK_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);            
			if ( !u16_cfg_changed )
			{	//周期下发
				m_u8_dc10_yk_idx++;
				m_u8_dc10_yk_idx %= DC10_YK_CMD_NUM;
				m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_dc10_yk_cmd[m_u8_dc10_yk_idx].u8_func_code;	
				m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_dc10_yk_cmd[m_u8_dc10_yk_idx].u16_reg_addr;				
				m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_dc10_yk_cmd[m_u8_dc10_yk_idx].u16_data_or_num;
				data_or_num = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			}
			os_mut_release(g_mut_share_data);
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = DC10_MODULE_START_ADDR;
	m_u8_com1_tx_buf[1] = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_dc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
函数名称: u16_com1_dc10_create_fault_id		           				
函数功能: 根据DC10模块索引和故障类型计算故障ID						
输入参数: u8_module_index -- 模块索引
          u8_fault_type   -- 故障类型    		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_dc10_create_fault_id(U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET);//(FAULT_FEEDER_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= u8_fault_type;

	return u16_fault_id;
}

/*************************************************************
函数名称: v_com1_dc10_module_comm_fault_alm		           				
函数功能: DC10模块通信故障告警						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dc10_module_comm_fault_alm(DC10_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_dc10_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
		{
			//报通信故障
			v_fauid_send_fault_id_occur(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dc_panel.t_dc10.u8_comm_state = 1;   //设置通信中断标志
			os_mut_release(g_mut_share_data);
		}
	}
}

/****************************************************************************
 *  函数名称：  v_com1_dc10_bus2_fault_judge(void) 
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   判断采集上来的二段直流母线数据是否异常，异常则至相应标志位 
 ***************************************************************************/
void v_com1_dc10_bus2_fault_judge(void)
 {
 	U16_T id;
 	DC_RT_DATA_T *dc_p_rt_data = NULL;
	DC_CFG_T *dc_p_cfg = NULL;
	BATT_MGMT_CFG_T *batt_p_mgmt_cfg = NULL;

	os_mut_wait(g_mut_share_data, 0xFFFF);

	dc_p_rt_data = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc);
	dc_p_cfg = &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc);
	batt_p_mgmt_cfg = &(g_t_share_data.t_sys_cfg.t_batt_mgmt);

	if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb)
	{	//合母过压判断
		if((dc_p_rt_data->f32_pb2_volt) >= (dc_p_cfg->u16_pb_high_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0001))
			{
				dc_p_rt_data->u16_state2 |= 0x0001;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0001)
			{
				dc_p_rt_data->u16_state2 &= (~0x0001);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//合母欠压判断
		if((dc_p_rt_data->f32_pb2_volt) <= (dc_p_cfg->u16_pb_low_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0002))
			{
				dc_p_rt_data->u16_state2 |= 0x0002;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0002)
			{
				dc_p_rt_data->u16_state2 &= (~0x0002);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}

		//控母过压判断
		if((dc_p_rt_data->f32_cb2_volt) >= (dc_p_cfg->u16_cb_high_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0004))
			{
				dc_p_rt_data->u16_state2 |= 0x0004;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0004)
			{
				dc_p_rt_data->u16_state2 &= (~0x0004);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//控母欠压判断
		if((dc_p_rt_data->f32_cb2_volt) <= (dc_p_cfg->u16_cb_low_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0008))
			{
				dc_p_rt_data->u16_state2 |= 0x0008;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0008)
			{
				dc_p_rt_data->u16_state2 &= (~0x0008);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	else
	{
		//母线过压判断
		if((dc_p_rt_data->f32_cb2_volt) >= (dc_p_cfg->u16_bus_high_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0010))
			{
				dc_p_rt_data->u16_state2 |= 0x0010;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0010)
			{
				dc_p_rt_data->u16_state2 &= (~0x0010);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//母线欠压判断
		if((dc_p_rt_data->f32_cb2_volt) <= (dc_p_cfg->u16_bus_low_volt))
		{
			if(!((dc_p_rt_data->u16_state2)&0x0020))
			{
				dc_p_rt_data->u16_state2 |= 0x0020;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state2)&0x0020)
			{
				dc_p_rt_data->u16_state2 &= (~0x0020);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	
	//电池过压判断
	if((dc_p_rt_data->f32_batt2_volt) >= (batt_p_mgmt_cfg->f32_high_volt_limit))
	{
		if(!((dc_p_rt_data->u16_state2)&0x0040))
		{
			dc_p_rt_data->u16_state2 |= 0x0040;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_OVER_VOLT;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else 
	{
		if((dc_p_rt_data->u16_state2)&0x0040)
		{
			dc_p_rt_data->u16_state2 &= (~0x0040);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_OVER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	//电池欠压判断
	if((dc_p_rt_data->f32_batt2_volt) <= (batt_p_mgmt_cfg->f32_low_volt_limit))
	{
		if(!((dc_p_rt_data->u16_state2)&0x0080))
		{
			dc_p_rt_data->u16_state2 |= 0x0080;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_UNDER_VOLT;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else
	{
		if((dc_p_rt_data->u16_state2)&0x0080)
		{
			dc_p_rt_data->u16_state2 &= (~0x0080);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_UNDER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	os_mut_release(g_mut_share_data);		
 }

/*************************************************************
函数名称: v_com1_dc10_module_unpacket		           				
函数功能: DC10模块解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
#define		LINE_VOL_TO_PHASE_VOL		1.732	   //线电压转换为相电压比例系数
static void v_com1_dc10_module_unpacket(DC10_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0, u16_data;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case DC10_MODULE_READ_YC_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DC10_RT_DATA_T *dc10 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc10);

				os_mut_wait(g_mut_share_data, 0xFFFF);
//				//一路交流
//				u16_data = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]);
//				f32_ac_ua = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]);
//				f32_ac_ub = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]);
//				f32_ac_uc = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;

                //二路交流
//				u16_data = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]);
//				f32_ac_ua = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]);
//				f32_ac_ub = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
//
//				u16_data = ((m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14]);
//				f32_ac_uc = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;

                //环境温度
				u16_data = ((m_u8_com1_rx_buf[i+15]<<8) + m_u8_com1_rx_buf[i+16]);

				//V1控母电压
				u16_data = ((m_u8_com1_rx_buf[i+17]<<8) + m_u8_com1_rx_buf[i+18]);
				dc10->f32_v1_volt = ((F32_T)(S16_T)u16_data) / 10.0;
				//A1分流器负载电流
				u16_data = ((m_u8_com1_rx_buf[i+19]<<8) + m_u8_com1_rx_buf[i+20]);
				dc10->f32_a1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (dc10->f32_a1_curr < 0)
				{
					dc10->f32_a1_curr = 0;
				}
				//V2电池电压
				u16_data = ((m_u8_com1_rx_buf[i+21]<<8) + m_u8_com1_rx_buf[i+22]);
				dc10->f32_v2_volt = ((F32_T)(S16_T)u16_data) / 10.0;
				//A2分流器电池电流
				u16_data = ((m_u8_com1_rx_buf[i+23]<<8) + m_u8_com1_rx_buf[i+24]);
				dc10->f32_a2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//V3合母电压
				u16_data = ((m_u8_com1_rx_buf[i+25]<<8) + m_u8_com1_rx_buf[i+26]);
				dc10->f32_v3_volt = ((F32_T)(S16_T)u16_data) / 10.0;
				//A3分流器电池2电流
				u16_data = ((m_u8_com1_rx_buf[i+27]<<8) + m_u8_com1_rx_buf[i+28]);
				dc10->f32_a3_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S1分流器负载电流
				u16_data = ((m_u8_com1_rx_buf[i+29]<<8) + m_u8_com1_rx_buf[i+30]);
				dc10->f32_s1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (dc10->f32_s1_curr < 0)
				{
					dc10->f32_s1_curr = 0;
				}
				//S2分流器电池电流
				u16_data = ((m_u8_com1_rx_buf[i+31]<<8) + m_u8_com1_rx_buf[i+32]);
				dc10->f32_s2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S3分流器电池2电流
				u16_data = ((m_u8_com1_rx_buf[i+33]<<8) + m_u8_com1_rx_buf[i+34]);
				dc10->f32_s3_curr = ((F32_T)(S16_T)u16_data) / 10.0;

				//二段母线数据提取
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_volt = dc10->f32_v3_volt;   // 二段合母电压
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_volt = dc10->f32_v1_volt;   // 二段控母电压
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr = dc10->f32_a1_curr; // 二段控母电流，也称负载电流	                                                     
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt2_volt = dc10->f32_v2_volt; // 二组电池电压

#if (BATT_CURR_FROM_SENSOR)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0] = dc10->f32_s1_curr;// 一组电池电流
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] = dc10->f32_s2_curr;// 二组电池电流
#else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] = dc10->f32_a2_curr;// 二组电池电流
#endif

				//二段母线异常判断处理
				v_com1_dc10_bus2_fault_judge();								

				//清通信中断	
				dc10->u8_comm_state = 0;                                    //清除通信中断标志				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dc10_module_comm_fault_alm(pt_record);
			}

			break;

		case DC10_MODULE_READ_YX_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])/8 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DC10_RT_DATA_T *dc10 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc10);
				U16_T u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				//state1 = ((m_u8_com1_rx_buf[i+4]<<8) + m_u8_com1_rx_buf[i+3]);
				//state2 = ((m_u8_com1_rx_buf[i+6]<<8) + m_u8_com1_rx_buf[i+5]);
				//state3 = ((m_u8_com1_rx_buf[i+8]<<8) + m_u8_com1_rx_buf[i+7]);
				u16_state_new = (U16_T)((m_u8_com1_rx_buf[i+10]<<8) + m_u8_com1_rx_buf[i+9]);
				dc10->u16_swt_state = u16_state_new;				
//				if(u16_state_new & 0x0001)
//				{	//熔断					
//					if( !(dc10->u16_sys_state & 0x0008) )
//					{
//						dc10->u16_sys_state |= 0x0008;
//						v_fauid_send_fault_id_occur(u16_com1_dc10_create_fault_id(FAULT_DCAC_DC10_FUSE_FAULT));
//					}
//				}
//				else
//				{	//正常
//					if( (dc10->u16_sys_state & 0x0008) )
//					{
//						dc10->u16_sys_state &= ~0x0008;
//						v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DCAC_DC10_FUSE_FAULT));
//					}
//				}				

				//清通信中断	
				dc10->u8_comm_state = 0;                                    //清除通信中断标志				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dc10_module_comm_fault_alm(pt_record);
			}

			break;

		case DC10_MODULE_WRITE_YK_CMD:
			rx_len = 0;
			byte_len = 8;  //地址码1个字节、功能码1个字节、2字节寄存器地址、2字节寄存器数据、CRC两个字节，总共加8个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i]   == m_u8_com1_tx_buf[0]) && 
						    (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1]))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				DC10_RT_DATA_T *dc10 = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc10);

				os_mut_wait(g_mut_share_data, 0xFFFF);	
				dc10->u8_comm_state = 0;                                    //清除通信中断标志				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_dc10_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_dc10_create_fault_id(FAULT_DC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_dc10_module_comm_fault_alm(pt_record);
			}
			break;
					
		default:
			break;
	}
}

/*************************************************************
函数名称: v_com1_dc10_module_comm_handle		           				
函数功能: DC10模块通信及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_dc10_module_comm_handle(DC10_RECORD_T *pt_record)
{ 
	v_com1_dc10_module_send_cmd(pt_record);
	v_com1_dc10_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= DC10_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
}

/******************************* DC10模块相关函数定义结束 ***************************/


/******************************* AC10模块相关函数定义开始 ***************************/

/*************************************************************
函数名称: v_com1_ac10_module_send_cmd		           				
函数功能: AC10模块发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_ac10_module_send_cmd(AC10_RECORD_T *pt_record)
{
	U16_T crc, data_or_num, u16_cfg_changed = 0;

#ifdef COM1_DEBUG
	U32_T i;
#endif

	os_mut_wait(g_mut_share_data, 0xFFFF);

	//配置改变及时下发处理
	if(m_t_ac10_yk_cmd[0].u16_data_or_num != 0x0200)
	{
		m_t_ac10_yk_cmd[0].u16_data_or_num = 0x0200;//g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_ac_meas_modle;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[0].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[0].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[0].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;		
		u16_cfg_changed = 1;
	}
	else if(m_t_ac10_yk_cmd[1].u16_data_or_num != g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range)
	{
		m_t_ac10_yk_cmd[1].u16_data_or_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a1_s1_shunt_range;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[1].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[1].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[1].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		u16_cfg_changed = 1;
	}
	else if(m_t_ac10_yk_cmd[2].u16_data_or_num != g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range)
	{
		m_t_ac10_yk_cmd[2].u16_data_or_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a2_s2_shunt_range;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[2].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[2].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[2].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		u16_cfg_changed = 1;
	}
	else if(m_t_ac10_yk_cmd[3].u16_data_or_num != g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range)
	{
		m_t_ac10_yk_cmd[3].u16_data_or_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u16_a3_s3_shunt_range;
		pt_record->u8_cmd_index = AC10_MODULE_WRITE_YK_CMD;
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[3].u8_func_code;	
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[3].u16_reg_addr;				
		m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[3].u16_data_or_num;
		data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		u16_cfg_changed = 1;
	}

	os_mut_release(g_mut_share_data);


	switch (pt_record->u8_cmd_index)
	{
		case AC10_MODULE_READ_YC_CMD:
			data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case AC10_MODULE_READ_YX_CMD:
			data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			break;

		case AC10_MODULE_WRITE_YK_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);            
			if ( !u16_cfg_changed )
			{	//周期下发
				m_u8_ac10_yk_idx++;
				m_u8_ac10_yk_idx %= AC10_YK_CMD_NUM;
				m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code = m_t_ac10_yk_cmd[m_u8_ac10_yk_idx].u8_func_code;	
				m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr = m_t_ac10_yk_cmd[m_u8_ac10_yk_idx].u16_reg_addr;				
				m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num = m_t_ac10_yk_cmd[m_u8_ac10_yk_idx].u16_data_or_num;
				data_or_num = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
			}
			os_mut_release(g_mut_share_data);
			break;

		default:
			return;
	}

	m_u8_com1_tx_buf[0] = AC10_MODULE_START_ADDR;
	m_u8_com1_tx_buf[1] = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
	m_u8_com1_tx_buf[2] = (m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
	m_u8_com1_tx_buf[3] = m_t_ac10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
	m_u8_com1_tx_buf[4] = (data_or_num >> 8);
	m_u8_com1_tx_buf[5] = data_or_num;
	crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, 6);
	m_u8_com1_tx_buf[6] = (U8_T)crc;
	m_u8_com1_tx_buf[7] = (U8_T)(crc >> 8);

#ifdef COM1_DEBUG
	DEBUG("send cmd: ");
	for (i=0; i<8; i++)
	{
		DEBUG("%02x ", m_u8_com1_tx_buf[i]);
	}
	DEBUG("\r\n");
#endif

	if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8) != TRUE)
	{
		os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
		Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, 8);
	}
}

/*************************************************************
函数名称: u16_com1_ac10_create_fault_id		           				
函数功能: 根据AC10模块索引和故障类型计算故障ID						
输入参数: u8_module_index -- 模块索引
          u8_fault_type   -- 故障类型    		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_ac10_create_fault_id(U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET);//(FAULT_FEEDER_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= u8_fault_type;

	return u16_fault_id;
}

/*************************************************************
函数名称: v_com1_ac10_module_comm_fault_alm		           				
函数功能: AC10模块通信故障告警						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_ac10_module_comm_fault_alm(AC10_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_ac10_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
		{
			//报通信故障
			v_fauid_send_fault_id_occur(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_comm_state = 1;   //设置通信中断标志
			os_mut_release(g_mut_share_data);
		}
	}
}

/****************************************************************************
 *  函数名称：  v_com1_ac10_bus_fault_judge(void) 
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   判断采集上来的交流柜母线数据是否异常，异常则至相应标志位 
 ***************************************************************************/
//	 static U32_T ac_ov_start1 = u32_delay_get_timer_val();
//	 static U32_T ac_ov_start2 = u32_delay_get_timer_val();
void v_com1_ac10_bus_fault_judge(void)
 {

	 U8_T   u8_ac_ov_delay ;	 
 	U16_T id;
 	AC10_RT_DATA_T *p_ac_rt_data = NULL;
	AC10_CFG_T *p_ac_cfg = NULL;
	 



	os_mut_wait(g_mut_share_data, 0xFFFF);
	u8_ac_ov_delay = g_t_share_data.t_sys_cfg.t_sys_param.u8_ats_offline_cnt;
	p_ac_rt_data = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);
	p_ac_cfg = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10);

	//一路交流过压判断
	if( (p_ac_rt_data->f32_ac1_uv_volt >= p_ac_cfg->u16_high_volt) || 
		(p_ac_rt_data->f32_ac1_vw_volt >= p_ac_cfg->u16_high_volt) ||
		(p_ac_rt_data->f32_ac1_wu_volt >= p_ac_cfg->u16_high_volt) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0004))
		{	
			if(u32_delay_time_elapse(m_u32_ac_ov_start1,u32_delay_get_timer_val()) > u8_ac_ov_delay *1000000)
			{
				p_ac_rt_data->u16_ac_state |= 0x0004;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
	}
	else if ( (p_ac_rt_data->f32_ac1_uv_volt < (p_ac_cfg->u16_high_volt - 3.0)) && 
			  (p_ac_rt_data->f32_ac1_vw_volt < (p_ac_cfg->u16_high_volt - 3.0)) &&
			  (p_ac_rt_data->f32_ac1_wu_volt < (p_ac_cfg->u16_high_volt - 3.0)) )
	{
		m_u32_ac_ov_start1 = u32_delay_get_timer_val();
		if((p_ac_rt_data->u16_ac_state)&0x0004)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0004);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OVER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//一路交流停电判断
	if( (p_ac_rt_data->f32_ac1_uv_volt < p_ac_cfg->u16_lack_phase) && 
		(p_ac_rt_data->f32_ac1_vw_volt < p_ac_cfg->u16_lack_phase) &&
		(p_ac_rt_data->f32_ac1_wu_volt < p_ac_cfg->u16_lack_phase) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0001))
		{
			p_ac_rt_data->u16_ac_state |= 0x0001;
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OFF;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else if ( (p_ac_rt_data->f32_ac1_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) || 
			  (p_ac_rt_data->f32_ac1_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) ||
			  (p_ac_rt_data->f32_ac1_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
	{
		if((p_ac_rt_data->u16_ac_state)&0x0001)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0001);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_OFF;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//一路交流缺相判断
	if( !((p_ac_rt_data->u16_ac_state)&0x0001) )
	{	//不停电时才判是否缺相
		if( (p_ac_rt_data->f32_ac1_uv_volt < p_ac_cfg->u16_lack_phase) || 
			(p_ac_rt_data->f32_ac1_vw_volt < p_ac_cfg->u16_lack_phase) ||
			(p_ac_rt_data->f32_ac1_wu_volt < p_ac_cfg->u16_lack_phase) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0008))
			{
				p_ac_rt_data->u16_ac_state |= 0x0008;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_LACK_PHASE;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac1_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) && 
				  (p_ac_rt_data->f32_ac1_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) &&
				  (p_ac_rt_data->f32_ac1_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0008)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0008);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_LACK_PHASE;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	
	//一路交流欠压判断
	if( !((p_ac_rt_data->u16_ac_state)&0x0009) )
	{	//不停电，且不缺相时才判是否欠压
		if( (p_ac_rt_data->f32_ac1_uv_volt < p_ac_cfg->u16_low_volt) || 
			(p_ac_rt_data->f32_ac1_vw_volt < p_ac_cfg->u16_low_volt) ||
			(p_ac_rt_data->f32_ac1_wu_volt < p_ac_cfg->u16_low_volt) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0002))
			{
				p_ac_rt_data->u16_ac_state |= 0x0002;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac1_uv_volt >= (p_ac_cfg->u16_low_volt + 3.0)) && 
				  (p_ac_rt_data->f32_ac1_vw_volt >= (p_ac_cfg->u16_low_volt + 3.0)) &&
				  (p_ac_rt_data->f32_ac1_wu_volt >= (p_ac_cfg->u16_low_volt + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0002)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0002);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH1_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}

	
	
	//二路交流过压判断
	if( (p_ac_rt_data->f32_ac2_uv_volt >= p_ac_cfg->u16_high_volt) || 
		(p_ac_rt_data->f32_ac2_vw_volt >= p_ac_cfg->u16_high_volt) ||
		(p_ac_rt_data->f32_ac2_wu_volt >= p_ac_cfg->u16_high_volt) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0040))
		{
			if(u32_delay_time_elapse(m_u32_ac_ov_start2,u32_delay_get_timer_val()) > u8_ac_ov_delay *1000000)
			{
				p_ac_rt_data->u16_ac_state |= 0x0040;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
	}
	else if ( (p_ac_rt_data->f32_ac2_uv_volt < (p_ac_cfg->u16_high_volt - 3.0)) && 
			  (p_ac_rt_data->f32_ac2_vw_volt < (p_ac_cfg->u16_high_volt - 3.0)) &&
			  (p_ac_rt_data->f32_ac2_wu_volt < (p_ac_cfg->u16_high_volt - 3.0)) )
	{
		m_u32_ac_ov_start2 = u32_delay_get_timer_val();
		if((p_ac_rt_data->u16_ac_state)&0x0040)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0040);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OVER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//二路交流停电判断
	if( (p_ac_rt_data->f32_ac2_uv_volt < p_ac_cfg->u16_lack_phase) && 
		(p_ac_rt_data->f32_ac2_vw_volt < p_ac_cfg->u16_lack_phase) &&
		(p_ac_rt_data->f32_ac2_wu_volt < p_ac_cfg->u16_lack_phase) )
	{
		if(!((p_ac_rt_data->u16_ac_state)&0x0010))
		{
			p_ac_rt_data->u16_ac_state |= 0x0010;
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OFF;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else if ( (p_ac_rt_data->f32_ac2_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) || 
			  (p_ac_rt_data->f32_ac2_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) ||
			  (p_ac_rt_data->f32_ac2_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
	{
		if((p_ac_rt_data->u16_ac_state)&0x0010)
		{
			p_ac_rt_data->u16_ac_state &= (~0x0010);
			id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_OFF;
			v_fauid_send_fault_id_resume(id);
		}
	}
	
	//二路交流缺相判断，单相输入无缺相
	if( !((p_ac_rt_data->u16_ac_state)&0x0010) )
	{	//不停电时才判是否缺相
		if( (p_ac_rt_data->f32_ac2_uv_volt < p_ac_cfg->u16_lack_phase) || 
			(p_ac_rt_data->f32_ac2_vw_volt < p_ac_cfg->u16_lack_phase) ||
			(p_ac_rt_data->f32_ac2_wu_volt < p_ac_cfg->u16_lack_phase) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0080))
			{
				p_ac_rt_data->u16_ac_state |= 0x0080;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_LACK_PHASE;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac2_uv_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) && 
				  (p_ac_rt_data->f32_ac2_vw_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) &&
				  (p_ac_rt_data->f32_ac2_wu_volt >= (p_ac_cfg->u16_lack_phase + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0080)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0080);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_LACK_PHASE;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	
	//二路交流欠压判断
	if( !((p_ac_rt_data->u16_ac_state)&0x0090) )
	{	//不停电，且不缺相时才判是否欠压
		if( (p_ac_rt_data->f32_ac2_uv_volt < p_ac_cfg->u16_low_volt) || 
			(p_ac_rt_data->f32_ac2_vw_volt < p_ac_cfg->u16_low_volt) ||
			(p_ac_rt_data->f32_ac2_wu_volt < p_ac_cfg->u16_low_volt) )
		{
			if(!((p_ac_rt_data->u16_ac_state)&0x0020))
			{
				p_ac_rt_data->u16_ac_state |= 0x0020;
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else if ( (p_ac_rt_data->f32_ac2_uv_volt >= (p_ac_cfg->u16_low_volt + 3.0)) && 
				  (p_ac_rt_data->f32_ac2_vw_volt >= (p_ac_cfg->u16_low_volt + 3.0)) &&
				  (p_ac_rt_data->f32_ac2_wu_volt >= (p_ac_cfg->u16_low_volt + 3.0)) )
		{
			if((p_ac_rt_data->u16_ac_state)&0x0020)
			{
				p_ac_rt_data->u16_ac_state &= (~0x0020);
				id = (FAULT_AC_GROUP<<FAULT_GROUP_OFFSET)|FAULT_AC2_PATH2_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}	
	
	os_mut_release(g_mut_share_data);		
 }

/*************************************************************
函数名称: v_com1_ac10_module_unpacket		           				
函数功能: AC10模块解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
//#define		LINE_VOL_TO_PHASE_VOL		1.732	   //线电压转换为相电压比例系数
static void v_com1_ac10_module_unpacket(AC10_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0, u16_data;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case AC10_MODULE_READ_YC_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])*2 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				AC10_RT_DATA_T *ac10 = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);

				os_mut_wait(g_mut_share_data, 0xFFFF);
				//一路交流，三相进线采样
				u16_data = ((m_u8_com1_rx_buf[i+3]<<8) + m_u8_com1_rx_buf[i+4]);
				ac10->f32_ac1_uv_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+5]<<8) + m_u8_com1_rx_buf[i+6]);
				ac10->f32_ac1_vw_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+7]<<8) + m_u8_com1_rx_buf[i+8]);
				ac10->f32_ac1_wu_volt = (F32_T)u16_data / 10.0;

                //二路交流，三相进线采样
				u16_data = ((m_u8_com1_rx_buf[i+9]<<8) + m_u8_com1_rx_buf[i+10]);
				ac10->f32_ac2_uv_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+11]<<8) + m_u8_com1_rx_buf[i+12]);
				//f32_ac_ub = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
				ac10->f32_ac2_vw_volt = (F32_T)u16_data / 10.0;

				u16_data = ((m_u8_com1_rx_buf[i+13]<<8) + m_u8_com1_rx_buf[i+14]);
				//f32_ac_uc = (F32_T)u16_data / 10.0 / LINE_VOL_TO_PHASE_VOL;
				ac10->f32_ac2_wu_volt = (F32_T)u16_data / 10.0;


                //环境温度
//				u16_data = ((m_u8_com1_rx_buf[i+15]<<8) + m_u8_com1_rx_buf[i+16]);

				//V1控母电压
//				u16_data = ((m_u8_com1_rx_buf[i+17]<<8) + m_u8_com1_rx_buf[i+18]);
//				ac10->f32_v1_volt = ((F32_T)(S16_T)u16_data) / 10.0;
//				//A1分流器负载电流
//				u16_data = ((m_u8_com1_rx_buf[i+19]<<8) + m_u8_com1_rx_buf[i+20]);
//				ac10->f32_a1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
//				if (ac10->f32_a1_curr < 0)
//				{
//					ac10->f32_a1_curr = 0;
//				}
//				//V2电池电压
//				u16_data = ((m_u8_com1_rx_buf[i+21]<<8) + m_u8_com1_rx_buf[i+22]);
//				ac10->f32_v2_volt = ((F32_T)(S16_T)u16_data) / 10.0;
//				//A2分流器电池电流
//				u16_data = ((m_u8_com1_rx_buf[i+23]<<8) + m_u8_com1_rx_buf[i+24]);
//				ac10->f32_a2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
//				//V3合母电压
//				u16_data = ((m_u8_com1_rx_buf[i+25]<<8) + m_u8_com1_rx_buf[i+26]);
//				ac10->f32_v3_volt = ((F32_T)(S16_T)u16_data) / 10.0;
//				//A3分流器电池2电流
//				u16_data = ((m_u8_com1_rx_buf[i+27]<<8) + m_u8_com1_rx_buf[i+28]);
//				ac10->f32_a3_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S1传感器一路交流电流
				u16_data = ((m_u8_com1_rx_buf[i+29]<<8) + m_u8_com1_rx_buf[i+30]);
				ac10->f32_s1_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (ac10->f32_s1_curr < 0)
				{
					ac10->f32_s1_curr = 0;
				}
//				//S2传感器
//				u16_data = ((m_u8_com1_rx_buf[i+31]<<8) + m_u8_com1_rx_buf[i+32]);
//				ac10->f32_s2_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				//S3传感器二路交流电流
				u16_data = ((m_u8_com1_rx_buf[i+33]<<8) + m_u8_com1_rx_buf[i+34]);
				ac10->f32_s3_curr = ((F32_T)(S16_T)u16_data) / 10.0;
				if (ac10->f32_s3_curr < 0)
				{
					ac10->f32_s3_curr = 0;
				}

				//交流母线异常判断处理
				v_com1_ac10_bus_fault_judge();								

				//清通信中断	
				ac10->u8_comm_state = 0;                                    //清除通信中断标志				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_ac10_module_comm_fault_alm(pt_record);
			}

			break;

		case AC10_MODULE_READ_YX_CMD:
			rx_len = 0;
			byte_len = ((m_u8_com1_tx_buf[4]<<8) + m_u8_com1_tx_buf[5])/8 + 5;  //地址码1个字节、功能码1个字节、返回字节数1个字节、CRC两个字节，总共加5个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1])
							&& (m_u8_com1_rx_buf[i+2] == (byte_len-5)))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				AC10_RT_DATA_T *ac10 = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);
				U16_T u16_state_new;

#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				os_mut_wait(g_mut_share_data, 0xFFFF);
				//state1 = ((m_u8_com1_rx_buf[i+4]<<8) + m_u8_com1_rx_buf[i+3]);
				//state2 = ((m_u8_com1_rx_buf[i+6]<<8) + m_u8_com1_rx_buf[i+5]);
				//state3 = ((m_u8_com1_rx_buf[i+8]<<8) + m_u8_com1_rx_buf[i+7]);
				u16_state_new = (U16_T)((m_u8_com1_rx_buf[i+10]<<8) + m_u8_com1_rx_buf[i+9]);
				//ac10->u16_swt_state = u16_state_new;
				for(i=0; i<DC10_SWT_BRANCH_MAX; i++)
				{
					if( u16_state_new & (0x0001<<i) )
					{
						ac10->u8_swt_state[i] = 1;	//闭合
					}
					else
					{
						ac10->u8_swt_state[i] = 0;	//断开
					}
				}
				
				//1~3HK开关跳闸报警处理
				for(i=3; i<DC10_SWT_BRANCH_MAX; i++)
				{				
					if( u16_state_new & (0x0001<<i) )
					{	//跳闸					
						if( !(ac10->u16_swt_state & (0x0001<<i)) )
						{
							ac10->u16_swt_state |= (0x0001<<i);
							//v_fauid_send_fault_id_occur(u16_com1_ac10_create_fault_id(FAULT_AC10_SWT_BEGIN+i));
							v_fauid_send_fault_id_occur((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+(FAULT_AC10_SWT_BEGIN+i));
						}
					}
					else
					{	//正常
						if( (ac10->u16_swt_state & (0x0001<<i)) )
						{
							ac10->u16_swt_state &= ~(0x0001<<i);
							//v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_SWT_BEGIN+i));
							v_fauid_send_fault_id_resume((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+(FAULT_AC10_SWT_BEGIN+i));
						}
					}
				}				

				//清通信中断	
				ac10->u8_comm_state = 0;                                    //清除通信中断标志				
				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_ac10_module_comm_fault_alm(pt_record);
			}

			break;

		case AC10_MODULE_WRITE_YK_CMD:
			rx_len = 0;
			byte_len = 8;  //地址码1个字节、功能码1个字节、2字节寄存器地址、2字节寄存器数据、CRC两个字节，总共加8个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i]   == m_u8_com1_tx_buf[0]) && 
						    (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1]))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
				AC10_RT_DATA_T *ac10 = &(g_t_share_data.t_rt_data.t_ac_panel.t_ac10);

				os_mut_wait(g_mut_share_data, 0xFFFF);	
				ac10->u8_comm_state = 0;                                    //清除通信中断标志				
				os_mut_release(g_mut_share_data);

				if (pt_record->u8_com1_fail_cnt >= m_u8_ac10_offline_cnt)
				{
					//复归通信故障报警
					v_fauid_send_fault_id_resume(u16_com1_ac10_create_fault_id(FAULT_AC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_ac10_module_comm_fault_alm(pt_record);
			}
			break;
					
		default:
			break;
	}
}

/*************************************************************
函数名称: v_com1_ac10_module_comm_handle		           				
函数功能: AC10模块通信及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_ac10_module_comm_handle(AC10_RECORD_T *pt_record)
{ 
	v_com1_ac10_module_send_cmd(pt_record);
	v_com1_ac10_module_unpacket(pt_record);

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= AC10_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
}

/******************************* AC10模块相关函数定义结束 ***************************/

/******************************* RC10模块相关函数定义开始 ***************************/
/*************************************************************
函数名称: v_com1_motor_switch_sync		           				
函数功能: 系统电操开关当前状态和监控控制状态关联更新						
输入参数: 无       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_motor_switch_sync()
{
	U16_T i;
	U32_T u32_cur_time;

	u32_cur_time = u32_delay_get_timer_val();
	if( u32_delay_time_elapse(m_u32_switch_sync_time, u32_cur_time) >= COM1_SWT_SYNC_UPDATE_TM )
	{	//定期将当前电操开关状态关联更新到电操控制量，以解决电操当前状态与控制量不同步而不能控制问题。
		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		for (i=0; i < FACT_SWT_CTRL_MAX; i++)
		{
			if ( g_t_swt_sheet[i].u8_swt_valid )
			{	//将当前电操开关状态同步更新到电操开关控制量
				(*g_t_swt_sheet[i].p_u8_swt_ctrl) = (*g_t_swt_sheet[i].p_u8_swt_state);
				//确保电操开关控制量同步更新后不再下发控制命令 
				m_u8_rc10_swt_bak[i] = (*g_t_swt_sheet[i].p_u8_swt_ctrl);
			}
		}

		os_mut_release(g_mut_share_data);

		m_u32_switch_sync_time = u32_cur_time;//重新开始电操开关同步更新计时
	} 
}

/*************************************************************
函数名称: u16_com1_rc10_module_send_cmd		           				
函数功能: RC10模块发送命令函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static U16_T u16_com1_rc10_module_send_cmd(MODULE_RECORD_T *pt_record)
{
	U16_T crc, update = 0;
	U16_T i, j, start_idx;
	U32_T u32_cur_time, u32_start_time;

	u32_cur_time = u32_delay_get_timer_val();
	u32_start_time = m_u32_rc10_module_ctrl_tm[pt_record->u8_module_index];
	if( !(u32_delay_time_elapse(u32_start_time, u32_cur_time) >= COM1_RC10_CTRL_SPACE_TM) )
	{	//命令发送间隔保护，防止频繁下发控制命令
		return update;
	}

	m_u32_rc10_module_ctrl_tm[pt_record->u8_module_index] = u32_delay_get_timer_val();
	memset(m_u8_rc10_write_data, 0, sizeof(m_u8_rc10_write_data));

	switch (pt_record->u8_cmd_index)
	{
		case RC10_MODULE_WRITE_CMD:
			os_mut_wait(g_mut_share_data, 0xFFFF);
			start_idx = pt_record->u8_module_index * SWT_CTRL_MAX;		  
			for (j=0, i=start_idx; ((j < SWT_CTRL_MAX) && (i < FACT_SWT_CTRL_MAX));  j++, i++)
			{
				if ( g_t_swt_sheet[i].u8_swt_valid )
				{
					if ( m_u8_rc10_swt_bak[i] != (*g_t_swt_sheet[i].p_u8_swt_ctrl) )//(*g_t_swt_sheet[i].p_u8_swt_state)
					{		
						m_u8_rc10_swt_bak[i] = (*g_t_swt_sheet[i].p_u8_swt_ctrl);

						if ((*g_t_swt_sheet[i].p_u8_swt_ctrl) == 0)	//0表示分闸，1表示合闸
						{
							m_u8_rc10_write_data[2*j]   = 0;	//合闸控制
							m_u8_rc10_write_data[2*j+1] = 1;	//分闸控制
						}
						else
						{
							m_u8_rc10_write_data[2*j]   = 1;	//合闸控制
							m_u8_rc10_write_data[2*j+1] = 0;	//分闸控制
						}

						update = 1;
					}
				}				
			}			
			os_mut_release(g_mut_share_data);			
			break;

		default:
			break;
	}

	if (update)
	{
		m_u32_switch_sync_time = u32_delay_get_timer_val();//发现有新电操开关控制时，重新开始电操开关同步更新计时

		i = 0;
		m_u8_com1_tx_buf[i++] = RC10_MODULE_START_ADDR + pt_record->u8_module_index;
		m_u8_com1_tx_buf[i++] = m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u8_func_code;
		m_u8_com1_tx_buf[i++] = (m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr >> 8);
		m_u8_com1_tx_buf[i++] = m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_reg_addr;
		m_u8_com1_tx_buf[i++] = (m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num >> 8);
		m_u8_com1_tx_buf[i++] = m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num;
		m_u8_com1_tx_buf[i++] = (U8_T)(m_t_rc10_module_normal_cmd[pt_record->u8_cmd_index].u16_data_or_num * 2);
		for (j=0; j<RC10_NODE_MAX; j++)
		{
			m_u8_com1_tx_buf[i++] = 0;
			m_u8_com1_tx_buf[i++] = m_u8_rc10_write_data[j];
		}
		crc = u16_crc_calculate_crc(m_u8_com1_tx_buf, i);
		m_u8_com1_tx_buf[i++] = (U8_T)crc;
		m_u8_com1_tx_buf[i++] = (U8_T)(crc >> 8);
	
		if (Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, i) != TRUE)
		{
			os_dly_wait(COM1_COMM_WAIT_TICK);     //发送失败，则等100ms再重试
			Uart_send(COM1_COM_PORT_NUM, m_u8_com1_tx_buf, i);
		}
	}

	return (update);
}

/*************************************************************
函数名称: u16_com1_rc10_create_fault_id		           				
函数功能: 根据RC10模块索引和故障类型计算故障ID						
输入参数: u8_module_index -- 模块索引
          u8_fault_type   -- 故障类型    		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
static U16_T u16_com1_rc10_create_fault_id(U8_T u8_module_index, U8_T u8_fault_type)
{
	U16_T u16_fault_id;

	u16_fault_id = (FAULT_RC_GROUP<<FAULT_GROUP_OFFSET);
	u16_fault_id |= (u8_fault_type + u8_module_index);

	return u16_fault_id;
}

/*************************************************************
函数名称: v_com1_rc10_module_comm_fault_alm		           				
函数功能: RC10模块RC10故障告警						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rc10_module_comm_fault_alm(MODULE_RECORD_T *pt_record)
{
	if (pt_record->u8_com1_fail_cnt < m_u8_rc10_offline_cnt)
	{
		pt_record->u8_com1_fail_cnt++;
		if (pt_record->u8_com1_fail_cnt >= m_u8_rc10_offline_cnt)
		{
			//报RC10故障
			v_fauid_send_fault_id_occur(u16_com1_rc10_create_fault_id(pt_record->u8_module_index, FAULT_RC10_COMM_FAIL));

			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_rc10[pt_record->u8_module_index].u8_comm_state = 1;   //设置RC10中断标志
			os_mut_release(g_mut_share_data);
		}
	}
}

/*************************************************************
函数名称: v_com1_rc10_module_unpacket		           				
函数功能: RC10模块解包函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rc10_module_unpacket(MODULE_RECORD_T *pt_record)
{
	U16_T byte_len, rx_len, timeout = 0;
	U32_T i, j, crc_scuess;

	switch (pt_record->u8_cmd_index)
	{
		case RC10_MODULE_WRITE_CMD:
			rx_len = 0;
			byte_len = 8;  //地址码1个字节、功能码1个字节、寄存器地址2个字节、寄存器个数2个字节、CRC两个字节，总共加8个字节

			crc_scuess = 0;
			timeout = 0;

			while (timeout < COM1_RECV_TIMEOUT_CNT)    //是否超时
			{
				os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
				os_dly_wait(COM1_RECV_WAIT_TICK);
				timeout++;

				rx_len += Uart_recv(COM1_COM_PORT_NUM, m_u8_com1_rx_buf+rx_len, COM1_RX_BUF_SIZE-rx_len);
				if (rx_len < byte_len)
				{
					continue;
				}
				else
				{
					for (i=0; i<rx_len-2; i++)
					{
						if ((m_u8_com1_rx_buf[i] == m_u8_com1_tx_buf[0])
							&& (m_u8_com1_rx_buf[i+1] == m_u8_com1_tx_buf[1]))
						{
							if (rx_len - i < byte_len)
							{
								break;
							}
							else
							{
								if (u16_crc_calculate_crc(&(m_u8_com1_rx_buf[i]), byte_len) != 0)
								{
									continue;
								}
								else
								{
									crc_scuess = 1;
									break;
								}
							}
						}
					}

					if (crc_scuess == 1)
					{
						break;
					}
					else
					{
						for (j=0; j<(rx_len-i); j++)
							m_u8_com1_rx_buf[j] = m_u8_com1_rx_buf[j+i];

						rx_len -= i;
					}
				}  // end of if (rx_len < byte_len)
			}  //end of while (timeout < COM1_RECV_TIMEOUT_CNT)

			if (crc_scuess == 1)
			{
#ifdef COM1_DEBUG
				DEBUG("recv data: ");
				for (j=0; j<byte_len; j++)
				{
					DEBUG("%02x ", m_u8_com1_rx_buf[i+j]);
				}
				DEBUG("\r\n");
#endif

				if (pt_record->u8_com1_fail_cnt >= m_u8_rc10_offline_cnt)
				{
					//复归RC10故障报警
					v_fauid_send_fault_id_resume(u16_com1_rc10_create_fault_id(pt_record->u8_module_index, FAULT_RC10_COMM_FAIL));
				}
				pt_record->u8_com1_fail_cnt = 0;
			}
			else
			{
				v_com1_rc10_module_comm_fault_alm(pt_record);
			}

			break;
					
		default:
			break;
	}
}

/*************************************************************
函数名称: v_com1_rc10_module_comm_handle		           				
函数功能: RC10模块RC10及数据处理函数						
输入参数: pt_record -- 模块记录数据        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_rc10_module_comm_handle(MODULE_RECORD_T *pt_record)
{ 
	if (u16_com1_rc10_module_send_cmd(pt_record))
	{
		v_com1_rc10_module_unpacket(pt_record);
	}

	pt_record->u8_cmd_index++;
	pt_record->u8_cmd_index %= RC10_MODULE_NORMAL_CMD_CNT;

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
}

/******************************* RC10模块相关函数定义结束 ***************************/


/*************************************************************
函数名称: v_com1_set_cmd_handle		           				
函数功能: 根据本地备份的数据和共享数据区的数据比较，判断下级模块的设置数据是否己发生改变，
          如果己改变，则发送设置命令，该函数在每个查询命令完成后都会调用一次，以加快设置数据的下发							
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
static void v_com1_set_cmd_handle(void)
{
	F32_T f32_rect_out_volt[2];
	F32_T f32_rect_curr_percent[2];
	F32_T u16_pb_high_volt;
	F32_T u16_pb_low_volt;
	F32_T f32_rect_offline_out_volt;
	U16_T u16_rect_ctrl[RECT_CNT_MAX];
	F32_T f32_dcdc_out_volt;
	F32_T f32_dcdc_curr_percent;
	U32_T rect_module_num, dcdc_module_num ,i;
	U8_T  u8_batt_group_num;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	u8_batt_group_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
	rect_module_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
	u16_pb_high_volt = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt;
	u16_pb_low_volt = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt;
	f32_rect_offline_out_volt = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_out_volt;
	memcpy(u16_rect_ctrl, g_t_share_data.t_sys_cfg.t_ctl.u16_rect_ctrl, sizeof(u16_rect_ctrl));
	
	dcdc_module_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	f32_dcdc_out_volt = g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt;
	f32_dcdc_curr_percent = g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent;
	
	os_mut_release(g_mut_share_data);
	
	for (i=0; i<u8_batt_group_num; i++)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		f32_rect_out_volt[i] = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[i];
		f32_rect_curr_percent[i] = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[i];		
		os_mut_release(g_mut_share_data);
		
		if (fabs(m_f32_rect_out_volt[i] - f32_rect_out_volt[i]) > 0.01)             //整流模块输出电压被改变，发送调压命令
		{
			m_f32_rect_out_volt[i] = f32_rect_out_volt[i];
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_VOLT_CMD);
		}
		
		if (fabs(m_f32_rect_curr_percent[i] - f32_rect_curr_percent[i]) > 0.01)     //整流模块限流点被改变，发送限流命令
		{
			m_f32_rect_curr_percent[i] = f32_rect_curr_percent[i];
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_CURR_CMD);  //发送限流命令完成，发送事件标志给电池管理任务
		}

		if (m_u16_pb_high_volt[i] != u16_pb_high_volt)                           //整流模块输出电压上限被改变，发送设置模块输出电压上限命令
		{
			m_u16_pb_high_volt[i] = u16_pb_high_volt;
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_HIGH_VOLT_CMD);
		}
		
		if (m_u16_pb_low_volt[i] != u16_pb_low_volt)                             //整流模块输出电压下限被改变，发送设置模块输出电压下限命令
		{
			m_u16_pb_low_volt[i] = u16_pb_low_volt;
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_LOW_VOLT_CMD);
		}
		
		if (fabs(m_f32_rect_offline_out_volt[i] - f32_rect_offline_out_volt) > 0.01)//整流模块默认输出电压被改变，发送设置模块默认输出电压命令
		{
			m_f32_rect_offline_out_volt[i] = f32_rect_offline_out_volt;
			v_com1_rect_module_send_broadcast_cmd(i, RECT_MODULE_SET_DEF_VOLT_CMD);
		}
	}

	for (i=0; i<rect_module_num; i++)                                                //整流模块开关机控制
	{
		if (m_u16_rect_ctrl[i] != u16_rect_ctrl[i])
		{
			m_u16_rect_ctrl[i] = u16_rect_ctrl[i];
			
			m_t_rect_module_record[i].u8_cmd_index = RECT_MODULE_SET_STATE_CMD;
			v_com1_rect_module_comm_handle(&(m_t_rect_module_record[i]));
		}	
	}

	if (dcdc_module_num > 0)
	{
		if (fabs(m_f32_dcdc_out_volt - f32_dcdc_out_volt) > 0.01)         //通信模块输出电压被改变，发送调压命令
		{
			m_f32_dcdc_out_volt = f32_dcdc_out_volt;
			v_com1_dcdc_module_send_broadcast_cmd(DCDC_MODULE_SET_VOLT_CMD);
			v_com1_dcdc_module_send_broadcast_cmd(DCDC_MODULE_SET_DEF_VOLT_CMD);
		}
		
		if (fabs(m_f32_dcdc_curr_percent - f32_dcdc_curr_percent) > 0.01) //通信模块限流点被改变，发送限流命令
		{
			m_f32_dcdc_curr_percent = f32_dcdc_curr_percent;
			v_com1_dcdc_module_send_broadcast_cmd(DCDC_MODULE_SET_CURR_CMD);
		}
	}

}


/*************************************************************
函数名称: v_com1_module_comm_task
函数功能: 下级模块通信及数据处理任务函数
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
__task void v_com1_module_comm_task(void)
{
	U16_T i, batt_num, bms_num1, bms_num2, rect_num, cps_num; 
	U16_T ups_num, dc10_num, ac10_num, rc10_num;
	BMS_TYPE_E e_bms_type;
	U8_T u8_rect_broadcast_cmd_index = 0;     //广播命令索引
	U8_T u8_dcdc_broadcast_cmd_index = 0;     //广播命令索引

	os_evt_set(COM1_FEED_DOG, g_tid_wdt);     //设置喂狗事件标志
	
	//增加以下代码以清除保存的遥控开关值
	os_mut_wait(g_mut_share_data, 0xFFFF);
	memset(g_t_share_data.t_sys_cfg.t_swt_ctrl, 0, (RC10_MODULE_MAX * sizeof(SWT_CTRL_T)));
	os_mut_release(g_mut_share_data);

	m_u32_switch_sync_time = u32_delay_get_timer_val();
	m_u32_ac_ov_start1 = u32_delay_get_timer_val();
	m_u32_ac_ov_start2 = u32_delay_get_timer_val();
	while (1)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);
		m_u8_bms_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_bms_offline_cnt;
		m_u8_rect_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_rect_offline_cnt;
		m_u8_dcdc_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dcdc_offline_cnt;
		m_u8_dcac_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dcac_offline_cnt;
//		m_u8_d21_offline_cnt = m_u8_rect_offline_cnt;	//共用整流模块的通信计数参考值
		m_u8_dc10_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt;
		m_u8_ac10_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc10_offline_cnt; 
		m_u8_rc10_offline_cnt = g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_offline_cnt; 

		//电池巡检通信处理
		batt_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		bms_num1 = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[0].u8_bms_num;
		bms_num2 = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[1].u8_bms_num;
		e_bms_type = g_t_share_data.t_sys_cfg.t_batt.e_bms_type;
		rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
		if (g_t_share_data.t_sys_cfg.t_dcdc_panel.e_protocol == DCDC_MODBUS)
			cps_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
		else
			cps_num = 0;
		ups_num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num;
		dc10_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc10.u8_dc10_module_num;
		ac10_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_ac10.u8_ac10_module_num;
		rc10_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_rc10_module_num;
		os_mut_release(g_mut_share_data);
		
		if (batt_num > 0)
		{
			for (i=0; i<bms_num1; i++)
			{
				v_com1_bms_comm_handle(&(m_t_bms_group1_record[i]));
				v_com1_set_cmd_handle();
			}
			
			if (batt_num > 1)
			{
				for (i=0; i<bms_num2; i++)
				{
					if (e_bms_type == B21)
						m_t_bms_group2_record[i].u8_start_addr = 5;
					else if (e_bms_type == B3)
						m_t_bms_group2_record[i].u8_start_addr = 2;
					else
						m_t_bms_group2_record[i].u8_start_addr = 1;
						
					m_t_bms_group2_record[i].u8_total_index = bms_num1 + i;
					v_com1_bms_comm_handle(&(m_t_bms_group2_record[i]));
					v_com1_set_cmd_handle();
				}
			}
		}
		
		//整流模块通信处理
		for (i=0; i<rect_num; i++)
		{
			v_com1_rect_module_comm_handle(&(m_t_rect_module_record[i]));
			v_com1_set_cmd_handle();
		}
		
		for (i=0; i<batt_num; i++)
		{
			v_com1_rect_module_send_broadcast_cmd(i, u8_rect_broadcast_cmd_index);
		}
		u8_rect_broadcast_cmd_index++;
		u8_rect_broadcast_cmd_index %= RECT_MODULE_BROADCAST_CMD_CNT;
		v_com1_set_cmd_handle();
		
		//通信模块通信处理
		if (cps_num > 0)
		{
			for (i=0; i<cps_num; i++)
			{
				v_com1_dcdc_module_comm_handle(&(m_t_dcdc_module_record[i]));
				v_com1_set_cmd_handle();
			}
			
			v_com1_dcdc_module_send_broadcast_cmd(u8_dcdc_broadcast_cmd_index);
			u8_dcdc_broadcast_cmd_index++;
			u8_dcdc_broadcast_cmd_index %= DCDC_MODULE_BROADCAST_CMD_CNT;
			v_com1_set_cmd_handle();
		}
		
		//逆变模块通信处理
		for (i=0; i<ups_num; i++)
		{
			v_com1_dcac_module_comm_handle(&(m_t_dcac_module_record[i]));
			v_com1_set_cmd_handle();
		}

	    //D21模块通信处理
//		os_mut_wait(g_mut_share_data, 0xFFFF);
//		num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num;
//		os_mut_release(g_mut_share_data);
//		if(num > 0)
//		{
//			v_com1_d21_module_comm_handle(&m_t_d21_module_record);
//		}

		//1#DC10模块通信处理
		if (dc10_num)
		{
			v_com1_dc10_module_comm_handle(&m_t_dc10_module_record);
		}

		//2#DC10模块通信处理
		if (ac10_num)
		{
			v_com1_ac10_module_comm_handle(&m_t_ac10_module_record);
		}

		//RC10模块通信处理
		v_com1_motor_switch_sync();
		for (i=0; i<rc10_num; i++)
		{
			v_com1_rc10_module_comm_handle(&m_t_rc10_module_record[i]);
		}
	}
}
