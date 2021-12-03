/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Relay.h
版    本：1.00
创建日期：2012-03-26
作    者：郭数理
功能描述：继电器输出头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-26  1.00     创建
**************************************************************/

#ifndef __RELAY_H_
#define __RELAY_H_


typedef enum
{
	BEEP_ON,
	BEEP_OFF,
	ALARM_LED_ON,
	ALARM_LED_OFF,
	DISABLE_BACKLIGHT,
	ENABLE_BACKLIGHT,
	SYS_FIAL_ON,
	SYS_FIAL_OFF,
	ALARM_RELAY1_ON,
	ALARM_RELAY1_OFF,
	ALARM_RELAY2_ON,
	ALARM_RELAY2_OFF,
	ALARM_RELAY3_ON,
	ALARM_RELAY3_OFF,
	ALARM_RELAY4_ON,
	ALARM_RELAY4_OFF,
	ALARM_RELAY5_ON,
	ALARM_RELAY5_OFF,
	ALARM_RELAY6_ON,
	ALARM_RELAY6_OFF,
	AC_CTR_CH1_ON,
	AC_CTR_CH1_OFF,
	AC_CTR_CH2_ON,
	AC_CTR_CH2_OFF,
}RLY_CMD_E;


/*************************************************************
函数名称: v_relay_relay_init		           				
函数功能: 初始化继电器相关的IO端口						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_relay_relay_init(void);

/*************************************************************
函数名称: v_relay_relay_init_mutex		           				
函数功能: 初始化继电器相关的互斥量，在RTX启动之后调用，在第一个任务的开头部分调用						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_relay_relay_init_mutex(void);


/*************************************************************
函数名称: v_relay_relay_operation		           				
函数功能: 继电器操作函数						
输入参数: e_cmd -- 操作命令，请看RLY_CMD命令定义       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_relay_relay_operation(RLY_CMD_E e_cmd);


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
U8_T u8_relay_get_diode_chain_status(void);


#endif
