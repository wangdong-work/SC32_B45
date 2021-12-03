/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Relay.c
版    本：1.00
创建日期：2012-03-26
作    者：郭数理
功能描述：继电器输出实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-26  1.00     创建
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
 
static U16_T m_u16_relay_val = 0x0008;    //继电器1~6不动作，7、8动作，系统总故障继电器动作
static OS_MUT m_mut_relay;

static void v_relay_relay_output(U16_T val);

/*************************************************************
函数名称: v_relay_relay_init		           				
函数功能: 初始化继电器相关的IO端口						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
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
	RLY_OUT_ENB_LOW();   //输出使能
}

/*************************************************************
函数名称: v_relay_relay_init_mutex		           				
函数功能: 初始化继电器相关的互斥量，在RTX启动之后调用，在第一个任务的开头部分调用						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_relay_relay_init_mutex(void)
{
	os_mut_init(m_mut_relay);             //初始化互斥量
}	

/*************************************************************
函数名称: v_relay_relay_output		           				
函数功能: 输出继电器控制数据						
输入参数: u16_val -- 要输出的数据       		   				
输出参数: 无
返回值  ：无														   				
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
函数名称: v_relay_relay_operation		           				
函数功能: 继电器操作函数						
输入参数: e_cmd -- 操作命令，请看RLY_CMD命令定义       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_relay_relay_operation(RLY_CMD_E e_cmd)
{
	U8_T u8_out_flag = 0;

	os_mut_wait(m_mut_relay, 0xffff);     //操作前获取互斥量

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

	os_mut_release(m_mut_relay);          //操作后释放互斥量
}

/*************************************************************
函数名称: v_relay_get_diode_chain_status		           				
函数功能: 获取硅链控制继电器的状态						
输入参数: 无        		   				
输出参数: 无
返回值  ：硅链控制继电器的状态，
          bit0:第一个继电器状态（0-断开，1-闭合），
		  bit1:第二个继电器状态（0-断开，1-闭合），
		  bit2:第三个继电器状态（0-断开，1-闭合）													   				
**************************************************************/
U8_T u8_relay_get_diode_chain_status(void)
{
	U8_T u8_ret;

	os_mut_wait(m_mut_relay, 0xffff);     //操作前获取互斥量
	u8_ret = (U8_T)((m_u16_relay_val >> 7) & 0x07);
	os_mut_release(m_mut_relay);          //操作后释放互斥量

	return u8_ret;
}

