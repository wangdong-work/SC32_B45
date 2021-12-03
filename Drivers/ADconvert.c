/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����ADconvert.c
��    ����1.00
�������ڣ�2012-03-15
��    �ߣ��� ΰ ��
����������ADת��ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	��ΰ��    2012-03-15  1.00     ����
**************************************************************/

#include <lpc17xx.h>
#include "Type.h"
#include"ADconvert.h"
#include"delay.h"
#include "usb.h"


static U16_T data[30];

static U32_T g_pb_v_ad = 0;
static U32_T g_cb_v_ad = 0;	
static U32_T g_batt_v_ad = 0;

static U32_T g_load_c_ad = 0;
static U32_T g_batt_c_ad = 0;
static U32_T g_rect_c_ad = 0;

static U8_T g_pb_v_t1;
static U8_T g_pb_v_t2;

static U8_T g_cb_v_t1;
static U8_T g_cb_v_t2;
	
static U8_T g_batt_v_t1;
static U8_T g_batt_v_t2;

static U8_T g_load_c_t1;
static U8_T g_load_c_t2;

static U8_T g_batt_c_t1;
static U8_T g_batt_c_t2;

static U8_T g_rect_c_t1;
static U8_T g_rect_c_t2;

#define LIMIT_TIME 3
#define AD_LIMIT 2			//���ڴβ���ADֵ֮�����AD_LIMIT����Ϊ��Ч

/****************************************************************************
 *  �������ƣ�  adc_init() 
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:	��	
 *  ���ܽ���:   ADת����ʼ�� 
 ***************************************************************************/
void v_adc_init(void)
  {
 	LPC_SC->PCONP |= (1<<10);			   //��λ���蹦�ʿ��ƼĴ���PCONP��SSP��Ӧ����λ
	LPC_SC->PCLKSEL0 |= 0x00300000;	   //����SSP��PCLK=CCLK/8
	LPC_SSP1->CPSR = 0x0000001E;		   //����CPSDVSR=30
	LPC_PINCON->PINSEL0 |= 0x000AA000;   //��������ΪSSP����
	LPC_SSP1->CR0 = 0x0000040F;		   //�������ݳ���Ϊ16λ��SPI֡,SCR=4
	LPC_GPIO2->FIODIR0 |= 0XC0;		   //����P2.6/P2.7Ϊ���
  }

 /****************************************************************************
 *  �������ƣ�  U32_T ad_convert(U8_T type) 
 *  �������:   typeΪҪ��������������
 *  �������:   ��
 *  ���ؽ��:	AD��������	
 *  ���ܽ���:   ADת������ 
 ***************************************************************************/
