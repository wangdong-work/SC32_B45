/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����CanComm.c
��    ����1.00
�������ڣ�2012-08-08
��    �ߣ�������
��������������ģ���ͨ�ż����ݴ�������ʵ��


�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-08-08  1.00     ����
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


//#define CAN_FAIL_TIMEOUT       (10*1000000)  //10sͨ�Ų��ϣ���ͨ�Ź���
#define CAN_TX_MAX_CNT         32             //ÿ�ε��÷��ͺ����ܷ���������������

/* ������ */
#define ID_FEEDER_MODULE_TYPE  0x14000000     //�豸����
#define ID_DEVICE_TYPE_MASK    0xFFE00000     //�豸����������
#define ID_DEVICE_ADDR_MASK    0x001FC000     //�豸��ַ������
#define ID_FUCN_CODE_MASK      0x00003C00     //������������
#define ID_INFO_CODE_MASK      0x000003FF     //��Ϣ��������
#define ID_ADDR_OFFSET         14             //��ַƫ����


/* ������ */
#define FC_REQUEST_CONFIG (1<<10)             //������������
#define FC_ACTIVE_UPLOAD  (2<<10)             //�����ϴ�����
#define FC_REQUEST_READ   (3<<10)             //�����ȡ����/�ش��ȡ����
#define FC_REQUEST_WRITE  (6<<10)             //����д����
#define FC_RETURN_SCUESS  (14<<10)            //����д����ִ�гɹ�
#define FC_RETURN_FAIL    (15<<10)            //����д����ִ��ʧ��
                                              
/* ��Ϣ�� */                                  
#define IC_BRANCH_START       0               //����֧·��ʼ��Ϣ��
#define IC_BRANCH_END         63              //����֧·������Ϣ��
#define IC_BUS_TO_GND_VOLT    128             //ĸ�߶Եص�ѹ
#define IC_BUS_TO_GND_RES     129             //ĸ�߶Եص���
#define IC_BUS_TO_GND_AC_VOLT 130             //ĸ�߶Եؽ�����ѹ
#define FC_IC_TOTAL_SWT_FAULT 131             //�ܿ��ع��ϸ澯״̬
#define IC_BRANCH_NUM         512             //�����ͼ��·��
#define IC_CURR_SENSOR_COEFF  513             //��������������ϵ��
#define IC_INSU_ALM_THR       515             //��Ե��������ֵ
#define IC_HW_VERSION         1001            //Ӳ���汾��
#define IC_SW_VERSION         1002            //����汾��

#define AC_PANEL_MODULE_BASE_ADDR      1      //����������ģ����ʼ��ַ
#define AC_PANEL_MODULE_END_ADDR       3      //����������ģ����ʼ��ַ
#define FEEDER_PANEL_MODULE_BASE_ADDR  14     //����������ģ����ʼ��ַ
#define FEEDER_PANEL_MODULE_BASE_ADDR2 30     //����������ģ����ʼ��ַ
#define DCDC_PANEL_MODULE_BASE_ADDR    112    //ͨ��������ģ����ʼ��ַ
#define DCAC_PANEL_MODULE_BASE_ADDR    114    //���������ģ����ʼ��ַ


static F32_T m_f32_delay_scale;

/* ����ģ�����ݼ�¼ */
typedef struct
{
	U8_T u8_module_addr;                      //����ģ���ַ
	U8_T u8_info_code_index;                  //��Ϣ������
	U32_T u32_fail_time;                  //ͨ��ʧ�ܼ���
}FEEDER_MODULE_RECORD_T;



static FEEDER_MODULE_RECORD_T m_t_feeder_module_record[] = 
{	//����������ģ��
	1, 0, 0,
	2, 0, 0,
	3, 0, 0,
	//һ��ֱ��1#��������ģ��
	14, 0, 0,
	15, 0, 0,
	16, 0, 0,
	17, 0, 0,
	//һ��ֱ��2#��������ģ��         
	18, 0, 0,
	19, 0, 0,
	20, 0, 0,
	21, 0, 0,
	//һ��ֱ��3#��������ģ��         
	22, 0, 0,
	23, 0, 0,
	24, 0, 0,
	25, 0, 0,
	//һ��ֱ��4#��������ģ��         
	26, 0, 0,
	27, 0, 0,
	28, 0, 0,
	29, 0, 0,
	//����ֱ��1#��������ģ��         
	30, 0, 0,
	31, 0, 0,
	32, 0, 0,
	33, 0, 0,
	//����ֱ��2#��������ģ��         
	34, 0, 0,
	35, 0, 0,
	36, 0, 0,
	37, 0, 0,
	//����ֱ��3#��������ģ��         
	38, 0, 0,
	39, 0, 0,
	40, 0, 0,
	41, 0, 0,
	//����ֱ��4#��������ģ��         
	42, 0, 0,
	43, 0, 0,
	44, 0, 0,
	45, 0, 0,
	//ͨ��������ģ��
	112, 0, 0,
	//UPS������ģ��
	114, 0, 0,	
};


