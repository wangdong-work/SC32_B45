/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����key.c
��    ����1.00
�������ڣ�2012-05-29
��    �ߣ�Ф��
����������ATT7022E chip������

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	Ф��    2012-05-29    1.00     ����
**************************************************************/
//drive for ATT7022E chip
#include "LPC17xx.h"
#include "Att7022eu.h"
#include "delay.h"
#include "Type.h"


#define ATT_SET()  (LPC_GPIO0->FIOCLR =(1<<16)) //ATTƬѡ �͵�ƽ��Ч
#define ATT_DET()  (LPC_GPIO0->FIOSET =(1<<16)) //ATTƬѡ �ߵ�ƽ��Ч

/********************************************
 *  �������ƣ� void  v_att_spi_init(U32_T baurate)
 *  �������:  �����ʿ���
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   SPI��ʼ��
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
	LPC_GPIO0->FIODIR  |=(1<<16);    //GPIO ����Ϊ�����ģʽĬ��Ϊ����
	LPC_SPI->SPCCR=i; //baurate����Ϊż������8����Χ8-254��
	LPC_SPI->SPCR=0x28;//8bitģʽ��������CLK����Ч��MSB���ȣ���һ��ʱ����
       
}

/********************************************
 *  �������ƣ� U32_T att_att_read(U32_T regaddr)
 *  �������:   reg��ַ
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   ATT reg ��ȡ
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
 *  �������ƣ� void ATT_write(uint32_t regaddr, uint32_t data)
 *  �������:  regaddr,Ҫд��������ַ��dataҪд�������
 *  �������:   ��
 *  ���ؽ��:	��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   ��Ҫ����У��ĸ���
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
 *  �������ƣ� void v_att_att_init(void)
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   ATT70E��ʼ������������ģʽ��������
*********************************************/
void v_att_att_init(void)
{
	v_delay_mdelay(50);
	v_att_att_write(0x81,0x00B9FE);   //�ϵ���Ĭ�ϵ�дУ�����ݣ��Ͷ������Ǽ������ݣ�ֱ�ӽ�У������д		  
	v_att_att_write(0x83,0x000004);
	v_delay_mdelay(5);
	v_att_att_write(JB_WEN,0x000000); //�ر�дSPIУ�����
}


 
	

