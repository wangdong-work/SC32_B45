/************************************************************
Copyright (C), ����Ӣ����Ƽ��������޹�˾, ����һ��Ȩ��
�� �� ����backstage.c
��    ����1.00
�������ڣ�2012-06-14
��    �ߣ�������
������������̨ͨ��ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-06-14  1.00     ����
***********************************************************/

#include <rtl.h>
#include <string.h>
#include <math.h>

#include "Backstage.h"
#include "BSModbus.h"
#include "BSCdt.h"
#include "BSInternal.h"
#include "FetchFlash.h"
#include "PublicData.h"

#include "../Drivers/Delay.h"
#include "../Drivers/uart_device.h"
#include "../Drivers/Dataflash.h"

//#define DEBUG

#ifdef DEBUG
	#define DPRINT(fmt, args...) u32_usb_debug_print(fmt, ##__VA_ARGS__)
#else
	#define DPRINT(fmt, args...)
#endif


#define BS_TYPE_U8  0
#define BS_TYPE_U16 1
#define BS_TYPE_F32 2



U8_T                        g_u8_addr;                   //��̨ͨ�ű�����ַ����
COM_BAUD_E                  g_e_baud;                    //��̨ͨ�Ų����ʱ���
static COM_PARITY_E         m_e_parity;                  //��̨ͨ����żУ�鱸��
static BACKSTAGE_PROTROL_E  m_e_protrol;                 //��̨ͨ��Э�鱸��


U8_T g_u8_bs_send_buf[BS_SEND_BUF_SIZE];                 //���ͻ�����
U8_T g_u8_bs_recv_buf[BS_RECV_BUF_SIZE];                 //���ջ�����
U8_T g_u8_bs_recv_len;                                   //���ջ����������ݵĳ���


S16_T g_s16_bs_modbus_yc[BS_YC_SIZE];                    //MODBUSң��������
U16_T g_u16_bs_mdobus_yc_size;                           //MODBUSң��������ʵ�ʴ�С
U16_T g_u16_bs_cdt_yc[BS_YC_SIZE];                       //CDTң��������
U16_T g_u16_bs_cdt_yc_size;                              //CDTң��������ʵ�ʴ�С
S16_T g_s16_bs_modbus_yc_feeder[BS_YC_FEEDER_SIZE];      //MODBUS֧·ң��������

U16_T g_u16_bs_yx[BS_YX_SIZE/16+1];                      //ң��������
U16_T g_u16_bs_yx_size;                                  //ң��������ʵ�ʴ�С
U32_T g_u32_bs_yx_max_addr;                              //ң������ַ
U16_T g_u16_bs_yx_feeder[BS_YX_FEEDER_SIZE/16+1];        //֧·ң��������

U16_T g_u16_bs_yk[BS_YK_SIZE];                           //ң��������
U16_T g_u16_bs_yk_size;                                  //ң��������ʵ�ʴ�С

U16_T g_u16_bs_yt[BS_YT_SIZE];                           //ң��������
U16_T g_u16_bs_yt_size;                                  //ң��������ʵ�ʴ�С


#define BS_ITEM_BUF_SIZE 32                              //���������С
static BS_ITEM_T m_t_item_buf[BS_ITEM_BUF_SIZE];         //���������



/*************************************************************
��������: u32_bs_calculate_time		           				
��������: ���㴮����byte_cnt���ֽ�����ʱ��						
�������: byte_cnt -- �ֽ���
          baud     -- ������        		   				
�������: ��
����ֵ  ����������ʱ�䣬��λΪms														   				
**************************************************************/
U32_T u32_bs_calculate_time(U32_T byte_cnt, COM_BAUD_E baud)
{
	int bytes;
	
	switch (baud) {
		case COM_BAUD_19200:
			bytes = 19200 / 11;
			break;
			
		case COM_BAUD_9600:
			bytes = 9600 / 11;
			break;
			
		case COM_BAUD_4800:
			bytes = 4800 / 11;
			break;
			
		case COM_BAUD_2400:
			bytes = 2400 / 11;
			break;
			
		case COM_BAUD_1200:
			bytes = 1200 / 11;
			break;
			
		default:
			bytes = 9600 / 11;
			break;
	}

	return ((byte_cnt*1000)/bytes);
}