#define IC_READ_MAX_NUM       67             //��Ϣ�����
static const U16_T m_u16_info_code[IC_READ_MAX_NUM] = 
{
	131, 1001, 1002,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
};


CAN_MSG_T m_t_tx_msg_buf;              //CAN���ͻ�����
CAN_MSG_T m_t_rx_msg_buf;              //CAN���ջ�����

U32_T u32_fc_module_flag = 0;
U32_T u32_seg2_fc_module_flag = 0; 	   //����ֱ��������ģ��
U32_T u32_dcdc_module_flag = 0;

static INSU_MEAS_WAY_E  m_e_insu_meas_way_bak;             // ĸ�ߵ��������ʽ������ģʽ�͵���ģʽ��Ĭ�Ϲ���ģʽ
static U8_T 			m_u8_res_switch_delay_bak;         // ƽ�⼰��ƽ�����Ͷ����ʱ��2~120���裬����Ϊ��λ��Ĭ��2��
static U16_T            m_u16_insu_sensor_range_bak;       // ֧·���������̣�1~500���裬��mAΪ��λ��Ĭ��10mA
static U8_T             m_u8_insu_bus_err_confirm_bak;     // ĸ�߾�Եѹ���ȷ��ʱ�䣬1~180���裬����Ϊ��λ��Ĭ��3��
static U8_T             m_u8_insu_meas_period_bak;         // ��Ե���ڲ�������ʱ�䣬0~180���裬��СʱΪ��λ��Ĭ��24Сʱ

