/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����FetchFlash.c
��    ����1.00
�������ڣ�2012-05-19
��    �ߣ�������
������������dataflash��ȡ����

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-05-19  1.00     ����
**************************************************************/

//FetchFlash.c�ļ�������дΪfetch
//coefficientX��дΪcoeff

#include <string.h>
#include "Type.h"
#include "Crc.h"
#include "PublicData.h"
#include "FaultId.h"
#include "FetchFlash.h"
#include "Fault.h"
#include "../Drivers/Dataflash.h"

typedef enum {
	AC_ADJUST,    //����У׼
	DC_ADJUST     //ֱ��У׼
}ADJUST_TYPE_E;


/*************************************************************
��������: v_fetch_save_cfg_data		           				
��������: �����������ݵ�dataflash���������ݣ������ʽ���ļ���IARM-SC32 DATAFLASH�ռ��ַ����						
�������: ��         		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_fetch_save_cfg_data(void)
{
	U8_T u8_head[8];
	U16_T u16_len, u16_crc, i, j;
	U32_T u32_addr[3] = { DATAFLASH_CFG_DATA1_ADDR, DATAFLASH_CFG_DATA2_ADDR, DATAFLASH_CFG_DATA3_ADDR };

	u16_len = sizeof(SYS_CFG_T);

	u8_head[0] = 0x55;
	u8_head[1] = 0xAA;
	u8_head[2] = CFG_DATA_VERSION_H;
	u8_head[3] = CFG_DATA_VERSION_L;
	u8_head[4] = (U8_T)(u16_len >> 8);
	u8_head[5] = (U8_T)u16_len;

	os_mut_wait(g_mut_share_data, 0xFFFF);        //��ȡ������

	u16_crc = u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len);
	u8_head[6] = (U8_T)(u16_crc >> 8);
	u8_head[7] = (U8_T)u16_crc;

	for (i=0; i<3; i++)            //д3������
	{		
		for (j=0; j<3; j++)        //���дʧ�ܣ�������3��
		{
			if ((s32_flash_dataflash_write(u32_addr[i], u8_head, 8) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+8, (U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //�ͷŻ�����
}

/*************************************************************
��������: s32_fetch_read_cfg_data		           				
��������: ��dataflash��ȡ�������ݣ������������ж�ȡ��ȷ�����ݣ�������ȷ��������д����ȷ������					
�������: ��         		   				
�������: ��
����ֵ  ����ȡ�ɹ��򷵻�0��ʧ�ܷ���-1														   				
**************************************************************/
S32_T s32_fetch_read_cfg_data(void)
{
	U8_T u8_head[8];
	U16_T u16_len, i, j, k;
	U32_T u32_addr[3] = { DATAFLASH_CFG_DATA1_ADDR, DATAFLASH_CFG_DATA2_ADDR, DATAFLASH_CFG_DATA3_ADDR };

	u16_len = sizeof(SYS_CFG_T);

	os_mut_wait(g_mut_share_data, 0xFFFF);        //��ȡ������

	for (i=0; i<3; i++)
	{
		if ((s32_flash_dataflash_read(u32_addr[i], u8_head, 8) == 0)
			&& (s32_flash_dataflash_read(u32_addr[i]+8, (U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len) == 0))
		{
			if ((u8_head[0] == 0x55)
				&& (u8_head[1] == 0xAA)
				&&(u8_head[2] == CFG_DATA_VERSION_H)                                                     //�汾��ͬ
				&& (u8_head[3] == CFG_DATA_VERSION_L)
				&& (u16_len == ((u8_head[4]<<8) | u8_head[5]))                                         //������ͬ
				&& (((u8_head[6]<<8) | u8_head[7]) ==  u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len)))  //CRCУ����ȷ
			{
				break;
			}
		}
	}

	if (i >= 3)                    //�����������ݶ�û�ж�ȡ�ɹ������˳�
	{
		os_mut_release(g_mut_share_data);             //�ͷŻ�����
		return -1;
	}
	else if (i == 0)               //��һ�ݾͶ�ȡ�ɹ������˳�
	{
		os_mut_release(g_mut_share_data);             //�ͷŻ�����
		return 0;
	}

	//�����ܵ����˵��������ȡ�ɹ����Ҳ��ǵ�һ�ݶ�ȡ�ɹ�����Ҫ�Զ�ȡʧ�ܵ�������д
	for (j=0; j<i; j++)
	{
		for (k=0; k<3; k++)        //���дʧ�ܣ�������3��
		{
			if ((s32_flash_dataflash_write(u32_addr[j], u8_head, 8) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+8, (U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //�ͷŻ�����

	return 0;
}

/*************************************************************
��������: v_fetch_read_swt_cfg_data
��������: ��dataflash��ȡ������������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_fetch_read_swt_cfg_data(void)
{
	U16_T u16_len, i, j;

	os_mut_wait(g_mut_share_data, 0xFFFF);        //��ȡ������

	u16_len = sizeof(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item);

	//�����ȡʧ�ܣ�����3��
	for (i=0; i<3; i++)
	{
		if (s32_flash_dataflash_read(DATAFLASH_SWT_ADDR, (U8_T *)(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item), u16_len) == 0)
		{
			//ͳ��״̬��������
			for (j=0; j<SWT_BRANCH_MAX; j++)
			{
				if (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[j].u8_meas_type == 0)
					break;   //��⵽��һ���澯���˳�
			}
			
			g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt = j;
			
			//����״̬����
			for ( ; j<SWT_BRANCH_MAX; j++)
			{
				v_fault_set_swt_fault_type(j-g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt,
											g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[j].u8_fault_type,
											g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[j].u8_fault_type);
			}
			
			break;
		}
	}
	
	if (i >= 3)                    //���ζ�û�ж�ȡ�ɹ���������˳�
	{
		memset(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item, 0, u16_len);
	}

	os_mut_release(g_mut_share_data);             //�ͷŻ�����
}

/*************************************************************
��������: v_fetch_save_adjust_coeff		           				
��������: ����У׼ϵ����dataflash���������ݣ������ʽ���ļ���IARM-SC32 DATAFLASH�ռ��ַ����						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_fetch_save_adjust_coeff(void)
{
	U8_T u8_head[6];
	U16_T u16_len, u16_crc, i, j;
	U32_T u32_addr[3] = { DATAFLASH_ADJUST1_ADDR, DATAFLASH_ADJUST2_ADDR, DATAFLASH_ADJUST3_ADDR };

	u16_len = sizeof(COEFF_DATA_T);

	u8_head[0] = 0x55;
	u8_head[1] = 0xAA;
	u8_head[2] = (U8_T)(u16_len >> 8);
	u8_head[3] = (U8_T)u16_len;

	os_mut_wait(g_mut_share_data, 0xFFFF);        //��ȡ������

	u16_crc = u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len);
	u8_head[4] = (U8_T)(u16_crc >> 8);
	u8_head[5] = (U8_T)u16_crc;

	for (i=0; i<3; i++)            //д3������
	{		
		for (j=0; j<3; j++)        //���дʧ�ܣ�������3��
		{
			if ((s32_flash_dataflash_write(u32_addr[i], u8_head, 6) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+6, (U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //�ͷŻ�����
}

/*************************************************************
��������: s32_fetch_read_adjust_coeff		           				
��������: ��dataflash��ȡУ׼ϵ���������������ж�ȡ��ȷ�����ݣ�������ȷ��������д����ȷ������					
�������: ��
����ֵ  ����ȡ�ɹ��򷵻�0��ʧ�ܷ���-1														   				
**************************************************************/
S32_T s32_fetch_read_adjust_coeff(void)
{
	U8_T u8_head[6];
	U16_T u16_len, i, j, k;
	U32_T u32_addr[3] = { DATAFLASH_ADJUST1_ADDR, DATAFLASH_ADJUST2_ADDR, DATAFLASH_ADJUST3_ADDR };

	u16_len = sizeof(COEFF_DATA_T);

	os_mut_wait(g_mut_share_data, 0xFFFF);        //��ȡ������

	for (i=0; i<3; i++)
	{
		if ((s32_flash_dataflash_read(u32_addr[i], u8_head, 6) == 0)
			&& (s32_flash_dataflash_read(u32_addr[i]+6, (U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len) == 0))
		{
			if ((u8_head[0] == 0x55)
				&& (u8_head[1] == 0xAA)
				&& (u16_len == ((u8_head[2]<<8) | u8_head[3]))                                         //������ͬ
				&& (((u8_head[4]<<8) | u8_head[5]) ==  u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len)))  //CRCУ����ȷ
			{
				break;
			}
		}
	}

	if (i >= 3)                    //�����������ݶ�û�ж�ȡ�ɹ������˳�
	{
		os_mut_release(g_mut_share_data);             //�ͷŻ�����
		return -1;
	}
	else if (i == 0)               //��һ�ݾͶ�ȡ�ɹ������˳�
	{
		os_mut_release(g_mut_share_data);             //�ͷŻ�����
		return 0;
	}

	//�����ܵ����˵��������ȡ�ɹ����Ҳ��ǵ�һ�ݶ�ȡ�ɹ�����Ҫ�Զ�ȡʧ�ܵ�������д
	for (j=0; j<i; j++)
	{
		for (k=0; k<3; k++)        //���дʧ�ܣ�������3��
		{
			if ((s32_flash_dataflash_write(u32_addr[j], u8_head, 6) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+6, (U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //�ͷŻ�����

	return 0;
}
