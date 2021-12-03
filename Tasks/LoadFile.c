/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����LoadFile.c
��    ����1.00
�������ڣ�2012-07-28
��    �ߣ�������
����������ͨ��usbת���������ֿ⵽dataflash�����뵼��֧·���ơ��澯���ơ���̨����

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-07-28  1.00     ����
**************************************************************/


#include <rtl.h>
#include <string.h>
#include <stdlib.h>

#include "../Drivers/Usb.h"
#include "../Drivers/Dataflash.h"
#include "../Drivers/Delay.h"
#include "../Drivers/Wdt.h"

#include "Type.h"
#include "BackstageTable.h"
#include "Crc.h"
#include "LoadFile.h"
#include "Display.h"
#include "PublicData.h"
#include "Encry.h"



#define STF     0x01      //�ֿ���д
#define STN     0x02      //֧·������д
#define STA     0x03      //�澯������д
#define STWYC   0x04      //ң������д
#define STWYX   0x05      //ң�ŵ����д
#define STWYK   0x06      //ң�ص����д
#define STWYT   0x07      //ң�������д
#define STRCYC  0x08      //��ȡ�����������ɵ�ң����
#define STRCYX  0x09      //��ȡ�����������ɵ�ң�ŵ��
#define STRCYK  0x0A      //��ȡ�����������ɵ�ң�ص��
#define STRCYT  0x0B      //��ȡ�����������ɵ�ң�����
#define STRFYC  0x0C      //��ȡFLASH�е�ң����
#define STRFYX  0x0D      //��ȡFLASH�е�ң�ŵ��
#define STRFYK  0x0E      //��ȡFLASH�е�ң�ص��
#define STRFYT  0x0F      //��ȡFLASH�е�ң�����
#define STRFN   0x14      //��ȡFLASH��֧·����
#define STRFA   0x15      //��ȡFLASH�и澯����
#define STABOUT 0x16      //������Ϣ��д
#define STENCRY 0x17      //�����ַ�����д
#define STRSWT  0x18      //������������
#define STWSWT  0x19      //д����������


#define EOT     0x10      //���ͷ�����������
#define ACK     0x11      //���շ����ճɹ�Ӧ������
#define NAK     0x12      //���շ�����ʧ��Ӧ������
#define CAN     0x13      //���շ�ǿ����ֹ���չ�����
#define CTRLZ   0x1A      //��Ч�ַ�

#define PACKET_DATA_SIZE 1056                     //�������ݳ���
#define PACKET_SIZE      (PACKET_DATA_SIZE+4)     //���ݰ�����
#define DELAY_MS         5                        //��ʱ5MS
#define TIMEOUT          400                      //2S��ʱ�˳�
#define RETRY_CNT        10                       //���Դ���
#define EOT_TIMEOUT      40                       //����EOT�󣬵ȴ�ACK��ʱ200MS�˳�


static U8_T m_u8_buf[PACKET_SIZE+1];

