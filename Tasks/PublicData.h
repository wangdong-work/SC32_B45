/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����PublicData.h
��    ����1.00
�������ڣ�2012-04-13
��    �ߣ�������
����������ȫ��ID����ͷ�ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-04-13  1.00     ����
**************************************************************/


#ifndef __PUBLIC_DATA_H_
#define __PUBLIC_DATA_H_


#include <rtl.h>
#include "Type.h"
#include "ShareDataStruct.h"

#define HW_VERSION "V1.0"

//��˾����.�������.��׼����汾��.��������汾���
#define SW_VERSION "B45.1800.C40.1"

//���ù��߰汾
#define TOOL_VERSION "V1.16"  //���Ӻ�̨�����������ѹ���������ݡ��ϴ�


/************************** �ͺ����� *******************************************/
#define PRODUCT_TYPE_SC12  0       //SC12
#define PRODUCT_TYPE_SC22  1       //SC22
#define PRODUCT_TYPE_SC32  2       //SC32

extern U8_T          g_u8_product_type;          //��Ʒ�ͺ�


/************************** ����ID���� *****************************************/
extern OS_TID g_tid_display;                     // ��ʾ��������������ID
extern OS_TID g_tid_dc_sample;                   // ֱ���ɼ�����ID
extern OS_TID g_tid_ac_sample;                   // �����ɼ�����ID
extern OS_TID g_tid_swt_sample;                  // �������ɼ�����ID
extern OS_TID g_tid_key;                         // �����жϼ���������ID
extern OS_TID g_tid_fault;                       // ���ϴ�������ID
extern OS_TID g_tid_batt;                        // ��ش�������ID
extern OS_TID g_tid_com1_comm;                   // ����1ͨ�Ŵ�������ID������1������ģ�顢ͨ��ģ�顢���ģ�顢���Ѳ��
extern OS_TID g_tid_can_comm;                    // CAN��ͨ�Ŵ�������ID
extern OS_TID g_tid_wdt;                         // ���Ź�����ID
extern OS_TID g_tid_bs;                          // ��̨ͨ������ID
extern OS_TID g_tid_load;                        // ��д�������ļ�����ID
extern OS_TID g_tid_compare_time;                // ��ʱ����ID


/************************** ȫ�ֹ����������� ***********************************/
extern SHARE_DATA_T  g_t_share_data;             // ȫ�ֹ������ݶ���
extern OS_MUT        g_mut_share_data;           // �������ȫ�ֹ������ݵĻ�����

/************************** ��ٿ����������� ***********************************/
extern const RC10_SWT_ITEM_T g_t_swt_sheet[];
extern U16_T u16_public_get_ctrl_swt_num(void);
extern U16_T u16_public_get_first_swt_index(void);
extern U16_T u16_public_get_last_swt_index(void);
extern U16_T u16_public_pre_mv_swt_index(U16_T *p_u16_index, U16_T n);
extern U16_T u16_public_next_mv_swt_index(U16_T *p_u16_index, U16_T n);
extern U16_T u16_public_get_swt_index_from_no(U16_T no);
extern void  v_public_fdl_swt_sync_update(void);


/******************* ��ʾ���������������¼���־���� ****************************/
#define KEY_K7                     0x0001
#define KEY_K6                     0x0002
#define KEY_K5                     0x0004
#define KEY_K4                     0x0008
#define KEY_K3                     0x0010
#define KEY_K2                     0x0020
#define KEY_EVT_FLAGS              (KEY_K7 | KEY_K6 | KEY_K5 | KEY_K4 | KEY_K3 | KEY_K2)

#define AC_ADJUST_SCUESS           0x0040     //����У׼�ɹ�
#define AC_ADJUST_FAIL             0x0080     //����У׼ʧ��
#define DC_ADJUST_SCUESS           0x0100     //ֱ��У׼�ɹ�
#define DC_ADJUST_FAIL             0x0200     //ֱ��У׼ʧ��
#define DC_ADJUST_FIRST_CURR_COMPLETE 0x0400  //��һ������У׼��У׼���
#define ADJUST_FLAGS               (AC_ADJUST_SCUESS | AC_ADJUST_FAIL | DC_ADJUST_SCUESS | DC_ADJUST_FAIL | DC_ADJUST_FIRST_CURR_COMPLETE)


#define FAULT_BUZZER_BEEP          0x0800    //����������
#define FAULT_BUZZER_QUEIT         0x1000    //�������ر�
#define FAULT_OCCURE               0x2000    //���Ϸ�����־
#define BUZZER_EVT_FLAGS           (FAULT_BUZZER_BEEP | FAULT_BUZZER_QUEIT | FAULT_OCCURE)


