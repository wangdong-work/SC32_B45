/************************************************************
Copyright (C), 2012-2020, ����Ӣ����Ƽ��������޹�˾
�� �� ����BatteryManage.h
��    ����1.00
�������ڣ�2012-03-22
��    �ߣ����ķ�
������������ع���ͷ�ļ�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	���ķ�    2012-05-25  1.00     ����
**************************************************************/

#ifndef __BATTERYMANAGE_H_
#define __BATTERYMANAGE_H_

#include "Type.h"

/*
��д     ȫ��                   ����
batm     Battery Management    ��ع���

*/


#define	BATM_TIME_SLOT					1			//��س��״̬������������SΪ��λ,Ҫ������10��Լ��
#define BATM_CUN_CAP_TIME_SLOT			10			//ÿ10S����һ�ε������
#define BATM_DISCHARGE_GAP				2			//����״̬�£���������趨��ѹ����ֹ��ѹ��2V
#define BATM_MDL_MAX_LIMIT_PERCENT		105.0		//ģ�����������ٷֱ�
#define BATM_MDL_MIN_LIMIT_PERCENT		5.0			//ģ����С������ٷֱ�
#define BATM_CAPACITY_COEFF             1000        //�������������õ�ϵ��


#define	BAT_CUR_MAX(a, b)				( ((a)>(b))?(a):(b) )	  //�����������ֵ��
#define	BAT_CUR_MIN(a, b)				( ((a)>(b))?(b):(a) )	  //����������Сֵ��


typedef struct
{

	F32_T	f32_charge_volt;		 		 //���Ƴ���������õ�ѹ
	F32_T	f32_charge_limit_percent;		 //���Ƴ������������	ǧ�ֱȱ�ʾ
	F32_T	f32_charge_limit_abs_vaule;		 //�����������ֵ���Ծ���ֵ��ʾ
	F32_T	f32_bat_capacity;		 	     //��ǰ�������
	U16_T	u16_charge_mode;			 	 //��ǰ��س��ģʽ
	U16_T	u16_persist_equ_t;               // �����ۼ�ʱ�䣬�Է�Ϊ��λ
	U16_T	u16_equ_count_down;				 // ���䵹��ʱ�����Է�Ϊ��λ
	U16_T	u16_persist_flo_t;               // �����ۼ�ʱ�䣬��ʱΪ��λ
	U16_T	u16_oc_durable_t; 				 //���������ά�����趨ת���������,����Ϊ��λ
	U16_T	u16_persist_chd_t;               // �����ۼ�ʱ�䣬�Է�Ϊ��λ 
	U16_T	u16_lost_ac_t;					 //����״̬�£�����ͣ���ʱ�����Է�Ϊ��λ
}BATM_CHARGE_DATA_T;
						   

/*************************************************************
��������: v_batm_battery_manage_task		           				
��������: ��ع�������ʵ�ֵ�ؾ�����ת����������������&��ѹ���ڣ��������ͨ��RTX����os_tsk_create������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
extern __task void v_batm_battery_manage_task(void);

#endif
