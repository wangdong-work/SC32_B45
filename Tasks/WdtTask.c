/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����WdtTask.c
��    ����1.00
�������ڣ�2012-06-05
��    �ߣ�������
�������������Ź�����ʵ���ļ�


�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-06-05  1.00     ����
**************************************************************/


#include <rtl.h>
#include "Type.h"
#include "PublicData.h"
#include "WdtTask.h"
#include "../Drivers/Wdt.h"
#include "../Drivers/Usb.h"
#include "../Drivers/Delay.h"


//#define WDT_DEBUG

/*************************************************************
��������: v_wtask_wdt_task		           				
��������: ���Ź�������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
__task void v_wtask_wdt_task(void)
{
#ifndef	WDT_DEBUG
	v_wdt_wdt_feed();         //��һ��ι������ʹ�ܿ��Ź�

	while (1)
	{
		/* ���Ź���ʱʱ��Ϊ2S�����Եȴ���ʱӦ�ô���2S����������¸ú�������2S����������ι����
		   ���û�������������Ź���λ */
		if (os_evt_wait_and(FEED_DOG_EVT_FLAGS, 0xFFFF) == OS_R_EVT)
		{
			v_wdt_wdt_feed();
			//u32_usb_debug_print("wdt task recv flag, %d\r\n", u32_delay_get_timer_val());
		}
	}
#else
	U32_T u32_last_time[12];
	U32_T u32_curr_time;
	U16_T flags;

	while (1)
	{
		if (os_evt_wait_or(FEED_DOG_EVT_FLAGS, 0xFFFF) == OS_R_EVT)
		{
			flags = os_evt_get();
			u32_curr_time = u32_delay_get_timer_val();
	
			if ((flags & DISPLAY_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: DISPLAY_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[0], u32_curr_time)/1000);
				u32_last_time[0] = u32_curr_time;
			}
			if ((flags & AC_SAMPLE_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: AC_SAMPLE_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[1], u32_curr_time)/1000);
				u32_last_time[1] = u32_curr_time;
			}
			if ((flags & DC_SAMPLE_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: DC_SAMPLE_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[2], u32_curr_time)/1000);
				u32_last_time[2] = u32_curr_time;
			}
			if ((flags & SWT_SAMPLE_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: SWT_SAMPLE_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[3], u32_curr_time)/1000);
				u32_last_time[3] = u32_curr_time;
			}
			if ((flags & KEY_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: KEY_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[4], u32_curr_time)/1000);
				u32_last_time[4] = u32_curr_time;
			}
			if ((flags & FAULT_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: FAULT_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[5], u32_curr_time)/1000);
				u32_last_time[5] = u32_curr_time;
			}
			if ((flags & BATT_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: BATT_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[6], u32_curr_time)/1000);
				u32_last_time[6] = u32_curr_time;
			}
			if ((flags & COM1_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: COM1_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[7], u32_curr_time)/1000);
				u32_last_time[7] = u32_curr_time;
			}
			if ((flags & CAN_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: CAN_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[8], u32_curr_time)/1000);
				u32_last_time[8] = u32_curr_time;
			}
			if ((flags & BACKSTAGE_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: BACKSTAGE_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[9], u32_curr_time)/1000);
				u32_last_time[9] = u32_curr_time;
			}
			if ((flags & LOAD_FILE_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: LOAD_FILE_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[10], u32_curr_time)/1000);
				u32_last_time[10] = u32_curr_time;
			}
			if ((flags & COMPARE_TIME_FEED_DOG) != 0)
			{
				u32_usb_debug_print("%d: COMPARE_TIME_FEED_DOG \r\n", u32_delay_time_elapse(u32_last_time[11], u32_curr_time)/1000);
				u32_last_time[11] = u32_curr_time;
			}
		}
	}
#endif
}
