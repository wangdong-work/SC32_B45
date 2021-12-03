/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Dataflash.h
版    本：1.00
创建日期：2012-03-12
作    者：郭数理
功能描述：AT45DB08驱动头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-12  1.00     创建
**************************************************************/

#ifndef __DATAFLASH_H_
#define __DATAFLASH_H_

#include "Type.h"


#define DATAFLASH_SIZE (264*4096)


                                         //起始地址            //空间大小(字节）    //存放内容
#define DATAFLASH_MODEL_ADDR             0                      
#define DATAFLASH_FONT_ADDR              1                     //200*264*4          //字库
#define	DATAFLASH_CFG_DATA1_ADDR         (200*264*4)           //4*264*4            //第1份设置数据
#define DATAFLASH_CFG_DATA2_ADDR         (204*264*4)           //4*264*4            //第2份设置数据
#define DATAFLASH_CFG_DATA3_ADDR         (208*264*4)           //4*264*4            //第3份设置数据
#define	DATAFLASH_ADJUST1_ADDR           (212*264*4)           //264                //第1份校准数据
#define	DATAFLASH_ADJUST2_ADDR           (212*264*4 + 264)     //264                //第2份校准数据
#define	DATAFLASH_ADJUST3_ADDR           (212*264*4 + 264*2)   //264                //第2份校准数据
#define	DATAFLASH_RESERVE1_ADDR          (212*264*4 + 264*3)   //264                //第1份保留数据，为了兼容老版本
#define	DATAFLASH_RESERVE2_ADDR          (212*264*4 + 264*4)   //264                //第2份保留数据，为了兼容老版本
#define DATAFLASH_RESERVE3_ADDR          (212*264*4 + 264*5)   //264                //第3份保留数据，为了兼容老版本
#define DATAFLASH_RECORD_ADDR            (212*264*4 + 264*6)   //4*264*4            //事件记录
#define DATAFLASH_HIS_FAULT_ADDR         (216*264*4 + 264*6)   //16*264*4           //历史告警记录
#define DATAFLASH_NO_RESUME_FAULT_ADDR   (232*264*4 + 264*6)   //8*264*4            //历史末复归告警记录
#define DATAFLASH_CURR_FAULT_ADDR        (240*264*4 + 264*6)   //4*264*4            //当前告警记录
#define DATAFLASH_BRANCH_NAME_ADDR       (244*264*4 + 264*6)   //30*264*4           //自定义支路名称
#define DATAFLASH_FAULT_NAME_ADDR        (274*264*4 + 264*6)   //240*264*4          //自定义告警名称
#define DATAFLASH_YC_DATA_ADDR           (514*264*4 + 264*6)   //8*264*4            //自定义遥测点表
#define DATAFLASH_YX_DATA_ADDR           (522*264*4 + 264*6)   //20*264*4           //自定义遥信点表
#define DATAFLASH_YK_DATA_ADDR           (542*264*4 + 264*6)   //264*4              //自定义遥控点表
#define DATAFLASH_YT_DATA_ADDR           (542*264*4 + 264*10)  //264*4              //自定义遥调点表
#define DATAFLASH_ABOUT_ADDR             (542*264*4 + 264*14)  //264*4              //自定义关于信息
#define DATAFLASH_SWT_ADDR               (542*264*4 + 264*18)  //2*264*4            //自带开关量配置信息
#define DATAFLASH_ENCRY_ADDR             (DATAFLASH_SIZE-264)            //加密字符串

#define DATAFLASH_ENCRY_SIZE             18             //加密字符串实际大小
#define DATAFLASH_ABOUT_ITEM_SIZE        44             //关于条目实际大小
#define DATAFLASH_BRANCH_NAME_ITME_SIZE  40             //支路名称实际大小
#define DATAFLASH_FAULT_NAME_ITEM_SIZE   80             //告警名称实际大小
#define DATAFLASH_SWT_ITEM_SIZE          68             //开关量配置条目大小

#define DATAFLASH_BRANCH_NAME_SIZE       (64*11*DATAFLASH_BRANCH_NAME_ITME_SIZE)            //支路名称实际大小
#define DATAFLASH_FAULT_NAME_SIZE        (2638*DATAFLASH_FAULT_NAME_ITEM_SIZE)              //告警名称实际大小
#define DATAFLASH_SWT_SIZE               (20*DATAFLASH_SWT_ITEM_SIZE)                       //开关量配置实际大小

