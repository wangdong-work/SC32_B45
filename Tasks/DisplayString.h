/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����DisplayString.h
��    ����1.00
�������ڣ�2012-05-09
��    �ߣ�������
������������ʾ�ַ���ͷ�ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-05-09  1.00     ����
**************************************************************/

#ifndef __DISPLAY_STRING_H_
#define __DISPLAY_STRING_H_

/************************** ������ʾ�ַ������� ********************************/
#define MMI_STR_ID_TYPE_MASK            0xF000       //�ַ���ID���������룬��4λ��������
#define MMI_STR_ID_INDEX_MASK           0x0FFF       //�ַ���ID���������룬��12λ��������
#define MMI_STR_ID_NORMAL_NAME          0x0000       //�ַ���ID���4λΪ0��ʾһ���ַ����������ַ���IDΪ������g_pu8_string�����в鵽��Ӧ���ַ���ָ��
#define MMI_STR_ID_SPECIAL_NAME         (1 << 12)    //�ַ���ID���4λΪ1��ʾ�����ַ������ַ�������MMI_WIN_RECORD_T�ṹ���u8_special_name�в鵽


extern const STR_T g_s_string[][2];             //ȫ���ַ�������
extern const STR_T g_s_batt_record[][2];        //��س�ŵ��¼������
extern const STR_T g_s_ctrl_swt_name[][2];      //��ٿ�������


#endif
