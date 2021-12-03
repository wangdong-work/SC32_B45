/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Delay.c
版    本：1.00
创建日期：2012-03-17
作    者：郭数理
功能描述：延时相关函数的实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-17  1.00     创建
	郭数理    2014-7-16   1.01     毫秒级延时由于运算溢出导致提前退出，将tc变量由32位改为64位，强制按64进行运算，避开溢出问题
								   u32_delay_time_elapse函数中u32_end_time强制转换成64位，避免溢出
**************************************************************/

#include <lpc17xx.h>

#include "Delay.h"


/*************************************************************
函数名称: v_delay_delay_init		           				
函数功能: 初始化定时器, 延时函数占用Timer0进行计时，其它驱动不能再使用Timer0						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_delay_delay_init(void)
{
	LPC_TIM0->TCR = 0;	          // disable timer
	LPC_TIM0->IR = 0x0000003F;    // clear interrrupt flag
	LPC_TIM0->TC = 0;
	LPC_TIM0->PR = 17;            // pclk=18MHZ, prescale=17, 分频后为1MHZ
	LPC_TIM0->PC = 0;
	LPC_TIM0->MCR = 0;
	LPC_TIM0->CCR = 0;
	LPC_TIM0->CTCR = 0;           // timer mode
	LPC_TIM0->TCR = 1;            // enable timer.
}

/*************************************************************
函数名称: v_delay_udelay		           				
函数功能: 微秒延时函数，此函数是忙等待延时，不会主动让出CPU						
输入参数: u32_us -- 延时的微秒数，请注意此函数误差为1us，所以如有需要可以延时的微秒数的基础上加1，
                例如：要延时5us，如果传入参数5，则延时时间在4us~5us之间，
				                 如果传入参数5+1，延时时间在5us~6us之间      		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_delay_udelay(U32_T u32_us)
{
	U32_T tc = LPC_TIM0->TC;
	U32_T cnt = 0xFFFFFFFF - tc;

	if (cnt < u32_us)
	{
		while ((LPC_TIM0->TC > 0x00FFFFFF) && (LPC_TIM0->TC <= 0xFFFFFFFF));
		cnt = u32_us - cnt;
		while (LPC_TIM0->TC < cnt);
	}
	else
	{
		cnt = tc + u32_us;
		while ((LPC_TIM0->TC >= tc) && (LPC_TIM0->TC < cnt));
	}
}

/*************************************************************
函数名称: v_delay_mdelay		           				
函数功能: 毫秒延时函数，此函数是忙等待延时，不会主动让出CPU						
输入参数: u32_ms -- 延时的毫秒数        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_delay_mdelay(U32_T u32_ms)
{
	uint64_t tc = LPC_TIM0->TC;
	uint64_t cnt = tc + u32_ms * 1000;

	if (cnt > 0xFFFFFFFF)
	{
		while ((LPC_TIM0->TC > 0x00FFFFFF) && (LPC_TIM0->TC <= 0xFFFFFFFF));
		cnt %= 0xFFFFFFFF;
		while (LPC_TIM0->TC < cnt);
	}
	else
	{
		while ((LPC_TIM0->TC >= tc) && (LPC_TIM0->TC < cnt));
	}
}

/*************************************************************
函数名称: u32_delay_get_timer_val		           				
函数功能: 获取timer的计数器数值						
输入参数: 无        		   				
输出参数: 无
返回值  ：timer的计数器数值														   				
**************************************************************/
U32_T u32_delay_get_timer_val(void)
{
	return LPC_TIM0->TC;
}

/*************************************************************
函数名称: u32_delay_time_elapse		           				
函数功能: 计算两个timer计数器值的时间差						
输入参数: u32_start_time, u32_end_time -- 这两个数值是get_timer_val返回的计数器数值，用来计算两次调用get_timer_val之间的流逝的时间，
                          两次调用之间最长时间差为71分钟，不能超过71分钟，u32_end_time是后一次调用get_timer_val返回的值       		   				
输出参数: 无
返回值  ：时间值，单位微秒(us)
例子    ；
			void f(void)
			{
				U32_T t1, t2, us;

				...

				t1 = u32_delay_get_timer_val();

				...   //执行一些其它操作

				t2 = u32_delay_get_timer_val();
				us = u32_delay_time_elapse(t1, t2);    //计算操作所用时间，单位是微秒(us)

				...
			}																   				
**************************************************************/
U32_T u32_delay_time_elapse(U32_T u32_start_time, U32_T u32_end_time)
{
	return ((0xFFFFFFFF + 1 + (uint64_t)u32_end_time - u32_start_time) % 0xFFFFFFFF);
}
