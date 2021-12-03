/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Log.h
版    本：1.00
创建日期：2012-06-02
作    者：郭数理
功能描述：事件记录保存和ID定义头文件


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-06-02  1.00     创建
**************************************************************/


#ifndef __LOG_H_
#define __LOG_H_


#include <rtl.h>
#include "Type.h"


#define LOG_MAX_CNT                 500        //事件记录的最大数量    
#define LOG_NO_CHANGE               0          //事件记录没有变化
#define LOG_CHANGE                  1          //事件记录己变化

/* 事件分组号 */
#define LOG_ID_GROUP_MASK           0xFC00    //事件组屏蔽码
#define LOG_ID_INDEX_MASK           0x03FF    //组内索引屏蔽码
#define LOG_ID_GROUP_OFFSET         10        //分组号偏移
#define LOG_ID_BATT_GROUP           0         //电池充放电转换组号

#define	LOG_ID_BATT_TO_AUTO         (0 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组充电管理转为自动
#define	LOG_ID_BATT_TO_MANUAL       (1 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组充电管理转为手动
#define	LOG_ID_BATT_AUTO_TO_FLO     (2 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组电池自动转浮充
#define LOG_ID_BATT_AUTO_TO_EQU     (3 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组电池自动转均充
#define	LOG_ID_BATT_AUTO_TO_DIS     (4 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组电池自动转核容
#define LOG_ID_BATT_MANUAL_TO_FLO   (5 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组电池手动转浮充
#define LOG_ID_BATT_MANUAL_TO_EQU   (6 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组电池手动转均充
#define	LOG_ID_BATT_MANUAL_TO_DIS   (7 | (LOG_ID_BATT_GROUP << LOG_ID_GROUP_OFFSET))   //一组电池手动转核容
#define LOG_ID_BATT_NUM             8        //电池管理事件记录数量



typedef struct
{
	U16_T u16_log_id;        //事件ID

	//发生时间
	U8_T  u8_occur_year;     //年
	U8_T  u8_occur_mon;      //月
	U8_T  u8_occur_day;      //日
	U8_T  u8_occur_hour;     //时
	U8_T  u8_occur_min;      //分
	U8_T  u8_occur_sec;      //秒
}LOG_DATA_T;


extern OS_MUT g_mut_log;                              //存取log数据的互斥量


/*************************************************************
函数名称: v_log_mut_init		           				
函数功能: 初始化事件记录互斥量						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_mut_init(void);


/*************************************************************
函数名称: v_log_save_record		           				
函数功能: 保存事件记录到位dataflash						
输入参数: pt_log_data -- 要保存的事件记录        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_save_record(LOG_DATA_T *pt_log_data);


/*************************************************************
函数名称: v_log_clear_record		           				
函数功能: 清除事件记录						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_clear_record(void);


/*************************************************************
函数名称: u8_log_read_log_state		           				
函数功能: 读取事件记录的状态，此函数由显示任务调用，用来获取记录是否己经发生改变						
输入参数: 无        		   				
输出参数: 无
返回值  ：历史故障状态														   				
**************************************************************/
U8_T u8_log_read_log_state(void);


/*************************************************************
函数名称: v_log_set_log_state		           				
函数功能: 设置记录的状态为己改变，改变记录的任务调用此函数						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_log_set_log_state(void);


/*************************************************************
函数名称: u16_log_read_log_cnt		           				
函数功能: 读取事件记录的数量，调用前需要获取互斥量g_mut_log						
输入参数: 无        		   				
输出参数: 无
返回值  ：历史故障数量														   				
**************************************************************/
U16_T u16_log_read_log_cnt(void);


/*************************************************************
函数名称: v_log_read_log_record		           				
函数功能: 从dataflash读取事件记录，此函数不会对u16_log_index进行范围判断，
          需在调用者进行判断，，调用前需要获取互斥量g_mut_log						
输入参数: u16_log_index -- 记录索引，从0开始，小于事件记录总数        		   				
输出参数: pt_log        -- 保存返回的事件记录
返回值  ：无														   				
**************************************************************/
void v_log_read_log_record(LOG_DATA_T *pt_log, U16_T u16_log_index);


#endif
