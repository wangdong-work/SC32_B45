/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Fault.h
版    本：1.00
创建日期：2012-05-22
作    者：郭数理
功能描述：故障任务相关函数的头文件


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-05-22  1.00     创建
**************************************************************/


#ifndef __FAULT_H_
#define __FAULT_H_

#include "Type.h"


#define CURR_FAULT_MAX_CNT            512        //当前告警最大数量
#define HIS_FAULT_MAX_CNT             999       //历史告警最大数量
#define NO_RESUME_FAULT_MAX_CNT       999       //历史末复归告警最大数量

#define AC_FAULT_CLASS                0          //交流故障类
#define DC_FAULT_CLASS                1          //直流母线故障类
#define BATT_FAULT_CLASS              2          //电池故障类
#define INSU_FAULT_CLASS              3          //绝缘故障类
#define RECT_FAULT_CLASS              4          //充电模块故障类
#define FUSE_FAULT_CLASS              5          //熔断器故障类
#define DCAC_FAULT_CLASS              6          //UPS及通信模块故障类
#define FEEDER_FAULT_CLASS            7          //馈线支路故障类
#define SYSTEM_NORMAL                 0xFF       //系统正常

#define FAULT_NO_OUTPUT               0xFF       //告警不输出
#define AC_FAULT_OUTPUT               0          //交流故障输出
#define DC_FAULT_OUTPUT               1          //直流母线故障输出
#define BATT_FAULT_OUTPUT             2          //电池故障输出
#define INSU_FAULT_OUTPUT             3          //绝缘故障输出
#define RECT_FAULT_OUTPUT             4          //充电模块故障输出
#define FUSE_FAULT_OUTPUT             5          //熔断器故障输出
#define DCAC_FAULT_OUTPUT             6          //UPS及通信模块故障输出
#define FEEDER_FAULT_OUTPUT           7          //馈线支路故障输出

#define FAULT_NO_CHANGE               0          //故障没变化
#define FAULT_CHANGE                  1          //故障有变化


typedef struct
{
	U16_T u16_id;            //故障ID

	//发生时间
	U8_T  u8_occur_year;     //年
	U8_T  u8_occur_mon;      //月
	U8_T  u8_occur_day;      //日
	U8_T  u8_occur_hour;     //时
	U8_T  u8_occur_min;      //分
	U8_T  u8_occur_sec;      //秒
}CURR_FAULT_DATA_T;

typedef struct
{
	U16_T u16_id;            //故障ID

	//发生时间
	U8_T  u8_occur_year;     //年
	U8_T  u8_occur_mon;      //月
	U8_T  u8_occur_day;      //日
	U8_T  u8_occur_hour;     //时
	U8_T  u8_occur_min;      //分
	U8_T  u8_occur_sec;      //秒

	//复归时间
	U8_T  u8_resume_year;    //年
	U8_T  u8_resume_mon;     //月
	U8_T  u8_resume_day;     //日
	U8_T  u8_resume_hour;    //时
	U8_T  u8_resume_min;     //分
	U8_T  u8_resume_sec;     //秒
}HIS_FAULT_DATA_T;


extern OS_MUT g_mut_fault_data;                           //定义访问上面两个变量的互斥量
extern OS_MUT g_mut_fetch_his_fault;                      //存取历史故障的互斥量


/*************************************************************
函数名称: v_fault_set_swt_fault_type
函数功能: 设置自带开关量的类型
输入参数: u8_index  -- 开关故障索引
          u8_class  -- 显示故障大类
          u8_output -- 输出故障类
输出参数: 无
返回值  ：无
**************************************************************/
void v_fault_set_swt_fault_type(U8_T u8_index, U8_T u8_class, U8_T u8_output);


/*************************************************************
函数名称: v_fault_occur_fault_output		           				
函数功能: 故障发生时，故障继电器动作，调用此函数前需先获取全局互斥锁g_mut_share_data
          如果系统有硅链控制，则4、5、6继电器用于硅链控制，不能用于故障输出和均浮充指示						
输入参数: u8_index -- 继电器索引，0为无效值，不输出        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fault_occur_fault_output(U8_T u8_index);


/*************************************************************
函数名称: v_fault_resume_fault_output		           				
函数功能: 故障发生时，故障继电器不动作，调用此函数前需先获取全局互斥锁g_mut_share_data
          如果系统有硅链控制，则4、5、6继电器用于硅链控制，不能用于故障输出和均浮充指示					
输入参数: u8_index -- 继电器索引，0为无效值，不输出        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fault_resume_fault_output(U8_T u8_index);


/*************************************************************
函数名称: u8_fault_read_his_fault_state		           				
函数功能: 读取当前故障的状态，调用前需要获取互斥量m_mut_fetch_his_fault						
输入参数: 无        		   				
输出参数: 无
返回值  ：当前故障状态														   				
**************************************************************/
U8_T u8_fault_read_his_fault_state(void);


