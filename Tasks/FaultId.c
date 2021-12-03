/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：FaultId.c
版    本：1.00
创建日期：2012-05-16
作    者：郭数理
功能描述：发送接收故障ID函数实现文件


修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-05-16  1.00     创建
**************************************************************/


#include <rtl.h>

#include "Type.h"
#include "FaultId.h"


os_mbx_declare(m_mbx_fault, 256);  //邮箱消息数设为256，设这么大的原因是避免系统在矩时间内产生大量的故障，导致邮箱阻塞，
                                   //如果在邮箱阻塞的时候持有互斥锁的话，可能会导致系统死锁


/*************************************************************
函数名称: v_fauid_fault_mbx_init		           				
函数功能: 初始化故障ID的邮箱，需在RTX启动后，其它任务启动前调用						
输入参数: 无         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fauid_fault_mbx_init(void)
{
	os_mbx_init(m_mbx_fault, sizeof(m_mbx_fault));
}

/*************************************************************
函数名称: v_fauid_send_fault_id_occur		           				
函数功能: 故障发生时，调用此函数通过邮箱发送故障ID						
输入参数: u16_fault_id -- 故障ID         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fauid_send_fault_id_occur(U16_T u16_fault_id)
{
	U32_T u32_msg;

	u32_msg = (FAULT_OCCUR | u16_fault_id);
	os_mbx_send(m_mbx_fault, (void *)u32_msg, 0xFFFF);
}

/*************************************************************
函数名称: v_fauid_send_fault_id_resume		           				
函数功能: 故障恢复时，调用此函数通过邮箱发送故障ID						
输入参数: u16_fault_id -- 故障ID         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fauid_send_fault_id_resume(U16_T u16_fault_id)
{
	U32_T u32_msg;

	u32_msg = (FAULT_RESUME | u16_fault_id);
	os_mbx_send(m_mbx_fault, (void *)u32_msg, 0xFFFF);
}

/*************************************************************
函数名称: u32_fauid_recv_fault_id		           				
函数功能: 接收故障ID，此函数由故障处理任务调用，获取其它任务发送过来的任务ID						
输入参数: timeout -- 超时值         		   				
输出参数: 无
返回值  ：故障ID														   				
**************************************************************/
U32_T u32_fauid_recv_fault_id(U16_T timeout)
{
	U32_T *u32_msg;

	if (os_mbx_wait (m_mbx_fault, (void **)(&u32_msg), timeout) == OS_R_TMO)
		return FAULT_INVALID;
	else
		return (U32_T)u32_msg;
}