/*************************************************************
��������: v_can_feeder_module_data_handle
��������: ����ģ��������ݴ����������������ϴ������ݣ�������02���ͻش��ȡ�����ݣ�������03��
�������: pt_msg -- ָ��CAN��Ϣ֡
�������: ��
����ֵ  ����
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
			if (u8_branch_index >= FEEDER_BRANCH_MAX)    //��ֹ����Խ��
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
			if (u8_branch_index >= FEEDER_BRANCH_MAX)    //��ֹ����Խ��
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
			if (u16_info_code >= FEEDER_BRANCH_MAX)    //��ֹ����Խ��
				return;
				
			pt_feeder_branch = &(g_t_share_data.t_rt_data.t_dcdc_panel.t_feeder[u16_info_code]);
			pu8_sensor_state = &(g_t_share_data.t_rt_data.t_dcdc_panel.u8_sensor_state[u16_info_code]);
			
			u16_swt_fault_id = u16_sensor_fault_id = (FAULT_DCDC_PANEL_GROUP<<FAULT_GROUP_OFFSET);
			u16_swt_fault_id |= (FAULT_DCDC_BRANCH_FAULT_CNT * u16_info_code + FAULT_DCDC_BRANCH_SWT + FAULT_DCDC_BRANCH_BASE);
			u16_sensor_fault_id |= (FAULT_DCDC_BRANCH_FAULT_CNT * u16_info_code + FAULT_DCDC_BRANCH_SENSOR + FAULT_DCDC_BRANCH_BASE);
		}
		else if (u8_addr == DCAC_PANEL_MODULE_BASE_ADDR)
		{
			if (u16_info_code >= FEEDER_BRANCH_MAX)    //��ֹ����Խ��
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
		
		//����һ��֧·�Եص�����Сֵ
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

		//���Ҷ���֧·�Եص�����Сֵ
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
		
		if ((u8_alarm_old != 0x02) && (u8_alarm_new == 0x02))                     //������բ����
			v_fauid_send_fault_id_occur(u16_swt_fault_id);
		else if ((u8_alarm_old == 0x02) && (u8_alarm_new != 0x02))                //������բ�ָ�
			v_fauid_send_fault_id_resume(u16_swt_fault_id);
		
		if ((u8_insu_state_old != 0x02) && (u8_insu_state_new == 0x02))           //֧·��Ե�½�����
			v_fauid_send_fault_id_occur(u16_insu_fault_id);
		else if ((u8_insu_state_old == 0x02) && (u8_insu_state_new != 0x02))      //֧·��Ե�½��ָ�
			v_fauid_send_fault_id_resume(u16_insu_fault_id);
		
		if ((u8_sensor_state_old != 0x02) && (u8_sensor_state_new == 0x02))       //�������쳣����
			v_fauid_send_fault_id_occur(u16_sensor_fault_id);
		else if ((u8_sensor_state_old == 0x02) && (u8_sensor_state_new != 0x02))  //�������쳣�ָ�
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
��������: v_can_feeder_module_clear_comm_fault_alm
��������: ��������ģ���ַ���ͨ���жϼ���������Ѿ�����ͨ���жϹ��ϣ���ָ�
�������: u8_addr -- ��ַ
�������: ��
����ֵ  ����
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
		return;        //0��ַ�������Ƿ���ַ�˳�
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
��������: v_can_feeder_module_comm_fault_alm
��������: ��������ģ���ַ����ͨ���жϼ�����ͨ���жϴ�������һ���Ĵ�������ͨ���жϹ���
�������: u8_addr -- ��ַ
�������: ��
����ֵ  ����
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
��������: v_can_feeder_module_meas_start
��������: �·�����ģ���Ե���ڲ�����������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
static void v_can_feeder_module_meas_start(void)
{
	U8_T  u8_addr = 127;	//�㲥
	U16_T u16_info_code = 520; //��Ե��������
	
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
��������: v_can_feeder_module_send_insu_para
��������: �·�����ģ���������ݴ�����
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
static void v_can_feeder_module_send_insu_para(void)
{
	U8_T  u8_addr = 127;	//�㲥
	U16_T u16_info_code = 514; //��Ե������������
	
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
��������: v_can_feeder_module_send_config
��������: �·�����ģ���������ݴ�����
�������: u8_addr       -- ��ַ
          u16_info_code -- ��Ϣ��
�������: ��
����ֵ  ����
**************************************************************/
static void v_can_feeder_module_send_config(U8_T u8_addr, U16_T u16_info_code)
{
	U8_T u8_panel_index, u8_module_index;
	FEEDER_MODULE_CFG_T *pt_feeder_module = NULL;
	U16_T u16_feeder_shunt_range;
	
	//����ַ�Ƿ�Ƿ�
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
		case IC_BRANCH_NUM:               //�����ͼ��·��
			m_t_tx_msg_buf.data[0] = pt_feeder_module->u8_alarm_feeder_num;
			m_t_tx_msg_buf.data[1] = pt_feeder_module->u8_state_feeder_num;
			m_t_tx_msg_buf.data[2] = pt_feeder_module->u8_insu_feeder_num;
			m_t_tx_msg_buf.data[3] = pt_feeder_module->u8_curr_feeder_num;
			m_t_tx_msg_buf.data[4] = pt_feeder_module->e_alarm_type;
			m_t_tx_msg_buf.data[5] = pt_feeder_module->e_state_type;
			
			m_t_tx_msg_buf.len = 6;
			break;

		case IC_CURR_SENSOR_COEFF:        //��������������ϵ��
			m_t_tx_msg_buf.data[0] = (U8_T)(u16_feeder_shunt_range >> 8);
			m_t_tx_msg_buf.data[1] = (U8_T)u16_feeder_shunt_range;
			m_t_tx_msg_buf.data[2] = u8_addr;
			m_t_tx_msg_buf.data[3] = u8_addr;
			
			m_t_tx_msg_buf.len = 4;
			break;
			
		case IC_INSU_ALM_THR:             //��Ե��������ֵ
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
��������: v_can_feeder_module_receive_handle
��������: ����ģ����մ�����
�������: ��
          ��
�������: ��
����ֵ  ����
**************************************************************/
static void v_can_feeder_module_receive_handle(void)
{
	U8_T i, j, u8_addr_flag, u8_recv_addr;
	
	//�˲��ַ�Ƿ�����Ч
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
	
	if (u8_addr_flag == 0)  //������յ��ĵ�ַ��Ч���򶪵����ݼ�������
		return;

	switch (m_t_rx_msg_buf.id & ID_FUCN_CODE_MASK)
	{
		case FC_REQUEST_CONFIG:              //������������
			if (m_t_rx_msg_buf.type == 1) //Զ��֡�Ŵ���
			{
				v_can_feeder_module_send_config(u8_recv_addr, m_t_rx_msg_buf.id&ID_INFO_CODE_MASK);

				//���ͨ���жϼ������ָ�ͨ���жϸ澯
				v_can_feeder_module_clear_comm_fault_alm(u8_recv_addr);
			}
			break;
			
		case FC_ACTIVE_UPLOAD:               //�����ϴ�����
		case FC_REQUEST_READ:                //�����ȡ����/�ش��ȡ����
			if (m_t_rx_msg_buf.type == 0)    //����֡�Ŵ���
			{
				v_can_feeder_module_data_handle(&m_t_rx_msg_buf);

				//���ͨ���жϼ������ָ�ͨ���жϸ澯
				v_can_feeder_module_clear_comm_fault_alm(u8_recv_addr);
			}
			break;
			
		case FC_RETURN_FAIL:                 //����д����ִ��ʧ��
		case FC_RETURN_SCUESS:               //����д����ִ�гɹ�
			if (m_t_rx_msg_buf.type == 0)    //����֡�Ŵ���
			{
				//���ͨ���жϼ������ָ�ͨ���жϸ澯
				v_can_feeder_module_clear_comm_fault_alm(u8_recv_addr);
			}
						
		case FC_REQUEST_WRITE:               //����д����
		default:
			break;
	}
}

/*************************************************************
��������: v_can_feeder_module_send_handle
��������: ����ģ�鷢�Ͳ�ѯ�������						
�������: ��
�������: ��
����ֵ  ����
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
		os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //����ι���¼���־
		
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
						
					if (u16_bus_info_code == IC_BUS_TO_GND_VOLT)       //ĸ�߶Եص�ѹ	
						u16_bus_info_code = IC_BUS_TO_GND_RES;
					else if (u16_bus_info_code == IC_BUS_TO_GND_RES)   //ĸ�߶Եص���
						u16_bus_info_code = IC_BUS_TO_GND_AC_VOLT;
					else                                           //ĸ�߶Եؽ�����ѹ
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
						
					if (u16_bus2_info_code == IC_BUS_TO_GND_VOLT)       //ĸ�߶Եص�ѹ	
						u16_bus2_info_code = IC_BUS_TO_GND_RES;
					else if (u16_bus2_info_code == IC_BUS_TO_GND_RES)   //ĸ�߶Եص���
						u16_bus2_info_code = IC_BUS_TO_GND_AC_VOLT;
					else                                           //ĸ�߶Եؽ�����ѹ
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
			
				//�㲥��ѹ����������
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
��������: v_can_module_can_comm_task
��������: �¼�ģ��ͨ�ż����ݴ���������
�������: ��
�������: ��
����ֵ  ����
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

	os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //����ι���¼���־

	//��ʼ��ͨ���жϼ�ʱ
	for (i=0; i<(sizeof(m_t_feeder_module_record) / sizeof(m_t_feeder_module_record[0])); i++)
		m_t_feeder_module_record[i].u32_fail_time = u32_delay_get_timer_val();

	//����ϵͳ�·�������������
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u32_fc_module_num = g_t_share_data.t_sys_cfg.t_ac_panel.t_feed.u8_feeder_module_num;
	os_mut_release(g_mut_share_data);
	
	if (u32_fc_module_num > 0)
	{
		for (j=0; j<u32_fc_module_num; j++)
		{
			v_can_feeder_module_send_config(m_t_feeder_module_record[j].u8_module_addr, IC_BRANCH_NUM);        //�����ͼ��·��
			v_can_feeder_module_send_config(m_t_feeder_module_record[j].u8_module_addr, IC_CURR_SENSOR_COEFF); //��������������ϵ��
			v_can_feeder_module_send_config(m_t_feeder_module_record[j].u8_module_addr, IC_INSU_ALM_THR);     //��Ե��������ֵ
		}

		os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //�����������õ��ٶȣ���֤�¼�ģ���ܹ���ʱ����������������򻺳�������������������
	}
	
	//ֱ��ϵͳ�·�������������
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
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+i*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_BRANCH_NUM);       //�����ͼ��·��
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+i*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //��������������ϵ��
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+i*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_INSU_ALM_THR);     //��Ե��������ֵ
				
				os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			}

			os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //�����������õ��ٶȣ���֤�¼�ģ���ܹ���ʱ����������������򻺳�������������������
		}

		for (i=0; i<u32_seg2_num; i++)
		{
			os_mut_wait(g_mut_share_data, 0xFFFF);
			u32_fc_module_num = g_t_share_data.t_sys_cfg.t_feeder_panel[4+i].u8_feeder_module_num;
			os_mut_release(g_mut_share_data);
				
			for (j=0; j<u32_fc_module_num; j++)
			{
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+(4+i)*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_BRANCH_NUM);       //�����ͼ��·��
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+(4+i)*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //��������������ϵ��
				v_can_feeder_module_send_config(m_t_feeder_module_record[3+(4+i)*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_INSU_ALM_THR);     //��Ե��������ֵ
				
				os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			}

			os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //�����������õ��ٶȣ���֤�¼�ģ���ܹ���ʱ����������������򻺳�������������������
		}
	}
	
	//ͨ��ϵͳ�·�������������
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u32_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	u32_fc_module_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_feeder_module_num;
	os_mut_release(g_mut_share_data);
	
	if (u32_fc_module_num > 0)
	{
		for (j=0; j<u32_fc_module_num; j++)
		{
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_BRANCH_NUM);        //�����ͼ��·��
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //��������������ϵ��
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+j].u8_module_addr, IC_INSU_ALM_THR);     //��Ե��������ֵ
		}

		os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //�����������õ��ٶȣ���֤�¼�ģ���ܹ���ʱ����������������򻺳�������������������
	}
	
	//���ϵͳ�·�������������
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u32_num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_dcac_module_num;
	u32_fc_module_num = g_t_share_data.t_sys_cfg.t_dcac_panel.u8_feeder_module_num;
	os_mut_release(g_mut_share_data);
	
	if (u32_fc_module_num > 0)
	{
		for (j=0; j<u32_fc_module_num; j++)
		{
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+1+j].u8_module_addr, IC_BRANCH_NUM);        //�����ͼ��·��
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+1+j].u8_module_addr, IC_CURR_SENSOR_COEFF); //��������������ϵ��
			v_can_feeder_module_send_config(m_t_feeder_module_record[3+FEEDER_PANEL_MAX*FEEDER_PANEL_MODULE_MAX+1+j].u8_module_addr, IC_INSU_ALM_THR);    //��Ե��������ֵ
		}

		os_dly_wait((U32_T)(m_f32_delay_scale * 10));        //�����������õ��ٶȣ���֤�¼�ģ���ܹ���ʱ����������������򻺳�������������������
	}
	
	while (1)
	{
		os_evt_set(CAN_FEED_DOG, g_tid_wdt);             //����ι���¼���־
		
		u32_fc_module_flag = 0;
		u32_seg2_fc_module_flag = 0;
		
		os_mut_wait(g_mut_share_data, 0xFFFF);

		v_public_fdl_swt_sync_update();			//��FC10ģ������ĵ�ٿ���״̬ͬ������

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
		
		if ((u32_fc_module_flag == 0) && (u32_dcdc_module_flag == 0))     //���û����ģ�飬���ó�CPU�������մ������ݰ�
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
			v_can_feeder_module_send_handle();           //���Ͳ�ѯ����
			os_dly_wait((U32_T)(m_f32_delay_scale * 2));
		}
		
		if (u32_fc_module_flag == 1)
		{
			v_can_feeder_module_comm_fault_alm();        //����ͨ���ж�

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
				//��Ե�������������·�
				v_can_feeder_module_send_insu_para();

				u32_fdl_cfg_time = u32_delay_get_timer_val();
			}
		}
			
		if (u32_dcdc_module_flag == 1)
		{
			v_can_dcdc_module_comm_fault_alm();          //����ͨ���ж�
			
			if (u32_delay_time_elapse(u32_dcdc_cfg_time, u32_delay_get_timer_val()) > DCDC_SEND_CFG_INTERVAL)
			{
				//�㲥�·���ѹֱ��ģ������	
				v_can_dcdc_module_send_config();

				u32_dcdc_cfg_time = u32_delay_get_timer_val();
			}			
		}
	}
}

