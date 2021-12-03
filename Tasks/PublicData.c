/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����PublicData.c
��    ����1.00
�������ڣ�2012-04-13
��    �ߣ�������
����������ȫ��ID�����ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-04-13  1.00     ����
**************************************************************/

#include <rtl.h>
#include "PublicData.h"
#include "ShareDataStruct.h"
#include "DisplayString.h"


/************************** ����ID���� *****************************************/
OS_TID g_tid_display;                     // ��ʾ��������������ID
OS_TID g_tid_dc_sample;                   // ֱ���ɼ�����ID
OS_TID g_tid_ac_sample;                   // �����ɼ�����ID
OS_TID g_tid_swt_sample;                  // �������ɼ�����ID
OS_TID g_tid_key;                         // �����жϼ���������ID
OS_TID g_tid_fault;                       // ���ϴ�������ID
OS_TID g_tid_batt;                        // ��ش�������ID
OS_TID g_tid_com1_comm;                   // ����1ͨ�Ŵ�������ID������1������ģ�顢ͨ��ģ�顢���ģ�顢���Ѳ��
OS_TID g_tid_can_comm;                    // CAN��ͨ�Ŵ�������ID
OS_TID g_tid_wdt;                         // ���Ź�����ID
OS_TID g_tid_bs;                          // ��̨ͨ������ID
OS_TID g_tid_load;                        // ��д�������ļ�����ID
OS_TID g_tid_compare_time;                // ��ʱ����ID


/************************** ȫ�ֹ����������� ***********************************/
SHARE_DATA_T  g_t_share_data;             // ȫ�ֹ������ݶ���
OS_MUT        g_mut_share_data;           // �������ȫ�ֹ������ݵĻ�����

U8_T          g_u8_product_type;          //��Ʒ�ͺ�


//�������½ṹ�����������ȫ����
const RC10_SWT_ITEM_T g_t_swt_sheet[FACT_SWT_CTRL_MAX] = {
	//������Ч��ʶ���������ƣ����ؿ���ֵ�����ص�ǰ״̬
	//1#RC10ģ�����ɵ��8������
	//1S3
	{VALID,
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[0])},
	//1S4
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[1])},
	//1S5
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[2])},
	//2S3
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[3])},
	//2S4
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[4])},
	//2S5
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[5])},
	//MLS
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[6])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[0].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[7])},

	//2#RC10ģ�����ɵ��8������
	//3S1
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[0])},
	//3S2
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[1])},
	//3S3
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[2])},
	//3S4
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[3])},
	//3S5
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[4])},
	//3S6
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[5])},
	//3S7
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[6])},
	//3S8
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[1].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[7])},

	//3#RC10ģ�����ɵ��8������
	//3S9
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[8])},
	//3S10
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[9])},
	//3S11
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[10])},
	//3S12
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[11])},
	//3S13
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[12])},
	//3S14
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[13])},
	//3S15
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[14])},
	//3S16
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[2].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[15])},

	//4#RC10ģ�����ɵ��8������
	//4S1
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[17])},
	//4S2
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[18])},
	//4S3
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[19])},
	//4S4
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[20])},
	//4S5
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[21])},
	//4S6
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[22])},
	//4S7
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[23])},
	//4S8
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[3].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[24])},
	
	//5#RC10ģ�����ɵ��8������
	//4S9
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[25])},
	//4S10
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[26])},
	//4S11
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[27])},
	//4S12
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[28])},
	//4S13
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[29])},
	//4S14
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[30])},
	//4S15
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[31])},
	//4S16
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[4].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[32])},
	
	//6#RC10ģ�����ɵ��8������
	//3S17
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[16])},
	//4S17
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[33])},
	//MLS
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[6])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[0])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[0])},
	//
	{INVALID,  
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[0])},
	//
	{INVALID,  
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[0])},
	//
	{INVALID,  
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[5].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[0])},

	//7#RC10ģ�����ɵ��8������
	//AC 1HK
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[0])},
	//AC 2HK
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[1])},
	//AC 3HK
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[2])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[3])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[4])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[5])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[6])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[6].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_ac10.u8_swt_state[7])},
	
	//8#RC10ģ�����ɵ��8������
	//AC 3S1
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[34])},
	//AC 3S2
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[35])},
	//AC 3S3
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[36])},
	//AC 3S4
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[37])},
	//AC 3S5
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[38])},
	//AC 3S6
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[39])},
	//AC 3S7
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[40])},
	//AC 3S8
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[7].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[41])},

	//9#RC10ģ�����ɵ��8������
	//AC 3S9
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[42])},
	//AC 3S10
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[43])},
	//AC 3S11
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[44])},
	//AC 3S12
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[45])},
	//AC 3S13
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[46])},
	//AC 3S14
	{VALID,
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[47])},
	//AC 3S15
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[48])},
	//AC 3S16
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[8].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[49])},
	
	//10#RC10ģ�����ɵ��8������
	//AC 4S1
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[50])},
	//AC 4S2
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[51])},
	//AC 4S3
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[52])},
	//AC 4S4
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[53])},
	//AC 4S5
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[54])},
	//AC 4S6
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[55])},
	//AC 4S7
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[56])},
	//AC 4S8
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[9].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[57])},
	
	//11#RC10ģ�����ɵ��8������
	//AC 4S9
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[0]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[58])},
	//AC 4S10
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[1]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[59])},
	//AC 4S11
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[2]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[60])},
	//AC 4S12
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[3]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[61])},
	//AC 4S13
	{VALID,
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[4]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[62])},
	//AC 4S14
	{VALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[5]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[63])},
	//AC 4S15
	{VALID,  
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[6]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[64])},
	//
	{INVALID, 
	&(g_t_share_data.t_sys_cfg.t_swt_ctrl[10].u8_swt_ctrl[7]), 
	&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[65])},
};

