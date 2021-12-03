/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：backstage.h
版    本：1.00
创建日期：2012-06-14
作    者：郭数理
功能描述：后台通信头文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-06-14  1.00     创建
***********************************************************/

#ifndef __BACKSTAGE_H_
#define __BACKSTAGE_H_

#include "Type.h"
#include "PublicData.h"
#include "BackstageTable.h"

#define BS_COM_PORT_NUM             0      //串口端口号
#define BS_SEND_BUF_SIZE            512    //发送缓冲区大小
#define BS_RECV_BUF_SIZE            256    //接收缓冲区大小

#define BS_YC_FEEDER_SIZE         	2048      //支路遥测数据个数
#define BS_YX_FEEDER_SIZE         	(2048*3)  //支路遥信数据个数
#define BS_YX_FEEDER_ALARM_INDEX  	0         //支路跳闸索引
#define BS_YX_FEEDER_STATE_INDEX  	(2048/16) //支路状态索引
#define BS_YX_FEEDER_INSU_INDEX   	(4096/16) //支路绝缘索引


extern U8_T                        g_u8_addr;                   //后台通信本机地址备份
extern COM_BAUD_E                  g_e_baud;                    //后台通信波特率备份

extern U8_T g_u8_bs_send_buf[BS_SEND_BUF_SIZE];                 //发送缓冲区
extern U8_T g_u8_bs_recv_buf[BS_RECV_BUF_SIZE];                 //接收缓冲区
extern U8_T g_u8_bs_recv_len;                                   //接收缓冲区的数据的长度

extern S16_T g_s16_bs_modbus_yc[BS_YC_SIZE];                    //MODBUS遥测数据区
extern U16_T g_u16_bs_mdobus_yc_size;                           //MODBUS遥测数据区实际大小
extern U16_T g_u16_bs_cdt_yc[BS_YC_SIZE];                       //CDT遥测数据区
extern U16_T g_u16_bs_cdt_yc_size;                              //CDT遥测数据区实际大小
extern S16_T g_s16_bs_modbus_yc_feeder[BS_YC_FEEDER_SIZE];      //MODBUS支路遥测数据区

extern U16_T g_u16_bs_yx[BS_YX_SIZE/16+1];                      //遥信数据区
extern U16_T g_u16_bs_yx_size;                                  //遥信数据区实际大小
extern U32_T g_u32_bs_yx_max_addr;                              //遥信最大地址
extern U16_T g_u16_bs_yx_feeder[BS_YX_FEEDER_SIZE/16+1];        //支路遥信数据区

extern U16_T g_u16_bs_yk[BS_YK_SIZE];                           //遥信数据区
extern U16_T g_u16_bs_yk_size;                                  //遥信数据区实际大小

extern U16_T g_u16_bs_yt[BS_YT_SIZE];                           //遥调数据区
extern U16_T g_u16_bs_yt_size;                                  //遥调数据区实际大小



/*************************************************************
函数名称: u32_bs_calculate_time		           				
函数功能: 计算串传输byte_cnt个字节所需时间						
输入参数: byte_cnt -- 字节数
          baud     -- 波特率        		   				
输出参数: 无
返回值  ：传输所需时间，单位为ms														   				
**************************************************************/
U32_T u32_bs_calculate_time(U32_T byte_cnt, COM_BAUD_E baud);


/*************************************************************
函数名称: v_bs_send_data		           				
函数功能: 通过串口发送数据						
输入参数: pu8_buf -- 指向要发送的数据区
          len     -- 要发送的字节数        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_bs_send_data(U8_T *pu8_buf, U32_T len);


/*************************************************************
函数名称: s32_bs_yk_handle		           				
函数功能: 后台遥控处理函数						
输入参数: addr -- 遥控地址
          val  -- 要设置的数值       		   				
输出参数: 无
返回值  ：0:设置成功，-1：设置失败														   				
**************************************************************/
S32_T s32_bs_yk_handle(U16_T addr, U16_T val);


/*************************************************************
函数名称: s32_bs_yt_handle		           				
函数功能: 后台遥调处理函数						
输入参数: addr -- 遥调地址
          val  -- 要设置的数值       		   				
输出参数: 无
返回值  ：0:设置成功，-1：设置失败														   				
**************************************************************/
S32_T s32_bs_yt_handle(U16_T addr, U16_T val);


/*************************************************************
函数名称: v_backstage_task		           				
函数功能: 后台通信任务函数，从全局共享数据区中提取后台数据区，处理后台相关的通信						
输入参数: 无       		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
__task void v_bs_backstage_task(void);

#endif
