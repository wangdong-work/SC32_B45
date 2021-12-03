/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：key.c
版    本：1.00
创建日期：2012-05-29
作    者：肖江
功能描述：读取开关的驱动

函数列表：

修改记录：
	作者      日期        版本     修改内容
	肖江    2012-05-29    1.00     创建
	肖江	2013-02-01	  1.00	   将光耦的时间修订为25us，以适应低速光耦的
**************************************************************/
#include "LPC17xx.h"
#include "type.h"
#include "delay.h"
#include "Switch.h"


#define   DELAY   25     //由于光耦延迟时间比较长，故采用很长的延迟时间
/********************************************
 *  函数名称：void v_switch_switch_init()
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:   无
 *  全局变量:	无
 *  功能介绍:   设置开关的时钟线，和——switch——push配置为输出
  P1.27=switch_data=I; P2.12=switch_CLK=O,P2.13=switch_push=0
*********************************************/
void v_switch_switch_init()
{   
	LPC_GPIO2->FIODIR |=(3<<12);   //switch_clock output
}
/********************************************
 *  函数名称： U32_T switch_switch_data()
 *  输出参数:   无
 *  返回结果:   当前的开关点表
 *  功能介绍: 采集当前的开关状态
*********************************************/
U32_T switch_switch_data()
{   
	U32_T data,j;
	data = 0; 

    LPC_GPIO2->FIOSET = 3<<12;       //load data  & clock lo
	v_delay_udelay(DELAY);

	if((LPC_GPIO1->FIOPIN&0x8000000)!=0x8000000)
	{
		data |=1<<7;
	}

    LPC_GPIO2->FIOCLR =1<<13;       //make switch_PUSH H
    v_delay_udelay(DELAY);

    for(j=7;j>0;j--)
	{ 
		LPC_GPIO2->FIOCLR =1<<12;
		v_delay_udelay(DELAY);
		LPC_GPIO2->FIOSET =1<<12;
       
		if((LPC_GPIO1->FIOPIN&0x8000000)!=0x8000000)
        {
			data |=1<<(j-1); 
		}
	    v_delay_udelay(DELAY);
	}

	for(j=15;j>=8;j--)
	{ 
		LPC_GPIO2->FIOCLR =1<<12;
		v_delay_udelay(DELAY);
		LPC_GPIO2->FIOSET =1<<12;
           
		if((LPC_GPIO1->FIOPIN&0x8000000)!=0x8000000)
		{
			data |=1<<j; 
		}
		v_delay_udelay(DELAY);
	}

	for(j=0;j<4;j++)
	{ 	
		LPC_GPIO2->FIOCLR =1<<12;
		v_delay_udelay(DELAY);
		LPC_GPIO2->FIOSET =1<<12;
		v_delay_udelay(DELAY);
	}

	for(j=19;j>=16;j--)
	{ 
		LPC_GPIO2->FIOCLR =1<<12;
		v_delay_udelay(DELAY);
		LPC_GPIO2->FIOSET =1<<12;
		if((LPC_GPIO1->FIOPIN&0x8000000)!=0x8000000)
		{
			data |=1<<j;
		} 
		v_delay_udelay(DELAY);
	}
     
	return data;
}  

