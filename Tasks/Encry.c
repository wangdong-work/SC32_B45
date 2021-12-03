/************************************************************
Copyright (C), 深圳英可瑞科技开发有限公司, 保留一切权利
文 件 名：Encry.c
版    本：1.00
创建日期：2012-12-06
作    者：郭数理
功能描述：加密实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-12-06  1.00     创建
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
函数名称: u16_encry_crc_calculate_crc		           				
函数功能: crc校验计算函数						
输入参数: pu8_buf  -- 指向校验的数据区
          u32_len  -- 数据长度         		   				
输出参数: 无
返回值  ：校验结果														   				
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
		        crc ^= 0x9008;     //校验多项式乱写的，防止破解
		    }
		    else
		        crc >>= 1;
		}
	}
	return crc;
}



/*************************************************************
函数名称: v_encry_read_cpu_serial_num
函数功能: 读取CPU序列号并加CRC校验
输入参数: 无
输出参数: pu8_serial_num -- 用于保存串号及校验结果
返回值  ：无
**************************************************************/
void v_encry_read_cpu_serial_num(U8_T pu8_serial_num[18])
{
	U32_T u32_command[5] = { 0, 0, 0, 0, 0 };
	U32_T u32_result[5] = {0, 0, 0, 0, 0 };
	U16_T u16_crc;

	u32_command[0] = IAP_READ_SERIAL_NUM;
	iap_entry(u32_command, u32_result);       //返回的状态码放在u32_result[0]，串号应该放在u32_result[1]~u32_result[4]
	u16_crc = u16_encry_crc_calculate_crc((U8_T *)(&u32_result[1]), sizeof(u32_result));

	pu8_serial_num[0] = (U8_T)(u32_result[1]);
	pu8_serial_num[1] = (U8_T)(u32_result[1]>>8);
	pu8_serial_num[2] = (U8_T)(u32_result[1]>>16);
	pu8_serial_num[3] = (U8_T)(u32_result[1]>>24);

	pu8_serial_num[4] = (U8_T)(u32_result[2]);
	pu8_serial_num[5] = (U8_T)(u32_result[2]>>8);
	pu8_serial_num[6] = (U8_T)(u32_result[2]>>16);
	pu8_serial_num[7] = (U8_T)(u32_result[2]>>24);

	pu8_serial_num[8] = (U8_T)(u16_crc);       //crc低字节

	pu8_serial_num[9] = (U8_T)(u32_result[3]);
	pu8_serial_num[10] = (U8_T)(u32_result[3]>>8);
	pu8_serial_num[11] = (U8_T)(u32_result[3]>>16);
	pu8_serial_num[12] = (U8_T)(u32_result[3]>>24);

	pu8_serial_num[13] = (U8_T)(u32_result[4]);
	pu8_serial_num[14] = (U8_T)(u32_result[4]>>8);
	pu8_serial_num[15] = (U8_T)(u32_result[4]>>16);
	pu8_serial_num[16] = (U8_T)(u32_result[4]>>24);

	pu8_serial_num[17] = (U8_T)(u16_crc>>8);   //crc高字节
}

/*************************************************************
函数名称: u8_encry_judge_encry
函数功能: 判断加密是否正确
输入参数: 无
输出参数: 无
返回值  ：1：加密正确；0：加密不正确
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