/******************* �����ɼ������¼���־ ***************************************/
#define AC_ADJUST_FIRST_PATH_UV    0x0001     //һ·UV��ѹУ׼
#define AC_ADJUST_FIRST_PATH_VW    0x0002     //һ·VW��ѹУ׼
#define AC_ADJUST_FIRST_PATH_WU    0x0004     //һ·WU��ѹУ׼
#define AC_ADJUST_SECOND_PATH_UV   0x0008     //��·UV��ѹУ׼
#define AC_ADJUST_SECOND_PATH_VW   0x0010     //��·VW��ѹУ׼
#define AC_ADJUST_SECOND_PATH_WU   0x0020     //��·WU��ѹУ׼
#define AC_ADJUST_EVT_FLAGS        (AC_ADJUST_FIRST_PATH_UV | AC_ADJUST_FIRST_PATH_VW | AC_ADJUST_FIRST_PATH_WU \
                                    | AC_ADJUST_SECOND_PATH_UV | AC_ADJUST_SECOND_PATH_VW | AC_ADJUST_SECOND_PATH_WU)


/******************* ֱ���ɼ������¼���־ ***************************************/
#define DC_ADJUST_BATT_VOLT            0x0001     //��ص�ѹУ׼
#define DC_ADJUST_PB_VOLT              0x0002     //��ĸ��ѹУ׼
#define DC_ADJUST_CB_VOLT              0x0004     //��ĸ��ѹУ׼
#define DC_ADJUST_BUS_NEG_TO_END_VOLT  0x0008     //ĸ�ߵ�ѹУ׼
#define DC_ADJUST_LOAD_CURR_1          0x0010     //���ص��1У׼
#define DC_ADJUST_LOAD_CURR_2          0x0020     //���ص���2У׼
#define DC_ADJUST_BATT1_CURR_1         0x0040     //һ���ص���1У׼
#define DC_ADJUST_BATT1_CURR_2         0x0080     //һ���ص���2У׼
#define DC_ADJUST_BATT2_CURR_1         0x0100     //�����ص���1У׼
#define DC_ADJUST_BATT2_CURR_2         0x0200     //�����ص���2У׼
#define DC_ADJUST_EVT_FLAGS        (DC_ADJUST_BATT_VOLT | DC_ADJUST_PB_VOLT | DC_ADJUST_CB_VOLT | DC_ADJUST_BUS_NEG_TO_END_VOLT \
                                    | DC_ADJUST_LOAD_CURR_1 | DC_ADJUST_LOAD_CURR_2 | DC_ADJUST_BATT1_CURR_1 | \
                                    DC_ADJUST_BATT1_CURR_2 | DC_ADJUST_BATT2_CURR_1 | DC_ADJUST_BATT2_CURR_2)


/******************* ��ع��������¼���־ **************************************/
#define BATT_CAPACITY_RESTORE      0x0001     //�ָ��������
#define RECT_SET_CURR_SCUESS1      0x0002     //һ����ģ�������㲥�������
#define RECT_SET_CURR_SCUESS2      0x0004     //������ģ�������㲥�������


/******************* ���Ź������¼���־ ****************************************/
#define DISPLAY_FEED_DOG           0x0001
#define AC_SAMPLE_FEED_DOG         0x0002
#define	DC_SAMPLE_FEED_DOG         0x0004
#define	SWT_SAMPLE_FEED_DOG        0x0008
#define	KEY_FEED_DOG               0x0010
#define	FAULT_FEED_DOG             0x0020
#define BATT_FEED_DOG              0x0040
#define	COM1_FEED_DOG              0x0080
#define CAN_FEED_DOG               0x0100
#define BACKSTAGE_FEED_DOG         0x0200
#define LOAD_FILE_FEED_DOG         0x0400
#define COMPARE_TIME_FEED_DOG      0x0800

#define FEED_DOG_EVT_FLAGS         (DISPLAY_FEED_DOG | AC_SAMPLE_FEED_DOG | DC_SAMPLE_FEED_DOG \
                                    | SWT_SAMPLE_FEED_DOG | KEY_FEED_DOG | FAULT_FEED_DOG \
									| BATT_FEED_DOG | COM1_FEED_DOG | CAN_FEED_DOG \
									| BACKSTAGE_FEED_DOG |LOAD_FILE_FEED_DOG | COMPARE_TIME_FEED_DOG )

#endif
