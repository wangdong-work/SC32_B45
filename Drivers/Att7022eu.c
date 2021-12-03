/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：key.c
版    本：1.00
创建日期：2012-05-29
作    者：肖江
功能描述：ATT7022E chip的驱动

函数列表：

修改记录：
	作者      日期        版本     修改内容
	肖江    2012-05-29    1.00     创建
**************************************************************/
//drive for ATT7022E chip
#include "LPC17xx.h"
#include "Att7022eu.h"
#include "delay.h"
#include "Type.h"


#define ATT_SET()  (LPC_GPIO0->FIOCLR =(1<<16)) //ATT片选 低电平有效
#define ATT_DET()  (LPC_GPIO0->FIOSET =(1<<16)) //ATT片选 高电平无效

/********************************************
 *  函数名称： void  v_att_spi_init(U32_T baurate)
 *  输入参数:  波特率控制
 *  输出参数:   无
 *  返回结果:   无
 *  全局变量:	无
 *  功能介绍:   SPI初始化
*********************************************/
void  v_att_spi_init(U32_T baurate)
{   
	U32_T i;

	i = 9000000/baurate;

	if((i>=8)&&(i<=254))
	{
		if(i%2==0)
		{
			i = i;
		}
		else
		{
			i = i-1;
		}
	}
	else
	{
		i = 8;
	} 
	    
	LPC_SC->PCLKSEL0 |=(3<<16); // PCSSP0 CLK = CCLK/8
	LPC_PINCON->PINSEL0 |=0xC0000000;  //select spi'pin SCK
	LPC_PINCON->PINSEL1 |=0x3C;      //1 :GPIO 2:MISO 3: MOSI
	LPC_GPIO0->FIODIR  |=(1<<16);    //GPIO 配置为输出，模式默认为上拉
	LPC_SPI->SPCCR=i; //baurate必须为偶数大于8。范围8-254；
	LPC_SPI->SPCR=0x28;//8bit模式，主机，CLK高有效。MSB在先，第一个时钟沿
       
}

/********************************************
 *  函数名称： U32_T att_att_read(U32_T regaddr)
 *  输入参数:   reg地址
 *  输出参数:   无
 *  返回结果:   无
 *  全局变量:	无
 *  功能介绍:   ATT reg 读取
*********************************************/
U32_T att_att_read(U32_T regaddr)
{ 
	U32_T data,reg;

	data = 0;
	reg = 0;

	ATT_SET();
	LPC_SPI->SPDR=regaddr;
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	v_delay_udelay(KPC_UDELAY);
	LPC_SPI->SPDR=0xAA;
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	reg=LPC_SPI->SPDR&0xff;
	data=reg<<16;
	LPC_SPI->SPDR=0xAA;  
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	reg=LPC_SPI->SPDR&0xff;
	data |=reg<<8;
	LPC_SPI->SPDR=0xAA;  
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	reg=LPC_SPI->SPDR&0xff;
	data |=reg;

	ATT_DET();
	return data&0xffffff;
}
      

 /********************************************
 *  函数名称： void ATT_write(uint32_t regaddr, uint32_t data)
 *  输入参数:  regaddr,要写入的命令地址，data要写入的数据
 *  输出参数:   无
 *  返回结果:	无
 *  全局变量:	无
 *  功能介绍:   主要用于校表的更新
*********************************************/
void  v_att_att_write(U32_T regaddr, U32_T data)
{ 
	U32_T reg;
	reg = 0;
	    
	ATT_SET();  
	__NOP();
	__NOP();
	LPC_SPI->SPDR=regaddr;
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	__NOP();
	reg=data&0xFF0000;
	LPC_SPI->SPDR=reg>>16;
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	reg=data&0x00FF00;
	LPC_SPI->SPDR=reg>>8;
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	LPC_SPI->SPDR=data&0xFF;
	while(!(LPC_SPI->SPSR&0x80))
	{
	}
	ATT_DET();

}

/********************************************
 *  函数名称： void v_att_att_init(void)
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:   无
 *  全局变量:	无
 *  功能介绍:   ATT70E初始化，配置它的模式，与增益
*********************************************/
void v_att_att_init(void)
{
	v_delay_mdelay(50);
	v_att_att_write(0x81,0x00B9FE);   //上电是默认的写校表数据，和读出的是计量数据，直接将校表数据写		  
	v_att_att_write(0x83,0x000004);
	v_delay_mdelay(5);
	v_att_att_write(JB_WEN,0x000000); //关闭写SPI校表操作
}


 
	