/*************************************************************
��������: v_load_download_file		           				
��������: �����ļ�������						
�������: u8_cmd -- ��д����       		   				
�������: ��
����ֵ  ����
**************************************************************/
static void v_load_download_file(U8_T u8_cmd)
{
	U32_T u32_load_addr, u32_timeout, u32_index, u32_cnt, u32_retry;
	U8_T u8_ret_cmd, u8_packet_num = 0;
	
	switch (u8_cmd)
	{
		case STF:
			if (u8_flash_read_font_task_pin() == 0)   //�����д�ֿ�����û�ж̽ӣ���������д��ֱ���˳�
				return;
			u32_load_addr = DATAFLASH_MODEL_ADDR;
			break;

		case STENCRY:
			if (u8_flash_read_font_task_pin() == 0)   //�����д�ֿ�����û�ж̽ӣ���������д�����ִ���ֱ���˳�
				return;
			u32_load_addr = DATAFLASH_ENCRY_ADDR;
			break;
			
		case STN:
			u32_load_addr = DATAFLASH_BRANCH_NAME_ADDR;
			break;
			
		case STA:
			u32_load_addr = DATAFLASH_FAULT_NAME_ADDR;
			break;
			
		case STWYC:
			u32_load_addr = DATAFLASH_YC_DATA_ADDR;
			break;
			
		case STWYX:
			u32_load_addr = DATAFLASH_YX_DATA_ADDR;
			break;
			
		case STWYK:
			u32_load_addr = DATAFLASH_YK_DATA_ADDR;
			break;
			
		case STWYT:
			u32_load_addr = DATAFLASH_YT_DATA_ADDR;
			break;
		
		case STABOUT:
			u32_load_addr = DATAFLASH_ABOUT_ADDR;
			break;
			
		case STWSWT:
			u32_load_addr = DATAFLASH_SWT_ADDR;
			break;
		
		default:
			return;
	}
	
	u8_ret_cmd = ACK;                //����Ӧ��
	u32_retry = 0;

	while (1)
	{
		if (u32_retry++ >= RETRY_CNT)
		{
			return;
		}
		
		u32_usb_usb_write(&u8_ret_cmd, 1);
		
		u32_timeout = 0;
		while (u32_timeout < TIMEOUT)
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			if (u32_usb_usb_read(m_u8_buf, 1) == 0)
			{
				v_delay_mdelay(DELAY_MS);
				u32_timeout++;
			}
			else
			{
				if (m_u8_buf[0] == u8_cmd)
				{
					break;
				}
				else if (m_u8_buf[0] == EOT)
				{
					if (u8_cmd == STF)               //�����д�ͺź��ֿ⣬��ָ�Ĭ��ֵ
						v_disp_cfg_data_restore();
						
					u8_ret_cmd = ACK;                //����Ӧ��
					u32_usb_usb_write(&u8_ret_cmd, 1);
					return;
				}
				else
				{
					u32_timeout = 0;
				}
			}
		}
		
		if (u32_timeout >= TIMEOUT)
		{
			continue;
		}
		
		u32_index = 0;
		u32_timeout = 0;
		while (u32_timeout < TIMEOUT)
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			u32_cnt = u32_usb_usb_read(m_u8_buf+u32_index, PACKET_SIZE-u32_index);
			
			if (u32_cnt == 0)
			{
				v_delay_mdelay(DELAY_MS);
				u32_timeout++;
			}
			else
			{
				u32_index += u32_cnt;
				if (u32_index >= PACKET_SIZE)
					break;
			}
		}
		
		if (u32_timeout >= TIMEOUT)
		{
			u8_ret_cmd = NAK;																						 
		}
		else if (m_u8_buf[0] != (U8_T)(~m_u8_buf[1]))
		{
			u8_ret_cmd = NAK;
		}	
		else if (u8_packet_num != m_u8_buf[0])
		{
			if ((u8_packet_num-1) == m_u8_buf[0])
			{
				u8_ret_cmd = ACK;
			}
			else
			{
				u8_ret_cmd = CAN;
				u32_usb_usb_write(&u8_ret_cmd, 1);     //����Ŵ��󣬷���CANǿ����ֹ����
				return;
			}
		}
		else if (u16_crc_calculate_crc(&m_u8_buf[2], PACKET_SIZE-2) != 0)
		{
			u8_ret_cmd = NAK;
		}
		else
		{
			if (u8_cmd == STENCRY)
			{
				U8_T u8_cpu_encry[DATAFLASH_ENCRY_SIZE];

				memset(u8_cpu_encry, 0, sizeof(u8_cpu_encry));
				v_encry_read_cpu_serial_num(u8_cpu_encry);
				s32_flash_dataflash_write(DATAFLASH_ENCRY_ADDR , u8_cpu_encry, DATAFLASH_ENCRY_SIZE);
			}
			else
			{
				s32_flash_dataflash_write(u32_load_addr , &m_u8_buf[2], PACKET_DATA_SIZE);
				u32_load_addr += (PACKET_DATA_SIZE);
			}
			u8_ret_cmd = ACK;
			u8_packet_num++;
			u32_retry = 0;
		}
	}
}

