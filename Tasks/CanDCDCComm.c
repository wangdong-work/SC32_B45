/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：CanDCDCComm.cpp
版    本：1.00
创建日期：2013-08-22
作    者：郭数理
功能描述：通信48V模块的通信及数据处理实现


修改记录：
	作者      日期        版本     修改内容
	郭数理    2013-08-22  1.00     创建
**************************************************************/


#include <rtl.h>
#include <string.h>
#include <lpc17xx.h>

#include "Type.h"
#include "PublicData.h"
#include "FaultID.h"
#include "CanDCDCComm.h"
#include "Com1Comm.h"

#include "../Drivers/Delay.h"
#include "../Drivers/CAN.h"


/* 屏蔽码 */
#define DCDC_ID_MSG_TYPE_MASK    0x01C00000             //报文类型屏蔽码
#define DCDC_ID_INFO_TYPE_MASK   0x003FF800             //信息类型屏蔽码
#define DCDC_ID_MODULE_TYPE_MASK 0x00000780             //模块类型屏蔽码
#define DCDC_ID_MODULE_ADDR_MASK 0x0000007F             //模块地址屏蔽码

/* 报文类型 */
#define DCDC_MSG_OFFSET          22                     //报文类型偏移量
#define DCDC_MSG_MODULE          (0<<DCDC_MSG_OFFSET)   //模块
#define DCDC_MSG_CSU             (4<<DCDC_MSG_OFFSET)   //CSU
#define DCDC_MSG_LONGACK         (3<<DCDC_MSG_OFFSET)   //longACK
#define DCDC_MSG_LONGSTART       (5<<DCDC_MSG_OFFSET)   //longStart
#define DCDC_MSG_LONGCONT        (6<<DCDC_MSG_OFFSET)   //longCont
#define DCDC_MSG_LONGFINAL       (7<<DCDC_MSG_OFFSET)   //longFinal

/* 模块类型 */
#define DCDC_MODULE_TYPE         (1<<7)                 //240V通信48V模块

/* 信息类型 */
#define DCDC_INFO_TYPE_OFFSET    11                                  //模块信息类型偏移量
#define DCDC_INFO_GET_STATE          (0x00<<DCDC_INFO_TYPE_OFFSET)   //获取模块状态
#define DCDC_INFO_GET_DETECT_DATE    (0x10<<DCDC_INFO_TYPE_OFFSET)   //获取模块侦测信息
#define DCDC_INFO_GET_RUN_TIME       (0x13<<DCDC_INFO_TYPE_OFFSET)   //获取模块运行时间和效能待机时间 
#define DCDC_INFO_GET_SET_VOLT       (0x20<<DCDC_INFO_TYPE_OFFSET)   //获取模块电压设定信息
#define DCDC_INFO_GET_SET_CURR       (0x21<<DCDC_INFO_TYPE_OFFSET)   //获取模块电流和过压设定信息
#define DCDC_INFO_GET_SET_THR        (0x22<<DCDC_INFO_TYPE_OFFSET)   //获取模块最大最小设定信息
#define DCDC_INFO_GET_ON_OFF         (0x23<<DCDC_INFO_TYPE_OFFSET)   //获取模块开关机参数信息
#define DCDC_INFO_GET_MODEL1         (0x24<<DCDC_INFO_TYPE_OFFSET)   //获取模块型号1 
#define DCDC_INFO_GET_MODEL2         (0x25<<DCDC_INFO_TYPE_OFFSET)   //获取模块型号2 
#define DCDC_INFO_GET_BARCODE1       (0x26<<DCDC_INFO_TYPE_OFFSET)   //获取模块序列号1
#define DCDC_INFO_GET_BARCODE2       (0x27<<DCDC_INFO_TYPE_OFFSET)   //获取模块序列号2

#define DCDC_INFO_SET_SAVE           (0x40<<DCDC_INFO_TYPE_OFFSET)   //配置参数并存储EEPROM
#define DCDC_INFO_SET_VOLT           (0x42<<DCDC_INFO_TYPE_OFFSET)   //配置参数不保存
#define DCDC_INFO_SET_CURR           (0x43<<DCDC_INFO_TYPE_OFFSET)   //配置参数不保存
#define DCDC_INFO_SET_THR            (0x44<<DCDC_INFO_TYPE_OFFSET)   //配置参数不保存
#define DCDC_INFO_SET_ON_OFF         (0x45<<DCDC_INFO_TYPE_OFFSET)   //模块关机复位
#define DCDC_INFO_RECV_ERROR         (0x50<<DCDC_INFO_TYPE_OFFSET)   //接收错误应答


