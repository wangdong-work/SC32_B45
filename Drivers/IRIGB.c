/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����IRIGB.c
��    ����1.00
�������ڣ�2012-03-15
��    �ߣ�������
����������B���ʱ����ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-03-15  1.00     ����
**************************************************************/

#include <lpc17xx.h>
#include <string.h>

#include "IRIGB.h"


#define RIT_10MS_COUNTER (18000*10)
#define RIT_2MS_COUNTER  (18000*2)
#define RIT_5MS_COUNTER  (18000*5)
#define RIT_8MS_COUNTER  (18000*8)
#define RIT_DIFF         (18000/5)


#define EINT1_SET_MODE(x) { 	U32_T reg = LPC_SC->EXTMODE; \
								reg &= (~(1<<1)); \
								reg |= (x<<1); \
								LPC_SC->EXTMODE = reg; \
							}

#define EINT1_SET_POLAR(x) { 	U32_T reg = LPC_SC->EXTPOLAR; \
								reg &= (~(1<<1)); \
								reg |= (x<<1); \
								LPC_SC->EXTPOLAR = reg; \
							}

#define EINT1_CLR_INT() (LPC_SC->EXTINT = (1<<1))


typedef enum
{
	LS_NONE,
	LS_PRE,
	LS_POSITIVE,
	LS_NEGATIVE
}LS_TYPE_E;

typedef enum
{
	BIT_STATUS_NONE,
	BIT_STATUS_CODE_P,
	BIT_STATUS_CODE_1,
	BIT_STATUS_CODE_0
}BIT_STATUS_E;

typedef enum
{
	CODE_E,
	CODE_P,
	CODE_1,
	CODE_0
}BIT_CODE_E;


typedef struct
{
	IRIGB_TIME_T t_time_data;
	IRIGB_TIME_T t_time_tmp;

	S32_T s32_bit_count;
	LS_TYPE_E e_ls;
	BIT_STATUS_E e_status;
}IRIGB_DATA_T;


static IRIGB_DATA_T t_data;


/*************************************************************
��������: v_irigb_eint1_enable_rising
��������: �����ⲿ�ж�1Ϊ�������ж�
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
static void v_irigb_eint1_enable_rising(void)
{
	NVIC_DisableIRQ(EINT1_IRQn);	     //�ر��ж�
	EINT1_SET_MODE(1);                   //�����ж�
	EINT1_SET_POLAR(1);	                 //������
	EINT1_CLR_INT();                     //����жϱ�־
	NVIC_ClearPendingIRQ(EINT1_IRQn);    //��������־
	NVIC_EnableIRQ(EINT1_IRQn);	         //ʹ���ж�
}

/*************************************************************
��������: v_irigb_eint1_enable_falling
��������: �����ⲿ�ж�1Ϊ�½����ж�
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
static void v_irigb_eint1_enable_falling(void)
{
	NVIC_DisableIRQ(EINT1_IRQn);	     //�ر��ж�
	EINT1_SET_MODE(1);                   //�����ж�
	EINT1_SET_POLAR(0);	                 //�½���
	EINT1_CLR_INT();                     //����жϱ�־
	NVIC_ClearPendingIRQ(EINT1_IRQn);    //��������־
	NVIC_EnableIRQ(EINT1_IRQn);	         //ʹ���ж�
}

/*************************************************************
��������: u32_irigb_rit_get_counter
��������: ��ȡTIMER�ļ���ֵ
�������: ��
�������: ��
����ֵ  ����������ֵ
**************************************************************/
static U32_T u32_irigb_rit_get_counter(void)
{
	U32_T reg;

	NVIC_DisableIRQ(RIT_IRQn);
	LPC_RIT->RICTRL = 0x03;

	reg = LPC_RIT->RICOUNTER;

	LPC_RIT->RICOUNTER = 0;
	LPC_RIT->RICTRL = 0x0F;
	NVIC_ClearPendingIRQ(RIT_IRQn);    //��������־
	NVIC_EnableIRQ(RIT_IRQn);

	return reg;
}

/*************************************************************
��������: v_irigb_rit_timer_disable
��������: �ر�TIMER
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
static void v_irigb_rit_timer_disable(void)
{
	NVIC_DisableIRQ(RIT_IRQn);
	LPC_RIT->RICTRL = 0x03;
	LPC_RIT->RICOMPVAL = 18000*10;	   // cclk/4, 10ms
	LPC_RIT->RIMASK = 0x00000000;
	LPC_RIT->RICOUNTER = 0;
	NVIC_ClearPendingIRQ(RIT_IRQn);    //��������־
}

/*************************************************************
��������: u32_irigb_get_pin_status
��������: �õ�B���ʱ���ŵĵ�ƽ
�������: ��
�������: ��
����ֵ  ��1���ߵ�ƽ��0���͵�ƽ
**************************************************************/
static U32_T u32_irigb_get_pin_status(void)
{
	if ((LPC_GPIO2->FIOPIN & (1<<11)) != 0)
		return 1;
	else
		return 0;
}

