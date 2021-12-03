/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：key.c
版    本：1.00
创建日期：2012-05-29
作    者：肖江
功能描述：读取按键的驱动

函数列表：

修改记录：
	作者      日期        版本     修改内容
	肖江    2012-05-29    1.00     创建
**************************************************************/
#include "KEY.h"
#include <LPC17xx.h>
#include "Type.h"


/********************************************
 *  函数名称： void v_key_key_init()
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:   无
 *  全局变量:	无
 *  功能介绍:   对按键端口初始化，p1.28=k_clk=O,p1.29=K_data=I,p2.8=K_push=O
 端口默认方式为GPIO，上拉，DIR=输入
*********************************************/
void v_key_key_init()
{   
	LPC_GPIO1->FIODIR |=(1<<28);   //key_clock output
	LPC_GPIO2->FIODIR |=(1<<8);    //key_push output
}
/********************************************
 *  函数名称： U32_T key_key_data()
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:   按键值
 *  全局变量:	无
 *  功能介绍:   采集按键的数据，返回键值，首先检查到有按键按下就返回当前的键值
 后面的键值就不扫描。
*********************************************/
   
U32_T key_key_data()
{    
	U32_T i,data=0; 
	LPC_GPIO1->FIOSET =(1<<28);       //make key_clock low
	LPC_GPIO2->FIOSET =(1<<8);       //load data
	__NOP();
	__NOP();
	LPC_GPIO2->FIOCLR =(1<<8);
	__NOP();
	__NOP();
	LPC_GPIO1->FIOCLR =(1<<28);   //H  打第一个数据
	__NOP();
	__NOP();
	LPC_GPIO1->FIOSET =(1<<28);   //L  保持第一个数据
	__NOP();
	__NOP(); 
	for(i=0;i<=5;i++)
	{ 
		LPC_GPIO1->FIOCLR =(1<<28);
		__NOP();
		__NOP();
		LPC_GPIO1->FIOSET =(1<<28);
		__NOP();
		__NOP();
		if(LPC_GPIO1->FIOPIN&0x20000000)
		{
			data &=(~(1<<i));
		}
		else 
		{
			data |=(1<<i);
		}
	}
			 
	return data;
} 
  