const U32_T m_u32_dcdc_query_cmd[] =
{
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_STATE | DCDC_MODULE_TYPE),          //获取模块状态
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_DETECT_DATE | DCDC_MODULE_TYPE),    //获取模块侦测信息
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_SET_CURR | DCDC_MODULE_TYPE),       //获取模块电流和过压设定信息
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGFINAL | DCDC_INFO_SET_ON_OFF | DCDC_MODULE_TYPE),   //模块关机复位指令
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_RUN_TIME | DCDC_MODULE_TYPE),       //获取模块运行时间和效能待机时间
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_SET_VOLT | DCDC_MODULE_TYPE),       //获取模块电压设定信息
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_SET_THR | DCDC_MODULE_TYPE),        //获取模块最大最小设定信息
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_ON_OFF | DCDC_MODULE_TYPE),         //获取模块开关机参数信息
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_MODEL1 | DCDC_MODULE_TYPE),         //获取模块型号1
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_MODEL2 | DCDC_MODULE_TYPE),         //获取模块型号2
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_BARCODE1 | DCDC_MODULE_TYPE),       //获取模块序列号1
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_BARCODE2 | DCDC_MODULE_TYPE),       //获取模块序列号2
};

static const U8_T m_u8_dcdc_rated_curr[] = { 5, 10, 20, 30, 40, 50, 60, 80, 100};

static U32_T m_u32_dcdc_fail_time[DCDC_MODULE_MAX];                //通信失败记时

extern CAN_MSG_T m_t_tx_msg_buf;              //CAN发送缓冲区
extern CAN_MSG_T m_t_rx_msg_buf;              //CAN接收缓冲区


/*************************************************************
函数名称: v_can_dcdc_module_data_handle
函数功能: 通信48V模块接收数据处理函数
输入参数: pt_msg -- 指向CAN消息帧
输出参数: 无
返回值  ：无
**************************************************************/	
static void v_can_dcdc_module_data_handle(CAN_MSG_T *pt_msg)
{
	U8_T u8_index;
	U16_T i, u16_old_state, u16_new_state;
	
	u8_index = (m_t_rx_msg_buf.id & DCDC_ID_MODULE_ADDR_MASK) - 1;
	
	switch (m_t_rx_msg_buf.id & DCDC_ID_INFO_TYPE_MASK)
	{
		case DCDC_INFO_GET_STATE:           //获取模块状态
			os_mut_wait(g_mut_share_data, 0xFFFF);
			
			u16_old_state = g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state;
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state = 0;    //清零状态字节
			
			if ((pt_msg->data[0] & 0x02) != 0)      //模块开关机状态位	
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state |= 0x0001;
			
			if ((pt_msg->data[0] & 0x10) != 0)      //模块故障状态位
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state |= 0x0008;
			
			if ((pt_msg->data[2] & 0x40) != 0)      //过流保护状态位
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state |= 0x0004;
			
			u16_new_state = g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state;
			
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].b_ctl_mode = 0;
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].e_module_state = ((u16_new_state & 0x0001) ? SHUT_DOWN : START_UP);
			if ((u16_new_state & DCDC_MODULE_EXCEPTION_MASK) != 0)
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].e_module_state = EXCEPTION;              
			
			os_mut_release(g_mut_share_data);

			v_com1_dcdc_module_send_fault_id(u8_index, u16_old_state, u16_new_state);     //发送告警ID
				
			break;
			
		case DCDC_INFO_GET_DETECT_DATE:     //获取模块侦测信息
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].f32_out_curr = ((pt_msg->data[0]<<8) | pt_msg->data[1]) / 10.0;
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].f32_out_volt = ((pt_msg->data[2]<<8) | pt_msg->data[3]) / 10.0;
			
			//计算通信母线电压、电流
			g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_curr = 0;
			g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt = 0;
			for (i=0; i<g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num; i++)
			{
				g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_curr += 
							g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[i].f32_out_curr;
				
				if (g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt
						< g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[i].f32_out_volt)
				{
					g_t_share_data.t_rt_data.t_dcdc_panel.f32_dcdc_bus_volt = 
						g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[i].f32_out_volt;
				}
			}
				
			os_mut_release(g_mut_share_data);
			break;

		case DCDC_INFO_GET_SET_CURR:        //获取模块电流和过压设定信息
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].f32_curr_percent = ((pt_msg->data[0]<<8) | pt_msg->data[1]) / 10.0 * 100 / 
																			m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr];
			os_mut_release(g_mut_share_data);
			break;
		
		case DCDC_INFO_GET_RUN_TIME:        //获取模块运行时间和效能待机时间
		case DCDC_INFO_GET_BARCODE1:        //获取模块序列号1
		case DCDC_INFO_GET_BARCODE2:        //获取模块序列号2	 
		case DCDC_INFO_GET_MODEL1:          //获取模块型号1 
		case DCDC_INFO_GET_MODEL2:          //获取模块型号2 
		case DCDC_INFO_GET_SET_VOLT:        //获取模块电压设定信息
		case DCDC_INFO_GET_SET_THR:         //获取模块最大最小设定信息
		case DCDC_INFO_GET_ON_OFF:          //获取模块开关机参数信息                           
		case DCDC_INFO_SET_SAVE:            //配置参数并存储EEPROM
		case DCDC_INFO_SET_VOLT:            //配置参数不保存
		case DCDC_INFO_SET_CURR:            //配置参数不保存
		case DCDC_INFO_SET_THR:             //配置参数不保存
		case DCDC_INFO_SET_ON_OFF:          //模块关机复位
		case DCDC_INFO_RECV_ERROR:          //接收错误应答
		default:
			break;
	}
}