/*************************************************************
��������: v_load_upload_flash_file		           				
��������: �ϴ�dataflash�е��ļ�������						
�������: u8_cmd -- ��д����       		   				
�������: ��
����ֵ  ����
**************************************************************/
static void v_load_upload_flash_file(U8_T u8_cmd)
{
	U32_T u32_load_addr, u32_timeout, u32_index, u32_cnt, u32_size, u32_retry;
	U8_T u8_ret_cmd, u8_packet_num = 0;
	U16_T u16_crc;
	
	switch (u8_cmd)
	{
		case STRFN:
			u32_load_addr = DATAFLASH_BRANCH_NAME_ADDR;
			u32_size = DATAFLASH_BRANCH_NAME_SIZE;
			break;
			
		case STRFA:
			u32_load_addr = DATAFLASH_FAULT_NAME_ADDR;
			u32_size = DATAFLASH_FAULT_NAME_SIZE;
			break;
			
		case STRFYC:
			u32_load_addr = DATAFLASH_YC_DATA_ADDR;
			u32_size = BS_YC_SIZE * BS_ITEM_SIZE; 
			break;
			
		case STRFYX:
			u32_load_addr = DATAFLASH_YX_DATA_ADDR;
			u32_size = BS_YX_SIZE * BS_ITEM_SIZE; 
			break;
			
		case STRFYK:
			u32_load_addr = DATAFLASH_YK_DATA_ADDR;
			u32_size = BS_YK_SIZE * BS_ITEM_SIZE; 
			break;
			
		case STRFYT:
			u32_load_addr = DATAFLASH_YT_DATA_ADDR;
			u32_size = BS_YT_SIZE * BS_ITEM_SIZE; 
			break;
			
		case STRSWT:
			u32_load_addr = DATAFLASH_SWT_ADDR;
			u32_size = DATAFLASH_SWT_SIZE; 
			break;
			
		default:
			return;
	}
	
	u8_ret_cmd = ACK;                //����Ӧ��
	u32_usb_usb_write(&u8_ret_cmd, 1);
	v_delay_mdelay(5);
	
	u32_cnt = 0;
	while (u32_cnt < u32_size)
	{
		m_u8_buf[0] = u8_cmd;
		m_u8_buf[1] = u8_packet_num;
		m_u8_buf[2] = (U8_T)(~u8_packet_num);
		s32_flash_dataflash_read(u32_load_addr+u32_cnt, &(m_u8_buf[3]), PACKET_DATA_SIZE);
		u16_crc = u16_crc_calculate_crc(&m_u8_buf[3], PACKET_DATA_SIZE);
		m_u8_buf[PACKET_SIZE-1] = (U8_T)u16_crc;
		m_u8_buf[PACKET_SIZE] = (U8_T)(u16_crc >> 8);
		
		u32_index = 0;
		u32_timeout = 0;
		while (u32_index < (PACKET_SIZE+1))
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			u32_index += u32_usb_usb_write(m_u8_buf+u32_index, PACKET_SIZE+1-u32_index);
			v_delay_mdelay(DELAY_MS);
			if (++u32_timeout > TIMEOUT)
				return;
		}
		
		u32_timeout = 0;
		while (1)
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			if (u32_usb_usb_read(&u8_ret_cmd, 1) == 0)
			{
				v_delay_mdelay(DELAY_MS);
				u32_timeout++;
			}
			else
			{
				if (u8_ret_cmd == CAN)
				{
					return;
				}
				else if (u8_ret_cmd == ACK)
				{
					u32_retry = 0;
					u32_cnt += (PACKET_SIZE-4);
					u8_packet_num++;
					break;
				}
				else if (u8_ret_cmd == NAK)
				{
					if (u32_retry++ >= RETRY_CNT)
						return;
					else
						break;
				}
				else
				{
					u32_timeout = 0;
				}
			}
			
			if (u32_timeout > TIMEOUT)
			{
				if (u32_retry++ >= RETRY_CNT)
					return;
				else
					break;
			}
		}
	}
	
	u8_ret_cmd = EOT;
	u32_retry = 0;
	while (u32_retry++ < RETRY_CNT)
	{
		u32_usb_usb_write(&u8_ret_cmd, 1);        //���ͽ�����
			
		u32_timeout = 0;
		while (u32_timeout < EOT_TIMEOUT)
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			if (u32_usb_usb_read(m_u8_buf, 1) == 0)
			{
				v_delay_mdelay(DELAY_MS);
				u32_timeout++;
			}
			else
			{
				if (m_u8_buf[0] == ACK)
					return;
			}
		}
	}

}

