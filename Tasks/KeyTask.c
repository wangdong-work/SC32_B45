#include "RTL.h"
#include "PublicData.h"
#include "type.h"
#include "../Drivers/key.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Delay.h"



/********************************************
 *  函数名称： __task void v_key_keytask(void)
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:   按键值
 *  全局变量:	无
 *  功能介绍:   采集按键的数据，返回键值，首先检查到有按键按下就返回当前的键值
 后面的键值就不扫描。
*********************************************/

__task void v_key_keytask(void)
{
	U32_T data,i,reg;
	data = 0;
	reg = 0;
	while (1) 
	{
		os_evt_set(KEY_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志

		data = key_key_data();
		if (data&0x3f) 
		{		    
			os_dly_wait(2);
			reg = key_key_data()&data;
			if(reg)
			{
				v_relay_relay_operation(BEEP_ON);
				//os_dly_wait(3);
				v_delay_mdelay(30);
				v_relay_relay_operation(BEEP_OFF);

				for(i=0;i<6;i++)
				{ 
					if(reg&(1<<i))
					{ 
						while(key_key_data()&(1<<i))
						{
							os_evt_set(KEY_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志

						   	os_dly_wait(2);
						}
						reg=0;			
			     		os_evt_set (0x0001<<i, g_tid_display);	//设置键值在此处

					}
					else
					{
						continue; 
					}   
				}		
			}
		}
		else
		{
			os_dly_wait(5);
		}
	}  
}    

