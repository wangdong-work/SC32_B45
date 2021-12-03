/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����CompareTime.c
��    ����1.00
�������ڣ�2012-08-03
��    �ߣ�������
������������ʱʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-08-03  1.00     ����
**************************************************************/

#include <rtl.h>

#include "PublicData.h"
#include "CompareTime.h"

#include "../Drivers/IRIGB.h"

static OS_MUT m_mut_compare_time;   //�������RTC��B���ʱ�Ļ�����


/*************************************************************
��������: u8_ctime_get_time
��������: ��ȡϵͳʱ�䣬���B���ʱ���ã��򷵻�B���ʱ�õ���ʱ�䣬����������򷵻ر�����RTCʱ��ֵ
�������: ��
�������: t_time -- ����ʱ��ֵ
����ֵ  ��TIME_RTC -- ���ص���RTC��ʱ�䣻TIME_IRIGB -- ���ص���B���ʱ��ʱ��
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
			//ϵͳû��ʹ�����ڣ���������û�м������ڣ�������Ҫ�������
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
��������: v_ctime_compare_time_task
��������: ��ʱ������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_ctime_init_mutex(void)
{
	os_mut_init(m_mut_compare_time);
}

/*************************************************************
��������: v_ctime_compare_time_task
��������: ��ʱ������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
__task void v_ctime_compare_time_task(void)
{
	RTC_TIME_T t_rtc_time;
	IRIGB_TIME_T t_irigb_time;
	HAVE_E       e_compare_time;
	
	while (1)
	{
		os_evt_set(COMPARE_TIME_FEED_DOG, g_tid_wdt);             //����ι���¼���־
		
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