#define DATAFLASH_AC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 0)            //交流分组起始地址
#define DATAFLASH_DC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 20*80)        //直流分组起始地址
#define DATAFLASH_BATT1_GROUP_START_ADDR       (DATAFLASH_FAULT_NAME_ADDR + 52*80)        //1组电池异常起始地址
#define DATAFLASH_BATT2_GROUP_START_ADDR       (DATAFLASH_FAULT_NAME_ADDR + 292*80)       //2组电池异常起始地址
#define	DATAFLASH_BMS_GROUP_START_ADDR         (DATAFLASH_FAULT_NAME_ADDR + 532*80)       //电池巡检故障起始地址
#define	DATAFLASH_RECT_GROUP_START_ADDR        (DATAFLASH_FAULT_NAME_ADDR + 542*80)       //整流模块故障起始地址
#define DATAFLASH_DCAC_GROUP_START_ADDR        (DATAFLASH_FAULT_NAME_ADDR + 662*80)       //逆变模块故障起始地址
#define	DATAFLASH_DCDC_GROUP_START_ADDR        (DATAFLASH_FAULT_NAME_ADDR + 710*80)       //通信模块故障起始地址
#define DATAFLASH_FC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 734*80)       //馈线模块故障起始地址
#define DATAFLASH_RC_GROUP_START_ADDR          (DATAFLASH_FAULT_NAME_ADDR + 768*80)       //RC10模块故障起始地址
#define DATAFLASH_SWT_GROUP_START_ADDR         (DATAFLASH_FAULT_NAME_ADDR + 784*80)       //自带开关量故障起始地址
#define	DATAFLASH_DC_PANEL1_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 836*80)       //分电屏01故障起始地址
#define DATAFLASH_DC_PANEL2_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1029*80)      //分电屏02故障起始地址
#define DATAFLASH_DC_PANEL3_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1222*80)      //分电屏03故障起始地址
#define	DATAFLASH_DC_PANEL4_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1415*80)      //分电屏04故障起始地址
#define DATAFLASH_DC_PANEL5_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1608*80)      //分电屏05故障起始地址
#define DATAFLASH_DC_PANEL6_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1801*80)      //分电屏06故障起始地址
#define	DATAFLASH_DC_PANEL7_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 1994*80)      //分电屏07故障起始地址
#define DATAFLASH_DC_PANEL8_GROUP_START_ADDR   (DATAFLASH_FAULT_NAME_ADDR + 2187*80)      //分电屏08故障起始地址
#define DATAFLASH_DCDC_PANEL_GROUP_START_ADDR  (DATAFLASH_FAULT_NAME_ADDR + 2380*80)      //通信屏故障起始地址
#define DATAFLASH_DCAC_PANEL_GROUP_START_ADDR  (DATAFLASH_FAULT_NAME_ADDR + 2509*80)      //UPS屏故障起始地址
#define DATAFLASH_AC_PANEL_GROUP_START_ADDR    (DATAFLASH_FAULT_NAME_ADDR + 2638*80)      //交流屏故障起始地址

/*************************************************************
函数名称: v_flash_dataflash_init		           				
函数功能: 初始化dataflash, 配置与dataflash相关的spi口						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_flash_dataflash_init(void);


/*************************************************************
函数名称: u8_flash_read_font_task_pin
函数功能: 读取字库和型号烧写控制引脚的状态
输入参数: 无
输出参数: 无
返回值  ：1：允许烧写；0：不允许烧写
**************************************************************/
U8_T u8_flash_read_font_task_pin(void);


/*************************************************************
函数名称: v_flash_dataflash_init_mutex		           				
函数功能: 初始化dataflash相关的互斥量，在RTX启动之后调用，在第一个任务的开头部分调用						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_flash_dataflash_init_mutex(void);

/*************************************************************
函数名称: s32_flash_dataflash_read		           				
函数功能: 读取dataflash数据						
输入参数: u32_from -- 要读取的地址
		  u32_len  -- 要读取的长度        		   				
输出参数: pu8_buf  -- 返回读取到的内容，调用者需为其分配足够的空间（长度不小于len)
返回值  ：返回0表示成功，若返回-1则表示出错          		   															   				
**************************************************************/
S32_T s32_flash_dataflash_read(U32_T u32_from, U8_T *pu8_buf, U32_T u32_len);

/*************************************************************
函数名称: s32_flash_dataflash_write		           				
函数功能: 写数据到dataflash						
输入参数: u32_to -- 要写入的地址
		  u32_len  -- 要写入数据的长度        		   				
          pu8_buf  -- 指向要写入的数据
输出参数: 无
返回值  ：返回0表示成功，若返回-1则表示出错          		   															   				
**************************************************************/
S32_T s32_flash_dataflash_write(U32_T u32_to, U8_T *pu8_buf, U32_T u32_len);


#endif
