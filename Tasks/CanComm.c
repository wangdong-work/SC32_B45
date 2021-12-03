/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：CanComm.c
版    本：1.00
创建日期：2012-08-08
作    者：郭数理
功能描述：馈线模块的通信及数据处理任务实现


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-08-08  1.00     创建
**************************************************************/

#include <rtl.h>
#include <string.h>
#include <lpc17xx.h>


#include "Type.h"
#include "PublicData.h"
#include "FaultId.h"
#include "CanDCDCComm.h"
#include "CompareTime.h"

#include "../Drivers/Delay.h"
#include "../Drivers/CAN.h"
#include "../Drivers/Rtc.h"


//#define CAN_FAIL_TIMEOUT       (10*1000000)  //10s通信不上，报通信故障
#define CAN_TX_MAX_CNT         32             //每次调用发送函数能发送命令的最多条数

/* 屏蔽码 */
#define ID_FEEDER_MODULE_TYPE  0x14000000     //设备类型
#define ID_DEVICE_TYPE_MASK    0xFFE00000     //设备类型屏蔽码
#define ID_DEVICE_ADDR_MASK    0x001FC000     //设备地址屏蔽码
#define ID_FUCN_CODE_MASK      0x00003C00     //功能码屏蔽码
#define ID_INFO_CODE_MASK      0x000003FF     //信息码屏蔽码
#define ID_ADDR_OFFSET         14             //地址偏移量


/* 功能码 */
#define FC_REQUEST_CONFIG (1<<10)             //请求配置数据
#define FC_ACTIVE_UPLOAD  (2<<10)             //主动上传数据
#define FC_REQUEST_READ   (3<<10)             //申请读取数据/回答读取数据
#define FC_REQUEST_WRITE  (6<<10)             //申请写数据
#define FC_RETURN_SCUESS  (14<<10)            //返回写命令执行成功
#define FC_RETURN_FAIL    (15<<10)            //返回写命令执行失败
                                              
/* 信息码 */                                  
#define IC_BRANCH_START       0               //馈出支路起始信息码
#define IC_BRANCH_END         63              //馈出支路结束信息码
#define IC_BUS_TO_GND_VOLT    128             //母线对地电压
#define IC_BUS_TO_GND_RES     129             //母线对地电阻
#define IC_BUS_TO_GND_AC_VOLT 130             //母线对地交流电压
#define FC_IC_TOTAL_SWT_FAULT 131             //总开关故障告警状态
#define IC_BRANCH_NUM         512             //各类型检测路数
#define IC_CURR_SENSOR_COEFF  513             //电流传感器比例系数
#define IC_INSU_ALM_THR       515             //绝缘报警门限值
#define IC_HW_VERSION         1001            //硬件版本号
#define IC_SW_VERSION         1002            //软件版本号

#define AC_PANEL_MODULE_BASE_ADDR      1      //交流屏馈线模块起始地址
#define AC_PANEL_MODULE_END_ADDR       3      //交流屏馈线模块起始地址
#define FEEDER_PANEL_MODULE_BASE_ADDR  14     //馈线屏馈线模块起始地址
#define FEEDER_PANEL_MODULE_BASE_ADDR2 30     //馈线屏馈线模块起始地址
#define DCDC_PANEL_MODULE_BASE_ADDR    112    //通信屏馈线模块起始地址
#define DCAC_PANEL_MODULE_BASE_ADDR    114    //逆变屏馈线模块起始地址


static F32_T m_f32_delay_scale;

/* 馈线模块数据记录 */
typedef struct
{
	U8_T u8_module_addr;                      //馈线模块地址
	U8_T u8_info_code_index;                  //信息码索引
	U32_T u32_fail_time;                  //通信失败记数
}FEEDER_MODULE_RECORD_T;



static FEEDER_MODULE_RECORD_T m_t_feeder_module_record[] = 
{	//交流屏馈线模块
	1, 0, 0,
	2, 0, 0,
	3, 0, 0,
	//一段直流1#分屏馈线模块
	14, 0, 0,
	15, 0, 0,
	16, 0, 0,
	17, 0, 0,
	//一段直流2#分屏馈线模块         
	18, 0, 0,
	19, 0, 0,
	20, 0, 0,
	21, 0, 0,
	//一段直流3#分屏馈线模块         
	22, 0, 0,
	23, 0, 0,
	24, 0, 0,
	25, 0, 0,
	//一段直流4#分屏馈线模块         
	26, 0, 0,
	27, 0, 0,
	28, 0, 0,
	29, 0, 0,
	//二段直流1#分屏馈线模块         
	30, 0, 0,
	31, 0, 0,
	32, 0, 0,
	33, 0, 0,
	//二段直流2#分屏馈线模块         
	34, 0, 0,
	35, 0, 0,
	36, 0, 0,
	37, 0, 0,
	//二段直流3#分屏馈线模块         
	38, 0, 0,
	39, 0, 0,
	40, 0, 0,
	41, 0, 0,
	//二段直流4#分屏馈线模块         
	42, 0, 0,
	43, 0, 0,
	44, 0, 0,
	45, 0, 0,
	//通信屏馈线模块
	112, 0, 0,
	//UPS屏馈线模块
	114, 0, 0,	
};


#define IC_READ_MAX_NUM       67             //信息码个数
static const U16_T m_u16_info_code[IC_READ_MAX_NUM] = 
{
	131, 1001, 1002,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
};


CAN_MSG_T m_t_tx_msg_buf;              //CAN发送缓冲区
CAN_MSG_T m_t_rx_msg_buf;              //CAN接收缓冲区

U32_T u32_fc_module_flag = 0;
U32_T u32_seg2_fc_module_flag = 0; 	   //二段直流有馈线模块
U32_T u32_dcdc_module_flag = 0;

static INSU_MEAS_WAY_E  m_e_insu_meas_way_bak;             // 母线电阻测量方式，工程模式和调试模式，默认工程模式
static U8_T 			m_u8_res_switch_delay_bak;         // 平衡及不平衡电桥投切延时，2~120可设，以秒为单位，默认2秒
static U16_T            m_u16_insu_sensor_range_bak;       // 支路传感器量程，1~500可设，以mA为单位，默认10mA
static U8_T             m_u8_insu_bus_err_confirm_bak;     // 母线绝缘压差报警确认时间，1~180可设，以秒为单位，默认3秒
static U8_T             m_u8_insu_meas_period_bak;         // 绝缘定期测量周期时间，0~180可设，以小时为单位，默认24小时

