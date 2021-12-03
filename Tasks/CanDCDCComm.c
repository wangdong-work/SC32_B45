/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����CanDCDCComm.cpp
��    ����1.00
�������ڣ�2013-08-22
��    �ߣ�������
����������ͨ��48Vģ���ͨ�ż����ݴ���ʵ��


�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2013-08-22  1.00     ����
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


/* ������ */
#define DCDC_ID_MSG_TYPE_MASK    0x01C00000             //��������������
#define DCDC_ID_INFO_TYPE_MASK   0x003FF800             //��Ϣ����������
#define DCDC_ID_MODULE_TYPE_MASK 0x00000780             //ģ������������
#define DCDC_ID_MODULE_ADDR_MASK 0x0000007F             //ģ���ַ������

/* �������� */
#define DCDC_MSG_OFFSET          22                     //��������ƫ����
#define DCDC_MSG_MODULE          (0<<DCDC_MSG_OFFSET)   //ģ��
#define DCDC_MSG_CSU             (4<<DCDC_MSG_OFFSET)   //CSU
#define DCDC_MSG_LONGACK         (3<<DCDC_MSG_OFFSET)   //longACK
#define DCDC_MSG_LONGSTART       (5<<DCDC_MSG_OFFSET)   //longStart
#define DCDC_MSG_LONGCONT        (6<<DCDC_MSG_OFFSET)   //longCont
#define DCDC_MSG_LONGFINAL       (7<<DCDC_MSG_OFFSET)   //longFinal

/* ģ������ */
#define DCDC_MODULE_TYPE         (1<<7)                 //240Vͨ��48Vģ��

/* ��Ϣ���� */
#define DCDC_INFO_TYPE_OFFSET    11                                  //ģ����Ϣ����ƫ����
#define DCDC_INFO_GET_STATE          (0x00<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ��״̬
#define DCDC_INFO_GET_DETECT_DATE    (0x10<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ�������Ϣ
#define DCDC_INFO_GET_RUN_TIME       (0x13<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ������ʱ���Ч�ܴ���ʱ�� 
#define DCDC_INFO_GET_SET_VOLT       (0x20<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ���ѹ�趨��Ϣ
#define DCDC_INFO_GET_SET_CURR       (0x21<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ������͹�ѹ�趨��Ϣ
#define DCDC_INFO_GET_SET_THR        (0x22<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ�������С�趨��Ϣ
#define DCDC_INFO_GET_ON_OFF         (0x23<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ�鿪�ػ�������Ϣ
#define DCDC_INFO_GET_MODEL1         (0x24<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ���ͺ�1 
#define DCDC_INFO_GET_MODEL2         (0x25<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ���ͺ�2 
#define DCDC_INFO_GET_BARCODE1       (0x26<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ�����к�1
#define DCDC_INFO_GET_BARCODE2       (0x27<<DCDC_INFO_TYPE_OFFSET)   //��ȡģ�����к�2

#define DCDC_INFO_SET_SAVE           (0x40<<DCDC_INFO_TYPE_OFFSET)   //���ò������洢EEPROM
#define DCDC_INFO_SET_VOLT           (0x42<<DCDC_INFO_TYPE_OFFSET)   //���ò���������
#define DCDC_INFO_SET_CURR           (0x43<<DCDC_INFO_TYPE_OFFSET)   //���ò���������
#define DCDC_INFO_SET_THR            (0x44<<DCDC_INFO_TYPE_OFFSET)   //���ò���������
#define DCDC_INFO_SET_ON_OFF         (0x45<<DCDC_INFO_TYPE_OFFSET)   //ģ��ػ���λ
#define DCDC_INFO_RECV_ERROR         (0x50<<DCDC_INFO_TYPE_OFFSET)   //���մ���Ӧ��


const U32_T m_u32_dcdc_query_cmd[] =
{
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_STATE | DCDC_MODULE_TYPE),          //��ȡģ��״̬
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_DETECT_DATE | DCDC_MODULE_TYPE),    //��ȡģ�������Ϣ
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_SET_CURR | DCDC_MODULE_TYPE),       //��ȡģ������͹�ѹ�趨��Ϣ
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_LONGFINAL | DCDC_INFO_SET_ON_OFF | DCDC_MODULE_TYPE),   //ģ��ػ���λָ��
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_RUN_TIME | DCDC_MODULE_TYPE),       //��ȡģ������ʱ���Ч�ܴ���ʱ��
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_SET_VOLT | DCDC_MODULE_TYPE),       //��ȡģ���ѹ�趨��Ϣ
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_SET_THR | DCDC_MODULE_TYPE),        //��ȡģ�������С�趨��Ϣ
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_ON_OFF | DCDC_MODULE_TYPE),         //��ȡģ�鿪�ػ�������Ϣ
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_MODEL1 | DCDC_MODULE_TYPE),         //��ȡģ���ͺ�1
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_MODEL2 | DCDC_MODULE_TYPE),         //��ȡģ���ͺ�2
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_BARCODE1 | DCDC_MODULE_TYPE),       //��ȡģ�����к�1
	(ID_DCDC_MODULE_TYPE | DCDC_MSG_CSU | DCDC_INFO_GET_BARCODE2 | DCDC_MODULE_TYPE),       //��ȡģ�����к�2
};

