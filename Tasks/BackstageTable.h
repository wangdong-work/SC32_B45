/************************************************************
Copyright (C), ����Ӣ����Ƽ��������޹�˾
�� �� ����BackstageTable.h
��    ����1.00
�������ڣ�2012-07-28
��    �ߣ�������
������������̨ͨ�����ݱ�ͷ�ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-07-28  1.00     ����
***********************************************************/


#ifndef __BACKSTAGE_TABLE_H_
#define __BACKSTAGE_TABLE_H_


#include "Type.h"


#define BS_TYPE_U8  0
#define BS_TYPE_U16 1
#define BS_TYPE_F32 2
#define BS_TYPE_S16 3

#define BS_YC_SIZE  911//898      //ң����Ŀ������ʵ���������������911,2021-12-05
#define BS_YX_SIZE  2439//2528//2304     //ң����Ŀ������ʵ���������������2439,2021-12-05
#define BS_YK_SIZE  50//82//26       //ң����Ŀ������ʵ���������������50,2021-12-05
#define BS_YT_SIZE  2        //ң����Ŀ����

#define BS_ITEM_SIZE 4       //�����Ŀ���ֽ���
#define BS_ITEM_INVALID    2
#define BS_ITEM_UPLOAD     1
#define BS_ITEM_NO_UPLOAD  0

#define BS_CB_POS_TO_GND_VOLT_INDEX  19   //��ĸ���Եص�ѹ����
#define BS_PB_POS_TO_GND_VOLT_INDEX  20   //��ĸ���Եص�ѹ����
#define BS_BUS_NEG_TO_GND_VOLT_INDEX 21   //ĸ�߸��Եص�ѹ����



//�����Ŀ
typedef struct
{
	U16_T u16_id;    //��ĿID
	U16_T u16_flag;  //�Ƿ��ϴ���־
}BS_ITEM_T;


//ң����װ��Ŀ
typedef struct
{
	void  *pv_val;                //ָ��ң������ָ��
	U16_T u16_mask;               //������
	U8_T  u8_type;                //pv_valָ�����������
	
	U16_T  u16_id;                //����ID
	
	U8_T   u8_major_val;          //���Ƚ�ֵ��*pu8_major_condition>=u8_major_val������������
	U8_T   u8_second_val;         //�αȽ�ֵ��*pu8_second_condition>=u8_second_val������������
	U8_T   *pu8_major_condition;  //ָ������������
	U8_T   *pu8_second_condition; //ָ�����������
}YX_ASSEMBLE_T;


//ң����װ��Ŀ
typedef struct
{
	F32_T  *pf32_val;             //ָ��ң������ָ��
	U16_T  u16_modbus_coeff;      //MODBUS���
	U16_T  u16_cdt_coeff;         //CDT���
	U16_T  u16_id;                //����ID
	
	U8_T   u8_major_val;          //���Ƚ�ֵ��*pu8_major_condition>=u8_major_val������������
	U8_T   u8_second_val;         //�αȽ�ֵ��*pu8_second_condition>=u8_second_val������������
	U8_T   *pu8_major_condition;  //ָ������������
	U8_T   *pu8_second_condition; //ָ�����������
}YC_ASSEMBLE_T;


//ң����װ��Ŀ
typedef struct
{
	void  *pv_val;                //ָ��ң������ָ��
	U16_T  u16_id;                //����ID
	U8_T  u8_type;                //pv_valָ�����������
	
	U8_T   u8_major_val;          //���Ƚ�ֵ��*pu8_major_condition>=u8_major_val������������
	U8_T   *pu8_major_condition;  //ָ������������
}YK_ASSEMBLE_T;



/* ң������ */
extern const YC_ASSEMBLE_T g_t_yc_assemble[BS_YC_SIZE];

/* ң������ */
extern const YX_ASSEMBLE_T g_t_yx_assemble[BS_YX_SIZE];

/* ң������ */
extern const YK_ASSEMBLE_T g_t_yk_assemble[BS_YK_SIZE];

/* ң������ */
extern const YC_ASSEMBLE_T g_t_yt_assemble[BS_YT_SIZE];

#endif
