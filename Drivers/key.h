/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：key.h
版    本：1.00
创建日期：2012-05-29
作    者：肖江
功能描述：按键驱动头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	肖江    2012-05-29    1.00     创建
**************************************************************/
#include <RTL.h>
#include "type.h"

/********************************************
 *  函数名称： void v_key_init()
 *  输出参数:   无
 *  返回结果:   无
 *  功能介绍:   对按键端口初始化，p1.28=k_clk=O,p1.29=K_data=I,p2.8=K_push=O
 端口默认方式为GPIO，上拉，DIR=输入
*********************************************/
void  v_key_key_init(void);




/********************************************
 *  函数名称： U32_T key_data()
 *  输入参数:   无
 *  返回结果:   按键值
 *  功能介绍:   采集按键的数据，返回键值
*********************************************/
U32_T key_key_data(void);




/********************************************
 *  函数名称： __task void v_key_keytask(void);
 *  输入参数:   无
 *  返回结果:   滤波后的按键值
 *  功能介绍:   采集按键的数据，返回键值，次函数做延时处理
*********************************************/
 __task void v_key_keytask(void);
