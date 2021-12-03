/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Com1Comm.h
版    本：1.00
创建日期：2012-08-01
作    者：郭数理
功能描述：下级模块的通信及数据处理任务头文件


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-08-01  1.00     创建
**************************************************************/

#ifndef __COM1_COMM_H_
#define __COM1_COMM_H_



#define DCDC_MODULE_PROTECT_MASK           0x0004 //通信模块保护状态屏蔽码
#define DCDC_MODULE_FAULT_MASK             0x0008 //通信模块故障状态屏蔽码
#define DCDC_MODULE_EXCEPTION_MASK         (DCDC_MODULE_PROTECT_MASK | DCDC_MODULE_FAULT_MASK)


/*************************************************************
函数名称: v_com1_dcdc_module_send_fault_id		           				
函数功能: 通信模块发送故障ID函数						
输入参数: u16_state_old -- 旧的模块状态值
          u16_state_new -- 新的模块状态值       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_com1_dcdc_module_send_fault_id(U8_T u8_module_index, U16_T u16_state_old, U16_T u16_state_new);



/*************************************************************
函数名称: v_com1_module_comm_task		           				
函数功能: 下级模块通信及数据处理任务函数						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
__task void v_com1_module_comm_task(void);

#endif
