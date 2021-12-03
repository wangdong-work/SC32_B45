/************************************************************
Copyright (C), ����Ӣ����Ƽ��������޹�˾, ����һ��Ȩ��
�� �� ����Encry.c
��    ����1.00
�������ڣ�2012-12-06
��    �ߣ�������
��������������ʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2012-12-06  1.00     ����
***********************************************************/

#include <string.h>

#include "../Drivers/Dataflash.h"
#include "Type.h"
#include "Encry.h"


#define IAP_LOCATION 0x1FFF1FF1

#define IAP_READ_SERIAL_NUM     58


typedef void (*IAP)(unsigned int [],unsigned int[]);

static IAP iap_entry=(IAP) IAP_LOCATION;



/*************************************************************
��������: u16_encry_crc_calculate_crc		           				
��������: crcУ����㺯��						
�������: pu8_buf  -- ָ��У���������
          u32_len  -- ���ݳ���         		   				
�������: ��
����ֵ  ��У����														   				
**************************************************************/
static U16_T u16_encry_crc_calculate_crc(U8_T *pu8_buf, U32_T u32_len)
{
	U16_T crc, q, r;
	
	crc = 0xFFFF;
	for(q = 0; q < u32_len; q++)
	{
		crc ^= pu8_buf[q];
		for(r = 0; r<8; r++)
		{
		    if ( crc & 1)
		    {
		        crc >>= 1;
		        crc ^= 0x9008;     //У�����ʽ��д�ģ���ֹ�ƽ�
		    }
		    else
		        crc >>= 1;
		}
	}
	return crc;
}



/*************************************************************
��������: v_encry_read_cpu_serial_num
��������: ��ȡCPU���кŲ���CRCУ��
�������: ��
�������: pu8_serial_num -- ���ڱ��洮�ż�У����
����ֵ  ����
**************************************************************/
void v_encry_read_cpu_serial_num(U8_T pu8_serial_num[18])
{
	U32_T u32_command[5] = { 0, 0, 0, 0, 0 };
	U32_T u32_result[5] = {0, 0, 0, 0, 0 };
	U16_T u16_crc;

	u32_command[0] = IAP_READ_SERIAL_NUM;
	iap_entry(u32_command, u32_result);       //���ص�״̬�����u32_result[0]������Ӧ�÷���u32_result[1]~u32_result[4]
	u16_crc = u16_encry_crc_calculate_crc((U8_T *)(&u32_result[1]), sizeof(u32_result));

	pu8_serial_num[0] = (U8_T)(u32_result[1]);
	pu8_serial_num[1] = (U8_T)(u32_result[1]>>8);
	pu8_serial_num[2] = (U8_T)(u32_result[1]>>16);
	pu8_serial_num[3] = (U8_T)(u32_result[1]>>24);

	pu8_serial_num[4] = (U8_T)(u32_result[2]);
	pu8_serial_num[5] = (U8_T)(u32_result[2]>>8);
	pu8_serial_num[6] = (U8_T)(u32_result[2]>>16);
	pu8_serial_num[7] = (U8_T)(u32_result[2]>>24);

	pu8_serial_num[8] = (U8_T)(u16_crc);       //crc���ֽ�

	pu8_serial_num[9] = (U8_T)(u32_result[3]);
	pu8_serial_num[10] = (U8_T)(u32_result[3]>>8);
	pu8_serial_num[11] = (U8_T)(u32_result[3]>>16);
	pu8_serial_num[12] = (U8_T)(u32_result[3]>>24);

	pu8_serial_num[13] = (U8_T)(u32_result[4]);
	pu8_serial_num[14] = (U8_T)(u32_result[4]>>8);
	pu8_serial_num[15] = (U8_T)(u32_result[4]>>16);
	pu8_serial_num[16] = (U8_T)(u32_result[4]>>24);

	pu8_serial_num[17] = (U8_T)(u16_crc>>8);   //crc���ֽ�
}

/*************************************************************
��������: u8_encry_judge_encry
��������: �жϼ����Ƿ���ȷ
�������: ��
�������: ��
����ֵ  ��1��������ȷ��0�����ܲ���ȷ
**************************************************************/
U8_T u8_encry_judge_encry(void)
{
	U8_T u8_flash_encry[DATAFLASH_ENCRY_SIZE];
	U8_T u8_cpu_encry[DATAFLASH_ENCRY_SIZE];

	memset(u8_flash_encry, 0, sizeof(u8_flash_encry));
	memset(u8_cpu_encry, 0, sizeof(u8_cpu_encry));
	
	s32_flash_dataflash_read(DATAFLASH_ENCRY_ADDR, u8_flash_encry, DATAFLASH_ENCRY_SIZE);
	v_encry_read_cpu_serial_num(u8_cpu_encry);
	
	if (memcmp(u8_flash_encry, u8_cpu_encry, DATAFLASH_ENCRY_SIZE) == 0)
		return 1;
	else
		return 0;
}