U32_T ad_convert(U8_T type)
 {
 	U8_T i,j,k;
	U16_T ad,temp,valid_data;
	U32_T cnt, ch,datasum = 0;
	LPC_SSP1->CR1 = 0x00000002;		   //ʹ��SSP1

	switch(type)					   
	{
		case 0:						  //S_HM
			ch = 0x00000C00;
			LPC_SSP1->DR = ch;
			break;

		case 1:						  //S_KM
			ch = 0x00001C00;
			LPC_SSP1->DR = ch;
			break;

		case 2:						  //S_BAT
			ch = 0x00002C00;
			LPC_SSP1->DR = ch;
			break;

		case 3:						  //LOAD_I
			ch = 0x00003C00;
			LPC_GPIO2->FIOSET0 = 0xC0;
			v_delay_mdelay(2);
			LPC_SSP1->DR = ch;
			break;

		case 4:						  //BAT1_I
			ch = 0x00003C00;
			LPC_GPIO2->FIOSET0 = 0x80;
			LPC_GPIO2->FIOCLR0 = 0x40;
			v_delay_mdelay(2);
			LPC_SSP1->DR = ch;
			break;

		case 5:						  //BAT2_I
			ch = 0x00003C00;
			LPC_GPIO2->FIOSET0 = 0x40;
			LPC_GPIO2->FIOCLR0 = 0x80;
			v_delay_mdelay(2);
			LPC_SSP1->DR = ch;
			break;

		case 6:					      //TEMPERATURE
			ch = 0x00009C00;
			LPC_SSP1->DR = ch;
			break;

		case 7:						 //S_PE
			ch = 0x00008C00;
			LPC_SSP1->DR = ch;
			break;
	}
	  
	v_delay_mdelay(1);

	//��ս��ջ�����
	cnt = 0;
	while ((LPC_SSP1->SR & 0x00000004) != 0)
	{
		data[0] = LPC_SSP1->DR;
		if (++cnt > 8)
			break;
	}

	for(i=0;i<30;i++)				   //�����ɼ�30��
	{
		LPC_SSP1->DR = ch;
		v_delay_udelay(500);

		cnt = 0;
	 	while ((LPC_SSP1->SR & 0x00000010) == 0x00000010) //����֡δ��/�������ȴ�
		{
			v_delay_udelay(50);
			if (++cnt > 1000)
			break;
		}

		data[i] = (LPC_SSP1->DR);			   //ȡ���ݴ�������
		data[i] = data[i]>>4;
/*		if (type == BATT1_CURR )
		{
			u32_usb_debug_print("%d\r\n", data[i]);
		}
*/
		
		v_delay_udelay(500);	
	}

//	u32_usb_debug_print( "\r\n" );

	/*�Բ����������ݽ����˲�����*/
	for(j=0;j<29;j++)				   //�����ݰ���С����˳������
	{
		for(k=0;k<29-j;k++)
		{
			if(data[k]>data[k+1])
			{
				temp = data[k];
				data[k] = data[k+1];
				data[k+1] = temp;
			}
		}
	}

	for(i=10;i<20;i++)				   //ȥ��10����С��10���
	{
		datasum += data[i];
	}
	ad = (U32_T)(datasum/10);					   //��ƽ��ֵ


	switch(type)
	{
		case 0:						  //S_HM
			if(ad>g_pb_v_ad+AD_LIMIT)
			{
				g_pb_v_t1++;
				g_pb_v_t2=0;
				if(g_pb_v_t1>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_pb_v_ad=ad;
					g_pb_v_t1=0;
				}
				else
				{
				 	valid_data=g_pb_v_ad;
				}			
			}
			else if(ad+AD_LIMIT<g_pb_v_ad)
			{
				g_pb_v_t2++;
				g_pb_v_t1=0;
				if(g_pb_v_t2>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_pb_v_ad=ad;
					g_pb_v_t2=0;
				}
				else
				{
				 	valid_data=g_pb_v_ad;
				}	
			}
			else if((ad+AD_LIMIT>=g_pb_v_ad)&&(ad<=g_pb_v_ad+AD_LIMIT))
			{
				g_pb_v_t2=0;
				g_pb_v_t1=0;
				valid_data=g_pb_v_ad;
			}		
			break;

		case 1:						  //S_KM
			if(ad>g_cb_v_ad+AD_LIMIT)
			{
				g_cb_v_t1++;
				g_cb_v_t2=0;
				if(g_cb_v_t1>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_cb_v_ad=ad;
					g_cb_v_t1=0;
				}
				else
				{
				 	valid_data=g_cb_v_ad;
				}			
			}
			else if(ad+AD_LIMIT<g_cb_v_ad) 
			{
				g_cb_v_t2++;
				g_cb_v_t1=0;
				if(g_cb_v_t2>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_cb_v_ad=ad;
					g_cb_v_t2=0;
				}
				else
				{
				 	valid_data=g_cb_v_ad;
				}	
			}
			else if((ad+AD_LIMIT>=g_cb_v_ad)&&(ad<=g_cb_v_ad+AD_LIMIT))
			{
				g_cb_v_t2=0;
				g_cb_v_t1=0;
				valid_data=g_cb_v_ad;
			}		
			break;

		case 2:						  //S_BAT
			if(ad>g_batt_v_ad+AD_LIMIT)
			{
				g_batt_v_t1++;
				g_batt_v_t2=0;
				if(g_batt_v_t1>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_batt_v_ad=ad;
					g_batt_v_t1=0;
				}
				else
				{
				 	valid_data=g_batt_v_ad;
				}			
			}
			else if(ad+AD_LIMIT<g_batt_v_ad)
			{
				g_batt_v_t2++;
				g_batt_v_t1=0;
				if(g_batt_v_t2>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_batt_v_ad=ad;
					g_batt_v_t2=0;
				}
				else
				{
				 	valid_data=g_batt_v_ad;
				}	
			}
			else if((ad+AD_LIMIT>=g_batt_v_ad)&&(ad<=g_batt_v_ad+AD_LIMIT))
			{
				g_batt_v_t2=0;
				g_batt_v_t1=0;
				valid_data=g_batt_v_ad;
			}		
			break;

		case 3:						  //LOAD_I
			if(ad>g_load_c_ad+AD_LIMIT)
			{
				g_load_c_t1++;
				g_load_c_t2=0;
				if(g_load_c_t1>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_load_c_ad=ad;
					g_load_c_t1=0;
				}
				else
				{
				 	valid_data=g_load_c_ad;
				}		
			}
			else if(ad+AD_LIMIT<g_load_c_ad)
			{
				g_load_c_t2++;
				g_load_c_t1=0;
				if(g_load_c_t2>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_load_c_ad=ad;
					g_load_c_t2=0;
				}
				else
				{
				 	valid_data=g_load_c_ad;
				}	
			}
			else if((ad+AD_LIMIT>=g_load_c_ad)&&(ad<=g_load_c_ad+AD_LIMIT))
			{
				g_load_c_t2=0;
				g_load_c_t1=0;
				valid_data=g_load_c_ad;
			}		
			break;

		case 4:						  //BAT1_I
			if(ad>g_batt_c_ad+AD_LIMIT)
			{
				g_batt_c_t1++;
				g_batt_c_t2=0;
				if(g_batt_c_t1>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_batt_c_ad=ad;
					g_batt_c_t1=0;
				}
				else
				{
				 	valid_data=g_batt_c_ad;
				}		
			}
			else if(ad+AD_LIMIT<g_batt_c_ad)
			{
				g_batt_c_t2++;
				g_batt_c_t1=0;
				if(g_batt_c_t2>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_batt_c_ad=ad;
					g_batt_c_t2=0;
				}
				else
				{
				 	valid_data=g_batt_c_ad;
				}	
			}
			else if((ad+AD_LIMIT>=g_batt_c_ad)&&(ad<=g_batt_c_ad+AD_LIMIT))
			{
				g_batt_c_t2=0;
				g_batt_c_t1=0;
				valid_data=g_batt_c_ad;
			}	
			break;

		case 5:						  //BAT2_I
			if(ad>g_rect_c_ad+AD_LIMIT)
			{
				g_rect_c_t1++;
				g_rect_c_t2=0;
				if(g_rect_c_t1>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_rect_c_ad=ad;
					g_rect_c_t1=0;
				}
				else
				{
				 	valid_data=g_rect_c_ad;
				}		
			}
			else if(ad+AD_LIMIT<g_rect_c_ad)
			{
				g_rect_c_t2++;
				g_rect_c_t1=0;
				if(g_rect_c_t2>=LIMIT_TIME)
				{
				 	valid_data=ad;
					g_rect_c_ad=ad;
					g_rect_c_t2=0;
				}
				else
				{
				 	valid_data=g_rect_c_ad;
				}	
			}
			else if((ad+AD_LIMIT>=g_rect_c_ad)&&(ad<=g_rect_c_ad+AD_LIMIT))
			{
				g_rect_c_t2=0;
				g_rect_c_t1=0;
				valid_data=g_rect_c_ad;
			}		
			break;

		case 6:					      //TEMPERATURE
			valid_data=ad;
			break;

		case 7:					      //ZERO_POINT
			valid_data=ad;
			break;
	}
	return valid_data;	

 }


