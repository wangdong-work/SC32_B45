/************************************************************
Copyright (C), 深圳英可瑞科技开发有限公司, 保留一切权利
文 件 名：BSInternal.h
版    本：1.00
创建日期：2013-03-01
作    者：郭数理
功能描述：内部MODBUS规约头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2013-03-01  1.00     创建
***********************************************************/


#ifndef __BS_INTERNAL_H_
#define __BS_INTERNAL_H_

#define DC10_START_ADDR            0x40	/*0x40*/

#define MODBUS_FUNC_CODE_02        0x02
#define MODBUS_FUNC_CODE_04        0x04
#define MODBUS_FUNC_CODE_03        0x03
#define MODBUS_FUNC_CODE_06        0x06
#define MODBUS_FUNC_CODE_16        0x10

/*************************************************************
函数名称: v_internal_dc10_handle		           				
函数功能: DC10协议处理函数						
输入参数: u8_code         -- 功能码
          u16_reg         -- 寄存器地址
          u16_cnt_or_data -- 寄存器数量或者设置数据     		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_internal_dc10_handle(U8_T u8_code, U16_T u16_reg, U16_T u16_cnt_or_data);

/*************************************************************
函数名称: v_internal_run		           				
函数功能: 内部modubs协议运行函数						
输入参数: 无     		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_internal_run(void);

#endif