/*************************************************************
��������: v_load_upload_file		           				
��������: �ϴ������������ɵĺ�̨����ļ�������						
�������: u8_cmd -- ��д����       		   				
�������: ��
����ֵ  ����
**************************************************************/
static void v_load_upload_file(U8_T u8_cmd)
{
	U32_T i, u32_timeout, u32_index, u32_cnt, u32_retry;
	U8_T u8_ret_cmd, u8_packet_num = 0;
	U16_T u16_crc;
	
	u8_ret_cmd = ACK;                //����Ӧ��
	u32_usb_usb_write(&u8_ret_cmd, 1);
	v_delay_mdelay(5);
	
	u32_cnt = 0;
	while (1)
	{
		m_u8_buf[0] = u8_cmd;
		m_u8_buf[1] = u8_packet_num;
		m_u8_buf[2] = (U8_T)(~u8_packet_num);
		
		switch (u8_cmd)
		{
			case STRCYC:
				if (u32_cnt >= BS_YC_SIZE)
					goto send_eot;
					
				for (i=0; i<PACKET_DATA_SIZE/BS_ITEM_SIZE; i++)
				{
					u32_index = i * BS_ITEM_SIZE + 3;
					
					if ((i + u32_cnt) < BS_YC_SIZE)
					{
						m_u8_buf[u32_index] = (U8_T)(g_t_yc_assemble[i + u32_cnt].u16_id);
						m_u8_buf[u32_index+1] = (U8_T)(g_t_yc_assemble[i + u32_cnt].u16_id>>8);
						if (g_t_yc_assemble[i + u32_cnt].pu8_major_condition != NULL) 
						{
//							if (*(g_t_yc_assemble[i + u32_cnt].pu8_major_condition) >= g_t_yc_assemble[i + u32_cnt].u8_major_val) //������������
//							{
//								if ((g_t_yc_assemble[i + u32_cnt].pu8_second_condition != NULL) //���������㣬�д������Ҵ�����������
//									&& (*(g_t_yc_assemble[i + u32_cnt].pu8_second_condition) < g_t_yc_assemble[i + u32_cnt].u8_second_val))
//								{
//									m_u8_buf[u32_index+2] = BS_ITEM_INVALID;         //���ϴ�
//									m_u8_buf[u32_index+3] = 0;
//								}
//								else                                                 //���������㣬�޴�����
								{
									m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;          //�ϴ�
									m_u8_buf[u32_index+3] = 0;
								}
//							}
//							else                                                    //������������
//							{
//								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;            //���ϴ�
//								m_u8_buf[u32_index+3] = 0;
//							}
						}
						else
						{
							if (g_t_yc_assemble[i + u32_cnt].u8_major_val == 1)              //�̶��ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;            //�ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
							else                                                   //�̶����ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;           //���ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
						}
					}
					else
					{
						m_u8_buf[u32_index] = CTRLZ;
						m_u8_buf[u32_index+1] = CTRLZ;
						m_u8_buf[u32_index+2] = CTRLZ;
						m_u8_buf[u32_index+3] = CTRLZ;
					}
				}
				break;
				
			case STRCYX:
				if (u32_cnt >= BS_YX_SIZE)
					goto send_eot;
					
				for (i=0; i<PACKET_DATA_SIZE/BS_ITEM_SIZE; i++)
				{
					u32_index = i * BS_ITEM_SIZE + 3;
					
					if ((i + u32_cnt) < BS_YX_SIZE)
					{
						m_u8_buf[u32_index] = (U8_T)(g_t_yx_assemble[i + u32_cnt].u16_id);
						m_u8_buf[u32_index+1] = (U8_T)(g_t_yx_assemble[i + u32_cnt].u16_id>>8);
						if (g_t_yx_assemble[i + u32_cnt].pu8_major_condition != NULL) 
						{
//							if (*(g_t_yx_assemble[i + u32_cnt].pu8_major_condition) >= g_t_yx_assemble[i + u32_cnt].u8_major_val) //������������
//							{
//								if ((g_t_yx_assemble[i + u32_cnt].pu8_second_condition != NULL) //���������㣬�д������Ҵ�����������
//									&& (*(g_t_yx_assemble[i + u32_cnt].pu8_second_condition) < g_t_yx_assemble[i + u32_cnt].u8_second_val))
//								{
//									m_u8_buf[u32_index+2] = BS_ITEM_INVALID;     //���ϴ�
//									m_u8_buf[u32_index+3] = 0;
//								}
//								else                                             //���������㣬�޴�����
								{
									m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;      //�ϴ�
									m_u8_buf[u32_index+3] = 0;
								}
//							}
//							else                                                 //������������
//							{
//								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;         //���ϴ�
//								m_u8_buf[u32_index+3] = 0;
//							}
						}
						else
						{
							if (g_t_yx_assemble[i + u32_cnt].u8_major_val == 1)            //�̶��ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;          //�ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
							else                                                 //�̶����ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;         //���ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
						}
					}
					else
					{
						m_u8_buf[u32_index] = CTRLZ;
						m_u8_buf[u32_index+1] = CTRLZ;
						m_u8_buf[u32_index+2] = CTRLZ;
						m_u8_buf[u32_index+3] = CTRLZ;
					}
				}
				break;
				
			case STRCYK:
				if (u32_cnt >= BS_YK_SIZE)
					goto send_eot;
					
				for (i=0; i<PACKET_DATA_SIZE/BS_ITEM_SIZE; i++)
				{
					u32_index = i * BS_ITEM_SIZE + 3;
					
					if ((i + u32_cnt) < BS_YK_SIZE)
					{
						m_u8_buf[u32_index] = (U8_T)(g_t_yk_assemble[i + u32_cnt].u16_id);
						m_u8_buf[u32_index+1] = (U8_T)(g_t_yk_assemble[i + u32_cnt].u16_id>>8);
						if (g_t_yk_assemble[i + u32_cnt].pu8_major_condition != NULL) 
						{
//							if (*(g_t_yk_assemble[i + u32_cnt].pu8_major_condition) >= g_t_yk_assemble[i + u32_cnt].u8_major_val) //������������
							{
								m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;          //�ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
//							else                                                 //������������
//							{
//								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;         //���ϴ�
//								m_u8_buf[u32_index+3] = 0;
//							}
						}
						else
						{
							if (g_t_yk_assemble[i + u32_cnt].u8_major_val == 1)            //�̶��ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;          //�ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
							else                                                 //�̶����ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;         //���ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
						}
					}
					else
					{
						m_u8_buf[u32_index] = CTRLZ;
						m_u8_buf[u32_index+1] = CTRLZ;
						m_u8_buf[u32_index+2] = CTRLZ;
						m_u8_buf[u32_index+3] = CTRLZ;
					}
				}
				break;
				
			case STRCYT:
				if (u32_cnt >= BS_YT_SIZE)
					goto send_eot;
					
				for (i=0; i<PACKET_DATA_SIZE/BS_ITEM_SIZE; i++)
				{
					u32_index = i * BS_ITEM_SIZE + 3;
					
					if ((i + u32_cnt) < BS_YT_SIZE)
					{
						m_u8_buf[u32_index] = (U8_T)(g_t_yt_assemble[i + u32_cnt].u16_id);
						m_u8_buf[u32_index+1] = (U8_T)(g_t_yt_assemble[i + u32_cnt].u16_id>>8);
						if (g_t_yt_assemble[i + u32_cnt].pu8_major_condition != NULL) 
						{
							if (*(g_t_yt_assemble[i + u32_cnt].pu8_major_condition) >= g_t_yt_assemble[i + u32_cnt].u8_major_val) //������������
							{
//								if ((g_t_yt_assemble[i + u32_cnt].pu8_second_condition != NULL) //���������㣬�д������Ҵ�����������
//									&& (*(g_t_yt_assemble[i + u32_cnt].pu8_second_condition) < g_t_yt_assemble[i + u32_cnt].u8_second_val))
								{
									m_u8_buf[u32_index+2] = BS_ITEM_INVALID;     //���ϴ�
									m_u8_buf[u32_index+3] = 0;
								}
//								else                                             //���������㣬�޴�����
//								{
//									m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;      //�ϴ�
//									m_u8_buf[u32_index+3] = 0;
//								}
							}
							else                                                 //������������
							{
								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;         //���ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
						}
						else
						{
							if (g_t_yt_assemble[i + u32_cnt].u8_major_val == 1)            //�̶��ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_UPLOAD;          //�ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
							else                                                 //�̶����ϴ�
							{
								m_u8_buf[u32_index+2] = BS_ITEM_INVALID;         //���ϴ�
								m_u8_buf[u32_index+3] = 0;
							}
						}
					}
					else
					{
						m_u8_buf[u32_index] = CTRLZ;
						m_u8_buf[u32_index+1] = CTRLZ;
						m_u8_buf[u32_index+2] = CTRLZ;
						m_u8_buf[u32_index+3] = CTRLZ;
					}
				}
				break;
				
			default:
				return;
		}
			
		u16_crc = u16_crc_calculate_crc(&m_u8_buf[3], PACKET_DATA_SIZE);
		m_u8_buf[PACKET_SIZE-1] = (U8_T)u16_crc;
		m_u8_buf[PACKET_SIZE] = (U8_T)(u16_crc >> 8);
		
		while (u32_usb_usb_read(&u8_ret_cmd, 1) != 0);                //������ջ�����

		u32_index = 0;
		u32_timeout = 0;
		while (u32_index < (PACKET_SIZE+1))
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			u32_index += u32_usb_usb_write(m_u8_buf+u32_index, PACKET_SIZE+1-u32_index);
			v_delay_mdelay(DELAY_MS);
			if (++u32_timeout > TIMEOUT)
				return;
		}
		
		u32_timeout = 0;
		while (1)
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			if (u32_usb_usb_read(&u8_ret_cmd, 1) == 0)
			{
				v_delay_mdelay(DELAY_MS);
				u32_timeout++;
			}
			else
			{
				if (u8_ret_cmd == CAN)
				{
					return;
				}
				else if (u8_ret_cmd == ACK)
				{
					u32_retry = 0;
					u32_cnt += PACKET_DATA_SIZE/BS_ITEM_SIZE;
					u8_packet_num++;
					break;
				}
				else if (u8_ret_cmd == NAK)
				{
					if (u32_retry++ >= RETRY_CNT)
						return;
					else
						break;
				}
				else
				{
					u32_timeout = 0;
				}
			}
			
			if (u32_timeout > TIMEOUT)
			{
				if (u32_retry++ >= RETRY_CNT)
					return;
				else
					break;
			}
		}
	}

send_eot:	
	u8_ret_cmd = EOT;
	u32_retry = 0;
	while (u32_retry++ < RETRY_CNT)
	{
		u32_usb_usb_write(&u8_ret_cmd, 1);        //���ͽ�����
			
		u32_timeout = 0;
		while (u32_timeout < EOT_TIMEOUT)
		{
			//os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			v_wdt_wdt_feed();        //ֱ��ι��
			
			if (u32_usb_usb_read(m_u8_buf, 1) == 0)
			{
				v_delay_mdelay(DELAY_MS);
				u32_timeout++;
			}
			else
			{
				if (m_u8_buf[0] == ACK)
					return;
			}
		}
	}
}

