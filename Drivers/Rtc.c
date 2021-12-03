/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Rtc.c
版    本：1.00
创建日期：2012-03-20
作    者：郭数理
功能描述：rtc驱动的实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-20  1.00     创建
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
函数名称: v_rtc_save_user_data		           				
函数功能: 保存数据到RTC的通用寄存器						
输入参数: u32_data -- 要保存的数据
          e_reg    -- 寄存器号，可以是通用寄存器1~4，通用寄存器0不能用，RTC驱动内部使用通用寄存器0        		   				
输出参数: 无
返回值  ：无														   				
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
函数名称: u32_rtc_read_user_data		           				
函数功能: 从RTC的通用寄存器读取数据						
输入参数: e_reg -- 寄存器号，可以是通用寄存器1~4，通用寄存器0不能用，RTC驱动内部使用通用寄存器0         		   				
输出参数: 无
返回值  ：通用寄存器中的数据														   				
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
函数名称: v_rtc_rtc_init		           				
函数功能: 初始化RTC						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_rtc_rtc_init(void)
{
	LPC_RTC->ILR  = 0x03;           // 清除中断标志
	LPC_RTC->CIIR = 0x00;           // Disable all counter increase interrupt
	LPC_RTC->AMR  = 0xFF;           // Disable alarm interrupt

	LPC_RTC->RTC_AUXEN   = 0x00;    // Disable Oscillator Fail detect interrupt
	LPC_RTC->RTC_AUX     = 0x10; 
	LPC_RTC->CALIBRATION = 0;

	if (LPC_RTC->CCR  != 0x11)
		LPC_RTC->CCR  = 0x11;       // 使能clock，关闭校准功能

	if (LPC_RTC->GPREG0 != RTC_TIME_INIT_FLAG)
	{
		v_rtc_rtc_set_time(&m_t_def_time);
		LPC_RTC->GPREG0 = RTC_TIME_INIT_FLAG;
	}
}

/*************************************************************
函数名称: u16_rtc_get_doy		           				
函数功能: 由年月日算出是一年中的第几天						
输入参数: u16_year -- 年
          u8_mon   -- 月
		  u8_dom   -- 日（一个月中的第几天）        		   				
输出参数: 无
返回值  ：一年中的第几天														   				
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
函数名称: u8_rtc_get_dow		           				
函数功能: 由年月日计算星期						
输入参数: u16_year -- 年
          u8_mon   -- 月
		  u8_dom   -- 日（一个月中的第几天）        		   				
输出参数: 无
返回值  ：星期，范围0-6，0表示星期日														   				
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
函数名称: v_rtc_rtc_set_time		           				
函数功能: 设置RTC的时间						
输入参数: pt_time -- 指向要设置的时间值，其中week不用设置，驱动会根据日期自动计算出星期
输出参数: 无
返回值  ：无          		   															   				
**************************************************************/
void v_rtc_rtc_set_time(RTC_TIME_T *pt_time)
{
	LPC_RTC->CCR   = 0x10;          // 设置之前，关闭RTC

	pt_time->week = u8_rtc_get_dow(pt_time->year, pt_time->month, pt_time->day);	

	LPC_RTC->SEC   = pt_time->sec;	// 设置时间
	LPC_RTC->MIN   = pt_time->min;
	LPC_RTC->HOUR  = pt_time->hour;
	LPC_RTC->DOM   = pt_time->day;
	LPC_RTC->DOW   = pt_time->week;
	LPC_RTC->DOY   = u16_rtc_get_doy(pt_time->year, pt_time->month, pt_time->day);
	LPC_RTC->MONTH = pt_time->month;
	LPC_RTC->YEAR  = pt_time->year;

	LPC_RTC->CCR   = 0x11;          // 使能clock，关闭校准功能
}

/*************************************************************
函数名称: rtc_get_time		           				
函数功能: 读取RTC的时间						
输入参数: 无
输出参数: pt_time -- 返回读取到的时间值，需调用者为其分配空间
返回值  ：无          		   															   				
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