//�������½ṹ�����������ȫ����
const FDL_SWT_PAIR_T g_u8_ecswt_state_from_fdl[] = {
	//Ŀ�꿪��״ֵ̬��Դ����״ֵ̬(��FC10ģ�����)
	//3S1
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[0]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[0].u8_state),
	FC10_SWT},
	//3S2
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[1]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[1].u8_state),
	FC10_SWT},
	//3S3
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[2]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[2].u8_state),
	FC10_SWT},
	//3S4
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[3]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[3].u8_state),
	FC10_SWT},
	//3S5
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[4]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[4].u8_state),
	FC10_SWT},
	//3S6
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[5]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[5].u8_state),
	FC10_SWT},
	//3S7
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[6]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[6].u8_state),
	FC10_SWT},
	//3S8
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[7]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[7].u8_state),
	FC10_SWT},
	//3S9
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[8]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[8].u8_state),
	FC10_SWT},
	//3S10
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[9]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[9].u8_state),
	FC10_SWT},
	//3S11
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[10]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[10].u8_state),
	FC10_SWT},
	//3S12
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[11]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[11].u8_state),
	FC10_SWT},	
	//3S13
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[12]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[12].u8_state),
	FC10_SWT},
	//3S14
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[13]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[13].u8_state),
	FC10_SWT},
	//3S15
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[14]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[14].u8_state),
	FC10_SWT},
	//3S16
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[15]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[15].u8_state),
	FC10_SWT},
	//3S17
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[16]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder[16].u8_state),
	FC10_SWT},

	//4S1
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[17]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[0].u8_state),
	FC10_SWT},
	//4S2
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[18]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[1].u8_state),
	FC10_SWT},
	//4S3
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[19]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[2].u8_state),
	FC10_SWT},
	//4S4
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[20]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[3].u8_state),
	FC10_SWT},
	//4S5
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[21]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[4].u8_state),
	FC10_SWT},
	//4S6
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[22]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[5].u8_state),
	FC10_SWT},
	//4S7
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[23]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[6].u8_state),
	FC10_SWT},
	//4S8
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[24]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[7].u8_state),
	FC10_SWT},
	//4S9
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[25]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[8].u8_state),
	FC10_SWT},
	//4S10
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[26]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[9].u8_state),
	FC10_SWT},
	//4S11
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[27]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[10].u8_state),
	FC10_SWT},
	//4S12
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[28]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[11].u8_state),
	FC10_SWT},
	//4S13
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[29]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[12].u8_state),
	FC10_SWT},
	//4S14
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[30]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[13].u8_state),
	FC10_SWT},
	//4S15
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[31]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[14].u8_state),
	FC10_SWT},
	//4S16
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[32]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[15].u8_state),
	FC10_SWT},
	//4S17
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[33]), 
	&(g_t_share_data.t_rt_data.t_feeder_panel[4].t_feeder[16].u8_state),
	FC10_SWT},

	//AC 3S1
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[34]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[0].u8_state),
	FC10_SWT},
	//AC 3S2
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[35]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[1].u8_state),
	FC10_SWT},
	//AC 3S3
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[36]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[2].u8_state),
	FC10_SWT},
	//AC 3S4
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[37]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[3].u8_state),
	FC10_SWT},
	//AC 3S5
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[38]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[4].u8_state),
	FC10_SWT},
	//AC 3S6
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[39]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[5].u8_state),
	FC10_SWT},
	//AC 3S7
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[40]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[6].u8_state),
	FC10_SWT},
	//AC 3S8
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[41]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[7].u8_state),
	FC10_SWT},
	//AC 3S9
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[42]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[8].u8_state),
	FC10_SWT},
	//AC 3S10
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[43]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[9].u8_state),
	FC10_SWT},
	//AC 3S11
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[44]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[10].u8_state),
	FC10_SWT},
	//AC 3S12
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[45]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[11].u8_state),
	FC10_SWT},
	//AC 3S13
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[46]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[12].u8_state),
	FC10_SWT},
	//AC 3S14
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[47]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[13].u8_state),
	FC10_SWT},
	//AC 3S15
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[48]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[14].u8_state),
	FC10_SWT},
	//AC 3S16
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[49]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[15].u8_state),
	FC10_SWT},

	//AC 4S1
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[50]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[16].u8_state),
	FC10_SWT},
	//AC 4S2
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[51]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[17].u8_state),
	FC10_SWT},
	//AC 4S3
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[52]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[18].u8_state),
	FC10_SWT},
	//AC 4S4
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[53]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[19].u8_state),
	FC10_SWT},
	//AC 4S5
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[54]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[20].u8_state),
	FC10_SWT},
	//AC 4S6
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[55]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[21].u8_state),
	FC10_SWT},
	//AC 4S7
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[56]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[22].u8_state),
	FC10_SWT},
	//AC 4S8
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[57]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[23].u8_state),
	FC10_SWT},
	//AC 4S9
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[58]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[24].u8_state),
	FC10_SWT},
	//AC 4S10
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[59]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[25].u8_state),
	FC10_SWT},
	//AC 4S11
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[60]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[26].u8_state),
	FC10_SWT},
	//AC 4S12
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[61]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[27].u8_state),
	FC10_SWT},
	//AC 4S13
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[62]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[28].u8_state),
	FC10_SWT},
	//AC 4S14
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[63]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[29].u8_state),
	FC10_SWT},
	//AC 4S15
	{&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_fdl_swt_state[64]), 
	&(g_t_share_data.t_rt_data.t_ac_panel.t_feeder[30].u8_state),
	FC10_SWT},
};