/*************************************************************
函数名称: v_can_dcdc_module_clear_comm_fault_alm
函数功能: 根据通信48V模块索引清除通信中断计数，如果已经报了通信中断故障，则恢复
输入参数: u8_addr -- 地址
输出参数: 无
返回值  ：无
**************************************************************/
static void v_can_dcdc_module_clear_comm_fault_alm(U8_T u8_addr)
{
	U8_T u8_comm_state = 0;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u8_comm_state = g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_addr-1].u8_comm_state;
	g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_addr-1].u8_comm_state = 0;	
	os_mut_release(g_mut_share_data);
	
	if (u8_comm_state != 0)
	{
		v_fauid_send_fault_id_resume((FAULT_DCDC_GROUP<<FAULT_GROUP_OFFSET) | (FAULT_DCDC_CNT*(u8_addr-1)+FAULT_DCDC_COMM_FAIL));
	}
		
	m_u32_dcdc_fail_time[u8_addr-1] = u32_delay_get_timer_val();
}

/*************************************************************
函数名称: v_can_dcdc_module_comm_fault_alm
函数功能: 通信48V模块增加通信中断计时，超过一定的时间未通信上，报通信中断故障
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_comm_fault_alm(void)
{
	U8_T i;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
					
	for (i=0; i<g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num; i++)
	{
		if (g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[i].u8_comm_state == 0)
		{
			if (u32_delay_time_elapse(m_u32_dcdc_fail_time[i], u32_delay_get_timer_val()) > 
						g_t_share_data.t_sys_cfg.t_sys_param.u8_dcdc_offline_cnt*OSC_SECOND)
			{
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[i].u8_comm_state = 1;
					
				os_mut_release(g_mut_share_data);
				v_fauid_send_fault_id_occur((FAULT_DCDC_GROUP<<FAULT_GROUP_OFFSET) | (FAULT_DCDC_CNT*i+FAULT_DCDC_COMM_FAIL));
				os_mut_wait(g_mut_share_data, 0xFFFF);
			}
		}
	}
	os_mut_release(g_mut_share_data);
}

/*************************************************************
函数名称: v_can_dcdc_module_receive_handle
函数功能: 通信48V模块接收处理函数
输入参数: 无
          无
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_receive_handle(void)
{
	U8_T u8_recv_addr, u8_module_num;
	
	if ((m_t_rx_msg_buf.id & DCDC_ID_MODULE_TYPE_MASK) != DCDC_MODULE_TYPE)
		return;
	
	//核查地址是否是有效
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u8_module_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	os_mut_release(g_mut_share_data);
	
	u8_recv_addr = (m_t_rx_msg_buf.id & DCDC_ID_MODULE_ADDR_MASK);
	if ((u8_recv_addr == 0) || (u8_recv_addr > u8_module_num))  //如果接收到的地址无效，则丢掉数据继续接收
		return;
		
	if (m_t_rx_msg_buf.type == 1)    //远程帧无效，退出
		return;

	switch (m_t_rx_msg_buf.id & DCDC_ID_MSG_TYPE_MASK)
	{
		case DCDC_MSG_MODULE:                //模块回答读取数据
			v_can_dcdc_module_data_handle(&m_t_rx_msg_buf);

			//清除通信中断计数，恢复通信中断告警
			v_can_dcdc_module_clear_comm_fault_alm(u8_recv_addr);
			break;
						
		case DCDC_MSG_LONGACK:               //长配置应答
			//清除通信中断计数，恢复通信中断告警
			v_can_dcdc_module_clear_comm_fault_alm(u8_recv_addr);
			break;
									
		case DCDC_MSG_CSU:
		case DCDC_MSG_LONGSTART:
		case DCDC_MSG_LONGCONT:
		case DCDC_MSG_LONGFINAL:
		default:
			break;
	}
}

/*************************************************************
函数名称: v_can_dcdc_module_send_config
函数功能: 广播下发通信48V模块配置数据处理函数，保存到EEPROM
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_send_config(void)
{
	U16_T u16_data;
	
	//设定离线电压
	memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
	m_t_tx_msg_buf.id = (ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGSTART | DCDC_INFO_SET_SAVE | DCDC_MODULE_TYPE);
	m_t_tx_msg_buf.ch = 0;
	m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
	m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
	m_t_tx_msg_buf.len = 8;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	u16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt*10);
	m_t_tx_msg_buf.data[6] = (u16_data>>8);
	m_t_tx_msg_buf.data[7] = u16_data;
	
	os_mut_release(g_mut_share_data);
	
	if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
	{
		os_dly_wait(2);
		u32_can_send(1, &m_t_tx_msg_buf);
	}
	
	//设定离线电流、启动模式、母线过压、电压最小值
	memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
	m_t_tx_msg_buf.id = (ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGCONT | DCDC_INFO_SET_SAVE | DCDC_MODULE_TYPE);
	m_t_tx_msg_buf.ch = 0;
	m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
	m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
	m_t_tx_msg_buf.len = 8;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	u16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent *
						m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr]*10/100);
	
	m_t_tx_msg_buf.data[0] = (u16_data>>8);
	m_t_tx_msg_buf.data[1] = u16_data;
	m_t_tx_msg_buf.data[2] = 0;
	
	u16_data = (U16_T)(59 * 10);
	m_t_tx_msg_buf.data[4] = (u16_data>>8);
	m_t_tx_msg_buf.data[5] = u16_data;
	
	u16_data = (U16_T)(42 * 10);
	m_t_tx_msg_buf.data[6] = (u16_data>>8);
	m_t_tx_msg_buf.data[7] = u16_data;
	
	os_mut_release(g_mut_share_data);
	
	if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
	{
		os_dly_wait(2);
		u32_can_send(1, &m_t_tx_msg_buf);
	}
	
	//设定电压最大值、电流最小值、电流最大值
	memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
	m_t_tx_msg_buf.id = (ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGFINAL | DCDC_INFO_SET_SAVE | DCDC_MODULE_TYPE);
	m_t_tx_msg_buf.ch = 0;
	m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
	m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
	m_t_tx_msg_buf.len = 6;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	u16_data = (U16_T)(58.5 * 10);
	m_t_tx_msg_buf.data[0] = (u16_data>>8);
	m_t_tx_msg_buf.data[1] = u16_data;
	
	u16_data = (U16_T)(5 * m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr]*10/100);
	m_t_tx_msg_buf.data[2] = (u16_data>>8);
	m_t_tx_msg_buf.data[3] = u16_data;
		
	u16_data = (U16_T)(110 * m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr]*10/100);
	m_t_tx_msg_buf.data[4] = (u16_data>>8);
	m_t_tx_msg_buf.data[5] = u16_data;
	
	os_mut_release(g_mut_share_data);	
	
	if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
	{
		os_dly_wait(2);
		u32_can_send(1, &m_t_tx_msg_buf);
	}
}

/*************************************************************
函数名称: v_can_dcdc_module_send_ctl
函数功能: 广播下发通信48V模块控制数据处理函数，不保存到EEPROM
输入参数: u8_cmd_index -- DCDC_SET_VOLT_CMD: 设定控制电压命令
                          DCDC_SET_CURR_CMD: 设定电流命令
                          DCDC_SET_THR_CMD:  设定电压、电流最大值/最小值
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_send_ctl(U8_T u8_cmd_index)
{
	U16_T u16_data;
	
	if (u8_cmd_index == DCDC_SET_VOLT_CMD)  	 //设定控制电压
	{
		memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
		m_t_tx_msg_buf.id = (ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGFINAL | DCDC_INFO_SET_VOLT | DCDC_MODULE_TYPE);
		m_t_tx_msg_buf.ch = 0;
		m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
		m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
		m_t_tx_msg_buf.len = 8;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		u16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_out_volt*10);
		m_t_tx_msg_buf.data[6] = (u16_data>>8);
		m_t_tx_msg_buf.data[7] = u16_data;
		
		os_mut_release(g_mut_share_data);
		
		if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
		{
			os_dly_wait(2);
			u32_can_send(1, &m_t_tx_msg_buf);
		}
	}
	
	else if (u8_cmd_index == DCDC_SET_CURR_CMD)    //设定限流值、启动模式、均流控制、母线过压
	{
		memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
		m_t_tx_msg_buf.id = (ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGFINAL | DCDC_INFO_SET_CURR | DCDC_MODULE_TYPE);
		m_t_tx_msg_buf.ch = 0;
		m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
		m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
		m_t_tx_msg_buf.len = 6;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		u16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dcdc_panel.f32_curr_percent *
						m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr]*10/100);
		m_t_tx_msg_buf.data[0] = (u16_data>>8);
		m_t_tx_msg_buf.data[1] = u16_data;
		m_t_tx_msg_buf.data[2] = 0;
		m_t_tx_msg_buf.data[3] = 0;
		
		u16_data = (U16_T)(59 * 10);
		m_t_tx_msg_buf.data[4] = (u16_data>>8);
		m_t_tx_msg_buf.data[5] = u16_data;
		
		os_mut_release(g_mut_share_data);
		
		if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
		{
			os_dly_wait(2);
			u32_can_send(1, &m_t_tx_msg_buf);
		}
	}
	
	else if (u8_cmd_index == DCDC_SET_THR_CMD)    //设定电压最小值、电压最大值、电流最小值、电流最大值
	{
		memset(&m_t_tx_msg_buf, 0, sizeof(m_t_tx_msg_buf));
		m_t_tx_msg_buf.id = (ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGFINAL | DCDC_INFO_SET_THR | DCDC_MODULE_TYPE);
		m_t_tx_msg_buf.ch = 0;
		m_t_tx_msg_buf.format = 1;    //0 - STANDARD, 1 - EXTENDED IDENTIFIER
		m_t_tx_msg_buf.type = 0;      //0 - DATA FRAME, 1 - REMOTE FRAME
		m_t_tx_msg_buf.len = 8;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		u16_data = (U16_T)(42 * 10);
		m_t_tx_msg_buf.data[0] = (u16_data>>8);
		m_t_tx_msg_buf.data[1] = u16_data;
		
		u16_data = (U16_T)(58.5 * 10);
		m_t_tx_msg_buf.data[2] = (u16_data>>8);
		m_t_tx_msg_buf.data[3] = u16_data;
		
		u16_data = (U16_T)(5 * m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr]*10/100);
		m_t_tx_msg_buf.data[4] = (u16_data>>8);
		m_t_tx_msg_buf.data[5] = u16_data;
			
		u16_data = (U16_T)(110 * m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr]*10/100);
		m_t_tx_msg_buf.data[6] = (u16_data>>8);
		m_t_tx_msg_buf.data[7] = u16_data;
		
		os_mut_release(g_mut_share_data);	
		
		if (u32_can_send(1, &m_t_tx_msg_buf) != 0)
		{
			os_dly_wait(2);
			u32_can_send(1, &m_t_tx_msg_buf);
		}
	}
}
