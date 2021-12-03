/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：CompareTime.c
版    本：1.00
创建日期：2012-08-03
作    者：郭数理
功能描述：对时实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-08-03  1.00     创建
**************************************************************/

#include <rtl.h>

#include "PublicData.h"
#include "CompareTime.h"

#include "../Drivers/IRIGB.h"

static OS_MUT m_mut_compare_time;   //定义访问RTC和B码对时的互斥量


/*************************************************************
函数名称: u8_ctime_get_time
函数功能: 获取系统时间，如果B码对时可用，则返回B码对时得到的时间，如果不可用则返回本机的RTC时间值
输入参数: 无
输出参数: t_time -- 返回时间值
返回值  ：TIME_RTC -- 返回的是RTC的时间；TIME_IRIGB -- 返回的是B码对时的时间
**************************************************************/
U8_T u8_ctime_get_time(RTC_TIME_T *t_time)
{
	RTC_TIME_T t_rtc_time;
	IRIGB_TIME_T t_irigb_time;
	HAVE_E       e_compare_time;
	U8_T ret;
	
	os_mut_wait(m_mut_compare_time, 0xFFFF);
	v_rtc_rtc_get_time(&t_rtc_time);
	v_irigb_compare_time_read(&t_irigb_time);
	os_mut_release(m_mut_compare_time);
		
	os_mut_wait(g_mut_share_data, 0xFFFF);
	e_compare_time = g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time;
	os_mut_release(g_mut_share_data);
		
	if (e_compare_time == HAVE)
	{
		if (t_irigb_time.quality < QUALITY_1S)
		{
			//系统没有使用星期，所以这里没有计算星期，如有需要可以添加
			t_time->sec = t_irigb_time.sec;
			t_time->min = t_irigb_time.min;
			t_time->hour = t_irigb_time.hour;
			t_time->day = t_irigb_time.day;
			t_time->month = t_irigb_time.mon;
			t_time->year = t_irigb_time.year + 2000;
			
			ret = TIME_IRIGB;
		}
		else
		{
			*t_time = t_rtc_time;
			ret = TIME_RTC;
		}
	}
	else
	{
		*t_time = t_rtc_time;
		ret = TIME_RTC;
	}
	
	return ret;
}

/*************************************************************
函数名称: v_ctime_compare_time_task
函数功能: 对时任务函数
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_ctime_init_mutex(void)
{
	os_mut_init(m_mut_compare_time);
}

/*************************************************************
函数名称: v_ctime_compare_time_task
函数功能: 对时任务函数
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
__task void v_ctime_compare_time_task(void)
{
	RTC_TIME_T t_rtc_time;
	IRIGB_TIME_T t_irigb_time;
	HAVE_E       e_compare_time;
	
	while (1)
	{
		os_evt_set(COMPARE_TIME_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志
		
		os_mut_wait(g_mut_share_data, 0xFFFF);
		e_compare_time = g_t_share_data.t_sys_cfg.t_sys_param.e_compare_time;
		os_mut_release(g_mut_share_data);
		
		if (e_compare_time == NO)
		{
			os_dly_wait(50);
		}
		else
		{
			os_mut_wait(m_mut_compare_time, 0xFFFF);
			
			v_rtc_rtc_get_time(&t_rtc_time);
			v_irigb_compare_time_read(&t_irigb_time);
			
			if (t_irigb_time.quality < QUALITY_1S)
			{
				if ((t_rtc_time.sec != t_irigb_time.sec)
					|| (t_rtc_time.min != t_irigb_time.min)
					|| (t_rtc_time.hour != t_irigb_time.hour)
					|| (t_rtc_time.day != t_irigb_time.day)
					|| (t_rtc_time.month != t_irigb_time.mon)
					|| (t_rtc_time.year != (t_irigb_time.year + 2000)))
				{
					t_rtc_time.sec = t_irigb_time.sec;
					t_rtc_time.min = t_irigb_time.min;
					t_rtc_time.hour = t_irigb_time.hour;
					t_rtc_time.day = t_irigb_time.day;
					t_rtc_time.month = t_irigb_time.mon;
					t_rtc_time.year = t_irigb_time.year + 2000;
					
					v_rtc_rtc_set_time(&t_rtc_time);
				}
			}
			
			os_mut_release(m_mut_compare_time);
			
			os_dly_wait(10);
		}
	}
	
}
