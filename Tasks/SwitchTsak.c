/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����switch.h
��    ����1.00
�������ڣ�2012-05-29
��    �ߣ�Ф��
��������������������

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	Ф��    2012-05-29    1.00     ����
**************************************************************/
#include "../Drivers/Switch.h"
#include "PublicData.h"
#include "ShareDataStruct.h"
#include "FaultId.h"
#include "string.h"


/********************************************
 *  �������ƣ�__task v_switch_switchtask(void)
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ���ܽ���: �Ե�ǰ���������ݽ��з���
*********************************************/
__task void v_switch_switchtask()
{  
	U32_T data, reg, i;
	U8_T u8_alm_new[SWT_BRANCH_MAX];                         //���ظ澯����1���澯��0�����澯
	U8_T u8_alm_old[SWT_BRANCH_MAX];                         //���ظ澯����1���澯��0�����澯
	U8_T u8_swt_cnt, u8_state_cnt;
	SWT_CFG_T *pt_swt_cfg = &(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt);

	if (g_u8_product_type == PRODUCT_TYPE_SC12)
		u8_swt_cnt = 4;
	else
		u8_swt_cnt = SWT_BRANCH_MAX;
		
	os_mut_wait(g_mut_share_data, 0xFFFF);
	if (pt_swt_cfg->u8_state_cnt > u8_swt_cnt)
		pt_swt_cfg->u8_state_cnt = u8_swt_cnt;
	u8_state_cnt = pt_swt_cfg->u8_state_cnt;
	os_mut_release(g_mut_share_data);

	while(1)
	{
		os_evt_set(SWT_SAMPLE_FEED_DOG, g_tid_wdt);             //����ι���¼���־

		data=switch_switch_data();
		os_dly_wait(2);		//�����ӳٶ೤ʱ���⣬�ڴ�����
		reg=switch_switch_data();
		reg=reg&data;	  //������Ϣ

		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		//���ݽ������ͽ��д���
		for (i=0; i<u8_swt_cnt; i++)
		{
			if ((reg & (1<<i)) != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[i] = 1;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[i] = 0;

			if (pt_swt_cfg->t_swt_item[i].u8_join_way == 0)   //����
			{
				if ((reg & (1<<i)) != 0)
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 1;
				else
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 0;
			}
			else                                              //����
			{
				if ((reg & (1<<i)) != 0)
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 0;
				else
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 1;
			}
			
		}
		
		//����״̬��
		for (i=0; i<u8_state_cnt; i++)
		{
			g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i];
		}
		
		//����澯��	
		for (i=0; i<u8_swt_cnt-u8_state_cnt; i++)
		{
			u8_alm_old[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_alm[i];
			g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_alm[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i+u8_state_cnt];
			u8_alm_new[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_alm[i];
		}
		
		os_mut_release(g_mut_share_data);									
				
		//�澯�����͸澯ID
		for (i=0; i<u8_swt_cnt-u8_state_cnt; i++)
		{
			if ((u8_alm_old[i] == 0) && (u8_alm_new[i] != 0))
				v_fauid_send_fault_id_occur((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+i);
			else if ((u8_alm_old[i] != 0) && (u8_alm_new[i] == 0))
				v_fauid_send_fault_id_resume((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+i);
		}
		
		os_dly_wait(10);		 
	}    		 
}


