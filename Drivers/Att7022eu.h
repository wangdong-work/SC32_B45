/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：key.c
版    本：1.00
创建日期：2012-05-29
作    者：肖江
功能描述：ATT7022E chip的头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	肖江    2012-05-29    1.00     创建
**************************************************************/
 /*this file for ATT70E,content about  struct and commonder*/
#include "LPC17xx.h"
#include "type.h"
#include <RTL.h> 
#define  KPC_UDELAY  3 

//ATT外设计量寄存器定义
#define  V_A1  0x0D      //第一组A相有效值电压 ，只读
#define  V_B1  0x0E      //第一组B相有效值电压 ，只读
#define  V_C1  0x0F      //第一组C相有效值电压 ，只读
#define  V_A2  0x10      //第二组A相有效值电压 ，只读
#define  V_B2  0x11      //第二组B相有效值电压 ，只读
#define  V_C2  0x12      //第二组C相有效值电压 ，只读
//ATT外设特殊寄存器定义
#define  CLEAN_JB 0xC3   //清除校表，数据为data=0x000000，只写
#define  JB_ROUT  0xC6   //用于校表数据读出，只写，上电复位默认是ATT_read读的是计量数据
                         //当data！=0x00005A，ATT_read读的是计量数据
	         				//当data=0x00005A，ATT_read读的是校表数据
#define  JB_WEN   0xc9   //用于校表数据写使能，只写，上电复位默认校表写使能
                         //当data！=0x00005A，关闭校表的写使能
	         			//当data=0x00005A，ATT_write可通过SPI写校表数据
#define  ATT_RESET 0xD3  //用于软件重启ATT，数据为0x000000。只写
/********************************************
 *  函数名称： void  v_att_spi_init(U32_T baurate); 
 *  输入参数:  波特率控制
 *  返回结果:   无
 *  功能介绍:   SPI初始化 //baurate必须为偶数大于8。范围8-254；
  SPI0 SCK rate calculated as: PCLK_SPI / baurate value.
  PCLK_SPI=FCLK/8;
*********************************************/
void  v_att_spi_init(U32_T baurate); 




/********************************************
 *  函数名称：  U32_T att_att_read(U32_T regaddr);
 *  输入参数:   regaddr  见上表的寄存器定义   
 *  返回结果:	读取的reg值 低24bit有效 实际的有效值为返回值右移13bit
 *  功能介绍:   ATT reg读取
*********************************************/
U32_T att_att_read(U32_T regaddr);




/********************************************
 *  函数名称： void  v_att_att_write(U32_T regaddr, U32_T data);
 *  输入参数:   regaddr  见上表的寄存器定义  
                data     低24位有效，表示要更新的寄存器的数据
 *  返回结果:	无
 *  功能介绍:   主要用于校表的更新
*********************************************/
void  v_att_att_write(U32_T regaddr, U32_T data);





/********************************************
 *  函数名称： void v_att_att_init(void);
 *  输入参数:   无
 *  返回结果:   无
 *  功能介绍:   ATT初始化，配置它的模式，与增益
*********************************************/
void v_att_att_init(void);




/********************************************
 *  函数名称： void v_att_att_jz(U32_T flag);
 *  输入参数:  V_value 任意的电压有效值,两组接同一个有效值电压
               data1--校表后的增益系数，
 *  返回结果:   无
 *  功能介绍:   ATT校准--增益校准

*********************************************/
void v_att_att_jz(U32_T flag); 




 
/********************************************
 *  函数名称： __task void v_att_atttask(void);
 *  输入参数:  
               
 *  返回结果:   无
 *  功能介绍:   任务函数

*********************************************/
__task void v_att_atttask(void);