/*************************************************************
函数名称: v_can_feeder_module_data_handle
函数功能: 馈线模块接收数据处理函数，处理主动上传的数据（功能码02）和回答读取的数据（功能码03）
输入参数: pt_msg -- 指向CAN消息帧
输出参数: 无
返回值  ：无
**************************************************************/	
static void v_can_feeder_module_data_handle(CAN_MSG_T *pt_msg)
{
	U8_T u8_addr, u8_panel_index, u8_module_index, u8_branch_index;
	U16_T i, j, u16_info_code;
	FEEDER_MODULE_RT_DATA_T *pt_feeder_module = NULL;
	FEEDER_RT_DATA_T *pt_feeder_branch = NULL;
	U8_T *pu8_sensor_state = NULL;

	u8_addr = ((pt_msg->id & ID_DEVICE_ADDR_MASK) >> ID_ADDR_OFFSET);
	u16_info_code = (pt_msg->id & ID_INFO_CODE_MASK);
	
	if (u16_info_code <= IC_BRANCH_END)
	{
		U8_T u8_insu_state_old, u8_insu_state_new, u8_sensor_state_old, u8_sensor_state_new, u8_alarm_old, u8_alarm_new;
		U16_T u16_swt_fault_id, u16_insu_fault_id, u16_sensor_fault_id;
		
		if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR) && (u8_addr <= AC_PANEL_MODULE_END_ADDR))
		{
			u8_panel_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) / ACS_FEEDER_MODULE_MAX;
			u8_module_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;
			os_mut_wait(g_mut_share_data, 0xFFFF);
			u8_branch_index = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_start_num[u8_module_index] + u16_info_code;
			os_mut_release(g_mut_share_data);
			if (u8_branch_index >= FEEDER_BRANCH_MAX)    //防止数组越界
				return;
			
			pt_feeder_branch = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[u8_branch_index]);
			pu8_sensor_state = &(g_t_share_data.t_rt_data.t_ac_panel.u8_sensor_state[u8_branch_index]);
			
			u16_swt_fault_id = u16_insu_fault_id = u16_sensor_fault_id = ((FAULT_AC_PANEL_GROUP)<<FAULT_GROUP_OFFSET);
			u16_swt_fault_id |= (FAULT_AC_BRANCH_FAULT_CNT * u8_branch_index + FAULT_AC_BRANCH_SWT + FAULT_AC_BRANCH_BASE);
			u16_sensor_fault_id |= (FAULT_AC_BRANCH_FAULT_CNT * u8_branch_index + FAULT_AC_BRANCH_SENSOR + FAULT_AC_BRANCH_BASE);
		}
		else if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
		{
			u8_panel_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
			u8_module_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;
			os_mut_wait(g_mut_share_data, 0xFFFF);
			u8_branch_index = g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel_index].u8_feeder_start_num[u8_module_index] + u16_info_code;
			os_mut_release(g_mut_share_data);
			if (u8_branch_index >= FEEDER_BRANCH_MAX)    //防止数组越界
				return;
			
			pt_feeder_branch = &(g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].t_feeder[u8_branch_index]);
			pu8_sensor_state = &(g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].u8_sensor_state[u8_branch_index]);
			
			u16_swt_fault_id = u16_insu_fault_id = u16_sensor_fault_id = ((FAULT_DC_PANEL1_GROUP+u8_panel_index)<<FAULT_GROUP_OFFSET);
			u16_swt_fault_id |= (FAULT_FEEDER_BRANCH_FAULT_CNT * u8_branch_index + FAULT_FEEDER_BRANCH_SWT + FAULT_FEEDER_BRANCH_BASE);
			u16_insu_fault_id |= (FAULT_FEEDER_BRANCH_FAULT_CNT * u8_branch_index + FAULT_FEEDER_BRANCH_INSU + FAULT_FEEDER_BRANCH_BASE);
			u16_sensor_fault_id |= (FAULT_FEEDER_BRANCH_FAULT_CNT * u8_branch_index + FAULT_FEEDER_BRANCH_SENSOR + FAULT_FEEDER_BRANCH_BASE);
		}
		else if (u8_addr == DCDC_PANEL_MODULE_BASE_ADDR)
		{
			if (u16_info_code >= FEEDER_BRANCH_MAX)    //防止数组越界
				return;
				
			pt_feeder_branch = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[u16_info_code]);
			pu8_sensor_state = &(g_t_share_data.t_rt_data.t_dcdc_panel.u8_sensor_state[u16_info_code]);
			
			u16_swt_fault_id = u16_sensor_fault_id = (FAULT_DCDC_PANEL_GROUP<<FAULT_GROUP_OFFSET);
			u16_swt_fault_id |= (FAULT_DCDC_BRANCH_FAULT_CNT * u16_info_code + FAULT_DCDC_BRANCH_SWT + FAULT_DCDC_BRANCH_BASE);
			u16_sensor_fault_id |= (FAULT_DCDC_BRANCH_FAULT_CNT * u16_info_code + FAULT_DCDC_BRANCH_SENSOR + FAULT_DCDC_BRANCH_BASE);
		}
		else if (u8_addr == DCAC_PANEL_MODULE_BASE_ADDR)
		{
			if (u16_info_code >= FEEDER_BRANCH_MAX)    //防止数组越界
				return;
				
			pt_feeder_branch = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder[u16_info_code]);
			pu8_sensor_state = &(g_t_share_data.t_rt_data.t_dcac_panel.u8_sensor_state[u16_info_code]);
			
			u16_swt_fault_id = u16_sensor_fault_id = (FAULT_DCAC_PANEL_GROUP<<FAULT_GROUP_OFFSET);
			u16_swt_fault_id |= (FAULT_DCAC_BRANCH_FAULT_CNT * u16_info_code + FAULT_DCAC_BRANCH_SWT + FAULT_DCAC_BRANCH_BASE);
			u16_sensor_fault_id |= (FAULT_DCAC_BRANCH_FAULT_CNT * u16_info_code + FAULT_DCAC_BRANCH_SENSOR + FAULT_DCAC_BRANCH_BASE);
		}
		else
		{
			return;
		}
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		u8_alarm_old = pt_feeder_branch->u8_alarm;
		u8_insu_state_old = pt_feeder_branch->u8_insu_state;
		u8_sensor_state_old = *pu8_sensor_state;
		
		pt_feeder_branch->u8_insu_state = pt_msg->data[2];
		if (pt_msg->data[2] != 0)
		{
			pt_feeder_branch->f32_res = ((S16_T)((pt_msg->data[0] << 8) | pt_msg->data[1])) / 10.0;
		}
		
		pt_feeder_branch->u8_curr_state = pt_msg->data[5];
		if (pt_msg->data[5] != 0)
		{
			pt_feeder_branch->f32_curr = ((S16_T)((pt_msg->data[3] << 8) | pt_msg->data[4])) / 10.0;
		}
		
		pt_feeder_branch->u8_alarm = pt_msg->data[6];
		pt_feeder_branch->u8_state = pt_msg->data[7];
		
		if ((pt_feeder_branch->u8_insu_state==0) && (pt_feeder_branch->u8_curr_state==0))
			*pu8_sensor_state = 0x00;
		else if ((pt_feeder_branch->u8_insu_state==0x03) || (pt_feeder_branch->u8_curr_state==0x03))
			*pu8_sensor_state = 0x02;
		else
			*pu8_sensor_state = 0x01;
		
		u8_alarm_new = pt_feeder_branch->u8_alarm;	
		u8_insu_state_new = pt_feeder_branch->u8_insu_state;	
		u8_sensor_state_new = *pu8_sensor_state;
		
		//查找一段支路对地电阻最小值
		if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR) && (u8_addr < DCDC_PANEL_MODULE_BASE_ADDR))
		{
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_feeder_min_to_gnd_res = 
								g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[0].f32_res;
			for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
			{
				for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_insu_feeder_num; j++)
				{
					if (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_feeder_min_to_gnd_res
						> g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].f32_res)
					{
						g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_feeder_min_to_gnd_res =
									g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[j].f32_res;
					}
				}
			}
		}

		//查找二段支路对地电阻最小值
		if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR2) && (u8_addr < DCDC_PANEL_MODULE_BASE_ADDR))
		{
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_feeder2_min_to_gnd_res = 
								g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[0].f32_res;
			for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i++)
			{
				for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_insu_feeder_num; j++)
				{
					if (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_feeder2_min_to_gnd_res
						> g_t_share_data.t_rt_data.t_feeder_panel[4+i].t_feeder[j].f32_res)
					{
						g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_feeder2_min_to_gnd_res =
									g_t_share_data.t_rt_data.t_feeder_panel[4+i].t_feeder[j].f32_res;
					}
				}
			}
		}
		
		os_mut_release(g_mut_share_data);
		
		if ((u8_alarm_old != 0x02) && (u8_alarm_new == 0x02))                     //开关跳闸发生
			v_fauid_send_fault_id_occur(u16_swt_fault_id);
		else if ((u8_alarm_old == 0x02) && (u8_alarm_new != 0x02))                //开关跳闸恢复
			v_fauid_send_fault_id_resume(u16_swt_fault_id);
		
		if ((u8_insu_state_old != 0x02) && (u8_insu_state_new == 0x02))           //支路绝缘下降发生
			v_fauid_send_fault_id_occur(u16_insu_fault_id);
		else if ((u8_insu_state_old == 0x02) && (u8_insu_state_new != 0x02))      //支路绝缘下降恢复
			v_fauid_send_fault_id_resume(u16_insu_fault_id);
		
		if ((u8_sensor_state_old != 0x02) && (u8_sensor_state_new == 0x02))       //传感器异常发生
			v_fauid_send_fault_id_occur(u16_sensor_fault_id);
		else if ((u8_sensor_state_old == 0x02) && (u8_sensor_state_new != 0x02))  //传感器异常恢复
			v_fauid_send_fault_id_resume(u16_sensor_fault_id);
	}
	
	else if (u16_info_code == IC_BUS_TO_GND_VOLT)
	{
		U16_T u16_state_old, u16_state_new;

		if (u8_addr == g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add)
		{			
			os_mut_wait(g_mut_share_data, 0xFFFF);
	
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_pos_to_gnd_volt = ((S16_T)((pt_msg->data[0] << 8) | pt_msg->data[1])) / 10.0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_pos_to_gnd_volt = ((S16_T)((pt_msg->data[2] << 8) | pt_msg->data[3])) / 10.0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_volt = ((S16_T)((pt_msg->data[4] << 8) | pt_msg->data[5])) / 10.0;
			
			u16_state_old = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state;
			if (pt_msg->data[6] != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state |= 0x1000;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state &= ~0x1000;
			u16_state_new = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state;
				
			os_mut_release(g_mut_share_data);
			
			if (((u16_state_old & 0x1000) == 0) && ((u16_state_new & 0x1000) != 0))
				v_fauid_send_fault_id_occur((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS_VOLT_IMBALANCE);
			else if (((u16_state_old & 0x1000) != 0) && ((u16_state_new & 0x1000) == 0))
				v_fauid_send_fault_id_resume((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS_VOLT_IMBALANCE);
		}
		else if (u8_addr == g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add)
		{			
			os_mut_wait(g_mut_share_data, 0xFFFF);
	
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb2_pos_to_gnd_volt = ((S16_T)((pt_msg->data[0] << 8) | pt_msg->data[1])) / 10.0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb2_pos_to_gnd_volt = ((S16_T)((pt_msg->data[2] << 8) | pt_msg->data[3])) / 10.0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_neg_to_gnd_volt = ((S16_T)((pt_msg->data[4] << 8) | pt_msg->data[5])) / 10.0;
			
			u16_state_old = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2;
			if (pt_msg->data[6] != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2 |= 0x0200;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2 &= ~0x0200;
			u16_state_new = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2;
				
			os_mut_release(g_mut_share_data);
			
			if (((u16_state_old & 0x0200) == 0) && ((u16_state_new & 0x0200) != 0))
				v_fauid_send_fault_id_occur((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS2_VOLT_IMBALANCE);
			else if (((u16_state_old & 0x0200) != 0) && ((u16_state_new & 0x0200) == 0))
				v_fauid_send_fault_id_resume((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS2_VOLT_IMBALANCE);
		}
	}
	
	else if (u16_info_code == IC_BUS_TO_GND_RES)
	{
		U16_T u16_state_old, u16_state_new;

		if (u8_addr == g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add)
		{			
			os_mut_wait(g_mut_share_data, 0xFFFF);
	
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_pos_to_gnd_res = ((S16_T)((pt_msg->data[0] << 8) | pt_msg->data[1])) / 10.0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_neg_to_gnd_res = ((S16_T)((pt_msg->data[2] << 8) | pt_msg->data[3])) / 10.0;
			
			u16_state_old = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state;
			if (pt_msg->data[4] != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state |= 0x2000;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state &= ~0x2000;
			u16_state_new = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state;
				
			os_mut_release(g_mut_share_data);
			
			if (((u16_state_old & 0x2000) == 0) && ((u16_state_new & 0x2000) != 0))
				v_fauid_send_fault_id_occur((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS_INSU_FAULT);
			else if (((u16_state_old & 0x2000) != 0) && ((u16_state_new & 0x2000) == 0))
				v_fauid_send_fault_id_resume((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS_INSU_FAULT);
		}
		else if (u8_addr == g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add)
		{
			os_mut_wait(g_mut_share_data, 0xFFFF);
	
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_pos_to_gnd_res = ((S16_T)((pt_msg->data[0] << 8) | pt_msg->data[1])) / 10.0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_neg_to_gnd_res = ((S16_T)((pt_msg->data[2] << 8) | pt_msg->data[3])) / 10.0;
			
			u16_state_old = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2;
			if (pt_msg->data[4] != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2 |= 0x0400;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2 &= ~0x0400;
			u16_state_new = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2;
				
			os_mut_release(g_mut_share_data);
			
			if (((u16_state_old & 0x0400) == 0) && ((u16_state_new & 0x0400) != 0))
				v_fauid_send_fault_id_occur((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS2_INSU_FAULT);
			else if (((u16_state_old & 0x0400) != 0) && ((u16_state_new & 0x0400) == 0))
				v_fauid_send_fault_id_resume((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS2_INSU_FAULT);
		}
	}
	
	else if (u16_info_code == IC_BUS_TO_GND_AC_VOLT)
	{
		U16_T u16_state_old, u16_state_new;

		if (u8_addr == g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add)
		{			
			os_mut_wait(g_mut_share_data, 0xFFFF);
	
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus_to_gnd_ac_volt = ((S16_T)((pt_msg->data[0] << 8) | pt_msg->data[1])) / 10.0;
			
			u16_state_old = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state;
			if (pt_msg->data[2] != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state |= 0x8000;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state &= ~0x8000;
			u16_state_new = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state;
				
			os_mut_release(g_mut_share_data);
			
			if (((u16_state_old & 0x8000) == 0) && ((u16_state_new & 0x8000) != 0))
				v_fauid_send_fault_id_occur((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS_INPUT_AC);
			else if (((u16_state_old & 0x8000) != 0) && ((u16_state_new & 0x8000) == 0))
				v_fauid_send_fault_id_resume((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS_INPUT_AC);
		}
		else if (u8_addr == g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add)
		{
			os_mut_wait(g_mut_share_data, 0xFFFF);
	
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_bus2_to_gnd_ac_volt = ((S16_T)((pt_msg->data[0] << 8) | pt_msg->data[1])) / 10.0;
			
			u16_state_old = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2;
			if (pt_msg->data[2] != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2 |= 0x1000;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2 &= ~0x1000;
			u16_state_new = g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state2;
				
			os_mut_release(g_mut_share_data);
			
			if (((u16_state_old & 0x1000) == 0) && ((u16_state_new & 0x1000) != 0))
				v_fauid_send_fault_id_occur((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS2_INPUT_AC);
			else if (((u16_state_old & 0x1000) != 0) && ((u16_state_new & 0x1000) == 0))
				v_fauid_send_fault_id_resume((FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DC_BUS2_INPUT_AC);
		}
	}

	else if (u16_info_code == FC_IC_TOTAL_SWT_FAULT)
	{
		U8_T u8_total_swt_falut_old = 0;
		U8_T u8_total_swt_falut_new = 0;
		U16_T u16_total_swt_fault_id;

		os_mut_wait(g_mut_share_data, 0xFFFF);

		if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr <= (AC_PANEL_MODULE_END_ADDR)))
		{
			u8_panel_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) / ACS_FEEDER_MODULE_MAX;
			u8_module_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;

			if (u8_module_index == 0)
			{
				u8_total_swt_falut_old = g_t_share_data.t_rt_data.t_ac_panel.u8_total_swt_falut;
				g_t_share_data.t_rt_data.t_ac_panel.u8_total_swt_falut = pt_msg->data[0];
				u8_total_swt_falut_new = g_t_share_data.t_rt_data.t_ac_panel.u8_total_swt_falut;
				u16_total_swt_fault_id = ((FAULT_AC_PANEL_GROUP<<FAULT_GROUP_OFFSET) | FAULT_AC_TOATL_SWT_FAULT);
			}
		}
		else if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
		{
			u8_panel_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
			u8_module_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;

			if (u8_module_index == 0)
			{
				u8_total_swt_falut_old = g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].u8_total_swt_falut;
				g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].u8_total_swt_falut = pt_msg->data[0];
				u8_total_swt_falut_new = g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].u8_total_swt_falut;
				u16_total_swt_fault_id = (((FAULT_DC_PANEL1_GROUP+u8_panel_index)<<FAULT_GROUP_OFFSET) | FAULT_FEEDER_TOATL_SWT_FAULT);
			}
		}
		else if (u8_addr == DCDC_PANEL_MODULE_BASE_ADDR)
		{
			u8_total_swt_falut_old = g_t_share_data.t_rt_data.t_dcdc_panel.u8_total_swt_falut;
			g_t_share_data.t_rt_data.t_dcdc_panel.u8_total_swt_falut = pt_msg->data[0];
			u8_total_swt_falut_new = g_t_share_data.t_rt_data.t_dcdc_panel.u8_total_swt_falut;
			u16_total_swt_fault_id = ((FAULT_DCDC_PANEL_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DCDC_TOATL_SWT_FAULT);
		}
		else if (u8_addr == DCAC_PANEL_MODULE_BASE_ADDR)
		{
			u8_total_swt_falut_old = g_t_share_data.t_rt_data.t_dcac_panel.u8_total_swt_falut;
			g_t_share_data.t_rt_data.t_dcac_panel.u8_total_swt_falut = pt_msg->data[0];
			u8_total_swt_falut_new = g_t_share_data.t_rt_data.t_dcac_panel.u8_total_swt_falut;
			u16_total_swt_fault_id = ((FAULT_DCAC_PANEL_GROUP<<FAULT_GROUP_OFFSET) | FAULT_DCAC_TOATL_SWT_FAULT);
		}

		os_mut_release(g_mut_share_data);

		if ((u8_total_swt_falut_old == 0) && (u8_total_swt_falut_new == 1))
			v_fauid_send_fault_id_occur(u16_total_swt_fault_id);
		else if ((u8_total_swt_falut_old == 1) && (u8_total_swt_falut_new == 0))
			v_fauid_send_fault_id_resume(u16_total_swt_fault_id);
	}
	
	else if (u16_info_code == IC_HW_VERSION)
	{
		if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr <= AC_PANEL_MODULE_END_ADDR))
		{
			u8_panel_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) / ACS_FEEDER_MODULE_MAX;
			u8_module_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[u8_module_index]);
		}
		else if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
		{
			u8_panel_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
			u8_module_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].t_feeder_module[u8_module_index]);
		}
		else if (u8_addr == DCDC_PANEL_MODULE_BASE_ADDR)
		{
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[0]);
		}
		else if (u8_addr == DCAC_PANEL_MODULE_BASE_ADDR)
		{
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[0]);
		}
		else
		{
			return;
		}
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		pt_feeder_module->u32_hw_ver = ((pt_msg->data[0] << 8) | pt_msg->data[1]);
		os_mut_release(g_mut_share_data);
	}
		
	else if (u16_info_code == IC_SW_VERSION)
	{
		if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr <= AC_PANEL_MODULE_END_ADDR))
		{
			u8_panel_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) / ACS_FEEDER_MODULE_MAX;
			u8_module_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[u8_module_index]);
		}
		else if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
		{
			u8_panel_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
			u8_module_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].t_feeder_module[u8_module_index]);
		}
		else if (u8_addr == DCDC_PANEL_MODULE_BASE_ADDR)
		{
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[0]);
		}
		else if (u8_addr == DCAC_PANEL_MODULE_BASE_ADDR)
		{
			pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[0]);
		}
		else
		{
			return;
		}
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		pt_feeder_module->u32_sw_ver = ((pt_msg->data[0] << 8) | pt_msg->data[1]);
		os_mut_release(g_mut_share_data);
	}
}