/*************************************************************
��������: u16_irigb_day_of_year
��������: ����һ���������
�������: ��
�������: ��
����ֵ  ��һ���������
**************************************************************/
static U16_T u16_irigb_day_of_year(U8_T year)
{
	int y = 2000 + year;

	if ((y%4==0 && y%100!=0) || y%400==0)
		return 366;
	else
		return 365;
}

/*************************************************************
��������: v_irigb_get_mon_day
��������: �������һ���е�ĳһ�������·ݺ�����
�������: year -- ���
          days -- һ���еĵڼ���
�������: mon  -- �·�
          day  -- һ�����еĵڼ���
����ֵ  ����
**************************************************************/
static void v_irigb_get_mon_day(U8_T year, U16_T days, U8_T *mon, U16_T *day)
{
	U8_T day_of_mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	U32_T i, sum = 0;;

	if ((year%4==0 && year%100!=0) || year%400==0)
		day_of_mon[1] = 29;
		
	for (i=0; i<12; i++)
	{
		if (days <= sum+day_of_mon[i])
		{
			*mon = i+1;
			*day = days - sum;
			break;
		}
			
	  	sum += day_of_mon[i];
	}
}

/*************************************************************
��������: v_irigb_decode
��������: ����λ��Ԫ����
�������: e_code -- ��Ԫ
�������: ��
����ֵ  ����
**************************************************************/
static void v_irigb_decode(BIT_CODE_E e_code)
{
	if (t_data.s32_bit_count == -1)
	{
		if (e_code == CODE_P)
		{
			t_data.s32_bit_count = 0;
			return;
		}
	}

	else if (t_data.s32_bit_count==0 || t_data.s32_bit_count==9 || t_data.s32_bit_count==19 || t_data.s32_bit_count==29
			|| t_data.s32_bit_count==39 || t_data.s32_bit_count==49 || t_data.s32_bit_count==59 || t_data.s32_bit_count==69
			|| t_data.s32_bit_count==79 || t_data.s32_bit_count==89 || t_data.s32_bit_count==99)
	{
		if (e_code !=  CODE_P)
		{
			t_data.s32_bit_count = -1;
		}
	}
	
	else if (t_data.s32_bit_count==5 || t_data.s32_bit_count==14 || t_data.s32_bit_count==18 || t_data.s32_bit_count==24
			|| t_data.s32_bit_count==27 || t_data.s32_bit_count==28 || t_data.s32_bit_count==34
			|| (t_data.s32_bit_count>=42 &&t_data.s32_bit_count<=48) || t_data.s32_bit_count==54 || t_data.s32_bit_count==98)
	{
		if (e_code != CODE_0)
		{
			t_data.s32_bit_count = -1;
		}
	}
	
	else if ((t_data.s32_bit_count>=1 && t_data.s32_bit_count<=4) || (t_data.s32_bit_count>=6 && t_data.s32_bit_count<=8))
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			t_data.t_time_tmp.sec >>= 1;
			if (e_code == CODE_1)
				t_data.t_time_tmp.sec |= 0x80;
			
			if (t_data.s32_bit_count == 8)
			{
				t_data.t_time_tmp.sec >>= 1;
				t_data.t_time_tmp.sec = ((t_data.t_time_tmp.sec&0x70)>>4)*10+(t_data.t_time_tmp.sec&0x0F);

				if (t_data.t_time_tmp.sec > 60)
				{
					t_data.s32_bit_count = -1;
				}
			}
		}
		
		else
		{
			t_data.s32_bit_count = -1;
		}
	}
	
	else if ((t_data.s32_bit_count>=10 && t_data.s32_bit_count<=13) || (t_data.s32_bit_count>=15 && t_data.s32_bit_count<=17))
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			t_data.t_time_tmp.min >>= 1;
			if (e_code == CODE_1)
				t_data.t_time_tmp.min |= 0x80;
			
			if (t_data.s32_bit_count == 17)
			{
				t_data.t_time_tmp.min >>= 1;
				t_data.t_time_tmp.min = ((t_data.t_time_tmp.min&0x70)>>4)*10+(t_data.t_time_tmp.min&0x0F);
				if (t_data.t_time_tmp.min > 59)
				{
					t_data.s32_bit_count = -1;
				}
			}
		}
		else
		{
			t_data.s32_bit_count = -1;
		}
	}
	
	else if ((t_data.s32_bit_count>=20 && t_data.s32_bit_count<=23) || t_data.s32_bit_count==25 || t_data.s32_bit_count==26)
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			t_data.t_time_tmp.hour >>= 1;
			if (e_code == CODE_1)
				t_data.t_time_tmp.hour |= 0x80;
			
			if (t_data.s32_bit_count == 26)
			{
				t_data.t_time_tmp.hour >>= 2;
				t_data.t_time_tmp.hour = ((t_data.t_time_tmp.hour&0x30)>>4)*10+(t_data.t_time_tmp.hour&0x0F);
				if (t_data.t_time_tmp.hour > 23)
				{
					t_data.s32_bit_count = -1;
				}
			}
		}

		else
		{
			t_data.s32_bit_count = -1;
		}
	}
	
	else if ((t_data.s32_bit_count>=30 && t_data.s32_bit_count<=33) ||(t_data.s32_bit_count>=35 && t_data.s32_bit_count<=38)
			|| t_data.s32_bit_count==40 || t_data.s32_bit_count==41)
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			t_data.t_time_tmp.day >>= 1;
			if (e_code == CODE_1)
				t_data.t_time_tmp.day |= 0x8000;
			
			if (t_data.s32_bit_count == 41)
			{
				t_data.t_time_tmp.day >>= 6;
				t_data.t_time_tmp.day = ((t_data.t_time_tmp.day&0x300)>>8)*100 + ((t_data.t_time_tmp.day&0xF0)>>4)*10 + (t_data.t_time_tmp.day&0x0F);
				if (t_data.t_time_tmp.day > 366)
				{
					t_data.s32_bit_count = -1;
				}
			}
		}

		else
		{
			t_data.s32_bit_count = -1;
		}
	}
	
	else if ((t_data.s32_bit_count>=50 && t_data.s32_bit_count<=53) || (t_data.s32_bit_count>=55 && t_data.s32_bit_count<=58))
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			t_data.t_time_tmp.year >>= 1;
			if (e_code == CODE_1)
				t_data.t_time_tmp.year |= 0x80;
			
			if (t_data.s32_bit_count == 58)
			{
				t_data.t_time_tmp.year = ((t_data.t_time_tmp.year&0xF0)>>4)*10+(t_data.t_time_tmp.year&0x0F);
			}
		}

		else
		{
			t_data.s32_bit_count = -1;
		}
	}

	else if (t_data.s32_bit_count==60)
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			if (t_data.t_time_tmp.sec==57)
			{
				if (e_code == CODE_1)
					t_data.e_ls = LS_PRE;
				else
					t_data.e_ls = LS_NONE;
			}
		}
		else
		{
			t_data.s32_bit_count = -1;
		}
	}

	else if (t_data.s32_bit_count==61)
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			if (t_data.t_time_tmp.sec==57 && t_data.e_ls==LS_PRE)
			{
				if (e_code == CODE_1)
					t_data.e_ls = LS_POSITIVE;
				else
					t_data.e_ls = LS_NEGATIVE;
			}
		}
		else
		{
			t_data.s32_bit_count = -1;
		}
	}
	
	else if (t_data.s32_bit_count>=71 && t_data.s32_bit_count<=74)
	{
		if (e_code == CODE_1 || e_code == CODE_0)
		{
			t_data.t_time_tmp.quality >>= 1;
			if (e_code == CODE_1)
				t_data.t_time_tmp.quality |= 0x80;
			
			if (t_data.s32_bit_count == 74)
			{
				t_data.t_time_tmp.quality >>= 4;
				t_data.t_time_tmp.quality = (t_data.t_time_tmp.quality&0x0F);
			}
		}
		else
		{
			t_data.s32_bit_count = -1;
		}
	}

	else
	{
		if (e_code == CODE_E)
			t_data.s32_bit_count = -1;
	}
	
	if (t_data.s32_bit_count != -1)
	{
		t_data.s32_bit_count++;
		if (t_data.s32_bit_count == 100)
		{
			U8_T sec_count;

			t_data.s32_bit_count = 0;
			t_data.t_time_data = t_data.t_time_tmp;
			t_data.t_time_data.ms = 0;

			if (t_data.e_ls == LS_POSITIVE) 
				sec_count = 60;
			else if (t_data.e_ls == LS_NEGATIVE) 
				sec_count = 58;
			else
				sec_count = 59;

			if (++t_data.t_time_data.sec > sec_count)
			{
				t_data.t_time_data.sec = 0;
				if (++t_data.t_time_data.min > 59)
				{
					t_data.t_time_data.min = 0;
					if (++t_data.t_time_data.hour > 23)
					{
						t_data.t_time_data.hour = 0;
						if (++t_data.t_time_data.day > u16_irigb_day_of_year(t_data.t_time_data.year))
						{
							t_data.t_time_data.day = 1;
							if (++t_data.t_time_data.year > 99)
								t_data.t_time_data.year = 0;
						}
					}
				}
			}

			v_irigb_get_mon_day(t_data.t_time_data.year, t_data.t_time_data.day, &(t_data.t_time_data.mon), &(t_data.t_time_data.day));
		}
		else
		{
			t_data.t_time_data.ms += 10;
		}
	}
	else
	{
		t_data.t_time_data.quality = QUALITY_BAD;
	}
}