/*************************************************************
��������: v_public_fdl_swt_sync_update
��������: ����ģ������ĵ�ٿ���״̬ͬ������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_public_fdl_swt_sync_update(void)
{
	U16_T i, max;
	U8_T  *p_u8_dest, *p_u8_sorce;

	max = sizeof(g_u8_ecswt_state_from_fdl) / sizeof(FDL_SWT_PAIR_T);
	for (i=0; i<max; i++)
	{
		p_u8_dest = g_u8_ecswt_state_from_fdl[i].p_u8_swt_dest;
		p_u8_sorce = g_u8_ecswt_state_from_fdl[i].p_u8_swt_sorce;
		if (g_u8_ecswt_state_from_fdl[i].data_sorce == FC10_SWT)
		{
			if ((*p_u8_sorce) == 2)
			{
				*p_u8_dest = 1;	//�պ�
			}
			else
			{
				*p_u8_dest = 0;	//�Ͽ�
			}
		}
		else
		{
			*p_u8_dest = *p_u8_sorce;
		}
	}
}

/*************************************************************
��������: u16_public_get_ctrl_swt_num
��������: ͳ�ƻ�ȡ��Ч�ĵ�ٿ�������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
U16_T u16_public_get_ctrl_swt_num(void)
{
	U16_T i, cnt;

	for (cnt=i=0; i<FACT_SWT_CTRL_MAX; i++)
	{
		if (g_t_swt_sheet[i].u8_swt_valid)
		{
			cnt++;
		}
	}

	return (cnt);
}

/*************************************************************
��������: u16_public_get_first_swt_index
��������: ͳ�ƻ�ȡ��һ����Ч�ĵ�ٿ�������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
U16_T u16_public_get_first_swt_index(void)
{
	U16_T i=0, idx;

	while(i < FACT_SWT_CTRL_MAX)
	{
		if (g_t_swt_sheet[i].u8_swt_valid)
		{
			idx = i;
			break;
		}
	}

	return (idx);
}

/*************************************************************
��������: u16_public_get_last_swt_index
��������: ͳ�ƻ�ȡ���һ����Ч�ĵ�ٿ�������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
U16_T u16_public_get_last_swt_index(void)
{
	U16_T i=0, idx;

    i = FACT_SWT_CTRL_MAX - 1;
	do
	{
		if (g_t_swt_sheet[i].u8_swt_valid)
		{
			idx = i;
			break;
		}
	}while(i--);

	return (idx);
}

/*************************************************************
��������: u16_public_pre_mv_swt_index
��������: ����ǰ��Ч��ٿ���������ǰ��n
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
U16_T u16_public_pre_mv_swt_index(U16_T *p_u16_index, U16_T n)
{
	U16_T idx;

    idx = *p_u16_index;
	if(n > 0)
	{
		do
		{
			idx--;
			if (g_t_swt_sheet[idx].u8_swt_valid)
			{
				n--;
			}			
		}while(idx && n);
	}

	*p_u16_index = idx;

	return (idx);
}

/*************************************************************
��������: u16_public_next_mv_swt_index
��������: ����ǰ��Ч��ٿ��������ź���n
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
U16_T u16_public_next_mv_swt_index(U16_T *p_u16_index, U16_T n)
{
	U16_T idx;

    idx = *p_u16_index;
	if(n > 0)
	{
		while((idx < (FACT_SWT_CTRL_MAX-1)) && n)
		{
			idx++;
			if (g_t_swt_sheet[idx].u8_swt_valid)
			{
				n--;
			}			
		}
	}

	*p_u16_index = idx;

	return (idx);
}

/*************************************************************
��������: u16_public_get_swt_index_from_no
��������: ����Ч��ٿ������(��1��ʼ)��ȡ�Ծ��б�����
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
U16_T u16_public_get_swt_index_from_no(U16_T no)
{
	U16_T i=0, idx=0, cnt=0;

   	if (no > 0)
	{
		while(i < FACT_SWT_CTRL_MAX)
		{
			if (g_t_swt_sheet[i].u8_swt_valid)
			{			
				idx = i;				
				cnt++;
				if(cnt == no)
				{
					break;
				}
			}
			i++;
		}
	}

	return (idx);
}

