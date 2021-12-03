/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：Main.c
版    本：1.00
创建日期：2012-03-19
作    者：郭数理
功能描述：主函数实现文件

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-03-19  1.00     创建
**************************************************************/

#include <lpc17xx.h>
#include <system_lpc17xx.h>
#include <rtl.h>
#include <string.h>
#include <stdio.h>

#include "../Drivers/Dataflash.h"
#include "../Drivers/Lcd.h"
#include "../Drivers/Usb.h"
#include "../Drivers/Delay.h"
#include "../Drivers/PinConfig.h"
#include "../Drivers/Rtc.h"
#include "../Drivers/Relay.h"
#include "../Drivers/key.h"
#include "../Drivers/uart_device.h"
#include "../Drivers/Att7022eu.h"
#include "../Drivers/Switch.h"
#include "../Drivers/Wdt.h"
#include "../Drivers/IRIGB.h"
#include "../Drivers/CAN.h"

#include "CompareTime.h"
#include "PublicData.h"
#include "Display.h"
#include "FaultId.h"
#include "Com1Comm.h"
#include "Fault.h"
#include "batterymanage.h"
#include "Log.h"
#include "WdtTask.h"
#include "DCcollect.h"
#include "Backstage.h"
#include "LoadFile.h"
#include "CanComm.h"
#include "Encry.h"


#define DEBUG(fmt, ...) u32_usb_debug_print(fmt, ##__VA_ARGS__)


__task void v_main_system_init_task(void)
{
	U32_T u32_can_baud;

	//驱动级的初始化函数先调用
	v_relay_relay_init_mutex();
	v_flash_dataflash_init_mutex();
	v_wdt_wdt_init_mutex();

	//应用级的初始化函数后调用
	v_fauid_fault_mbx_init();
	v_disp_cfg_data_init();      //配置数据区初始化
	v_fault_fault_init();
	v_log_mut_init();

    os_mut_wait(g_mut_share_data, 0xFFFF);
	switch(g_t_share_data.t_sys_cfg.t_sys_param.e_can_baud)
	{
		case CAN_BAUD_50K:
			u32_can_baud = 50000;
			break;
		case CAN_BAUD_20K:
			u32_can_baud = 20000;
			break;
		case CAN_BAUD_10K:
			u32_can_baud = 10000;
			break; 		
		case CAN_BAUD_125K:
		default:
			u32_can_baud = 125000;
			break;	
	}
	os_mut_release(g_mut_share_data);

	v_can_init(1, u32_can_baud);
	v_can_start(1);
	v_can_timer_init();

	if (u8_encry_judge_encry() == 1)   //加密正确则正常运行所有任务
	{
		g_tid_wdt = os_tsk_create(v_wtask_wdt_task, 3);
		g_tid_compare_time = os_tsk_create(v_ctime_compare_time_task, 1);
		g_tid_fault = os_tsk_create(v_fault_handle_task, 1);
		g_tid_com1_comm = os_tsk_create(v_com1_module_comm_task, 1);
		g_tid_batt = os_tsk_create(v_batm_battery_manage_task, 1);
		g_tid_dc_sample = os_tsk_create(v_dc_col_task,1);
		g_tid_ac_sample = os_tsk_create(v_att_atttask,1);
		g_tid_swt_sample = os_tsk_create(v_switch_switchtask,1);
		g_tid_display = os_tsk_create(v_disp_display_task, 1);
		g_tid_key = os_tsk_create(v_key_keytask, 2);
		g_tid_bs = os_tsk_create(v_bs_backstage_task, 1);
		g_tid_load = os_tsk_create(v_load_load_file_task, 1);
		g_tid_can_comm = os_tsk_create(v_can_module_can_comm_task, 1);
	}
	else                              //加密错误则只运行烧写任务
	{
		g_tid_load = os_tsk_create(v_load_load_file_task, 1);
	}

	os_tsk_delete_self();
}

int main(void)
{
	v_pcfg_pin_config();
	v_delay_delay_init();
	v_adc_init();
	v_relay_relay_init();
	v_rtc_rtc_init();
	v_usb_usb_init();
	v_flash_dataflash_init();
	v_key_key_init(); 
	v_switch_switch_init();
	v_lcd_lcd_init();
	v_wdt_wdt_init();
	Uart_init(0, 9600, 0x0B);    //波特率9600，奇校验位、8位数据位、1位停止位（N81）
	Uart_init(1, 9600, 0x0B);    //波特率9600，奇校验位、8位数据位、1位停止位（N81）
	//Uart_init(2, 9600, 0x0B);    //波特率9600，奇校验位、8位数据位、1位停止位（N81）
	v_irigb_compare_time_init();

	os_sys_init(v_main_system_init_task);
}

