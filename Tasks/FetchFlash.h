/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：FetchFlash.h
版    本：1.00
创建日期：2012-05-19
作    者：郭数理
功能描述：从dataflash存取数据头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-05-19  1.00     创建
**************************************************************/


#ifndef __FETCH_FLASH_H_
#define __FETCH_FLASH_H_


/*************************************************************
函数名称: v_fetch_save_cfg_data		           				
函数功能: 保存配置数据到dataflash，保存三份，具体格式见文件：IARM-SC32 DATAFLASH空间地址定义						
输入参数: 无         		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fetch_save_cfg_data(void);


/*************************************************************
函数名称: s32_fetch_read_cfg_data		           				
函数功能: 从dataflash读取配置数据，从三份数据中读取正确的数据，并用正确的数据重写不正确的数据					
输入参数: 无         		   				
输出参数: 无
返回值  ：读取成功则返回0，失败返回-1														   				
**************************************************************/
S32_T s32_fetch_read_cfg_data(void);


/*************************************************************
函数名称: v_fetch_read_swt_cfg_data
函数功能: 从dataflash读取开关配置数据
输入参数: 无
输出参数: 无
返回值  ：无
**************************************************************/
void v_fetch_read_swt_cfg_data(void);


/*************************************************************
函数名称: v_fetch_save_adjust_coeff		           				
函数功能: 保存校准系数到dataflash，保存三份，具体格式见文件：IARM-SC32 DATAFLASH空间地址定义						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_fetch_save_adjust_coeff(void);


/*************************************************************
函数名称: s32_fetch_read_adjust_coeff		           				
函数功能: 从dataflash读取校准系数，从三份数据中读取正确的数据，并用正确的数据重写不正确的数据					
输入参数: e_type   -- 校准类型，AC_ADJUST - 交流校准， DC_ADJUST - 直流校准          		   				
输出参数: pt_coeff -- 保存返回的系数
返回值  ：读取成功则返回0，失败返回-1														   				
**************************************************************/
S32_T s32_fetch_read_adjust_coeff(void);


#endif
