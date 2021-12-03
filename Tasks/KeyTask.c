#include "RTL.h"
#include "PublicData.h"
#include "type.h"
#include "../Drivers/key.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Delay.h"



/********************************************
 *  �������ƣ� __task void v_key_keytask(void)
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:   ����ֵ
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   �ɼ����������ݣ����ؼ�ֵ�����ȼ�鵽�а������¾ͷ��ص�ǰ�ļ�ֵ
 ����ļ�ֵ�Ͳ�ɨ�衣
*********************************************/

__task void v_key_keytask(void)
{
	U32_T data,i,reg;
	data = 0;
	reg = 0;
	while (1) 
	{
		os_evt_set(KEY_FEED_DOG, g_tid_wdt);             //����ι���¼���־

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
							os_evt_set(KEY_FEED_DOG, g_tid_wdt);             //����ι���¼���־

						   	os_dly_wait(2);
						}
						reg=0;			
			     		os_evt_set (0x0001<<i, g_tid_display);	//���ü�ֵ�ڴ˴�

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