static const U8_T m_u8_dcdc_rated_curr[] = { 5, 10, 20, 30, 40, 50, 60, 80, 100};

static U32_T m_u32_dcdc_fail_time[DCDC_MODULE_MAX];                //ͨ��ʧ�ܼ�ʱ

extern CAN_MSG_T m_t_tx_msg_buf;              //CAN���ͻ�����
extern CAN_MSG_T m_t_rx_msg_buf;              //CAN���ջ�����


/*************************************************************
��������: v_can_dcdc_module_data_handle
��������: ͨ��48Vģ��������ݴ�����
�������: pt_msg -- ָ��CAN��Ϣ֡
�������: ��
����ֵ  ����
**************************************************************/	
static void v_can_dcdc_module_data_handle(CAN_MSG_T *pt_msg)
{
	U8_T u8_index;
	U16_T i, u16_old_state, u16_new_state;
	
	u8_index = (m_t_rx_msg_buf.id & DCDC_ID_MODULE_ADDR_MASK) - 1;
	
	switch (m_t_rx_msg_buf.id & DCDC_ID_INFO_TYPE_MASK)
	{
		case DCDC_INFO_GET_STATE:           //��ȡģ��״̬
			os_mut_wait(g_mut_share_data, 0xFFFF);
			
			u16_old_state = g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state;
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state = 0;    //����״̬�ֽ�
			
			if ((pt_msg->data[0] & 0x02) != 0)      //ģ�鿪�ػ�״̬λ	
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state |= 0x0001;
			
			if ((pt_msg->data[0] & 0x10) != 0)      //ģ�����״̬λ
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state |= 0x0008;
			
			if ((pt_msg->data[2] & 0x40) != 0)      //��������״̬λ
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state |= 0x0004;
			
			u16_new_state = g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].u16_state;
			
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].b_ctl_mode = 0;
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].e_module_state = ((u16_new_state & 0x0001) ? SHUT_DOWN : START_UP);
			if ((u16_new_state & DCDC_MODULE_EXCEPTION_MASK) != 0)
				g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].e_module_state = EXCEPTION;              
			
			os_mut_release(g_mut_share_data);

			v_com1_dcdc_module_send_fault_id(u8_index, u16_old_state, u16_new_state);     //���͸澯ID
				
			break;
			
		case DCDC_INFO_GET_DETECT_DATE:     //��ȡģ�������Ϣ
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].f32_out_curr = ((pt_msg->data[0]<<8) | pt_msg->data[1]) / 10.0;
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].f32_out_volt = ((pt_msg->data[2]<<8) | pt_msg->data[3]) / 10.0;
			
			//����ͨ��ĸ�ߵ�ѹ������
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

		case DCDC_INFO_GET_SET_CURR:        //��ȡģ������͹�ѹ�趨��Ϣ
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_rt_data.t_dcdc_panel.t_dcdc_module[u8_index].f32_curr_percent = ((pt_msg->data[0]<<8) | pt_msg->data[1]) / 10.0 * 100 / 
																			m_u8_dcdc_rated_curr[g_t_share_data.t_sys_cfg.t_dcdc_panel.e_rated_curr];
			os_mut_release(g_mut_share_data);
			break;
		
		case DCDC_INFO_GET_RUN_TIME:        //��ȡģ������ʱ���Ч�ܴ���ʱ��
		case DCDC_INFO_GET_BARCODE1:        //��ȡģ�����к�1
		case DCDC_INFO_GET_BARCODE2:        //��ȡģ�����к�2	 
		case DCDC_INFO_GET_MODEL1:          //��ȡģ���ͺ�1 
		case DCDC_INFO_GET_MODEL2:          //��ȡģ���ͺ�2 
		case DCDC_INFO_GET_SET_VOLT:        //��ȡģ���ѹ�趨��Ϣ
		case DCDC_INFO_GET_SET_THR:         //��ȡģ�������С�趨��Ϣ
		case DCDC_INFO_GET_ON_OFF:          //��ȡģ�鿪�ػ�������Ϣ                           
		case DCDC_INFO_SET_SAVE:            //���ò������洢EEPROM
		case DCDC_INFO_SET_VOLT:            //���ò���������
		case DCDC_INFO_SET_CURR:            //���ò���������
		case DCDC_INFO_SET_THR:             //���ò���������
		case DCDC_INFO_SET_ON_OFF:          //ģ��ػ���λ
		case DCDC_INFO_RECV_ERROR:          //���մ���Ӧ��
		default:
			break;
	}
}

