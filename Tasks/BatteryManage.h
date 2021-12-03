/************************************************************
Copyright (C), 2012-2020, 深圳英可瑞科技开发有限公司
文 件 名：BatteryManage.h
版    本：1.00
创建日期：2012-03-22
作    者：刘文锋
功能描述：电池管理头文件

修改记录：
	作者      日期        版本     修改内容
	刘文锋    2012-05-25  1.00     创建
**************************************************************/

#ifndef __BATTERYMANAGE_H_
#define __BATTERYMANAGE_H_

#include "Type.h"

/*
缩写     全称                   翻译
batm     Battery Management    电池管理

*/


#define	BATM_TIME_SLOT					1			//电池充电状态管理间间间隔，以S为单位,要求其是10的约数
#define BATM_CUN_CAP_TIME_SLOT			10			//每10S计算一次电池容量
#define BATM_DISCHARGE_GAP				2			//核容状态下，充电机输出设定电压比终止电压低2V
#define BATM_MDL_MAX_LIMIT_PERCENT		105.0		//模块最大限流点百分比
#define BATM_MDL_MIN_LIMIT_PERCENT		5.0			//模块最小限流点百分比
#define BATM_CAPACITY_COEFF             1000        //保存电池容量所用的系数


#define	BAT_CUR_MAX(a, b)				( ((a)>(b))?(a):(b) )	  //求两数中最大值宏
#define	BAT_CUR_MIN(a, b)				( ((a)>(b))?(b):(a) )	  //求两数中最小值宏


typedef struct
{

	F32_T	f32_charge_volt;		 		 //控制充电机输出设置电压
	F32_T	f32_charge_limit_percent;		 //控制充电机输出限流点	千分比表示
	F32_T	f32_charge_limit_abs_vaule;		 //充电机输出限流值，以绝对值表示
	F32_T	f32_bat_capacity;		 	     //当前电池容量
	U16_T	u16_charge_mode;			 	 //当前电池充电模式
	U16_T	u16_persist_equ_t;               // 均充累加时间，以分为单位
	U16_T	u16_equ_count_down;				 // 均充倒计时器，以分为单位
	U16_T	u16_persist_flo_t;               // 浮充累加时间，以时为单位
	U16_T	u16_oc_durable_t; 				 //浮充电流持维大于设定转均充电流点,以秒为单位
	U16_T	u16_persist_chd_t;               // 核容累加时间，以分为单位 
	U16_T	u16_lost_ac_t;					 //浮充状态下，交流停电计时器，以分为单位
}BATM_CHARGE_DATA_T;
						   

/*************************************************************
函数名称: v_batm_battery_manage_task		           				
函数功能: 电池管理任务，实现电池均浮充转换、充电机限流调节&电压调节，该任务可通过RTX函数os_tsk_create来启动						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
extern __task void v_batm_battery_manage_task(void);

#endif