/*************************************************************
函数名称: v_can_feeder_module_clear_comm_fault_alm
函数功能: 根据馈线模块地址清除通信中断计数，如果已经报了通信中断故障，则恢复
输入参数: u8_addr -- 地址
输出参数: 无
返回值  ：无
**************************************************************/
static void v_can_feeder_module_clear_comm_fault_alm(U8_T u8_addr)
{
	U8_T u8_panel_index, u8_module_index, u8_record_index, u8_comm_state;
	U16_T u16_fault_id;
	FEEDER_MODULE_RT_DATA_T *pt_feeder_module = NULL;
	
	if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR) && (u8_addr <= AC_PANEL_MODULE_END_ADDR))
	{
		u8_record_index = u8_addr - AC_PANEL_MODULE_BASE_ADDR;
		u8_panel_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) / ACS_FEEDER_MODULE_MAX;
		u8_module_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;
		pt_feeder_module = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[u8_module_index]);
	}
	else if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
				&& (u8_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
	{
		u8_record_index = u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR;
		u8_panel_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
		u8_module_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;
		pt_feeder_module = &(g_t_share_data.t_rt_data.t_feeder_panel[u8_panel_index].t_feeder_module[u8_module_index]);
	}
	else if (u8_addr == DCDC_PANEL_MODULE_BASE_ADDR)
	{
		u8_record_index = FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX;
		pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[0]);
	}
	else if (u8_addr == DCAC_PANEL_MODULE_BASE_ADDR)
	{
		u8_record_index = FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX + 1;
		pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[0]);
	}
	else
	{
		return;        //0地址或其它非法地址退出
	}
		
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u8_comm_state = pt_feeder_module->u8_comm_state;
	pt_feeder_module->u8_comm_state = 0;
	os_mut_release(g_mut_share_data);
	
	if (u8_comm_state != 0)
	{
		if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR) && (u8_addr <= AC_PANEL_MODULE_END_ADDR))
		{
			u16_fault_id = ((FAULT_AC_GROUP<<FAULT_GROUP_OFFSET) | (FAULT_AC_FC1_COMM_FAIL+u8_record_index));
			v_fauid_send_fault_id_resume(u16_fault_id);
			m_t_feeder_module_record[u8_record_index].u32_fail_time = u32_delay_get_timer_val();
		}
		else
		{
			u16_fault_id = ((FAULT_FC_GROUP<<FAULT_GROUP_OFFSET) | u8_record_index);
			v_fauid_send_fault_id_resume(u16_fault_id);
			m_t_feeder_module_record[3+u8_record_index].u32_fail_time = u32_delay_get_timer_val();
		}
	}
	
	if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR) && (u8_addr <= AC_PANEL_MODULE_END_ADDR))
	{
		m_t_feeder_module_record[u8_record_index].u32_fail_time = u32_delay_get_timer_val();
	}
	else
	{
		m_t_feeder_module_record[3+u8_record_index].u32_fail_time = u32_delay_get_timer_val();
	}	
}

