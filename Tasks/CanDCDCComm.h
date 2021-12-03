/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：CanDCDCComm.cpp
版    本：1.00
创建日期：2013-08-22
作    者：郭数理
功能描述：通信48V模块的通信及数据处理头文件


修改记录：
	作者      日期        版本     修改内容
	郭数理    2013-08-22  1.00     创建
**************************************************************/


#ifndef __CAN_DCDC_COMM_H
#define __CAN_DCDC_COMM_H


#define FDL_SEND_CFG_INTERVAL  (60*OSC_SECOND)  //60s广播一次馈线模块配置数据
#define DCDC_SEND_CFG_INTERVAL (30*OSC_SECOND)  //30s广播一次通信48V模块配置数据
#define ID_DCDC_MODULE_TYPE    0x12000000        //通信48V模块设备类型

#define DCDC_SET_VOLT_CMD       0    //设定控制电压命令
#define DCDC_SET_CURR_CMD       1    //设定电流命令
#define DCDC_SET_THR_CMD        2    //设定电压、电流最大值/最小值

#define DCDC_QUERY_CMD_MAX_NUM  3    //查询命令个数


extern const U32_T m_u32_dcdc_query_cmd[];


/*************************************************************
函数名称: v_can_dcdc_module_comm_fault_alm
函数功能: 通信48V模块增加通信中断计时，超过一定的时间未通信上，报通信中断故障
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_comm_fault_alm(void);


/*************************************************************
函数名称: v_can_dcdc_module_receive_handle
函数功能: 通信48V模块接收处理函数
输入参数: 无
          无
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_receive_handle(void);


/*************************************************************
函数名称: v_can_dcdc_module_send_config
函数功能: 广播下发通信48V模块配置数据处理函数，保存到EEPROM
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_send_config(void);


/*************************************************************
函数名称: v_can_dcdc_module_send_ctl
函数功能: 广播下发通信48V模块控制数据处理函数，不保存到EEPROM
输入参数: u8_cmd_index -- DCDC_SET_VOLT_CMD: 设定控制电压命令
                          DCDC_SET_CURR_CMD: 设定电流命令
                          DCDC_SET_THR_CMD:  设定电压、电流最大值/最小值
输出参数: 无
返回值  ：无
**************************************************************/
void v_can_dcdc_module_send_ctl(U8_T u8_cmd_index);


#endif
