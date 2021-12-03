/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Log.c
版    本：1.00
创建日期：2012-06-02
作    者：郭数理
功能描述：事件记录保存和ID定义文件


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-06-02  1.00     创建
**************************************************************/

#include <string.h>
#include "Log.h"
#include "../Drivers/Dataflash.h"



OS_MUT g_mut_log;                              //存取log数据的互斥量
static U8_T m_u8_log_change = LOG_NO_CHANGE;   //log记录是否被改变了，此变量由改变log的任务置位，由显示任务清零


/*************************************************************
函数名称: v_log_mut_init		           				
函数功能: 初始化事件记录互斥量						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_mut_init(void)
{
	os_mut_init(g_mut_log);           //初始化互斥量
}


/*************************************************************
函数名称: v_log_save_record		           				
函数功能: 保存事件记录到位dataflash						
输入参数: pt_log_data -- 要保存的事件记录        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_save_record(LOG_DATA_T *pt_log_data)
{
	U8_T u8_head[6];
	U16_T u16_cnt, u16_index;
	U32_T u32_addr;

	os_mut_wait(g_mut_log, 0xFFFF);                 //读写事件记录需要持有互斥量

	s32_flash_dataflash_read(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	
	u16_cnt = ((u8_head[2]<<8) | u8_head[3]);
	u16_index = ((u8_head[4]<<8) | u8_head[5]);

	if ((u8_head[0] == 0x55) && (u8_head[1] == 0xAA)
		&& (u16_cnt <= LOG_MAX_CNT) && (u16_index < LOG_MAX_CNT))
	{
		if (u16_cnt < LOG_MAX_CNT)
			u16_cnt++;

		u32_addr = (DATAFLASH_RECORD_ADDR + sizeof(u8_head) + u16_index * sizeof(LOG_DATA_T));
		u16_index++;
		u16_index %= LOG_MAX_CNT;

		u8_head[2] = (U8_T)(u16_cnt>>8);
		u8_head[3] = (U8_T)u16_cnt;
		u8_head[4] = (U8_T)(u16_index>>8);
		u8_head[5] = (U8_T)u16_index;
	}
	else
	{
		u8_head[0] = 0x55;
		u8_head[1] = 0xAA;
		u8_head[2] = 0;
		u8_head[3] = 1;
		u8_head[4] = 0;
		u8_head[5] = 1;
		u32_addr = DATAFLASH_RECORD_ADDR + sizeof(u8_head);
	}

	s32_flash_dataflash_write(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	s32_flash_dataflash_write(u32_addr, (U8_T *)pt_log_data, sizeof(LOG_DATA_T));

	os_mut_release(g_mut_log);               //释放互斥量
}

/*************************************************************
函数名称: v_log_clear_record		           				
函数功能: 清除事件记录						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_clear_record(void)
{
	U8_T u8_head[6];

	os_mut_wait(g_mut_log, 0xFFFF);                 //读写事件记录需要持有互斥量

	u8_head[0] = 0x55;
	u8_head[1] = 0xAA;
	u8_head[2] = 0;
	u8_head[3] = 0;
	u8_head[4] = 0;
	u8_head[5] = 0;

	s32_flash_dataflash_write(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));

	os_mut_release(g_mut_log);                      //释放互斥量

}

/*************************************************************
函数名称: u8_log_read_log_state		           				
函数功能: 读取事件记录的状态，此函数由显示任务调用，用来获取记录是否己经发生改变						
输入参数: 无        		   				
输出参数: 无
返回值  ：历史故障状态														   				
**************************************************************/
U8_T u8_log_read_log_state(void)
{
	U8_T state;

	os_mut_wait(g_mut_log, 0xFFFF);                 //读写事件记录需要持有互斥量
	state = m_u8_log_change;
	m_u8_log_change = LOG_NO_CHANGE;
	os_mut_release(g_mut_log);                      //释放互斥量

	return state;
}


/*************************************************************
函数名称: v_log_set_log_state		           				
函数功能: 设置记录的状态为己改变，改变记录的任务调用此函数						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_set_log_state(void)
{
	os_mut_wait(g_mut_log, 0xFFFF);                 //读写事件记录需要持有互斥量
	m_u8_log_change = LOG_CHANGE;
	os_mut_release(g_mut_log);                      //释放互斥量
}


/*************************************************************
函数名称: u16_log_read_log_cnt		           				
函数功能: 读取事件记录的数量，调用前需要获取互斥量g_mut_log						
输入参数: 无        		   				
输出参数: 无
返回值  ：历史故障数量														   				
**************************************************************/
U16_T u16_log_read_log_cnt(void)
{
	U8_T u8_head[6];
	U16_T u16_cnt, u16_index;

	memset(u8_head, 0, sizeof(u8_head));
	s32_flash_dataflash_read(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	u16_cnt = ((u8_head[2]<<8) | u8_head[3]);
	u16_index = ((u8_head[4]<<8) | u8_head[5]);

	if ((u8_head[0] == 0x55) && (u8_head[1] == 0xAA)
		&& (u16_cnt <= LOG_MAX_CNT) && (u16_index < LOG_MAX_CNT))
	{
		return u16_cnt;
	}

	return 0;
}

/*************************************************************
函数名称: v_log_read_log_record		           				
函数功能: 从dataflash读取事件记录，此函数不会对u16_log_index进行范围判断，
          需在调用者进行判断，，调用前需要获取互斥量g_mut_log						
输入参数: u16_log_index -- 记录索引，从0开始，小于事件记录总数        		   				
输出参数: pt_log        -- 保存返回的事件记录
返回值  ：无														   				
**************************************************************/
void v_log_read_log_record(LOG_DATA_T *pt_log, U16_T u16_log_index)
{
	U8_T u8_head[6];
	U16_T u16_index, u16_read_index;
	U32_T u32_addr;

	memset(u8_head, 0, sizeof(u8_head));
	s32_flash_dataflash_read(DATAFLASH_RECORD_ADDR, u8_head, sizeof(u8_head));
	u16_index = ((u8_head[4]<<8) | u8_head[5]);

	u16_read_index = ((LOG_MAX_CNT + u16_index - u16_log_index -1) % LOG_MAX_CNT);
	u32_addr = (DATAFLASH_RECORD_ADDR + sizeof(u8_head) + u16_read_index*sizeof(LOG_DATA_T));
	s32_flash_dataflash_read(u32_addr, (U8_T *)pt_log, sizeof(LOG_DATA_T));
}