/*************************************************************
函数名称: v_can_feeder_module_comm_fault_alm
函数功能: 根据馈线模块地址增加通信中断计数，通信中断次数超过一定的次数，报通信中断故障
输入参数: u8_addr -- 地址
输出参数: 无
返回值  ：无
**************************************************************/
static void v_can_feeder_module_comm_fault_alm(void)
{
	U8_T i, u8_panel, u8_module;
	U16_T u16_fault_id;
	FEEDER_MODULE_RT_DATA_T *pt_feeder_module = NULL;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	for (i=0; i<(sizeof(m_t_feeder_module_record) / sizeof(m_t_feeder_module_record[0])); i++)
	{
		pt_feeder_module = NULL;
		
		if ((m_t_feeder_module_record[i].u8_module_addr >= AC_PANEL_MODULE_BASE_ADDR)
			&& (m_t_feeder_module_record[i].u8_module_addr <= AC_PANEL_MODULE_END_ADDR))
		{
			u8_panel = (m_t_feeder_module_record[i].u8_module_addr - AC_PANEL_MODULE_BASE_ADDR) / ACS_FEEDER_MODULE_MAX;
			u8_module = (m_t_feeder_module_record[i].u8_module_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;
			
			if ((u8_module < g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num))
			{
				pt_feeder_module = &(g_t_share_data.t_rt_data.t_ac_panel.t_feeder_module[u8_module]);
			}
		}
		else if ((m_t_feeder_module_record[i].u8_module_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
			&& (m_t_feeder_module_record[i].u8_module_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
		{
			u8_panel = (m_t_feeder_module_record[i].u8_module_addr - FEEDER_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
			u8_module = (m_t_feeder_module_record[i].u8_module_addr - FEEDER_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;
			
			if ((u8_panel < g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num)
				&& (u8_module < g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel].u8_feeder_module_num))
			{
				pt_feeder_module = &(g_t_share_data.t_rt_data.t_feeder_panel[u8_panel].t_feeder_module[u8_module]);
			}
			else if ((u8_panel < (4+g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num))
				&& (u8_module < g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel].u8_feeder_module_num))
			{
				pt_feeder_module = &(g_t_share_data.t_rt_data.t_feeder_panel[u8_panel].t_feeder_module[u8_module]);
			}
		}
		else if (m_t_feeder_module_record[i].u8_module_addr == DCDC_PANEL_MODULE_BASE_ADDR)
		{
			if ((g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 0)
				&& (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0))
			{
				pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder_module[0]);
			}
		}
		else if (m_t_feeder_module_record[i].u8_module_addr == DCAC_PANEL_MODULE_BASE_ADDR)
		{
			if ((g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num > 0)
				&& (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0))
			{
				pt_feeder_module = &(g_t_share_data.t_rt_data.t_dcac_panel.t_feeder_module[0]);
			}
		}
		
		if (pt_feeder_module == NULL)
			continue;
		
		if (pt_feeder_module->u8_comm_state == 0)
		{
			if (u32_delay_time_elapse(m_t_feeder_module_record[i].u32_fail_time, u32_delay_get_timer_val()) > 
						(g_t_share_data.t_sys_cfg.t_sys_param.u8_fc10_offline_time * OSC_SECOND))
			{
				if (i < 3)
				{
					u16_fault_id = ((FAULT_AC_GROUP<<FAULT_GROUP_OFFSET) | (FAULT_AC_FC1_COMM_FAIL+i));
				}
				else
				{
					u16_fault_id = ((FAULT_FC_GROUP<<FAULT_GROUP_OFFSET) | (i-3));
				}
				os_mut_release(g_mut_share_data);
				v_fauid_send_fault_id_occur(u16_fault_id);
				os_mut_wait(g_mut_share_data, 0xFFFF);
				
				pt_feeder_module->u8_comm_state = 1;
			}
		}
	}
	
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_can_feeder_module_meas_start
函数功能: 下发馈线模块绝缘定期测量启动函数
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
static void v_can_feeder_module_meas_start(void)
{
	U8_T  u8_addr = 127;	//广播
	U16_T u16_info_code = 520; //绝缘测量启动
	
	memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));

	m_t_tx_msg_buf.data[0] = 1;
	m_t_tx_msg_buf.len = 1;	
	
	m_t_tx_msg_buf.id = (ID_FEEDER_MODULE_TYPE | FC_REQUEST_WRITE);
	m_t_tx_msg_buf.id |= ((u8_addr & 0x7F)<<ID_ADDR_OFFSET);
	m_t_tx_msg_buf.id |= (u16_info_code & 0x3FF);

	m_t_tx_msg_buf.ch = 0;
	m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
	m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
	
	if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
	{
		os_dly_wait((U32_T)(m_f32_delay_scale * 1));
		u32_can_send(1, &m_t_tx_msg_buf);
	}
}

/*************************************************************
函数名称: v_can_feeder_module_send_insu_para
函数功能: 下发馈线模块配置数据处理函数
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
static void v_can_feeder_module_send_insu_para(void)
{
	U8_T  u8_addr = 127;	//广播
	U16_T u16_info_code = 514; //绝缘测量参数设置
	
	memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
	
	os_mut_wait(g_mut_share_data, 0xFFFF);

	m_t_tx_msg_buf.data[0] = (U8_T)(g_t_share_data.t_sys_cfg.t_sys_param.e_insu_meas_way);
	m_t_tx_msg_buf.data[1] = (U8_T)(g_t_share_data.t_sys_cfg.t_sys_param.u8_res_switch_delay);
	m_t_tx_msg_buf.data[2] = (U8_T)((g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range) >> 8);
	m_t_tx_msg_buf.data[3] = (U8_T)(g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range);
	m_t_tx_msg_buf.data[4] = (U8_T)(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_bus_err_confirm);
	m_t_tx_msg_buf.data[5] = (U8_T)(g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_period);
	m_t_tx_msg_buf.len = 6;	

	os_mut_release(g_mut_share_data);
	
	m_t_tx_msg_buf.id = (ID_FEEDER_MODULE_TYPE | FC_REQUEST_WRITE);
	m_t_tx_msg_buf.id |= ((u8_addr & 0x7F)<<ID_ADDR_OFFSET);
	m_t_tx_msg_buf.id |= (u16_info_code & 0x3FF);

	m_t_tx_msg_buf.ch = 0;
	m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
	m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
	
	if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
	{
		os_dly_wait((U32_T)(m_f32_delay_scale * 1));
		u32_can_send(1, &m_t_tx_msg_buf);
	}
}

/*************************************************************
函数名称: v_can_feeder_module_send_config
函数功能: 下发馈线模块配置数据处理函数
输入参数: u8_addr       -- 地址
          u16_info_code -- 信息码
输出参数: 无
返回值  ：无
**************************************************************/
static void v_can_feeder_module_send_config(U8_T u8_addr, U16_T u16_info_code)
{
	U8_T u8_panel_index, u8_module_index;
	FEEDER_MODULE_CFG_T *pt_feeder_module = NULL;
	U16_T u16_feeder_shunt_range;
	
	//检查地址是否非法
	if (!(((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR) && (u8_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
	    || ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR ) && (u8_addr <= AC_PANEL_MODULE_END_ADDR))
		|| (u8_addr == DCDC_PANEL_MODULE_BASE_ADDR) || (u8_addr == DCAC_PANEL_MODULE_BASE_ADDR)))
		return;
	
	memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if ((u8_addr >= AC_PANEL_MODULE_BASE_ADDR)
		&& (u8_addr <= AC_PANEL_MODULE_END_ADDR))
	{
		u8_panel_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) / ACS_FEEDER_MODULE_MAX;
		u8_module_index = (u8_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;
		pt_feeder_module = &(g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[u8_module_index]);
		u16_feeder_shunt_range = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_feeder_shunt_range;
	}
	else if ((u8_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
		&& (u8_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
	{
		u8_panel_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
		u8_module_index = (u8_addr - FEEDER_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;
		pt_feeder_module = &(g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel_index].t_feeder_module[u8_module_index]);
		u16_feeder_shunt_range = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_feeder_shunt_range;
	}
	else if (u8_addr == DCDC_PANEL_MODULE_BASE_ADDR)
	{
		pt_feeder_module = &(g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0]);
		u16_feeder_shunt_range = g_t_share_data.t_sys_cfg.t_dcdc_panel.u16_feeder_shunt_range;
	}
	else
	{
		pt_feeder_module = &(g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0]);
		u16_feeder_shunt_range = g_t_share_data.t_sys_cfg.t_dcac_panel.u16_feeder_shunt_range;
	}
	
	switch (u16_info_code)
	{
		case IC_BRANCH_NUM:               //各类型检测路数
			m_t_tx_msg_buf.data[0] = pt_feeder_module->u8_alarm_feeder_num;
			m_t_tx_msg_buf.data[1] = pt_feeder_module->u8_state_feeder_num;
			m_t_tx_msg_buf.data[2] = pt_feeder_module->u8_insu_feeder_num;
			m_t_tx_msg_buf.data[3] = pt_feeder_module->u8_curr_feeder_num;
			m_t_tx_msg_buf.data[4] = pt_feeder_module->e_alarm_type;
			m_t_tx_msg_buf.data[5] = pt_feeder_module->e_state_type;
			
			m_t_tx_msg_buf.len = 6;
			break;

		case IC_CURR_SENSOR_COEFF:        //电流传感器比例系数
			m_t_tx_msg_buf.data[0] = (U8_T)(u16_feeder_shunt_range >> 8);
			m_t_tx_msg_buf.data[1] = (U8_T)u16_feeder_shunt_range;
			m_t_tx_msg_buf.data[2] = u8_addr;
			m_t_tx_msg_buf.data[3] = u8_addr;
			
			m_t_tx_msg_buf.len = 4;
			break;
			
		case IC_INSU_ALM_THR:             //绝缘报警门限值
			m_t_tx_msg_buf.data[0] = (U8_T)((g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_volt_imbalance*10) >> 8);
			m_t_tx_msg_buf.data[1] = (U8_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_volt_imbalance*10);
			m_t_tx_msg_buf.data[2] = (U8_T)((g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_res_thr*10) >> 8);
			m_t_tx_msg_buf.data[3] = (U8_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_res_thr*10);
			m_t_tx_msg_buf.data[4] = (U8_T)((g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_dc_bus_input_ac_thr*10) >> 8);
			m_t_tx_msg_buf.data[5] = (U8_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_dc_bus_input_ac_thr*10);
			
			m_t_tx_msg_buf.len = 6;
			break;
			
		default:
			os_mut_release(g_mut_share_data);
			return;
	}
	
	os_mut_release(g_mut_share_data);
	
	m_t_tx_msg_buf.id = (ID_FEEDER_MODULE_TYPE | FC_REQUEST_WRITE);
	m_t_tx_msg_buf.id |= ((u8_addr & 0x7F)<<ID_ADDR_OFFSET);
	m_t_tx_msg_buf.id |= (u16_info_code & 0x3FF);

	m_t_tx_msg_buf.ch = 0;
	m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
	m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
	
	if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
	{
		os_dly_wait((U32_T)(m_f32_delay_scale * 1));
		u32_can_send(1, &m_t_tx_msg_buf);
	}
}

/*************************************************************
函数名称: v_can_feeder_module_receive_handle
函数功能: 馈线模块接收处理函数
输入参数: 无
          无
输出参数: 无
返回值  ：无
**************************************************************/
static void v_can_feeder_module_receive_handle(void)
{
	U8_T i, j, u8_addr_flag, u8_recv_addr;
	
	//核查地址是否是有效
	u8_addr_flag = 0;
	u8_recv_addr = ((m_t_rx_msg_buf.id & ID_DEVICE_ADDR_MASK) >> ID_ADDR_OFFSET);
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	for (j=0; j<g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num; j++)
	{
		if (u8_recv_addr == (AC_PANEL_MODULE_BASE_ADDR+j))
		{
			u8_addr_flag = 1;
			break;
		}
	}

	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
	{
		for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num; j++)
		{
			if (u8_recv_addr == (FEEDER_PANEL_MODULE_BASE_ADDR+i*4+j))
			{
				u8_addr_flag = 1;
				break;
			}
		}
			
		if (u8_addr_flag == 1)
			break;
	}

	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i++)
	{
		for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num; j++)
		{
			if (u8_recv_addr == (FEEDER_PANEL_MODULE_BASE_ADDR2+i*4+j))
			{
				u8_addr_flag = 1;
				break;
			}
		}
			
		if (u8_addr_flag == 1)
			break;
	}
	
	os_mut_release(g_mut_share_data);
	
	if (u8_addr_flag == 0)
	{
		if ((u8_recv_addr == DCDC_PANEL_MODULE_BASE_ADDR)
			|| (u8_recv_addr == DCAC_PANEL_MODULE_BASE_ADDR)
			/*|| (u8_recv_addr == 0)*/)
		{
			u8_addr_flag = 1;
		}
	}
	
	if (u8_addr_flag == 0)  //如果接收到的地址无效，则丢掉数据继续接收
		return;

	switch (m_t_rx_msg_buf.id & ID_FUCN_CODE_MASK)
	{
		case FC_REQUEST_CONFIG:              //请求配置数据
			if (m_t_rx_msg_buf.type == 1) //远程帧才处理
			{
				v_can_feeder_module_send_config(u8_recv_addr, m_t_rx_msg_buf.id&ID_INFO_CODE_MASK);

				//清除通信中断计数，恢复通信中断告警
				v_can_feeder_module_clear_comm_fault_alm(u8_recv_addr);
			}
			break;
			
		case FC_ACTIVE_UPLOAD:               //主动上传数据
		case FC_REQUEST_READ:                //申请读取数据/回答读取数据
			if (m_t_rx_msg_buf.type == 0)    //数据帧才处理
			{
				v_can_feeder_module_data_handle(&m_t_rx_msg_buf);

				//清除通信中断计数，恢复通信中断告警
				v_can_feeder_module_clear_comm_fault_alm(u8_recv_addr);
			}
			break;
			
		case FC_RETURN_FAIL:                 //返回写命令执行失败
		case FC_RETURN_SCUESS:               //返回写命令执行成功
			if (m_t_rx_msg_buf.type == 0)    //数据帧才处理
			{
				//清除通信中断计数，恢复通信中断告警
				v_can_feeder_module_clear_comm_fault_alm(u8_recv_addr);
			}
						
		case FC_REQUEST_WRITE:               //申请写数据
		default:
			break;
	}
}

