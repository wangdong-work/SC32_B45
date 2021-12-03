/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Log.c
��    ����1.00
�������ڣ�2012-06-02
��    �ߣ�������
�����������¼���¼�����ID�����ļ�


�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-06-02  1.00     ����
**************************************************************/

#include <string.h>
#include "Log.h"
#include "../Drivers/Dataflash.h"



OS_MUT g_mut_log;                              //��ȡlog���ݵĻ�����
static U8_T m_u8_log_change = LOG_NO_CHANGE;   //log��¼�Ƿ񱻸ı��ˣ��˱����ɸı�log��������λ������ʾ��������


/*************************************************************
��������: v_log_mut_init		           				
��������: ��ʼ���¼���¼������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_mut_init(void)
{
	os_mut_init(g_mut_log);           //��ʼ��������
}


/*************************************************************
��������: v_log_save_record		           				
��������: �����¼���¼��λdataflash						
�������: pt_log_data -- Ҫ������¼���¼        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_save_record(LOG_DATA_T *pt_log_data)
{
	U8_T u8_head[6];
	U16_T u16_cnt, u16_index;
	U32_T u32_addr;

	os_mut_wait(g_mut_log, 0xFFFF);                 //��д�¼���¼��Ҫ���л�����

	s32_flash_dataflash_read(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	
	u16_cnt = ((u8_head[2]<<8) | u8_head[3]);
	u16_index = ((u8_head[4]<<8) | u8_head[5]);

	if ((u8_head[0] == 0x55) && (u8_head[1] == 0xAA)
		&& (u16_cnt <= LOG_MAX_CNT) && (u16_index < LOG_MAX_CNT))
	{
		if (u16_cnt < LOG_MAX_CNT)
			u16_cnt++;

		u32_addr = (DATAFLASH_RECORD_ADDR + sizeof(u8_head) + u16_index * sizeof(LOG_DATA_T));
		u16_index++;
		u16_index %= LOG_MAX_CNT;

		u8_head[2] = (U8_T)(u16_cnt>>8);
		u8_head[3] = (U8_T)u16_cnt;
		u8_head[4] = (U8_T)(u16_index>>8);
		u8_head[5] = (U8_T)u16_index;
	}
	else
	{
		u8_head[0] = 0x55;
		u8_head[1] = 0xAA;
		u8_head[2] = 0;
		u8_head[3] = 1;
		u8_head[4] = 0;
		u8_head[5] = 1;
		u32_addr = DATAFLASH_RECORD_ADDR + sizeof(u8_head);
	}

	s32_flash_dataflash_write(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	s32_flash_dataflash_write(u32_addr, (U8_T *)pt_log_data, sizeof(LOG_DATA_T));

	os_mut_release(g_mut_log);               //�ͷŻ�����
}

/*************************************************************
��������: v_log_clear_record		           				
��������: ����¼���¼						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_clear_record(void)
{
	U8_T u8_head[6];

	os_mut_wait(g_mut_log, 0xFFFF);                 //��д�¼���¼��Ҫ���л�����

	u8_head[0] = 0x55;
	u8_head[1] = 0xAA;
	u8_head[2] = 0;
	u8_head[3] = 0;
	u8_head[4] = 0;
	u8_head[5] = 0;

	s32_flash_dataflash_write(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));

	os_mut_release(g_mut_log);                      //�ͷŻ�����

}

/*************************************************************
��������: u8_log_read_log_state		           				
��������: ��ȡ�¼���¼��״̬���˺�������ʾ������ã�������ȡ��¼�Ƿ񼺾������ı�						
�������: ��        		   				
�������: ��
����ֵ  ����ʷ����״̬														   				
**************************************************************/
U8_T u8_log_read_log_state(void)
{
	U8_T state;

	os_mut_wait(g_mut_log, 0xFFFF);                 //��д�¼���¼��Ҫ���л�����
	state = m_u8_log_change;
	m_u8_log_change = LOG_NO_CHANGE;
	os_mut_release(g_mut_log);                      //�ͷŻ�����

	return state;
}


/*************************************************************
��������: v_log_set_log_state		           				
��������: ���ü�¼��״̬Ϊ���ı䣬�ı��¼��������ô˺���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_set_log_state(void)
{
	os_mut_wait(g_mut_log, 0xFFFF);                 //��д�¼���¼��Ҫ���л�����
	m_u8_log_change = LOG_CHANGE;
	os_mut_release(g_mut_log);                      //�ͷŻ�����
}


/*************************************************************
��������: u16_log_read_log_cnt		           				
��������: ��ȡ�¼���¼������������ǰ��Ҫ��ȡ������g_mut_log						
�������: ��        		   				
�������: ��
����ֵ  ����ʷ��������														   				
**************************************************************/
U16_T u16_log_read_log_cnt(void)
{
	U8_T u8_head[6];
	U16_T u16_cnt, u16_index;

	memset(u8_head, 0, sizeof(u8_head));
	s32_flash_dataflash_read(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	u16_cnt = ((u8_head[2]<<8) | u8_head[3]);
	u16_index = ((u8_head[4]<<8) | u8_head[5]);

	if ((u8_head[0] == 0x55) && (u8_head[1] == 0xAA)
		&& (u16_cnt <= LOG_MAX_CNT) && (u16_index < LOG_MAX_CNT))
	{
		return u16_cnt;
	}

	return 0;
}

/*************************************************************
��������: v_log_read_log_record		           				
��������: ��dataflash��ȡ�¼���¼���˺��������u16_log_index���з�Χ�жϣ�
          ���ڵ����߽����жϣ�������ǰ��Ҫ��ȡ������g_mut_log						
�������: u16_log_index -- ��¼��������0��ʼ��С���¼���¼����        		   				
�������: pt_log        -- ���淵�ص��¼���¼
����ֵ  ����														   				
**************************************************************/
void v_log_read_log_record(LOG_DATA_T *pt_log, U16_T u16_log_index)
{
	U8_T u8_head[6];
	U16_T u16_index, u16_read_index;
	U32_T u32_addr;

	memset(u8_head, 0, sizeof(u8_head));
	s32_flash_dataflash_read(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	u16_index = ((u8_head[4]<<8) | u8_head[5]);

	u16_read_index = ((LOG_MAX_CNT + u16_index - u16_log_index -1) % LOG_MAX_CNT);
	u32_addr = (DATAFLASH_RECORD_ADDR + sizeof(u8_head) + u16_read_index*sizeof(LOG_DATA_T));
	s32_flash_dataflash_read(u32_addr, (U8_T *)pt_log, sizeof(LOG_DATA_T));
}
