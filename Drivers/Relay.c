/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����Relay.c
��    ����1.00
�������ڣ�2012-03-26
��    �ߣ�������
�����������̵������ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-03-26  1.00     ����
**************************************************************/

#include <lpc17xx.h>
#include <rtl.h>
#include "Delay.h"

#include "Relay.h"
#include "Type.h"


#define BEEP_OFFSET         0
#define ALARM_LED_OFFSET    1
#define BACKLIGHT_OFFSET    2
#define SYS_FAIL_OFFSET     3
#define ALARM_RELAY1_OFFSET 4
#define ALARM_RELAY2_OFFSET 5
#define ALARM_RELAY3_OFFSET 6
#define ALARM_RELAY4_OFFSET 7
#define ALARM_RELAY5_OFFSET 8
#define ALARM_RELAY6_OFFSET 9
#define AC_CTR_CH1_OFFSET   10
#define AC_CTR_CH2_OFFSET   11



#define RLY_DATA_HIGH() (LPC_GPIO2->FIOSET = (1<<9))
#define RLY_DATA_LOW() (LPC_GPIO2->FIOCLR = (1<<9))

#define RLY_LOCK_HIGH() (LPC_GPIO3->FIOSET = (1<<25))
#define RLY_LOCK_LOW() (LPC_GPIO3->FIOCLR = (1<<25))

#define RLY_CLK_HIGH() (LPC_GPIO3->FIOSET = (1<<26))
#define RLY_CLK_LOW() (LPC_GPIO3->FIOCLR = (1<<26))

#define RLY_OUT_ENB_HIGH() (LPC_GPIO2->FIOSET = (1<<4))
#define RLY_OUT_ENB_LOW() (LPC_GPIO2->FIOCLR = (1<<4)) 


static U32_T m_u32_relay1_cnt = 0;
static U32_T m_u32_relay2_cnt = 0;
static U32_T m_u32_relay3_cnt = 0;
static U32_T m_u32_relay4_cnt = 0;
static U32_T m_u32_relay5_cnt = 0;
static U32_T m_u32_relay6_cnt = 0;
 
static U16_T m_u16_relay_val = 0x0008;    //�̵���1~6��������7��8������ϵͳ�ܹ��ϼ̵�������
static OS_MUT m_mut_relay;

static void v_relay_relay_output(U16_T val);