/*************************************************************
��������: EINT1_IRQHandler
��������: �ⲿ�ж�1�жϴ�����
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void EINT1_IRQHandler(void)
{
	U32_T pin;
	U32_T time;

	time = u32_irigb_rit_get_counter();

	__NOP();
	__NOP();
	pin = u32_irigb_get_pin_status();

	if (pin == 1)
	{
		switch (t_data.e_status)
		{
			case BIT_STATUS_CODE_P:
				if ((time>RIT_2MS_COUNTER-RIT_DIFF) && (time<RIT_2MS_COUNTER+RIT_DIFF))
					v_irigb_decode(CODE_P);
				else
					v_irigb_decode(CODE_E);
				break;
					
			case BIT_STATUS_CODE_1:
				if ((time>RIT_5MS_COUNTER-RIT_DIFF) && (time<RIT_5MS_COUNTER+RIT_DIFF))
					v_irigb_decode(CODE_1);
				else
					v_irigb_decode(CODE_E);
				break;

			case BIT_STATUS_CODE_0:
				if ((time>RIT_8MS_COUNTER-RIT_DIFF) && (time<RIT_8MS_COUNTER+RIT_DIFF))
					v_irigb_decode(CODE_0);
				else
					v_irigb_decode(CODE_E);
				break;

			case BIT_STATUS_NONE:
			default:
				break;
		}

		v_irigb_eint1_enable_falling();                  //ʹ���½����ж�
	}
	else
	{
		if ((time>RIT_8MS_COUNTER-RIT_DIFF) && (time<RIT_8MS_COUNTER+RIT_DIFF))
			t_data.e_status = BIT_STATUS_CODE_P;
		else if ((time>RIT_5MS_COUNTER-RIT_DIFF) && (time<RIT_5MS_COUNTER+RIT_DIFF))
			t_data.e_status = BIT_STATUS_CODE_1;
		else if ((time>RIT_2MS_COUNTER-RIT_DIFF) && (time<RIT_2MS_COUNTER+RIT_DIFF))
			t_data.e_status = BIT_STATUS_CODE_0;
		else
		{
			t_data.e_status = BIT_STATUS_NONE;
			v_irigb_decode(CODE_E);
		}
		 
		v_irigb_eint1_enable_rising();
	}
}

/*************************************************************
��������: RIT_IRQHandler
��������: RIT�жϴ�����
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void RIT_IRQHandler(void)
{
	v_irigb_rit_timer_disable();
	v_irigb_eint1_enable_rising();

	t_data.e_status = BIT_STATUS_NONE;
	v_irigb_decode(CODE_E);
}

/*************************************************************
��������: v_irigb_compare_time_init
��������: B���ʱ��ʼ������
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_irigb_compare_time_init(void)
{
	LPC_GPIO4 -> FIODIR |= 0x10000000;   //UART2_DIR��Ϊ����ߵ�ƽ��оƬ75176Ϊ����״̬
	LPC_GPIO4 -> FIOSET = 0x10000000;

	v_irigb_rit_timer_disable();
	v_irigb_eint1_enable_rising();

	memset(&t_data, 0, sizeof(t_data));
	t_data.s32_bit_count = -1;
	t_data.e_status = BIT_STATUS_NONE;
	t_data.e_ls = LS_NONE;
	t_data.t_time_data.quality = QUALITY_BAD;
}

/*************************************************************
��������: v_irigb_compare_time_read
��������: B���ʱ��ȡʱ�亯��
�������: ��
�������: ��
����ֵ  ����
**************************************************************/
void v_irigb_compare_time_read(IRIGB_TIME_T *time)
{
	NVIC_DisableIRQ(EINT1_IRQn);
	NVIC_DisableIRQ(RIT_IRQn);

	*time = t_data.t_time_data;

	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(RIT_IRQn);
}
