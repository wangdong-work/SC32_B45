/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：FetchFlash.c
版    本：1.00
创建日期：2012-05-19
作    者：郭数理
功能描述：从dataflash存取数据

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-05-19  1.00     创建
**************************************************************/

//FetchFlash.c文件名的缩写为fetch
//coefficientX缩写为coeff

#include <string.h>
#include "Type.h"
#include "Crc.h"
#include "PublicData.h"
#include "FaultId.h"
#include "FetchFlash.h"
#include "Fault.h"
#include "../Drivers/Dataflash.h"

typedef enum {
	AC_ADJUST,    //交流校准
	DC_ADJUST     //直流校准
}ADJUST_TYPE_E;


/*************************************************************
函数名称: v_fetch_save_cfg_data		           				
函数功能: 保存配置数据到dataflash，保存三份，具体格式见文件：IARM-SC32 DATAFLASH空间地址定义						
输入参数: 无         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fetch_save_cfg_data(void)
{
	U8_T u8_head[8];
	U16_T u16_len, u16_crc, i, j;
	U32_T u32_addr[3] = { DATAFLASH_CFG_DATA1_ADDR, DATAFLASH_CFG_DATA2_ADDR, DATAFLASH_CFG_DATA3_ADDR };

	u16_len = sizeof(SYS_CFG_T);

	u8_head[0] = 0x55;
	u8_head[1] = 0xAA;
	u8_head[2] = CFG_DATA_VERSION_H;
	u8_head[3] = CFG_DATA_VERSION_L;
	u8_head[4] = (U8_T)(u16_len >> 8);
	u8_head[5] = (U8_T)u16_len;

	os_mut_wait(g_mut_share_data, 0xFFFF);        //获取互斥锁

	u16_crc = u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len);
	u8_head[6] = (U8_T)(u16_crc >> 8);
	u8_head[7] = (U8_T)u16_crc;

	for (i=0; i<3; i++)            //写3份数据
	{		
		for (j=0; j<3; j++)        //如果写失败，则重试3次
		{
			if ((s32_flash_dataflash_write(u32_addr[i], u8_head, 8) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+8, (U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //释放互斥锁
}

/*************************************************************
函数名称: s32_fetch_read_cfg_data		           				
函数功能: 从dataflash读取配置数据，从三份数据中读取正确的数据，并用正确的数据重写不正确的数据					
输入参数: 无         		   				
输出参数: 无
返回值  ：读取成功则返回0，失败返回-1														   				
**************************************************************/
S32_T s32_fetch_read_cfg_data(void)
{
	U8_T u8_head[8];
	U16_T u16_len, i, j, k;
	U32_T u32_addr[3] = { DATAFLASH_CFG_DATA1_ADDR, DATAFLASH_CFG_DATA2_ADDR, DATAFLASH_CFG_DATA3_ADDR };

	u16_len = sizeof(SYS_CFG_T);

	os_mut_wait(g_mut_share_data, 0xFFFF);        //获取互斥锁

	for (i=0; i<3; i++)
	{
		if ((s32_flash_dataflash_read(u32_addr[i], u8_head, 8) == 0)
			&& (s32_flash_dataflash_read(u32_addr[i]+8, (U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len) == 0))
		{
			if ((u8_head[0] == 0x55)
				&& (u8_head[1] == 0xAA)
				&&(u8_head[2] == CFG_DATA_VERSION_H)                                                     //版本相同
				&& (u8_head[3] == CFG_DATA_VERSION_L)
				&& (u16_len == ((u8_head[4]<<8) | u8_head[5]))                                         //长度相同
				&& (((u8_head[6]<<8) | u8_head[7]) ==  u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len)))  //CRC校验正确
			{
				break;
			}
		}
	}

	if (i >= 3)                    //三份配置数据都没有读取成功，则退出
	{
		os_mut_release(g_mut_share_data);             //释放互斥锁
		return -1;
	}
	else if (i == 0)               //第一份就读取成功，则退出
	{
		os_mut_release(g_mut_share_data);             //释放互斥锁
		return 0;
	}

	//程序跑到这里，说明己经读取成功，且不是第一份读取成功，需要对读取失败的数据重写
	for (j=0; j<i; j++)
	{
		for (k=0; k<3; k++)        //如果写失败，则重试3次
		{
			if ((s32_flash_dataflash_write(u32_addr[j], u8_head, 8) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+8, (U8_T *)(&(g_t_share_data.t_sys_cfg)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //释放互斥锁

	return 0;
}

/*************************************************************
函数名称: v_fetch_read_swt_cfg_data
函数功能: 从dataflash读取开关配置数据
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_fetch_read_swt_cfg_data(void)
{
	U16_T u16_len, i, j;

	os_mut_wait(g_mut_share_data, 0xFFFF);        //获取互斥锁

	u16_len = sizeof(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item);

	//如果读取失败，则尝试3次
	for (i=0; i<3; i++)
	{
		if (s32_flash_dataflash_read(DATAFLASH_SWT_ADDR, (U8_T *)(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item), u16_len) == 0)
		{
			//统计状态量的数量
			for (j=0; j<SWT_BRANCH_MAX; j++)
			{
				if (g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[j].u8_meas_type == 0)
					break;   //检测到第一个告警就退出
			}
			
			g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt = j;
			
			//更新状态归类
			for ( ; j<SWT_BRANCH_MAX; j++)
			{
				v_fault_set_swt_fault_type(j-g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.u8_state_cnt,
											g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[j].u8_fault_type,
											g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item[j].u8_fault_type);
			}
			
			break;
		}
	}
	
	if (i >= 3)                    //三次都没有读取成功，则清空退出
	{
		memset(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt.t_swt_item, 0, u16_len);
	}

	os_mut_release(g_mut_share_data);             //释放互斥锁
}

/*************************************************************
函数名称: v_fetch_save_adjust_coeff		           				
函数功能: 保存校准系数到dataflash，保存三份，具体格式见文件：IARM-SC32 DATAFLASH空间地址定义						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fetch_save_adjust_coeff(void)
{
	U8_T u8_head[6];
	U16_T u16_len, u16_crc, i, j;
	U32_T u32_addr[3] = { DATAFLASH_ADJUST1_ADDR, DATAFLASH_ADJUST2_ADDR, DATAFLASH_ADJUST3_ADDR };

	u16_len = sizeof(COEFF_DATA_T);

	u8_head[0] = 0x55;
	u8_head[1] = 0xAA;
	u8_head[2] = (U8_T)(u16_len >> 8);
	u8_head[3] = (U8_T)u16_len;

	os_mut_wait(g_mut_share_data, 0xFFFF);        //获取互斥锁

	u16_crc = u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len);
	u8_head[4] = (U8_T)(u16_crc >> 8);
	u8_head[5] = (U8_T)u16_crc;

	for (i=0; i<3; i++)            //写3份数据
	{		
		for (j=0; j<3; j++)        //如果写失败，则重试3次
		{
			if ((s32_flash_dataflash_write(u32_addr[i], u8_head, 6) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+6, (U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //释放互斥锁
}

/*************************************************************
函数名称: s32_fetch_read_adjust_coeff		           				
函数功能: 从dataflash读取校准系数，从三份数据中读取正确的数据，并用正确的数据重写不正确的数据					
输入参数: 无
返回值  ：读取成功则返回0，失败返回-1														   				
**************************************************************/
S32_T s32_fetch_read_adjust_coeff(void)
{
	U8_T u8_head[6];
	U16_T u16_len, i, j, k;
	U32_T u32_addr[3] = { DATAFLASH_ADJUST1_ADDR, DATAFLASH_ADJUST2_ADDR, DATAFLASH_ADJUST3_ADDR };

	u16_len = sizeof(COEFF_DATA_T);

	os_mut_wait(g_mut_share_data, 0xFFFF);        //获取互斥锁

	for (i=0; i<3; i++)
	{
		if ((s32_flash_dataflash_read(u32_addr[i], u8_head, 6) == 0)
			&& (s32_flash_dataflash_read(u32_addr[i]+6, (U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len) == 0))
		{
			if ((u8_head[0] == 0x55)
				&& (u8_head[1] == 0xAA)
				&& (u16_len == ((u8_head[2]<<8) | u8_head[3]))                                         //长度相同
				&& (((u8_head[4]<<8) | u8_head[5]) ==  u16_crc_calculate_crc((U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len)))  //CRC校验正确
			{
				break;
			}
		}
	}

	if (i >= 3)                    //三份配置数据都没有读取成功，则退出
	{
		os_mut_release(g_mut_share_data);             //释放互斥锁
		return -1;
	}
	else if (i == 0)               //第一份就读取成功，则退出
	{
		os_mut_release(g_mut_share_data);             //释放互斥锁
		return 0;
	}

	//程序跑到这里，说明己经读取成功，且不是第一份读取成功，需要对读取失败的数据重写
	for (j=0; j<i; j++)
	{
		for (k=0; k<3; k++)        //如果写失败，则重试3次
		{
			if ((s32_flash_dataflash_write(u32_addr[j], u8_head, 6) != 0)
				|| (s32_flash_dataflash_write(u32_addr[i]+6, (U8_T *)(&(g_t_share_data.t_coeff_data)), u16_len) != 0))
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	os_mut_release(g_mut_share_data);             //释放互斥锁

	return 0;
}