/*************************************************************
��������: v_load_load_file_task		           				
��������: ���롢�����ļ�������						
�������: ��        		   				
�������: ��
����ֵ  ����
**************************************************************/
__task void v_load_load_file_task(void)
{
	U8_T u8_command;
	
	while (1)
	{
		while (1)
		{
			os_evt_set(LOAD_FILE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
			
			if (u32_usb_usb_read(&u8_command, 1) == 0)
			{
				os_dly_wait(10);
			}
			else
			{
				if ((u8_command == STF)
					|| (u8_command == STN)
					|| (u8_command == STA)
					|| (u8_command == STRFN)
					|| (u8_command == STRFA)
					|| (u8_command == STABOUT)
					|| (u8_command == STENCRY)
					|| (u8_command == STWYC)
					|| (u8_command == STWYX)
					|| (u8_command == STWYK)
					|| (u8_command == STWYT)
					|| (u8_command == STRCYC)
					|| (u8_command == STRCYX)
					|| (u8_command == STRCYK)
					|| (u8_command == STRCYT)
					|| (u8_command == STRFYC)
					|| (u8_command == STRFYX)
					|| (u8_command == STRFYK)
					|| (u8_command == STRFYT)
					|| (u8_command == STRSWT)
					|| (u8_command == STWSWT))
				{
					break;
				}
			}		
		}
		
		switch (u8_command)
		{
			case STF:
			case STN:
			case STA:
			case STWYC:
			case STWYX:
			case STWYK:
			case STWYT:
			case STABOUT:
			case STENCRY:
			case STWSWT:
				v_load_download_file(u8_command);
				break;
				
			case STRFN:
			case STRFA:
			case STRFYC:
			case STRFYX:
			case STRFYK:
			case STRFYT:
			case STRSWT:
				v_load_upload_flash_file(u8_command);
				break;
				
			case STRCYC:
			case STRCYX:
			case STRCYK:
			case STRCYT:
				v_load_upload_file(u8_command);
				break;
				
			default:
				break;
		}
	}
}

