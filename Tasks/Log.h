/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Log.h
��    ����1.00
�������ڣ�2012-06-02
��    �ߣ�������
�����������¼���¼�����ID����ͷ�ļ�


�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-06-02  1.00     ����
**************************************************************/


#ifndef __LOG_H_
#define __LOG_H_


#include <rtl.h>
#include "Type.h"


#define LOG_MAX_CNT                 500        //�¼���¼���������    
#define LOG_NO_CHANGE               0          //�¼���¼û�б仯
#define LOG_CHANGE                  1          //�¼���¼���仯

/* �¼������ */
#define LOG_ID_GROUP_MASK           0xFC00    //�¼���������
#define LOG_ID_INDEX_MASK           0x03FF    //��������������
#define LOG_ID_GROUP_OFFSET         10        //�����ƫ��
#define LOG_ID_BATT_GROUP           0         //��س�ŵ�ת�����

#define	LOG_ID_BATT_TO_AUTO         (0 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�������תΪ�Զ�
#define	LOG_ID_BATT_TO_MANUAL       (1 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�������תΪ�ֶ�
#define	LOG_ID_BATT_AUTO_TO_FLO     (2 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�����Զ�ת����
#define LOG_ID_BATT_AUTO_TO_EQU     (3 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�����Զ�ת����
#define	LOG_ID_BATT_AUTO_TO_DIS     (4 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�����Զ�ת����
#define LOG_ID_BATT_MANUAL_TO_FLO   (5 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�����ֶ�ת����
#define LOG_ID_BATT_MANUAL_TO_EQU   (6 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�����ֶ�ת����
#define	LOG_ID_BATT_MANUAL_TO_DIS   (7 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //һ�����ֶ�ת����
#define LOG_ID_BATT_NUM             8        //��ع����¼���¼����



typedef struct
{
	U16_T u16_log_id;        //�¼�ID

	//����ʱ��
	U8_T  u8_occur_year;     //��
	U8_T  u8_occur_mon;      //��
	U8_T  u8_occur_day;      //��
	U8_T  u8_occur_hour;     //ʱ
	U8_T  u8_occur_min;      //��
	U8_T  u8_occur_sec;      //��
}LOG_DATA_T;


extern OS_MUT g_mut_log;                              //��ȡlog���ݵĻ�����


/*************************************************************
��������: v_log_mut_init		           				
��������: ��ʼ���¼���¼������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_mut_init(void);


/*************************************************************
��������: v_log_save_record		           				
��������: �����¼���¼��λdataflash						
�������: pt_log_data -- Ҫ������¼���¼        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_save_record(LOG_DATA_T *pt_log_data);


/*************************************************************
��������: v_log_clear_record		           				
��������: ����¼���¼						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_clear_record(void);


/*************************************************************
��������: u8_log_read_log_state		           				
��������: ��ȡ�¼���¼��״̬���˺�������ʾ������ã�������ȡ��¼�Ƿ񼺾������ı�						
�������: ��        		   				
�������: ��
����ֵ  ����ʷ����״̬														   				
**************************************************************/
U8_T u8_log_read_log_state(void);


/*************************************************************
��������: v_log_set_log_state		           				
��������: ���ü�¼��״̬Ϊ���ı䣬�ı��¼��������ô˺���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_log_set_log_state(void);


/*************************************************************
��������: u16_log_read_log_cnt		           				
��������: ��ȡ�¼���¼������������ǰ��Ҫ��ȡ������g_mut_log						
�������: ��        		   				
�������: ��
����ֵ  ����ʷ��������														   				
**************************************************************/
U16_T u16_log_read_log_cnt(void);


/*************************************************************
��������: v_log_read_log_record		           				
��������: ��dataflash��ȡ�¼���¼���˺��������u16_log_index���з�Χ�жϣ�
          ���ڵ����߽����жϣ�������ǰ��Ҫ��ȡ������g_mut_log						
�������: u16_log_index -- ��¼��������0��ʼ��С���¼���¼����        		   				
�������: pt_log        -- ���淵�ص��¼���¼
����ֵ  ����														   				
**************************************************************/
void v_log_read_log_record(LOG_DATA_T *pt_log, U16_T u16_log_index);


#endif
