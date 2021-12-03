/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����key.c
��    ����1.00
�������ڣ�2012-05-29
��    �ߣ�Ф��
������������ȡ���ص�����

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	Ф��    2012-05-29    1.00     ����
	Ф��	2013-02-01	  1.00	   �������ʱ���޶�Ϊ25us������Ӧ���ٹ����
**************************************************************/
#include "LPC17xx.h"
#include "type.h"
#include "delay.h"
#include "Switch.h"


#define   DELAY   25     //���ڹ����ӳ�ʱ��Ƚϳ����ʲ��úܳ����ӳ�ʱ��
/********************************************
 *  �������ƣ�void v_switch_switch_init()
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   ���ÿ��ص�ʱ���ߣ��͡���switch����push����Ϊ���
  P1.27=switch_data=I; P2.12=switch_CLK=O,P2.13=switch_push=0
*********************************************/
void v_switch_switch_init()
{   
	LPC_GPIO2->FIODIR |=(3<<12);   //switch_clock output
}
/********************************************
 *  �������ƣ� U32_T switch_switch_data()
 *  �������:   ��
 *  ���ؽ��:   ��ǰ�Ŀ��ص��
 *  ���ܽ���: �ɼ���ǰ�Ŀ���״̬
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