/*************************************************************
函数名称: v_can_feeder_module_send_handle
函数功能: 馈线模块发送查询命令处理函数						
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/	
static void v_can_feeder_module_send_handle(void)
{
	static U8_T  u8_record = 0;
	static U16_T u16_bus_info_code = IC_BUS_TO_GND_VOLT;
	static U16_T u16_bus2_info_code = IC_BUS_TO_GND_VOLT;
	static U8_T  u8_dcdc_module_index = 0;
	static U8_T  u8_fc_tx_flag = 0;
	static U8_T  u8_dcdc_tx_flag = 0;
	static U8_T  u8_dcdc_cmd_index = 0;
	static U8_T  u8_main_fdl1_read = 0, u8_main_fdl2_read = 0;
	U8_T  u8_panel, u8_module, u8_branch_num;
	U8_T  u8_tx_cnt = 0;
	U8_T  u8_main_fdl1, u8_main_fdl2;
	
	while (u8_tx_cnt++ < CAN_TX_MAX_CNT)
	{
		os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
		
		if ((u32_fc_module_flag == 1) && (u8_fc_tx_flag == 0))
		{
			if (u8_record == 0)
			{
				u8_main_fdl1_read = 1;
				u8_main_fdl2_read = 1;				
			}

			os_mut_wait(g_mut_share_data, 0xFFFF);
			u8_main_fdl1 = g_t_share_data.t_sys_cfg.t_sys_param.u8_seg1_fdl_master_add;
			u8_main_fdl2 = g_t_share_data.t_sys_cfg.t_sys_param.u8_seg2_fdl_master_add;
			os_mut_release(g_mut_share_data);
			if (u8_main_fdl1_read)
			{	
				if (m_t_feeder_module_record[u8_record].u8_module_addr == u8_main_fdl1)
				{				
					memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
					m_t_tx_msg_buf.id = (ID_FEEDER_MODULE_TYPE | FC_REQUEST_READ | u16_bus_info_code);
					m_t_tx_msg_buf.id |= (u8_main_fdl1 << ID_ADDR_OFFSET);
					m_t_tx_msg_buf.len = 8;
					m_t_tx_msg_buf.ch = 0;
					m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
					m_t_tx_msg_buf.type = 1;      //0 - DATA FRAME, 1 - REMOTE FRAME
					
					if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
						break;
						
					if (u16_bus_info_code == IC_BUS_TO_GND_VOLT)       //母线对地电压	
						u16_bus_info_code = IC_BUS_TO_GND_RES;
					else if (u16_bus_info_code == IC_BUS_TO_GND_RES)   //母线对地电阻
						u16_bus_info_code = IC_BUS_TO_GND_AC_VOLT;
					else                                           //母线对地交流电压
						u16_bus_info_code = IC_BUS_TO_GND_VOLT;

					u8_main_fdl1_read = 0;
				}				
			}

			if (u8_main_fdl2_read && u32_seg2_fc_module_flag)
			{
				if (m_t_feeder_module_record[u8_record].u8_module_addr == u8_main_fdl2)
				{				
					memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
					m_t_tx_msg_buf.id = (ID_FEEDER_MODULE_TYPE | FC_REQUEST_READ | u16_bus2_info_code);
					m_t_tx_msg_buf.id |= (u8_main_fdl2 << ID_ADDR_OFFSET);
					m_t_tx_msg_buf.len = 8;
					m_t_tx_msg_buf.ch = 0;
					m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
					m_t_tx_msg_buf.type = 1;      //0 - DATA FRAME, 1 - REMOTE FRAME
					
					if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
						break;
						
					if (u16_bus2_info_code == IC_BUS_TO_GND_VOLT)       //母线对地电压	
						u16_bus2_info_code = IC_BUS_TO_GND_RES;
					else if (u16_bus2_info_code == IC_BUS_TO_GND_RES)   //母线对地电阻
						u16_bus2_info_code = IC_BUS_TO_GND_AC_VOLT;
					else                                           //母线对地交流电压
						u16_bus2_info_code = IC_BUS_TO_GND_VOLT;

					u8_main_fdl2_read = 0;
				}
			}
			
			u8_branch_num = 0;
			
			os_mut_wait(g_mut_share_data, 0xFFFF);
			if ((m_t_feeder_module_record[u8_record].u8_module_addr >= AC_PANEL_MODULE_BASE_ADDR)
				&& (m_t_feeder_module_record[u8_record].u8_module_addr <= AC_PANEL_MODULE_END_ADDR))
			{
				u8_module = (m_t_feeder_module_record[u8_record].u8_module_addr - AC_PANEL_MODULE_BASE_ADDR) % ACS_FEEDER_MODULE_MAX;
				
				if (u8_module < g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num)
				{
					u8_branch_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[u8_module].u8_feeder_num;
				}
			}
			else if ((m_t_feeder_module_record[u8_record].u8_module_addr >= FEEDER_PANEL_MODULE_BASE_ADDR)
				&& (m_t_feeder_module_record[u8_record].u8_module_addr < (FEEDER_PANEL_MODULE_BASE_ADDR+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX)))
			{
				u8_panel = (m_t_feeder_module_record[u8_record].u8_module_addr - FEEDER_PANEL_MODULE_BASE_ADDR) / FEEDER_PANEL_MODULE_MAX;
				u8_module = (m_t_feeder_module_record[u8_record].u8_module_addr - FEEDER_PANEL_MODULE_BASE_ADDR) % FEEDER_PANEL_MODULE_MAX;
				
				if ((u8_panel < g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num)
					&& (u8_module < g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel].u8_feeder_module_num))
				{
					u8_branch_num = g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel].t_feeder_module[u8_module].u8_feeder_num;
				}
				else if ((u8_panel < (4 + g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num))
					&& (u8_module < g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel].u8_feeder_module_num))
				{
					u8_branch_num = g_t_share_data.t_sys_cfg.t_feeder_panel[u8_panel].t_feeder_module[u8_module].u8_feeder_num;
				}

			}
			else if (m_t_feeder_module_record[u8_record].u8_module_addr == DCDC_PANEL_MODULE_BASE_ADDR)
			{
				if ((g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 0)
					&& (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num > 0))
				{
					u8_branch_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[0].u8_feeder_num;
				}
			}
			else if (m_t_feeder_module_record[u8_record].u8_module_addr == DCAC_PANEL_MODULE_BASE_ADDR)
			{
				if ((g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num > 0) && 
					(g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num > 0))
				{
					u8_branch_num = g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[0].u8_feeder_num;
				}
			}
			os_mut_release(g_mut_share_data);
			
			if (u8_branch_num == 0)
			{
				u8_record++;
				u8_record %= (sizeof(m_t_feeder_module_record) / sizeof(m_t_feeder_module_record[0]));
				if (u8_record == 0)
				{
					u8_fc_tx_flag = 1;
					u8_dcdc_tx_flag = 0;
				}
				continue;
			}
        	
			memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
			m_t_tx_msg_buf.id = (ID_FEEDER_MODULE_TYPE | FC_REQUEST_READ);
			m_t_tx_msg_buf.id |= (m_t_feeder_module_record[u8_record].u8_module_addr<<ID_ADDR_OFFSET);
			m_t_tx_msg_buf.id |= m_u16_info_code[m_t_feeder_module_record[u8_record].u8_info_code_index];
    		
			m_t_tx_msg_buf.len = 8;
			m_t_tx_msg_buf.ch = 0;
			m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
			m_t_tx_msg_buf.type = 1;      //0 - DATA FRAME, 1 - REMOTE FRAME
			
			if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
			{
				if (u8_record == 0)
				{
					os_dly_wait((U32_T)(m_f32_delay_scale * 1));
					u32_can_send(1, &m_t_tx_msg_buf);
				}
				else
				{
					break;
				}
			}
			
			m_t_feeder_module_record[u8_record].u8_info_code_index++;
			m_t_feeder_module_record[u8_record].u8_info_code_index %= (u8_branch_num + 3);
			
			u8_record++;
			u8_record %= (sizeof(m_t_feeder_module_record) / sizeof(m_t_feeder_module_record[0]));
			if (u8_record == 0)
			{
				u8_fc_tx_flag = 1;
				u8_dcdc_tx_flag = 0;
			}
		}
		else if ((u32_dcdc_module_flag == 1) && (u8_dcdc_tx_flag == 0))
		{
			memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
			m_t_tx_msg_buf.id = m_u32_dcdc_query_cmd[u8_dcdc_cmd_index];
			m_t_tx_msg_buf.id |= (u8_dcdc_module_index+1);
    		
    		m_t_tx_msg_buf.len = 0;	
			m_t_tx_msg_buf.ch = 0;
			m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
			m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
			
			if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
				break;
				
			u8_dcdc_module_index++;
			
			os_mut_wait(g_mut_share_data, 0xFFFF);
			u8_module = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
			os_mut_release(g_mut_share_data);
			
			if (u8_dcdc_module_index >= u8_module)
			{
				u8_dcdc_tx_flag = 1;
				u8_fc_tx_flag = 0;
				u8_dcdc_module_index = 0;
				
				u8_dcdc_cmd_index++;
				u8_dcdc_cmd_index %= DCDC_QUERY_CMD_MAX_NUM;
			
				//广播调压、限流命令
				v_can_dcdc_module_send_ctl(DCDC_SET_VOLT_CMD); 	
				v_can_dcdc_module_send_ctl(DCDC_SET_CURR_CMD);	
				v_can_dcdc_module_send_ctl(DCDC_SET_THR_CMD);
			}
		}
		else
		{
			u8_fc_tx_flag = 0;
			u8_dcdc_tx_flag = 0;
		}
	}
}

/*************************************************************
函数名称: v_can_module_can_comm_task
函数功能: 下级模块通信及数据处理任务函数
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
__task void v_can_module_can_comm_task(void)
{
	U32_T i, j, u32_num, u32_seg2_num, u32_fc_module_num;
	U32_T u32_dcdc_cfg_time = u32_delay_get_timer_val();
	U32_T u32_fdl_cfg_time = u32_delay_get_timer_val();
	F32_T f32_can_baud;
	INSU_MEAS_WAY_E  e_insu_meas_way;
	U8_T 			 u8_res_switch_delay;
	U16_T            u16_insu_sensor_range;
	U8_T             u8_insu_bus_err_confirm;
	U8_T             u8_insu_meas_period;
	U8_T             u8_insu_meas_hour, u8_insu_meas_min;
	RTC_TIME_T       t_rtc_time;
	U8_T             u8_disable = 0;

	os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志

	//初始化通信中断计时
	for (i=0; i<(sizeof(m_t_feeder_module_record) / sizeof(m_t_feeder_module_record[0])); i++)
		m_t_feeder_module_record[i].u32_fail_time = u32_delay_get_timer_val();

	//交流系统下发馈线配置数据
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u32_fc_module_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num;
	os_mut_release(g_mut_share_data);
	
	if (u32_fc_module_num > 0)
	{
		for (j=0; j<u32_fc_module_num; j++)
		{
			v_can_feeder_module_send_config(m_t_feeder_module_record[j].u8_module_addr, IC_BRANCH_NUM);        //各类型检测路数
			v_can_feeder_module_send_config(m_t_feeder_module_record[j].u8_module_addr, IC_CURR_SENSOR_COEFF); //电流传感器比例系数
			v_can_feeder_module_send_config(m_t_feeder_module_record[j].u8_module_addr, IC_INSU_ALM_THR);     //绝缘报警门限值
		}

		os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //减慢发送配置的速度，保证下级模块能够及时处理配置命令，不会因缓冲区满而丢掉配置命令
	}
	
	//直流系统下发馈线配置数据
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u32_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num;
	u32_seg2_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num;
	switch(g_t_share_data.t_sys_cfg.t_sys_param.e_can_baud)
	{
		case CAN_BAUD_50K:
			f32_can_baud = 50.0;
			break;
		case CAN_BAUD_20K:
			f32_can_baud = 20.0;
			break;
		case CAN_BAUD_10K:
			f32_can_baud = 10.0;
			break; 		
		case CAN_BAUD_125K:
		default:
			f32_can_baud = 125.0;
			break;	
	}
	os_mut_release(g_mut_share_data);
	m_f32_delay_scale = 125 / f32_can_baud;
		
	if ((u32_num > 0) || (u32_seg2_num > 0))
	{
		v_can_feeder_module_send_insu_para();

		os_mut_wait(g_mut_share_data, 0xFFFF);
		e_insu_meas_way         = g_t_share_data.t_sys_cfg.t_sys_param.e_insu_meas_way;
		u8_res_switch_delay     = g_t_share_data.t_sys_cfg.t_sys_param.u8_res_switch_delay;         
		u16_insu_sensor_range   = g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range;       
		u8_insu_bus_err_confirm = g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_bus_err_confirm;     
		u8_insu_meas_period     = g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_period;  

		m_e_insu_meas_way_bak         = e_insu_meas_way;
		m_u8_res_switch_delay_bak     = u8_res_switch_delay;         
		m_u16_insu_sensor_range_bak   = u16_insu_sensor_range;       
		m_u8_insu_bus_err_confirm_bak = u8_insu_bus_err_confirm;     
		m_u8_insu_meas_period_bak     = u8_insu_meas_period; 
		      
		os_mut_release(g_mut_share_data);

		for (i=0; i<u32_num; i++)
		{
			os_mut_wait(g_mut_share_data, 0xFFFF);
			u32_fc_module_num = g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num;
			os_mut_release(g_mut_share_data);
				
			for (j=0; j<u32_fc_module_num; j++)
			{
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+i*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_BRANCH_NUM);       //各类型检测路数
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+i*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //电流传感器比例系数
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+i*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_INSU_ALM_THR);     //绝缘报警门限值
				
				os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
			}

			os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //减慢发送配置的速度，保证下级模块能够及时处理配置命令，不会因缓冲区满而丢掉配置命令
		}

		for (i=0; i<u32_seg2_num; i++)
		{
			os_mut_wait(g_mut_share_data, 0xFFFF);
			u32_fc_module_num = g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num;
			os_mut_release(g_mut_share_data);
				
			for (j=0; j<u32_fc_module_num; j++)
			{
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+(4+i)*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_BRANCH_NUM);       //各类型检测路数
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+(4+i)*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //电流传感器比例系数
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+(4+i)*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_INSU_ALM_THR);     //绝缘报警门限值
				
				os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
			}

			os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //减慢发送配置的速度，保证下级模块能够及时处理配置命令，不会因缓冲区满而丢掉配置命令
		}
	}
	
	//通信系统下发馈线配置数据
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u32_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	u32_fc_module_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num;
	os_mut_release(g_mut_share_data);
	
	if (u32_fc_module_num > 0)
	{
		for (j=0; j<u32_fc_module_num; j++)
		{
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_BRANCH_NUM);        //各类型检测路数
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //电流传感器比例系数
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_INSU_ALM_THR);     //绝缘报警门限值
		}

		os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //减慢发送配置的速度，保证下级模块能够及时处理配置命令，不会因缓冲区满而丢掉配置命令
	}
	
	//逆变系统下发馈线配置数据
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u32_num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num;
	u32_fc_module_num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num;
	os_mut_release(g_mut_share_data);
	
	if (u32_fc_module_num > 0)
	{
		for (j=0; j<u32_fc_module_num; j++)
		{
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+1+j].u8_module_addr, IC_BRANCH_NUM);        //各类型检测路数
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+1+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //电流传感器比例系数
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+1+j].u8_module_addr, IC_INSU_ALM_THR);    //绝缘报警门限值
		}

		os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //减慢发送配置的速度，保证下级模块能够及时处理配置命令，不会因缓冲区满而丢掉配置命令
	}
	
	while (1)
	{
		os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
		
		u32_fc_module_flag = 0;
		u32_seg2_fc_module_flag = 0;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);

		v_public_fdl_swt_sync_update();			//由FC10模块采样的电操开关状态同步更新

		e_insu_meas_way         = g_t_share_data.t_sys_cfg.t_sys_param.e_insu_meas_way;
		u8_res_switch_delay     = g_t_share_data.t_sys_cfg.t_sys_param.u8_res_switch_delay;         
		u16_insu_sensor_range   = g_t_share_data.t_sys_cfg.t_sys_param.u16_insu_sensor_range;       
		u8_insu_bus_err_confirm = g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_bus_err_confirm;     
		u8_insu_meas_period     = g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_period; 
		u8_insu_meas_hour = g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_hour;
		u8_insu_meas_min  = g_t_share_data.t_sys_cfg.t_sys_param.u8_insu_meas_min;

		for (j=0; j<g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num; j++)
		{
			if (g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.t_feeder_module[j].u8_feeder_num > 0)
			{
				u32_fc_module_flag = 1;
				break;
			}
		}

		for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
		{
			for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num; j++)
			{
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_feeder_num > 0)
				{
					u32_fc_module_flag = 1;
					break;
				}
			}
			
			if (u32_fc_module_flag == 1)
				break;
		}

		for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc2_feeder_panel_num; i++)
		{
			for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num; j++)
			{
				if (g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].t_feeder_module[j].u8_feeder_num > 0)
				{
					u32_fc_module_flag = 1;
					u32_seg2_fc_module_flag = 1;
					break;
				}
			}
			
			if (u32_fc_module_flag == 1)
				break;
		}
		
		if ((u32_fc_module_flag == 0) && (g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 0))
		{
			for (j=0; j<g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num; j++)
			{
				if (g_t_share_data.t_sys_cfg.t_dcdc_panel.t_feeder_module[j].u8_feeder_num > 0)
				{
					u32_fc_module_flag = 1;
					break;
				}
			}
		}
		
		if ((u32_fc_module_flag == 0) && (g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num > 0))
		{
			for (j=0; j<g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num; j++)
			{
				if (g_t_share_data.t_sys_cfg.t_dcac_panel.t_feeder_module[j].u8_feeder_num > 0)
				{
					u32_fc_module_flag = 1;
					break;
				}
			}
		}
		
		if ((g_t_share_data.t_sys_cfg.t_dcdc_panel.e_protocol == DCDC_CAN) &&
			(g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num > 0))
		{
			if (u32_dcdc_module_flag != 1)
				v_can_set_tx_interval(1, 1);
			
			u32_dcdc_module_flag = 1;
		}
		else
		{
			if (u32_dcdc_module_flag != 0)
				v_can_set_tx_interval(1, 0);
			
			u32_dcdc_module_flag = 0;
		}
		
		os_mut_release(g_mut_share_data);

		if(	(m_e_insu_meas_way_bak         != e_insu_meas_way) ||
			(m_u8_res_switch_delay_bak     != u8_res_switch_delay) ||         
			(m_u16_insu_sensor_range_bak   != u16_insu_sensor_range) ||       
			(m_u8_insu_bus_err_confirm_bak != u8_insu_bus_err_confirm) ||     
			(m_u8_insu_meas_period_bak     != u8_insu_meas_period) )
		{
			v_can_feeder_module_send_insu_para();
			u32_fdl_cfg_time = u32_delay_get_timer_val();

			m_e_insu_meas_way_bak         = e_insu_meas_way;
			m_u8_res_switch_delay_bak     = u8_res_switch_delay;         
			m_u16_insu_sensor_range_bak   = u16_insu_sensor_range;       
			m_u8_insu_bus_err_confirm_bak = u8_insu_bus_err_confirm;     
			m_u8_insu_meas_period_bak     = u8_insu_meas_period;
		}
		
		if ((u32_fc_module_flag == 0) && (u32_dcdc_module_flag == 0))     //如果没有配模块，则让出CPU，不接收处理数据包
		{
			os_dly_wait((U32_T)(50));
			continue;
		}
		
		if (u32_can_receive(1, &m_t_rx_msg_buf) == 0)
		{
			if ((u32_fc_module_flag == 1) && ((m_t_rx_msg_buf.id & ID_DEVICE_TYPE_MASK) == ID_FEEDER_MODULE_TYPE))
				v_can_feeder_module_receive_handle();
			else if ((u32_dcdc_module_flag == 1) && ((m_t_rx_msg_buf.id & ID_DEVICE_TYPE_MASK) == ID_DCDC_MODULE_TYPE))
				v_can_dcdc_module_receive_handle();
		}
		else
		{
			v_can_feeder_module_send_handle();           //发送查询命令
			os_dly_wait((U32_T)(m_f32_delay_scale * 2));
		}
		
		if (u32_fc_module_flag == 1)
		{
			v_can_feeder_module_comm_fault_alm();        //处理通信中断

			u8_ctime_get_time(&t_rtc_time);
			if((t_rtc_time.hour == u8_insu_meas_hour) && (t_rtc_time.min == u8_insu_meas_min))
			{
				if(!u8_disable)
				{
					v_can_feeder_module_meas_start();
					u8_disable = 1;
				}
			}
			else
			{
				u8_disable = 0;
			}

			if (u32_delay_time_elapse(u32_fdl_cfg_time, u32_delay_get_timer_val()) > FDL_SEND_CFG_INTERVAL)
			{
				//绝缘测量参数周期下发
				v_can_feeder_module_send_insu_para();

				u32_fdl_cfg_time = u32_delay_get_timer_val();
			}
		}
			
		if (u32_dcdc_module_flag == 1)
		{
			v_can_dcdc_module_comm_fault_alm();          //处理通信中断
			
			if (u32_delay_time_elapse(u32_dcdc_cfg_time, u32_delay_get_timer_val()) > DCDC_SEND_CFG_INTERVAL)
			{
				//广播下发高压直流模块数据	
				v_can_dcdc_module_send_config();

				u32_dcdc_cfg_time = u32_delay_get_timer_val();
			}			
		}
	}
}