/*************************************************************
��������: v_can_dcdc_module_clear_comm_fault_alm
��������: ����ͨ��48Vģ���������ͨ���жϼ���������Ѿ�����ͨ���жϹ��ϣ���ָ�
�������: u8_addr -- ��ַ
�������: ��
����ֵ  ����
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
��������: v_can_dcdc_module_comm_fault_alm
��������: ͨ��48Vģ������ͨ���жϼ�ʱ������һ����ʱ��δͨ���ϣ���ͨ���жϹ���
�������: ��
�������: ��
����ֵ  ����
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
��������: v_can_dcdc_module_receive_handle
��������: ͨ��48Vģ����մ�����
�������: ��
          ��
�������: ��
����ֵ  ����
**************************************************************/
void v_can_dcdc_module_receive_handle(void)
{
	U8_T u8_recv_addr, u8_module_num;
	
	if ((m_t_rx_msg_buf.id & DCDC_ID_MODULE_TYPE_MASK) != DCDC_MODULE_TYPE)
		return;
	
	//�˲��ַ�Ƿ�����Ч
	os_mut_wait(g_mut_share_data, 0xFFFF);
	u8_module_num = g_t_share_data.t_sys_cfg.t_dcdc_panel.u8_dcdc_module_num;
	os_mut_release(g_mut_share_data);
	
	u8_recv_addr = (m_t_rx_msg_buf.id & DCDC_ID_MODULE_ADDR_MASK);
	if ((u8_recv_addr == 0) || (u8_recv_addr > u8_module_num))  //������յ��ĵ�ַ��Ч���򶪵����ݼ�������
		return;
		
	if (m_t_rx_msg_buf.type == 1)    //Զ��֡��Ч���˳�
		return;

	switch (m_t_rx_msg_buf.id & DCDC_ID_MSG_TYPE_MASK)
	{
		case DCDC_MSG_MODULE:                //ģ��ش��ȡ����
			v_can_dcdc_module_data_handle(&m_t_rx_msg_buf);

			//���ͨ���жϼ������ָ�ͨ���жϸ澯
			v_can_dcdc_module_clear_comm_fault_alm(u8_recv_addr);
			break;
						
		case DCDC_MSG_LONGACK:               //������Ӧ��
			//���ͨ���жϼ������ָ�ͨ���жϸ澯
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
��������: v_can_dcdc_module_send_config
��������: �㲥�·�ͨ��48Vģ���������ݴ����������浽EEPROM
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_can_dcdc_module_send_config(void)
{
	U16_T u16_data;
	
	//�趨���ߵ�ѹ
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
	
	//�趨���ߵ���������ģʽ��ĸ�߹�ѹ����ѹ��Сֵ
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
	
	//�趨��ѹ���ֵ��������Сֵ���������ֵ
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
��������: v_can_dcdc_module_send_ctl
��������: �㲥�·�ͨ��48Vģ��������ݴ������������浽EEPROM
�������: u8_cmd_index -- DCDC_SET_VOLT_CMD: �趨���Ƶ�ѹ����
                          DCDC_SET_CURR_CMD: �趨��������
                          DCDC_SET_THR_CMD:  �趨��ѹ���������ֵ/��Сֵ
�������: ��
����ֵ  ����
**************************************************************/
void v_can_dcdc_module_send_ctl(U8_T u8_cmd_index)
{
	U16_T u16_data;
	
	if (u8_cmd_index == DCDC_SET_VOLT_CMD)  	 //�趨���Ƶ�ѹ
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
	
	else if (u8_cmd_index == DCDC_SET_CURR_CMD)    //�趨����ֵ������ģʽ���������ơ�ĸ�߹�ѹ
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
	
	else if (u8_cmd_index == DCDC_SET_THR_CMD)    //�趨��ѹ��Сֵ����ѹ���ֵ��������Сֵ���������ֵ
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