/*************************************************************
��������: v_relay_relay_init		           				
��������: ��ʼ���̵�����ص�IO�˿�						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_relay_relay_init(void)
{
	U32_T reg;

	reg = LPC_GPIO2->FIODIR;
	reg |= ((1 << 9) | (1 << 4));
	LPC_GPIO2->FIODIR = reg;

	reg = LPC_GPIO3->FIODIR;
	reg |= (3 << 25);
	LPC_GPIO3->FIODIR = reg;

	v_relay_relay_output(m_u16_relay_val);
	RLY_OUT_ENB_LOW();   //���ʹ��
}

/*************************************************************
��������: v_relay_relay_init_mutex		           				
��������: ��ʼ���̵�����صĻ���������RTX����֮����ã��ڵ�һ������Ŀ�ͷ���ֵ���						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_relay_relay_init_mutex(void)
{
	os_mut_init(m_mut_relay);             //��ʼ��������
}	

/*************************************************************
��������: v_relay_relay_output		           				
��������: ����̵�����������						
�������: u16_val -- Ҫ���������       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
static void v_relay_relay_output(U16_T u16_val)
{
	U32_T i;
	U16_T v = u16_val;

	RLY_LOCK_LOW();
	v_delay_udelay(1);

	for (i=0; i<16; i++)
	{
		RLY_CLK_LOW();
		v_delay_udelay(1);

		if ((v & 0x8000) != 0)
			RLY_DATA_HIGH();
		else
			RLY_DATA_LOW();
		v_delay_udelay(1);

		RLY_CLK_HIGH();
		v_delay_udelay(2);

		v <<= 1;
	}

	RLY_CLK_LOW();
	v_delay_udelay(1);

	RLY_LOCK_HIGH();
	v_delay_udelay(2);
	RLY_LOCK_LOW();
}

/*************************************************************
��������: v_relay_relay_operation		           				
��������: �̵�����������						
�������: e_cmd -- ��������뿴RLY_CMD�����       		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_relay_relay_operation(RLY_CMD_E e_cmd)
{
	U8_T u8_out_flag = 0;

	os_mut_wait(m_mut_relay, 0xffff);     //����ǰ��ȡ������

	switch (e_cmd)
	{
		case BEEP_ON:
			m_u16_relay_val |= (1 << BEEP_OFFSET);
			u8_out_flag = 1;
			break;

		case BEEP_OFF:
			m_u16_relay_val &= (~(1 << BEEP_OFFSET));
			u8_out_flag = 1;
			break;

		case ALARM_LED_ON:
			m_u16_relay_val |= (1 << ALARM_LED_OFFSET);
			u8_out_flag = 1;
			break;

		case ALARM_LED_OFF:
			m_u16_relay_val &= (~(1 << ALARM_LED_OFFSET));
			u8_out_flag = 1;
			break;

		case DISABLE_BACKLIGHT:
			m_u16_relay_val &= (~(1 << BACKLIGHT_OFFSET));
			u8_out_flag = 1;
			break;

		case ENABLE_BACKLIGHT:
			m_u16_relay_val |= (1 << BACKLIGHT_OFFSET);
			u8_out_flag = 1;
			break;

		case SYS_FIAL_ON:
			m_u16_relay_val |= (1 << SYS_FAIL_OFFSET);
			u8_out_flag = 1;
			break;

		case SYS_FIAL_OFF:
			m_u16_relay_val &= (~(1 << SYS_FAIL_OFFSET));
			u8_out_flag = 1;
			break;

		case ALARM_RELAY1_ON:
			m_u32_relay1_cnt++;
			if (m_u32_relay1_cnt > 0)
			{
				m_u16_relay_val |= (1 << ALARM_RELAY1_OFFSET);
				u8_out_flag = 1;
			}
			break;

		case ALARM_RELAY1_OFF:
			if (m_u32_relay1_cnt > 0)
			{
				m_u32_relay1_cnt--;
				if (m_u32_relay1_cnt == 0)
				{
					m_u16_relay_val &= (~(1 << ALARM_RELAY1_OFFSET));
					u8_out_flag = 1;
				}
			}
			break;

		case ALARM_RELAY2_ON:
			m_u32_relay2_cnt++;
			if (m_u32_relay2_cnt > 0)
			{
				m_u16_relay_val |= (1 << ALARM_RELAY2_OFFSET);
				u8_out_flag = 1;
			}
			break;

		case ALARM_RELAY2_OFF:
			if (m_u32_relay2_cnt > 0)
			{
				m_u32_relay2_cnt--;
				if (m_u32_relay2_cnt == 0)
				{
					m_u16_relay_val &= (~(1 << ALARM_RELAY2_OFFSET));
					u8_out_flag = 1;
				}
			}
			break;

		case ALARM_RELAY3_ON:
			m_u32_relay3_cnt++;
			if (m_u32_relay3_cnt > 0)
			{
				m_u16_relay_val |= (1 << ALARM_RELAY3_OFFSET);
				u8_out_flag = 1;
			}
			break;

		case ALARM_RELAY3_OFF:
			if (m_u32_relay3_cnt > 0)
			{
				m_u32_relay3_cnt--;
				if (m_u32_relay3_cnt == 0)
				{
					m_u16_relay_val &= (~(1 << ALARM_RELAY3_OFFSET));
					u8_out_flag = 1;
				}
			}
			break;

		case ALARM_RELAY4_ON:
			m_u32_relay4_cnt++;
			if (m_u32_relay4_cnt > 0)
			{
				m_u16_relay_val |= (1 << ALARM_RELAY4_OFFSET);
				u8_out_flag = 1;
			}
			break;

		case ALARM_RELAY4_OFF:
			if (m_u32_relay4_cnt > 0)
			{
				m_u32_relay4_cnt--;
				if (m_u32_relay4_cnt == 0)
				{
					m_u16_relay_val &= (~(1 << ALARM_RELAY4_OFFSET));
					u8_out_flag = 1;
				}
			}
			break;

		case ALARM_RELAY5_ON:
			m_u32_relay5_cnt++;
			if (m_u32_relay5_cnt > 0)
			{
				m_u16_relay_val |= (1 << ALARM_RELAY5_OFFSET);
				u8_out_flag = 1;
			}
			break;

		case ALARM_RELAY5_OFF:
			if (m_u32_relay5_cnt > 0)
			{
				m_u32_relay5_cnt--;
				if (m_u32_relay5_cnt == 0)
				{
					m_u16_relay_val &= (~(1 << ALARM_RELAY5_OFFSET));
					u8_out_flag = 1;
				}
			}
			break;

		case ALARM_RELAY6_ON:
			m_u32_relay6_cnt++;
			if (m_u32_relay6_cnt > 0)
			{
				m_u16_relay_val |= (1 << ALARM_RELAY6_OFFSET);
				u8_out_flag = 1;
			}
			break;

		case ALARM_RELAY6_OFF:
			if (m_u32_relay6_cnt > 0)
			{
				m_u32_relay6_cnt--;
				if (m_u32_relay6_cnt == 0)
				{
					m_u16_relay_val &= (~(1 << ALARM_RELAY6_OFFSET));
					u8_out_flag = 1;
				}
			}
			break;

		case AC_CTR_CH1_ON:
			m_u16_relay_val &= (~(1 << AC_CTR_CH1_OFFSET));
			u8_out_flag = 1;
			break;

		case AC_CTR_CH1_OFF:
			m_u16_relay_val |= (1 << AC_CTR_CH1_OFFSET);
			u8_out_flag = 1;
			break;

		case AC_CTR_CH2_ON:
			m_u16_relay_val &= (~(1 << AC_CTR_CH2_OFFSET));
			u8_out_flag = 1;
			break;

		case AC_CTR_CH2_OFF:
			m_u16_relay_val |= (1 << AC_CTR_CH2_OFFSET);
			u8_out_flag = 1;
			break;

		default:
			break;
	}

	if (u8_out_flag != 0)
	{
		v_relay_relay_output(m_u16_relay_val);
		u8_out_flag = 0;
	}

	os_mut_release(m_mut_relay);          //�������ͷŻ�����
}

/*************************************************************
��������: v_relay_get_diode_chain_status		           				
��������: ��ȡ�������Ƽ̵�����״̬						
�������: ��        		   				
�������: ��
����ֵ  ���������Ƽ̵�����״̬��
          bit0:��һ���̵���״̬��0-�Ͽ���1-�պϣ���
		  bit1:�ڶ����̵���״̬��0-�Ͽ���1-�պϣ���
		  bit2:�������̵���״̬��0-�Ͽ���1-�պϣ�													   				
**************************************************************/
U8_T u8_relay_get_diode_chain_status(void)
{
	U8_T u8_ret;

	os_mut_wait(m_mut_relay, 0xffff);     //����ǰ��ȡ������
	u8_ret = (U8_T)((m_u16_relay_val >> 7) & 0x07);
	os_mut_release(m_mut_relay);          //�������ͷŻ�����

	return u8_ret;
}

