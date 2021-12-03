/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Rtc.c
��    ����1.00
�������ڣ�2012-03-20
��    �ߣ�������
����������rtc������ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-03-20  1.00     ����
**************************************************************/

#include <lpc17xx.h>
#include "Rtc.h"


#define RTC_TIME_INIT_FLAG 0x55555555

static RTC_TIME_T m_t_def_time = 
{
	RTC_DEF_SEC,
	RTC_DEF_MIN,
	RTC_DEF_HOUR,
	RTC_DEF_WEEK,
	RTC_DEF_DAY,
	RTC_DEF_MONTH,
	RTC_DEF_YEAR,
};

/*************************************************************
��������: v_rtc_save_user_data		           				
��������: �������ݵ�RTC��ͨ�üĴ���						
�������: u32_data -- Ҫ���������
          e_reg    -- �Ĵ����ţ�������ͨ�üĴ���1~4��ͨ�üĴ���0�����ã�RTC�����ڲ�ʹ��ͨ�üĴ���0        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_rtc_save_user_data(U32_T u32_data, RTC_GPREG_E e_reg)
{
	switch (e_reg)
	{
		case GP_REG_1:
			LPC_RTC->GPREG1 = u32_data;
			break;

		case GP_REG_2:
			LPC_RTC->GPREG2 = u32_data;
			break;

		case GP_REG_3:
			LPC_RTC->GPREG3 = u32_data;
			break;

		case GP_REG_4:
			LPC_RTC->GPREG4 = u32_data;
			break;

		default:
			break;
	}
}

/*************************************************************
��������: u32_rtc_read_user_data		           				
��������: ��RTC��ͨ�üĴ�����ȡ����						
�������: e_reg -- �Ĵ����ţ�������ͨ�üĴ���1~4��ͨ�üĴ���0�����ã�RTC�����ڲ�ʹ��ͨ�üĴ���0         		   				
�������: ��
����ֵ  ��ͨ�üĴ����е�����														   				
**************************************************************/
U32_T u32_rtc_read_user_data(RTC_GPREG_E e_reg)
{
	U32_T u32_data = 0;

	switch (e_reg)
	{
		case GP_REG_1:
			u32_data = LPC_RTC->GPREG1;
			break;

		case GP_REG_2:
			u32_data = LPC_RTC->GPREG2;
			break;

		case GP_REG_3:
			u32_data = LPC_RTC->GPREG3;
			break;

		case GP_REG_4:
			u32_data = LPC_RTC->GPREG4;
			break;

		default:
			break;
	}

	return u32_data;
}

/*************************************************************
��������: v_rtc_rtc_init		           				
��������: ��ʼ��RTC						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_rtc_rtc_init(void)
{
	LPC_RTC->ILR  = 0x03;           // ����жϱ�־
	LPC_RTC->CIIR = 0x00;           // Disable all counter increase interrupt
	LPC_RTC->AMR  = 0xFF;           // Disable alarm interrupt

	LPC_RTC->RTC_AUXEN   = 0x00;    // Disable Oscillator Fail detect interrupt
	LPC_RTC->RTC_AUX     = 0x10; 
	LPC_RTC->CALIBRATION = 0;

	if (LPC_RTC->CCR  != 0x11)
		LPC_RTC->CCR  = 0x11;       // ʹ��clock���ر�У׼����

	if (LPC_RTC->GPREG0 != RTC_TIME_INIT_FLAG)
	{
		v_rtc_rtc_set_time(&m_t_def_time);
		LPC_RTC->GPREG0 = RTC_TIME_INIT_FLAG;
	}
}

/*************************************************************
��������: u16_rtc_get_doy		           				
��������: �������������һ���еĵڼ���						
�������: u16_year -- ��
          u8_mon   -- ��
		  u8_dom   -- �գ�һ�����еĵڼ��죩        		   				
�������: ��
����ֵ  ��һ���еĵڼ���														   				
**************************************************************/
static U16_T u16_rtc_get_doy(U16_T u16_year, U8_T u8_mon, U8_T u8_dom)
{
	U8_T day_of_mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	U16_T i, sum = u8_dom;

	if ((u16_year%4==0 && u16_year%100!=0) || u16_year%400==0)
		day_of_mon[1] = 29;

	for (i=0; i<u8_mon-1; i++)
		sum += day_of_mon[i];
	
	return sum; 
}

/*************************************************************
��������: u8_rtc_get_dow		           				
��������: �������ռ�������						
�������: u16_year -- ��
          u8_mon   -- ��
		  u8_dom   -- �գ�һ�����еĵڼ��죩        		   				
�������: ��
����ֵ  �����ڣ���Χ0-6��0��ʾ������														   				
**************************************************************/
static U8_T u8_rtc_get_dow(U16_T u16_year, U8_T u8_mon, U8_T u8_dom)
{
	U32_T week, c, y;

	c = (u16_year / 100);
	y = (u16_year % 100);

	if (u8_mon < 3)
		u8_mon += 12;

	week = (y + y/4 + c/4 - 2*c + 26*(u8_mon+1)/10 + u8_dom - 1);

	return (U8_T)(week%7);
}

/*************************************************************
��������: v_rtc_rtc_set_time		           				
��������: ����RTC��ʱ��						
�������: pt_time -- ָ��Ҫ���õ�ʱ��ֵ������week�������ã���������������Զ����������
�������: ��
����ֵ  ����          		   															   				
**************************************************************/
void v_rtc_rtc_set_time(RTC_TIME_T *pt_time)
{
	LPC_RTC->CCR   = 0x10;          // ����֮ǰ���ر�RTC

	pt_time->week = u8_rtc_get_dow(pt_time->year, pt_time->month, pt_time->day);	

	LPC_RTC->SEC   = pt_time->sec;	// ����ʱ��
	LPC_RTC->MIN   = pt_time->min;
	LPC_RTC->HOUR  = pt_time->hour;
	LPC_RTC->DOM   = pt_time->day;
	LPC_RTC->DOW   = pt_time->week;
	LPC_RTC->DOY   = u16_rtc_get_doy(pt_time->year, pt_time->month, pt_time->day);
	LPC_RTC->MONTH = pt_time->month;
	LPC_RTC->YEAR  = pt_time->year;

	LPC_RTC->CCR   = 0x11;          // ʹ��clock���ر�У׼����
}

/*************************************************************
��������: rtc_get_time		           				
��������: ��ȡRTC��ʱ��						
�������: ��
�������: pt_time -- ���ض�ȡ����ʱ��ֵ���������Ϊ�����ռ�
����ֵ  ����          		   															   				
**************************************************************/
void v_rtc_rtc_get_time(RTC_TIME_T *pt_time)
{
	RTC_TIME_T t;
	U32_T i=0;
	
	while (1) {
		pt_time->sec = LPC_RTC->SEC;
		pt_time->min = LPC_RTC->MIN;
		pt_time->hour = LPC_RTC->HOUR;
		pt_time->week = LPC_RTC->DOW;
		pt_time->day = LPC_RTC->DOM;
		pt_time->month = LPC_RTC->MONTH;
		pt_time->year = LPC_RTC->YEAR;

		if (t.sec == pt_time->sec && t.min == pt_time->min &&
			t.hour == pt_time->hour && t.week == pt_time->week &&
			t.day == pt_time->day && t.month == pt_time->month &&
			t.year == pt_time->year) {
			break;
		}

		if (++i > 10)
			break;
	}
}