/*************************************************************
��������: v_bs_send_data		           				
��������: ͨ�����ڷ�������						
�������: pu8_buf -- ָ��Ҫ���͵�������
          len     -- Ҫ���͵��ֽ���        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_bs_send_data(U8_T *pu8_buf, U32_T len)
{
	U32_T space, tx_len = 0;
	
	while (tx_len < len)
	{
		space = Uart_get_txbuff_num(BS_COM_PORT_NUM);
		if (space > 0)
		{
			if (len - tx_len > space)
			{
				Uart_send(BS_COM_PORT_NUM, pu8_buf+tx_len, space);
				tx_len += space;
			}
			else
			{
				Uart_send(BS_COM_PORT_NUM, pu8_buf+tx_len, len - tx_len);
				tx_len += (len - tx_len);
			}
		}
		
		if (tx_len < len)
		{
			v_delay_mdelay(u32_bs_calculate_time(UART_TX_Q_SIZE, g_e_baud));
		}
		
		os_evt_set(BACKSTAGE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
	}
}

/*************************************************************
��������: v_bs_set_com_baud		           				
��������: ���ô��ڲ�����						
�������: e_baud -- ������       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_bs_set_com_baud(COM_BAUD_E e_baud)
{
	switch (e_baud)
	{
		case COM_BAUD_1200:
			Set_baudrate(BS_COM_PORT_NUM, 1200);
			break;
			
		case COM_BAUD_2400:
			Set_baudrate(BS_COM_PORT_NUM, 2400);
			break;
			
		case COM_BAUD_4800:
			Set_baudrate(BS_COM_PORT_NUM, 4800);
			break;
		
		case COM_BAUD_9600:
			Set_baudrate(BS_COM_PORT_NUM, 9600);
			break;
			
		case COM_BAUD_19200:
			Set_baudrate(BS_COM_PORT_NUM, 19200);
			break;
			
		default:
			break;
	}
}

/*************************************************************
��������: v_bs_set_com_parity		           				
��������: ���ô�����żУ��						
�������: e_parity -- ��żУ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_bs_set_com_parity(COM_PARITY_E e_parity)
{
	switch (e_parity)
	{
		case ODD_PARITY:
			Set_frame(BS_COM_PORT_NUM, 0x0B);
			break;
			
		case EVEN_PARITY:
			Set_frame(BS_COM_PORT_NUM, 0x1B);
			break;
			
		case NONE_PARITY:
			Set_frame(BS_COM_PORT_NUM, 0x03);
			break;
			
		default:
			break;
	}
}


/*************************************************************
��������: v_bs_sync_data		           				
��������: ͬ����̨�������͹��������������ô˺���ǰ��Ҫ��ȡ������g_mut_share_data						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_bs_sync_data(void)
{
	U32_T i, j, k, l, m;
	S16_T s16_cdt_val;
	
	//ң������ͬ��
	g_u16_bs_mdobus_yc_size = g_u16_bs_cdt_yc_size = 0;
	for (i=0; i<sizeof(g_t_yc_assemble) / sizeof(g_t_yc_assemble[0]); i++)
	{
		if ((i % BS_ITEM_BUF_SIZE) == 0)
			s32_flash_dataflash_read(DATAFLASH_YC_DATA_ADDR+BS_ITEM_SIZE*i, (U8_T *)m_t_item_buf, BS_ITEM_BUF_SIZE*BS_ITEM_SIZE);
		
		//if (m_t_item_buf[i%BS_ITEM_BUF_SIZE].u16_flag == BS_ITEM_UPLOAD)
		{
			g_s16_bs_modbus_yc[g_u16_bs_mdobus_yc_size] = (S16_T)((*(g_t_yc_assemble[i].pf32_val)) * g_t_yc_assemble[i].u16_modbus_coeff);
			
			s16_cdt_val = (S16_T)((*(g_t_yc_assemble[i].pf32_val)) * g_t_yc_assemble[i].u16_cdt_coeff);
			if (s16_cdt_val >= 0)
				s16_cdt_val &= ~(1<<11);
			else
				s16_cdt_val |= (1<<11);	
			s16_cdt_val &= 0x0FFF;
			g_u16_bs_cdt_yc[g_u16_bs_cdt_yc_size] = s16_cdt_val;
			
			if (i == BS_CB_POS_TO_GND_VOLT_INDEX)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num == 0)    //û��������������ʱ��ĸ�߾�Ե��ѹֵȡֱ���ɵ�������
				{
					g_s16_bs_modbus_yc[g_u16_bs_mdobus_yc_size] = (S16_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_cb_pos_to_gnd_volt
															 * g_t_yc_assemble[i].u16_modbus_coeff);
			
					s16_cdt_val = (S16_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_cb_pos_to_gnd_volt
										* g_t_yc_assemble[i].u16_cdt_coeff);
					if (s16_cdt_val >= 0)
						s16_cdt_val &= ~(1<<11);
					else
						s16_cdt_val |= (1<<11);	
					s16_cdt_val &= 0x0FFF;
					g_u16_bs_cdt_yc[g_u16_bs_cdt_yc_size] = s16_cdt_val;
					
				}
			}
			else if (i == BS_PB_POS_TO_GND_VOLT_INDEX)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num == 0)    //û��������������ʱ��ĸ�߾�Ե��ѹֵȡֱ���ɵ�������
				{
					g_s16_bs_modbus_yc[g_u16_bs_mdobus_yc_size] = (S16_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_pb_pos_to_gnd_volt
															   * g_t_yc_assemble[i].u16_modbus_coeff);
															 
					s16_cdt_val = (S16_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_pb_pos_to_gnd_volt
							* g_t_yc_assemble[i].u16_cdt_coeff);
					if (s16_cdt_val >= 0)
						s16_cdt_val &= ~(1<<11);
					else
						s16_cdt_val |= (1<<11);	
					s16_cdt_val &= 0x0FFF;
					g_u16_bs_cdt_yc[g_u16_bs_cdt_yc_size] = s16_cdt_val;	
				}
			}
			else if (i == BS_BUS_NEG_TO_GND_VOLT_INDEX)
			{
				if (g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num == 0)    //û��������������ʱ��ĸ�߾�Ե��ѹֵȡֱ���ɵ�������
				{
					g_s16_bs_modbus_yc[g_u16_bs_mdobus_yc_size] = (S16_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt
															 * g_t_yc_assemble[i].u16_modbus_coeff);
			
					s16_cdt_val = (S16_T)(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt
										* g_t_yc_assemble[i].u16_cdt_coeff);
					if (s16_cdt_val >= 0)
						s16_cdt_val &= ~(1<<11);
					else
						s16_cdt_val |= (1<<11);	
					s16_cdt_val &= 0x0FFF;
					g_u16_bs_cdt_yc[g_u16_bs_cdt_yc_size] = s16_cdt_val;
				}
			}
			
			g_u16_bs_mdobus_yc_size++;
			g_u16_bs_cdt_yc_size++;
		}
	}
	
	//֧·ң������ͬ��
	l = 0;
	memset(g_s16_bs_modbus_yc_feeder, 0, sizeof(g_s16_bs_modbus_yc_feeder));
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
	{
		m = 0;
		
		for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num; j++)
		{
			for (k=0; k<g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_insu_feeder_num; k++)
			{
				g_s16_bs_modbus_yc_feeder[l++] = (g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[m++].f32_res + 0.05) * 10;
			}
		}
	}

	
	//ң������ͬ��
	g_u32_bs_yx_max_addr = g_u16_bs_yx_size = 0;
	memset(g_u16_bs_yx, 0, sizeof(g_u16_bs_yx));
	for (i=0; i<sizeof(g_t_yx_assemble) / sizeof(g_t_yx_assemble[0]); i++)
	{
		if ((i % BS_ITEM_BUF_SIZE) == 0)
			s32_flash_dataflash_read(DATAFLASH_YX_DATA_ADDR+BS_ITEM_SIZE*i, (U8_T *)m_t_item_buf, BS_ITEM_BUF_SIZE*BS_ITEM_SIZE);
		
		//if (m_t_item_buf[i%BS_ITEM_BUF_SIZE].u16_flag == BS_ITEM_UPLOAD)
		{
			if (g_t_yx_assemble[i].u8_type == BS_TYPE_U8)
			{
				if (((*(U8_T *)(g_t_yx_assemble[i].pv_val)) & (U8_T)(g_t_yx_assemble[i].u16_mask)) != 0)
				{
					g_u16_bs_yx[g_u32_bs_yx_max_addr/16] |= (1 << (g_u32_bs_yx_max_addr%16));
				} 
			}
			else if (g_t_yx_assemble[i].u8_type == BS_TYPE_U16)
			{
				if (((*(U16_T *)(g_t_yx_assemble[i].pv_val)) & g_t_yx_assemble[i].u16_mask) != 0)
				{
					g_u16_bs_yx[g_u32_bs_yx_max_addr/16] |= (1 << (g_u32_bs_yx_max_addr%16));
				} 
			}
			
			g_u32_bs_yx_max_addr++;
		}
	}
	
	g_u16_bs_yx_size = (g_u32_bs_yx_max_addr + 15) / 16;
	
	//֧·��բң������ͬ��
	memset(g_u16_bs_yx_feeder, 0, sizeof(g_u16_bs_yx_feeder));
	l = 0;
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
	{
		m = 0;
		
		for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num; j++)
		{
			for (k=0; k<g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_alarm_feeder_num; k++)
			{
				if (g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[m++].u8_alarm == 0x02)
					g_u16_bs_yx_feeder[l/16] |= (1 << (l%16));
				
				l++;
			}
		}
	}
	
	//֧·״̬ң������ͬ��
	l = 0;
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
	{
		m = 0;
		
		for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num; j++)
		{
			for (k=0; k<g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_state_feeder_num; k++)
			{
				if (g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[m++].u8_state == 0x02)
					g_u16_bs_yx_feeder[BS_YX_FEEDER_STATE_INDEX+l/16] |= (1 << (l%16));
				
				l++;
			}
		}
	}
	
	//֧·��Եң������ͬ��
	l = 0;
	for (i=0; i<g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num; i++)
	{
		m = 0;
		
		for (j=0; j<g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num; j++)
		{
			for (k=0; k<g_t_share_data.t_sys_cfg.t_feeder_panel[i].t_feeder_module[j].u8_insu_feeder_num; k++)
			{
				if (g_t_share_data.t_rt_data.t_feeder_panel[i].t_feeder[m++].u8_insu_state == 0x02)
					g_u16_bs_yx_feeder[BS_YX_FEEDER_INSU_INDEX+l/16] |= (1 << (l%16));
				
				l++;
			}
		}
	}
	
	
	//ң������ͬ��
	g_u16_bs_yk_size = 0;
	for(i=0; i<sizeof(g_t_yk_assemble) / sizeof(g_t_yk_assemble[0]); i++)
	{
		if ((i % BS_ITEM_BUF_SIZE) == 0)
			s32_flash_dataflash_read(DATAFLASH_YK_DATA_ADDR+BS_ITEM_SIZE*i, (U8_T *)m_t_item_buf, BS_ITEM_BUF_SIZE*BS_ITEM_SIZE);
		
		//if (m_t_item_buf[i%BS_ITEM_BUF_SIZE].u16_flag == BS_ITEM_UPLOAD)
		{
			if (g_t_yk_assemble[i].u8_type == BS_TYPE_U8)
			{
				g_u16_bs_yk[g_u16_bs_yk_size] = (U16_T)(*(U8_T *)(g_t_yk_assemble[i].pv_val));
			}
			else if (g_t_yk_assemble[i].u8_type == BS_TYPE_U16)
			{
				g_u16_bs_yk[g_u16_bs_yk_size] = (U16_T)(*(U16_T *)(g_t_yk_assemble[i].pv_val));
			}
			
			g_u16_bs_yk_size++;
		}
	}
	
	
	//ң������ͬ��
	g_u16_bs_yt_size = 0;
	for (i=0; i<sizeof(g_t_yt_assemble) / sizeof(g_t_yt_assemble[0]); i++)
	{
		if ((i % BS_ITEM_BUF_SIZE) == 0)
			s32_flash_dataflash_read(DATAFLASH_YT_DATA_ADDR+BS_ITEM_SIZE*i, (U8_T *)m_t_item_buf, BS_ITEM_BUF_SIZE*BS_ITEM_SIZE);
		
		//if (m_t_item_buf[i%BS_ITEM_BUF_SIZE].u16_flag == BS_ITEM_UPLOAD)
		{
			g_u16_bs_yt[g_u16_bs_yt_size] = (U16_T)((*(g_t_yt_assemble[i].pf32_val)) * g_t_yt_assemble[i].u16_modbus_coeff);
			g_u16_bs_yt_size++;
		}
	}
}


/*************************************************************
��������: s32_bs_yk_handle		           				
��������: ��̨ң�ش�����						
�������: addr -- ң�ص�ַ
          val  -- Ҫ���õ���ֵ       		   				
�������: ��
����ֵ  ��0:���óɹ���-1������ʧ��														   				
**************************************************************/
S32_T s32_bs_yk_handle(U16_T addr, U16_T val)
{
	if (val <= 1)
	{
		os_mut_wait(g_mut_share_data, 0xFFFF);

		if (g_t_yk_assemble[addr].u8_type == BS_TYPE_U8)
		{
			if (*(U8_T *)(g_t_yk_assemble[addr].pv_val) != (U8_T)val)
			{
				*(U8_T *)(g_t_yk_assemble[addr].pv_val) = (U8_T)val;
				v_fetch_save_cfg_data();      //�����������ݵ�dataflash
			}
		}
		else if (g_t_yk_assemble[addr].u8_type == BS_TYPE_U16)
		{
			if (*(U16_T *)(g_t_yk_assemble[addr].pv_val) != (U16_T)val)
			{
				*(U16_T *)(g_t_yk_assemble[addr].pv_val) = (U16_T)val;
				v_fetch_save_cfg_data();      //�����������ݵ�dataflash
			}
		}

		os_mut_release(g_mut_share_data);

		return 0;
	}

	return -1;
}