/*************************************************************
函数名称: v_fault_reset_his_fault_state		           				
函数功能: 复位历史故障的状态，调用前需要获取互斥量m_mut_fetch_his_fault						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fault_reset_his_fault_state(void);


/*************************************************************
函数名称: u16_fault_read_his_fault_cnt		           				
函数功能: 读取历史故障的数量，调用前需要获取互斥量m_mut_fetch_his_fault						
输入参数: 无        		   				
输出参数: 无
返回值  ：历史故障数量														   				
**************************************************************/
U16_T u16_fault_read_his_fault_cnt(void);


/*************************************************************
函数名称: v_fault_read_his_fault		           				
函数功能: 从dataflash读取历史故障记录，此函数不会对u16_fault_index进行范围判断，
          需在调用者进行判断，，调用前需要获取互斥量m_mut_fetch_his_fault						
输入参数: u16_fault_index -- 记录索引，从0开始，小于历史故障记录总数        		   				
输出参数: t_his_fault     -- 保存返回的历史故障
返回值  ：无														   				
**************************************************************/
void v_fault_read_his_fault(HIS_FAULT_DATA_T *t_his_fault, U16_T u16_fault_index);


/*************************************************************
函数名称: v_fault_clear_his_fault		           				
函数功能: 清除历史记录						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fault_clear_his_fault(void);


/*************************************************************
函数名称: u16_fault_read_no_resume_fault_cnt		           				
函数功能: 读取历史末复归故障的数量						
输入参数: 无        		   				
输出参数: 无
返回值  ：历史末复归故障数量														   				
**************************************************************/
U16_T u16_fault_read_no_resume_fault_cnt(void);


/*************************************************************
函数名称: v_fault_read_no_resume_fault		           				
函数功能: 从dataflash读取历史末复归故障记录，此函数不会对u16_fault_index进行范围判断，需在调用者进行判断					
输入参数: u16_fault_index -- 记录索引，从0开始，小于历史末复归故障记录总数        		   				
输出参数: t_fault         -- 保存返回的历史末复归故障
返回值  ：无														   				
**************************************************************/
void v_fault_read_no_resume_fault(CURR_FAULT_DATA_T *t_fault, U16_T u16_fault_index);


/*************************************************************
函数名称: v_fault_clear_no_resume_fault		           				
函数功能: 清除历史记录						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fault_clear_no_resume_fault(void);


/*************************************************************
函数名称: u8_fault_read_curr_fault_state		           				
函数功能: 读取当前故障的状态，调用前需要获取互斥量g_mut_fault_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：当前故障状态														   				
**************************************************************/
U8_T u8_fault_read_curr_fault_state(void);


/*************************************************************
函数名称: v_fault_reset_curr_fault_state		           				
函数功能: 复位当前故障的状态，调用前需要获取互斥量g_mut_fault_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fault_reset_curr_fault_state(void);


/*************************************************************
函数名称: u16_fault_read_curr_fault_cnt		           				
函数功能: 读取当前故障的数量，调用前需要获取互斥量g_mut_fault_data						
输入参数: 无        		   				
输出参数: 无
返回值  ：当前故障数量														   				
**************************************************************/
U16_T u16_fault_read_curr_fault_cnt(void);


/*************************************************************
函数名称: v_fault_read_curr_fault		           				
函数功能: 读取当前故障记录，此函数不会对u16_fault_index进行范围判断，
          需在调用者进行判断，调用前需要获取互斥量g_mut_fault_data					
输入参数: u16_fault_index -- 记录索引，从0开始，小于历史故障记录总数        		   				
输出参数: t_curr_fault    -- 保存返回的当前故障
返回值  ：无														   				
**************************************************************/
void v_fault_read_curr_fault(CURR_FAULT_DATA_T *t_curr_fault, U16_T u16_fault_index);


/*************************************************************
函数名称: u8_fault_get_fault_class		           				
函数功能: 通过故障ID查找故障的显示大类						
输入参数: u16_id -- 故障ID        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
U8_T u8_fault_get_fault_class(void);


/*************************************************************
函数名称: v_fault_fault_init		           				
函数功能: 初始化故障相关的互斥量，将复位前的当前告警记录添加到末复归告警记录中，
          清除dataflash中的当前告警记录，此函数在系统上电后的初始化部分调用					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fault_fault_init(void);


/*************************************************************
函数名称: v_fault_handle_task		           				
函数功能: 故障处理任务函数						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
__task void v_fault_handle_task(void);



#endif