/*************************************************************
��������: s32_bs_yt_handle		           				
��������: ��̨ң��������						
�������: addr -- ң����ַ
          val  -- Ҫ���õ���ֵ       		   				
�������: ��
����ֵ  ��0:���óɹ���-1������ʧ��														   				
**************************************************************/
S32_T s32_bs_yt_handle(U16_T addr, U16_T val)
{
	F32_T f32_volt = ((F32_T)val) / g_t_yt_assemble[addr].u16_modbus_coeff;    //ֻ��Modbusң����CDTû��ң��
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	
	if ((f32_volt > g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit)
		&& (f32_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit))
	{
		if (fabs(*(g_t_yt_assemble[addr].pf32_val) - f32_volt) > 0.0001)
		{
			*(g_t_yt_assemble[addr].pf32_val) = f32_volt;
			v_fetch_save_cfg_data();      //�����������ݵ�dataflash
		}
		
		os_mut_release(g_mut_share_data);

		return 0;
	}

	os_mut_release(g_mut_share_data);
	return -1;
}


/*************************************************************
��������: v_backstage_task		           				
��������: ��̨ͨ������������ȫ�ֹ�������������ȡ��̨�������������̨��ص�ͨ��						
�������: ��       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
__task void v_bs_backstage_task(void)
{
	U32_T init_flag = 1;
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	g_e_baud = g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_baud;
	m_e_parity = g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_parity;
	g_u8_addr = g_t_share_data.t_sys_cfg.t_sys_param.u8_local_addr;
	m_e_protrol = g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_protrol;
	os_mut_release(g_mut_share_data);
	
	v_bs_set_com_baud(g_e_baud);
	v_bs_set_com_parity(m_e_parity);

	while (1)
	{
		os_evt_set(BACKSTAGE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		if (g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_baud != g_e_baud)
		{
			g_e_baud = g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_baud;
			v_bs_set_com_baud(g_e_baud);
			init_flag = 1;
		}
	
		if (g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_parity != m_e_parity)
		{
			m_e_parity = g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_parity;
			v_bs_set_com_parity(m_e_parity);
			init_flag = 1;
		}
	
		if (g_t_share_data.t_sys_cfg.t_sys_param.u8_local_addr != g_u8_addr)
		{
			g_u8_addr = g_t_share_data.t_sys_cfg.t_sys_param.u8_local_addr;
			init_flag = 1;
		}
		
		if (g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_protrol != m_e_protrol)
		{
			m_e_protrol = g_t_share_data.t_sys_cfg.t_sys_param.e_backstage_protrol;
			init_flag = 1;
		}
	   	
//		if (m_e_protrol != BS_ADJUST)
			v_bs_sync_data();    //ͬ��������

		os_mut_release(g_mut_share_data);
	
		if (init_flag == 1)
		{
			memset(g_u8_bs_recv_buf, 0, sizeof(g_u8_bs_recv_buf));
			g_u8_bs_recv_len = 0;
			init_flag = 0;
		}
	
		if (m_e_protrol == BS_MODBUS)
		{
			v_modbus_run();
			os_dly_wait(1);
		}
		else if (m_e_protrol == BS_CDT)
		{
			memset(g_u8_bs_recv_buf, 0, sizeof(g_u8_bs_recv_buf));
			g_u8_bs_recv_len = 0;
			v_cdt_run();
		}
		else if (m_e_protrol == BS_ADJUST)
		{
			v_modbus_run();	//v_internal_run();
			os_dly_wait(1);
		}
	}
}
