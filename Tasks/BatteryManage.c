/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：BatteryManage.c
版    本：1.00
创建日期：2012-05-25
作    者：刘文锋
功能描述：电池管理实现文件，实现电池充电状态处理&充电模块输出电压控制、电池容量计算、限流调节

函数列表：

修改记录：
	作者      日期        版本     修改内容
	刘文锋    2012-05-25  1.00     创建
	钟欲飞	  2012-10-25  1.01     增加第二组电池容量计算以及第二组电池的充放电管理。
	钟欲飞	  2012-10-31  1.02     充电状态改变后，对各充电状态下的时间计时器变量清零，以解决计时不准确的问题。
	                               增加核容指示干结点输出控制。
								   宏定义改为放入头文件中，并由原来的小写改为大写，以符合宏定的规则。
	钟欲飞	  2012-12-08  1.03     限流误差很大时，根据电池的组数来附不同的电池限流初始值。
**************************************************************/

#include <rtl.h>
#include <string.h>
#include <stdio.h>
#include <math.h>  

#include "../Drivers/Lcd.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Rtc.h"
#include "../Drivers/key.h"
#include "../Drivers/Delay.h"
#include "BatteryManage.h"

#include "PublicData.h"
#include "ShareDataStruct.h"
#include "Type.h"
#include "Log.h"
#include "Fault.h"
#include "FetchFlash.h"
#include "BatteryManage.h" 



static	BATM_CHARGE_DATA_T	m_t_batm_charge_data[2];		 	//电池充电管理数据
static	U16_T	m_u16_batm_copy_batt[2];					 	//设置电池充电状态备份
static	U16_T	m_u16_batm_relay_batt[2];						//用于电池均浮充结点输出指示，备分当前充电状态
static	BATT_MGMT_MODE_E	m_e_batm_copy_mode[2];				//备份系统管理模式

static	U32_T	m_u32_last_estimate_t[2] = {0};	   	        //上次充电状转换调时间点
static	U16_T	m_u16_flo_lost_ac_minute[2] = {0};	        //浮充状态下交流停电计时时累加器
static	U32_T	m_u32_last_ajdust_limit_t[2] = {0};	        //上次限流调节时间点
static	U32_T	m_u32_board_code_count[2] = {0};	        //两次限流计算间限流广播命令次数
static	U8_T	m_u8_dis_setup_basic_limit[2] = {1};        //放电状态下是否有设定过预限流值，1表示没有，0表示有
static	U8_T	m_u8_chr_setup_basic_limit[2] = {1};        //充电状态下是否有设定过预限流值，1表示没有，0表示有


static	U16_T	m_u16_flo_hour_acc[2] = {0};	   	   	    //浮充计时时累加器，
static	U16_T	m_u16_equ_limit_minute[2] = {0};	   	    //均充持维时间计时累加器	  
static	U16_T	m_u16_chd_limit_minute[2] = {0};	   	    //核容状态时间计时累加器 

static const F32_T m_f32_mdl_rated_cur[] = {5.0,7.0,10.0,20.0,30.0,35.0,40.0,50.0};	 //整流模块额定输出电流值


#define KI_DOT_NUM	10								//设置的放电曲线数据
/* 放电时间*/
const F32_T *f32_batt_dt[KI_DOT_NUM] = {
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_01c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_02c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_03c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_04c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_05c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_06c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_07c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_08c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_09c_dis_rate,
								&g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_10c_dis_rate,
						};

/* 放电电流率*/
F32_T	f32_batt_ki[KI_DOT_NUM] = {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};





/*************************************************************
函数名称: v_batm_save_record		           				
函数功能: 保存充电事件转换记录	 		 					
输入参数: recond_id 事件ID        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_batm_save_record(U16_T recond_id)
{
	RTC_TIME_T	pt_time;					
	LOG_DATA_T	log_data;

	v_rtc_rtc_get_time(&pt_time);		  	//取时间
	
   	log_data.u16_log_id = 	recond_id;		   
    log_data.u8_occur_year = (U8) (pt_time.year - 2000);
	log_data.u8_occur_mon = pt_time.month;
	log_data.u8_occur_day = pt_time.day;
	log_data.u8_occur_hour = pt_time.hour;
	log_data.u8_occur_min = pt_time.min;
	log_data.u8_occur_sec = pt_time.sec;

	v_log_save_record(&log_data);		 //保存事件信息
	v_log_set_log_state();				 //设置事件记录信息变化标志
}




/*************************************************************
函数名称: v_batm_reset_batm_t		           				
函数功能: 复位所有充电计时器为0	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/

void v_batm_reset_batm_t(U8_T no)
{
	m_u16_flo_hour_acc[no] = 0;	   	   	//浮充计时时累加器，
	m_u16_equ_limit_minute[no] = 0;	   	//均充持维时间计时累加器	  
	m_u16_chd_limit_minute[no] = 0;	   	//核容状态时间计时累加器 

	m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //浮充转换均充计时器清0
	m_t_batm_charge_data[no].u16_persist_flo_t = 0;   		  //持维浮充时间计时器清0
	m_t_batm_charge_data[no].u16_persist_equ_t = 0;			  //均充计时器清0
	m_t_batm_charge_data[no].u16_equ_count_down = 0;		  //均充倒计时清0	
	m_t_batm_charge_data[no].u16_persist_chd_t = 0;			  //核容计时器清0  	
	m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //清交流停电计时器
	g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = 0;   //充电状态累加时间
}



/*************************************************************
函数名称: v_batm_equ_manage		           				
函数功能: 电池均充管理，当均充时间到达设定的最大限时点，或均充倒计时时间到，则转为浮充
		  调用本函数前，需要先竞争到共享数据区的互斥量g_mut_share_data	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/

void v_batm_equ_manage(U8_T no)	
{
	if ( (m_t_batm_charge_data[no].u16_persist_equ_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_equ_time)
  	     || (m_t_batm_charge_data[no].u16_equ_count_down >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_curr_go_time) )
	{  
		m_e_batm_copy_mode[no] = AUTO_MODE;		 					 //核容结束后转自动电池管理方式
		g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] = m_e_batm_copy_mode[no];
		
		m_t_batm_charge_data[no].u16_charge_mode = FLO;
		g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no] = FLO;
		v_fetch_save_cfg_data();                                   //保存配置数据到dataflash 						
		
		v_batm_reset_batm_t(no); 
		v_batm_save_record(LOG_ID_BATT_AUTO_TO_FLO + LOG_ID_BATT_NUM * no);			  //存自动转浮充事件  		
	}
}			



/*************************************************************
函数名称: v_batm_flo_manage		           				
函数功能: 电池浮充管理，当浮充时间到达设定的周期均充点，或电池电流大于转均充电流点所需要的持续时间
		  或交流停电达到设定的转均充时间长度，则转为均充
		  调用本函数前，需要先竞争到共享数据区的互斥量g_mut_share_data	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/	  
void v_batm_flo_manage(U8_T no)	
{
	if ((m_t_batm_charge_data[no].u16_persist_flo_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_equ_cycle)
  	     || (m_t_batm_charge_data[no].u16_oc_durable_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_to_equ_dur_time)
		 ||	(m_t_batm_charge_data[no].u16_lost_ac_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_ac_fail_time))
	{
		m_t_batm_charge_data[no].u16_charge_mode = EQU;
		v_batm_reset_batm_t(no);
		v_batm_save_record(LOG_ID_BATT_AUTO_TO_EQU + LOG_ID_BATT_NUM * no);			  //存自动转均充事件 
	}
}


/*************************************************************
函数名称: v_batm_chd_manage		           				
函数功能: 电池核容管理，当核容时间到达设定的放电时间，或电池组电压低于设定的放电终止电压
		  或任一单体电池电压有低于设定的单体放电电压，则转为浮充
		  调用本函数前，需要先竞争到共享数据区的互斥量g_mut_share_data	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/	  
void v_batm_chd_manage(U8_T no)	
{
	U8_T  batt_bxx_num;	 //每组电池巡检单元个数
	F32_T min_cell_volt;	 //每组电池最小单体电压

	min_cell_volt = g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_min_cell_volt;
	batt_bxx_num  = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[no].u8_bms_num;

	if ( (m_t_batm_charge_data[no].u16_persist_chd_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_dis_time)
  	     || (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt)
		 /*如果有电池巡检并最低电压低于设定放电终目单体值，如无电池巡检，则本条件不生效*/
		 ||	((min_cell_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_cell_end_volt) ? batt_bxx_num : 0) )	   
	{ 			
		m_e_batm_copy_mode[no] = AUTO_MODE;		 					 //核容结束后转自动电池管理方式
		g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] = m_e_batm_copy_mode[no];

		m_t_batm_charge_data[no].u16_charge_mode = EQU;
		g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no] = EQU;
		v_fetch_save_cfg_data();                                   //保存配置数据到dataflash

		v_batm_reset_batm_t(no);
		v_batm_save_record(LOG_ID_BATT_AUTO_TO_EQU + LOG_ID_BATT_NUM * no);			  //存自动转均充事件 
	}
}



/*************************************************************
函数名称: v_batm_setup_charg_panl_out_vol		           				
函数功能: 根据充电状态，设置充电机输出电压值,如果核容状态，则设置输出电压为放电终止电压减去2V
		  浮充电压有采用温补，而均充则无温补
		  调用本函数前，需要先竞争到共享数据区的互斥量g_mut_share_data	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/	

void v_batm_setup_charg_panl_out_vol(U8_T no)
{ 	
	F32_T	  equalize_vol;			//温补值,以V为单位

	/*计算温补值，以25度为中心，当有电池巡检时，则用电池巡检上温度计算，如无，则用监控器上自带的温度计算*/
	if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[no].u8_bms_num != 0)
	{
		equalize_vol = (25.0-g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_temperature1) 
		               * g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt / 1000;		  //温补系数是以mV为单位
	 
	}
	else
	{
		equalize_vol = (25.0-g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature) 
		               * g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt / 1000;		 //温补系数是以mV为单位
	}

	/*最高温补值在+3.0V~-3.0V*/
	if (equalize_vol > 3.0)
	{
		equalize_vol = 3.0;
	}

	else if (equalize_vol < -3.0)
	{
		equalize_vol = -3.0;
	}



	switch (m_t_batm_charge_data[no].u16_charge_mode)
	{ 
		case	EQU:
				m_t_batm_charge_data[no].f32_charge_volt = 	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_equ_volt;
				g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[no] = m_t_batm_charge_data[no].f32_charge_volt;
				break;

		case	DIS:
				m_t_batm_charge_data[no].f32_charge_volt = 	(float)(g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt - BATM_DISCHARGE_GAP);
				g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[no] = m_t_batm_charge_data[no].f32_charge_volt;
				break;
			
		default:			
		case	FLO:
				m_t_batm_charge_data[no].f32_charge_volt = 	g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_flo_volt + equalize_vol;
				g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_out_volt[no] = m_t_batm_charge_data[no].f32_charge_volt;
				break;	
	}
}

/*************************************************************
函数名称: v_batm_count_single_battery_capability		           				
函数功能: 根据时间差和电池组号，计算单组电池容量	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_batm_count_single_battery_capability(F32_T dt, U8_T no)
{
	U8_T	i;
	F32_T	C10, Ki, Ib, Kc;

	/* 一组电池容量估算开始 */
    C10 = g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;   			// C10电池额定容量
    Ib  = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];  	// 一组电池电流 
    
	/* 充电 */
	if ( Ib >= 0 ) 
	{
		m_t_batm_charge_data[no].f32_bat_capacity += dt * Ib * 0.9500000; 		//固定充电效率为0.95  		
	}
	/* 放电 */
    else
	{
		if (Ib > -0.5)    //当放电电流比较小时，不计算电池容量
			goto save_capability;

		if (Ib < -C10)    //如果放电电流大于1C，则以1C的放电电流来计算容量
			Ib = -C10;

		Ki = Ib / C10;
		Ki = -Ki;
	
		for( i = 1; i < KI_DOT_NUM; i++ )
		{
			if ( Ki <= f32_batt_ki[i] )
				break;
		}

		if (i >= KI_DOT_NUM)						   				//下标保护，防止测量异常或电池电流大于C1时下标溢出
		{
			i = KI_DOT_NUM - 1;
	   	}

		Kc  = ( (*f32_batt_dt[i]) * f32_batt_ki[i]  - (*f32_batt_dt[i-1]) * f32_batt_ki[i-1] )
			 / ( f32_batt_ki[i] - f32_batt_ki[i-1] ); 	/* k */

		Kc *= ( Ki - f32_batt_ki[i-1] );		/* k * ( x - x0) */
		Kc += (*f32_batt_dt[i-1]) * f32_batt_ki[i-1];	/* k * ( x - x0) + x0 */
	
		if(  Kc <= 0 )
		{
			Kc = 0.0000001;
		}
	
		m_t_batm_charge_data[no].f32_bat_capacity +=  ( dt * Ib / Kc ); 
	}

save_capability:
	if ( m_t_batm_charge_data[no].f32_bat_capacity < 10.000 )
	{
		m_t_batm_charge_data[no].f32_bat_capacity = 10.000;
	}
	else if ( m_t_batm_charge_data[no].f32_bat_capacity > C10 )
	{
		m_t_batm_charge_data[no].f32_bat_capacity = C10;
	}

	g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_capacity = m_t_batm_charge_data[no].f32_bat_capacity;		  //刷新显示

	/*将容量记录到RTC寄存器，实现断电后下次上电可读取到之前计算的结果*/
	v_rtc_save_user_data((U32_T)(m_t_batm_charge_data[no].f32_bat_capacity*BATM_CAPACITY_COEFF), (RTC_GPREG_E)(GP_REG_1 + no));

	/* 一组电池容量估算结束 */
}

/*************************************************************
函数名称: v_batm_count_battery_capability		           				
函数功能: 电池容量计算，每10S时间间隔计算一次
		  充电时固定充电效率为0.95，放电时则根据设置的放电曲线进行系数的修正	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_batm_count_battery_capability(U8_T no)
{
	static	U32_T	last_count_cap_t = 0;	   //上次计算电池容量时间点	
	U32_T	this_count_cap_t;	       		   //本次计算电池容量时间点
	F32_T	dt;	

	/*根据界面操作来恢复电池容量*/
	if (os_evt_wait_or(BATT_CAPACITY_RESTORE, 0) == OS_R_EVT) 
	{ 	
		os_mut_wait(g_mut_share_data, 0xFFFF);					//等待互斥量
		m_t_batm_charge_data[0].f32_bat_capacity = g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;
		m_t_batm_charge_data[1].f32_bat_capacity = g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;
		g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity = m_t_batm_charge_data[0].f32_bat_capacity;		  //刷新显示 
		g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity = m_t_batm_charge_data[1].f32_bat_capacity;
		os_mut_release(g_mut_share_data);			 			//释放互斥量   		
	}


	this_count_cap_t = u32_delay_get_timer_val();

	if (u32_delay_time_elapse(last_count_cap_t, this_count_cap_t) < BATM_CUN_CAP_TIME_SLOT * OSC_SECOND)		 //每10S时间间隔判断一次
	{ 			
		return;
	}

	last_count_cap_t = (last_count_cap_t + BATM_CUN_CAP_TIME_SLOT * OSC_SECOND) % 0xFFFFFFFF;				  //更新时间记录点

	os_mut_wait(g_mut_share_data, 0xFFFF);								//等待互斥量
	
	dt =  (float)BATM_CUN_CAP_TIME_SLOT	/ 3600.0;		   				//计算充电时间间隔，以时表示	
	
	v_batm_count_single_battery_capability(dt, no);                      //一组电池容量计算
	
	/* 两组电池容量之和计算 */	
//	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num <= 1)
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity;
//	else	
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity + m_t_batm_charge_data[1].f32_bat_capacity;			  

	os_mut_release(g_mut_share_data);			 			//释放互斥量
}




/*************************************************************
函数名称: v_batm_estimate_charge_state		           				
函数功能: 充电状态转换,每1秒钟作一次判断	 		 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_batm_estimate_charge_state(U8_T no)
{
	U32_T	this_estimate_t, time_diff_t;	    //本次充电状态转换调节点，以uS为单位
	F32_T	refer_bat_cur;
    U32_T	u32_charge_state_time;              //充电状态及持续时间


	this_estimate_t = u32_delay_get_timer_val();

	time_diff_t = u32_delay_time_elapse(m_u32_last_estimate_t[no],this_estimate_t);
	if (time_diff_t < BATM_TIME_SLOT * OSC_SECOND)		 //每1S时间间隔判断一次
	{ 			
		return;
	}

	m_u32_last_estimate_t[no] = (m_u32_last_estimate_t[no] + BATM_TIME_SLOT * OSC_SECOND) % 0xFFFFFFFF;				  //更新时间记录点

	os_mut_wait(g_mut_share_data, 0xFFFF);					//等待互斥量
	 
	/*充电时间统计，按充电状态分别统计*/
   	switch (m_t_batm_charge_data[no].u16_charge_mode)
	{

		/*浮充模式下的计时器处理*/
		case	FLO:			
				/*浮充状态下，持维浮充时间统计，以时为单位*/
				if ((m_u16_flo_hour_acc[no] += BATM_TIME_SLOT) >= 3600) 
				{
					m_t_batm_charge_data[no].u16_persist_flo_t++;
					g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] =  m_t_batm_charge_data[no].u16_persist_flo_t; 		//刷新电池显示页面充电持续时间
					m_u16_flo_hour_acc[no] = 0;
				}

				/*浮充状态下，交流持维失电时间统计，以分为单位*/
				switch ( g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path )
				{
					case ONE_PATH: 
						if ((g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state & 0x1) == 0x1 )	   //1路交流，则判一路是否停电
						{
							if ((m_u16_flo_lost_ac_minute[no] += BATM_TIME_SLOT) >= 60) 
							{
								m_t_batm_charge_data[no].u16_lost_ac_t ++;
								m_u16_flo_lost_ac_minute[no] = 0;
							}							
						}
						else
						{										
							m_t_batm_charge_data[no].u16_lost_ac_t = 0;								     //当有某路交流正常，则清停电计时器
							m_u16_flo_lost_ac_minute[no] = 0;
						}
						break;

					 case TWO_PATH: 
						if ((g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state & 0x11) == 0x11 )	   //2路交流，则判2路是否同时停电
						{
							if ((m_u16_flo_lost_ac_minute[no] += BATM_TIME_SLOT) >= 60) 
							{
								m_t_batm_charge_data[no].u16_lost_ac_t++;
								m_u16_flo_lost_ac_minute[no] = 0;
							}							
						}
						else
						{
							m_t_batm_charge_data[no].u16_lost_ac_t = 0;									 //当有某路交流正常，则清停电计时器
							m_u16_flo_lost_ac_minute[no] = 0;
						}
						break;

					default:
						m_t_batm_charge_data[no].u16_lost_ac_t = 0;
						m_u16_flo_lost_ac_minute[no] = 0;
						break;	
			   	}

				/*如果电池电流大于设定转均充电流*/
				refer_bat_cur = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];

				if (refer_bat_cur >= (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr))
				{
					m_t_batm_charge_data[no].u16_oc_durable_t += BATM_TIME_SLOT;
				}
				else
				{
					m_t_batm_charge_data[no].u16_oc_durable_t = 0;
				}					   				
				
			   	m_t_batm_charge_data[no].u16_persist_equ_t = 0;			   //均充计时器清0
			   	m_t_batm_charge_data[no].u16_equ_count_down = 0;		   //均充倒计时清0
				m_t_batm_charge_data[no].u16_persist_chd_t = 0;			   //核容计时器清0
				break;

		/*均充模式下的计时器处理*/
		case	EQU:
					/*均充持维时间计时器*/
				if ((m_u16_equ_limit_minute[no] += BATM_TIME_SLOT) >= 60) 
				{
					m_t_batm_charge_data[no].u16_persist_equ_t++;
					g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] =  m_t_batm_charge_data[no].u16_persist_equ_t; 		//刷新电池显示页面充电持续时间
					m_u16_equ_limit_minute[no] = 0;
				

					/*如果电池电流小于设定转浮充电流*/
					refer_bat_cur = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];

					if (refer_bat_cur < (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_flo_curr))
					{
						m_t_batm_charge_data[no].u16_equ_count_down += BATM_TIME_SLOT;
					}
					else
					{
						m_t_batm_charge_data[no].u16_equ_count_down = 0;
					}
				}

				m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //浮充转换均充计时器清0
				m_t_batm_charge_data[no].u16_persist_flo_t = 0;   		  //持维浮充时间计时器清0
				m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //清交流停电计时器
				m_t_batm_charge_data[no].u16_persist_chd_t = 0;			  //核容计时器清0 				
				break;

		/*核容模式下计时器处理*/
		case	DIS:
			   	if ((m_u16_chd_limit_minute[no] += BATM_TIME_SLOT) >= 60) 
				{
					m_t_batm_charge_data[no].u16_persist_chd_t++;
					g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] =  m_t_batm_charge_data[no].u16_persist_chd_t; 		//刷新电池显示页面充电持续时间
					m_u16_chd_limit_minute[no] = 0;
				}
				m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //浮充转换均充计时器清0
				m_t_batm_charge_data[no].u16_persist_flo_t = 0;   		  //持维浮充时间计时器清0
				m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //清交流停电计时器
				m_t_batm_charge_data[no].u16_persist_equ_t = 0;			  //均充计时器清0
			   	m_t_batm_charge_data[no].u16_equ_count_down = 0;		  //均充倒计时清0 				
				break;

		default:
				break;
	}
	

	/* 存手/自动转换事件 */
	g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[1] = g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0];//第二组手自动控制共用第一组值
	if (m_e_batm_copy_mode[no] != g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no])	
	{
		U16_T	e_mode_id;
		m_e_batm_copy_mode[no] = g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no];		//备份新状态
		if (m_e_batm_copy_mode[no] == AUTO_MODE)
		{
			e_mode_id = LOG_ID_BATT_TO_AUTO;		   //充电管理转为自动
		}
		else
		{
			e_mode_id = LOG_ID_BATT_TO_MANUAL;		   //充电管理转为手动	
		}

		v_batm_save_record(e_mode_id + LOG_ID_BATT_NUM * no);			  
	}


	/*自动充电管理*/
	if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] == AUTO_MODE)		 
	{		
		switch (m_t_batm_charge_data[no].u16_charge_mode)
		{
			case	EQU:						//均充管理
				v_batm_equ_manage(no);
				break;

			case	DIS:					   //核容管理
				v_batm_chd_manage(no);
				break;

			case	FLO:					  //浮充管理
			default:
				v_batm_flo_manage(no);
				break;

		}

		/*备份当前充电状态，以便于在转为手动状态时，能立即与手动设置状态相一致*/
		m_u16_batm_copy_batt[no] = m_t_batm_charge_data[no].u16_charge_mode;		
	}  


	/*手动充电管理	 */
	else
	{
		if (m_u16_batm_copy_batt[no] != g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no])		  //如果设定的状态有变，则将电池状态转为设定的新状态
		{ 	
			U16_T	charge_mode_id;
			m_u16_batm_copy_batt[no] = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];
			m_t_batm_charge_data[no].u16_charge_mode = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];
			v_batm_reset_batm_t(no);													  //所有计器器重新开始	
			
			switch (m_t_batm_charge_data[no].u16_charge_mode)	
			{
				case	EQU:
					charge_mode_id = LOG_ID_BATT_MANUAL_TO_EQU;						   //电池手动转均充
					break;

				case	DIS:
					charge_mode_id = LOG_ID_BATT_MANUAL_TO_DIS;						   //电池手动转核容
					break;

				case	FLO:
				default:
					charge_mode_id = LOG_ID_BATT_MANUAL_TO_FLO;						   //电池手动转浮充
					break;	 
			}	
			
			v_batm_save_record(charge_mode_id + LOG_ID_BATT_NUM * no);								  

		}

		switch (m_t_batm_charge_data[no].u16_charge_mode)
		{
			case	EQU:						//均充管理
				v_batm_equ_manage(no);
				break;

			case	DIS:					   //核容管理
				v_batm_chd_manage(no);
				break;

			case	FLO:					  //浮充管理，手动情况下，处于浮充状态不对其进行管理
			default:
				break;

		}

	}

	

	/*根据充电状态，设置充电机输出电压值*/
	v_batm_setup_charg_panl_out_vol(no); 	 

	g_t_share_data.t_rt_data.t_batt.e_state[no] = (BATT_STATE_E)m_t_batm_charge_data[no].u16_charge_mode;			  //刷新界面显示

   	u32_charge_state_time = ((U32_T)m_t_batm_charge_data[no].u16_charge_mode) << 16;	//电池当前充电状态
	u32_charge_state_time += g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];
	v_rtc_save_user_data(u32_charge_state_time, (RTC_GPREG_E)(GP_REG_3 + no));	 		            //电池组当前充电状态

	os_mut_release(g_mut_share_data);			 //释放互斥量



}



/*************************************************************
函数名称: v_batm_adjust_rect_limit		           				
函数功能: 充电机限流点调节，根据当前的电池电流、负载电流值、设定的恒流点来调节限流点 	
		  为了保证调节的稳定性，每5秒调节一次,在电池放电及过充50%限流点时，给一次预限流值
		  ，预限流值在一个调节过程中，只可给一次， 以防止由于测量的异常而进入死调节过程	 					
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
void v_batm_adjust_rect_limit(U8_T no)
{
 	U32_T	this_ajdust_limit_t = 0;	   			//本次限流调节时间点
	F32_T	adjust_limit_value_abs;					//本次应调节的电流误差，以绝对值表示，不是百分比
	F32_T	setup_bat_limit_cur;				//系统设定的充电电流值，以绝对值l表示
	F32_T   refer_bat_cur;						//两组电池中最大的充电电流
	U32_T	u32_elapse_time;
	U8_T    u8_rect_num;
	RATED_CURR_E e_rated_curr;


	//手动限流方式，直接用界面设置的限流点下发，不用进行闭环操作
	os_mut_wait(g_mut_share_data, 0xFFFF);
	if (g_t_share_data.t_sys_cfg.t_sys_param.e_limit_curr == MANUAL_LIMIT_CURR)
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[no] = g_t_share_data.t_sys_cfg.t_sys_param.f32_manual_limit_curr;	   //更新限流点
		
		os_mut_release(g_mut_share_data);			 //释放互斥量
		return;
	}
	os_mut_release(g_mut_share_data);			 //释放互斥量

	//下面是自动限流方式的处理，需要进行闭环等待
	/*根据限流命令执行情况来调节限流 */
	if (os_evt_wait_or((RECT_SET_CURR_SCUESS1 + 2 * no), 0) != OS_R_EVT) 
	{ 	
		return; 		
	}
	m_u32_board_code_count[no]++;

	//固定时间间隔调节限流点
	this_ajdust_limit_t = u32_delay_get_timer_val();
	u32_elapse_time = u32_delay_time_elapse(m_u32_last_ajdust_limit_t[no],this_ajdust_limit_t);
	if ((u32_elapse_time < 6 * OSC_SECOND) && (m_u32_board_code_count[no] < 10))		 //每5S时间间隔调节一次
	{ 			
		return;
	}
	m_u32_board_code_count[no] = 0;		//恢复计数器到0
	m_u32_last_ajdust_limit_t[no] =  this_ajdust_limit_t;   


	os_mut_wait(g_mut_share_data, 0xFFFF);					//等待互斥量

	setup_bat_limit_cur = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;	 //设定限流值

	/*电池如是处于放电状态*/
	refer_bat_cur = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];

	if (refer_bat_cur < 0) 
	{
		/*之前没有给定过初值，则给限流初值为充电限流值+负载电流*/
		if (m_u8_dis_setup_basic_limit[no] && (m_t_batm_charge_data[no].f32_charge_limit_percent < BATM_MDL_MAX_LIMIT_PERCENT))		  
		{
			m_u8_dis_setup_basic_limit[no] = 0;
			if (no == 0)
				adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr;			//当前总的限流值为充电限流值+负载电流
			else
				adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr;		//当前总的限流值为充电限流值+负载电流

			/*将充电限流值转换为百分比*/
			e_rated_curr = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr;
			if( g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
				u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num / 2;
			else
				u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
			m_t_batm_charge_data[no].f32_charge_limit_percent = (float)(adjust_limit_value_abs * 100 / u8_rect_num
														                / m_f32_mdl_rated_cur[e_rated_curr]); 			 
		}
	    /*之前有给定过初值，则给限流初值以5%步长放开*/
		else
		{
			m_t_batm_charge_data[no].f32_charge_limit_percent += BATM_MDL_MIN_LIMIT_PERCENT;
		}
		 
	}
	/*电池如是处于充电状态*/
	else
	{
		F32_T	allow_limit_error;				//充电限流误差设定值

		m_u8_dis_setup_basic_limit[no] = 1;			 //置下次放电可置预限流值操作
		/*如果充电电流值在30A以内，则充电电流值误差允许在0.3A*/
		if (setup_bat_limit_cur <= 30.0)
		{
			allow_limit_error = 0.3;
		}

		/*设定的充电电流值在30A以上，允许误差1%设定恒流值*/
		else
		{
			allow_limit_error =  setup_bat_limit_cur * 0.01;
		}

		/*判断充电电流是否在设定限流值范围内*/
		refer_bat_cur = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];

		if (fabs (refer_bat_cur - setup_bat_limit_cur) <= allow_limit_error )
		{
		}
		/*不在范围内*/
		else 
		{	
			F32_T	limit_error;		//充电电流误差
			limit_error = refer_bat_cur - setup_bat_limit_cur;

			/*如果充电电流大于设定限流值1.5倍，并没有在给定过预限流值，则限流点设定为充电电流+负载电流，以避电池长时间过充*/
			if ((limit_error > setup_bat_limit_cur * 0.5) && m_u8_chr_setup_basic_limit[no] ) 
			{
				m_u8_chr_setup_basic_limit[no] = 0;
				if (no == 0)
					adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr;			//当前总的限流值为充电限流值+负载电流
				else
					adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr;		//当前总的限流值为充电限流值+负载电流

				/*将充电限流值转换为百分比*/
				e_rated_curr = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr;
				if( g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num / 2;
				else
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
				m_t_batm_charge_data[no].f32_charge_limit_percent = (float)(adjust_limit_value_abs * 100 / u8_rect_num
															                / m_f32_mdl_rated_cur[e_rated_curr]); 			 
			}

		    /*否则将当前限流点减去误差百分比*/
			else
			{
				e_rated_curr = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr;
				if( g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num / 2;
				else
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
				m_t_batm_charge_data[no].f32_charge_limit_percent -= (float)(limit_error * 100 / u8_rect_num
														                     / m_f32_mdl_rated_cur[e_rated_curr]); 

				if (limit_error < 0)				   //如果小于设定限流点，允许下次过电流给预限流值操作
				{
					m_u8_chr_setup_basic_limit[no] = 1;
				} 
			}

		} 	
	}

	/*充电限流值限定为105%~5%范围内*/
	if (m_t_batm_charge_data[no].f32_charge_limit_percent > BATM_MDL_MAX_LIMIT_PERCENT)
	{
		m_t_batm_charge_data[no].f32_charge_limit_percent = BATM_MDL_MAX_LIMIT_PERCENT;
	} 
	else if (m_t_batm_charge_data[no].f32_charge_limit_percent < BATM_MDL_MIN_LIMIT_PERCENT)
	{
		m_t_batm_charge_data[no].f32_charge_limit_percent = BATM_MDL_MIN_LIMIT_PERCENT;	
	}

	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[no] = m_t_batm_charge_data[no].f32_charge_limit_percent;	   //更新限流点

	os_mut_release(g_mut_share_data);			 //释放互斥量

}

  	

/*************************************************************
函数名称: v_batm_relay_out		           				
函数功能: 均浮充状态输出继电器控制,直流不充电继电器输出控制						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/

void v_batm_relay_out(U8_T no)
{
	U8_T	u8_setdata;	

	os_mut_wait(g_mut_share_data, 0xFFFF);					//等待互斥量   

	if (m_u16_batm_relay_batt[no] != m_t_batm_charge_data[no].u16_charge_mode)		  //判状态是否发生改变
	{  		 
		switch (m_t_batm_charge_data[no].u16_charge_mode)
		{
			/*当前状态为均充*/
			case   EQU:																 
				u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output;
				v_fault_occur_fault_output(u8_setdata);	  						//输出均充结点


				if (m_u16_batm_relay_batt[no] == DIS)										//如以前为核容状态，则释放核容状态继电器
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output; 
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}
				break;

			/*当前状态为浮充*/
			case   FLO:					 
				if (m_u16_batm_relay_batt[no] == EQU)										//如以前为均充状态，则释放均充状态继电器
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output;
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}

				if (m_u16_batm_relay_batt[no] == DIS)										//如以前为核容状态，则释放核容状态继电器
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output; 
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}
				break;

			case	DIS:
			default:
				if (m_u16_batm_relay_batt[no] == EQU)										//如以前为均充状态，则释放均充状态继电器
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output; 
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}

				u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output;
				v_fault_occur_fault_output(u8_setdata);	  						//输出核容结点
				break;
		}

		m_u16_batm_relay_batt[no] = m_t_batm_charge_data[no].u16_charge_mode;		//刷新当前数据
	}

	os_mut_release(g_mut_share_data);			 //释放互斥量
	
}



/*************************************************************
函数名称: v_batm_batt_manage_init		           				
函数功能: 电池管理数据初始化，初始化充电状态，充电限流点，充电时间计时器						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/

void v_batm_batt_manage_init(U8_T no)	
{
	U8_T	u8_setdata;
	U32_T   u32_charge_state_time;

	os_mut_wait(g_mut_share_data, 0xFFFF);


	/*初始化充电状态*/
	m_t_batm_charge_data[no].f32_bat_capacity = (F32_T)u32_rtc_read_user_data((RTC_GPREG_E)(GP_REG_1 + no)) / BATM_CAPACITY_COEFF;	//从RTC寄存器中读取到当前电池容量
	g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_capacity = m_t_batm_charge_data[no].f32_bat_capacity;		  //刷新显示

	/* 两组电池容量之和计算 */	
//	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num <= 1)
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity;
//	else	
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity + m_t_batm_charge_data[1].f32_bat_capacity;

	u32_charge_state_time  =  u32_rtc_read_user_data((RTC_GPREG_E)(GP_REG_3 + no));
	m_t_batm_charge_data[no].u16_charge_mode = (U16_T)(u32_charge_state_time >> 16);	//电池当前充电状态
	g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = (U16_T)u32_charge_state_time;
	if(m_t_batm_charge_data[no].u16_charge_mode >= 3)  //非法充电状态
	{
		g_t_share_data.t_rt_data.t_batt.e_state[no] = FLO;        //电池组当前充电状态
		g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = 0;	 //电池组当前充电状态持续时间 
	}
	else
	{
		g_t_share_data.t_rt_data.t_batt.e_state[no] = (BATT_STATE_E)(m_t_batm_charge_data[no].u16_charge_mode);        //电池组当前充电状态
		g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];	 //电池组当前充电状态持续时间 
	}

	/*if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] == AUTO_MODE)		 			//如是自动状态，则初始化为浮充
	{
		m_t_batm_charge_data[no].u16_charge_mode = FLO;
	}   
	else
	{
		m_t_batm_charge_data[no].u16_charge_mode = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];	   //如是手动动状态，则初始化为手动设置的状态		
	}*/
	m_t_batm_charge_data[no].u16_charge_mode = g_t_share_data.t_rt_data.t_batt.e_state[no];	    //复制充电状态
	m_u16_batm_copy_batt[no] = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];					//复制手动充电状态
	m_e_batm_copy_mode[no] = g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no];				//备份系统管理模式	

   

	/*初始化充电电压*/
	v_batm_setup_charg_panl_out_vol(no); 	


	/*初始化限流点，限流点初值为当前设置的脱机电流点*/
    m_t_batm_charge_data[no].f32_charge_limit_percent = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_curr_percent;


	/*初始化充电计时器为0*/
	m_u16_flo_hour_acc[no]     = 0;	   	//浮充计时时累加器，
	m_u16_equ_limit_minute[no] = 0;	   	//均充持维时间计时累加器	  
	m_u16_chd_limit_minute[no] = 0;	   	//核容状态时间计时累加器 

	m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //浮充转换均充计时器清0
	m_t_batm_charge_data[no].u16_equ_count_down = 0;		  //均充倒计时清0
	m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //清交流停电计时器


	/*初始化故障输出结点状态*/
	if (m_t_batm_charge_data[no].u16_charge_mode == FLO)
	{ 		
		m_t_batm_charge_data[no].u16_persist_flo_t = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];

		/*u8_setdata =  g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_flo_output; 
		v_fault_occur_fault_output(u8_setdata);	  						//输出浮充结点*/
	}
	else if (m_t_batm_charge_data[no].u16_charge_mode == EQU)
	{
	    m_t_batm_charge_data[no].u16_persist_equ_t = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];

		u8_setdata =  g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output; 
		v_fault_occur_fault_output(u8_setdata);	  						//输出均充结点 
	}
	else if (m_t_batm_charge_data[no].u16_charge_mode == DIS)
	{
		m_t_batm_charge_data[no].u16_persist_chd_t = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];

		u8_setdata =  g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output; 
		v_fault_occur_fault_output(u8_setdata);
	}
	m_u16_batm_relay_batt[no] = m_t_batm_charge_data[no].u16_charge_mode;					//初始化均浮充控制继电器与当前充电状态一致	
	

	os_mut_release(g_mut_share_data);  					   //释放互斥量

}





/*************************************************************
函数名称: v_batm_battery_manage_task		           				
函数功能: 电池管理任务，实现电池均浮充转换&电压调节、充电机限流调节、
		  电容容量计算、均浮充状态结点输出、该任务可通过RTX函数os_tsk_create来启动						
输入参数: 无        		   				
输出参数: 无
返回值  ：无														   				
**************************************************************/
__task void v_batm_battery_manage_task(void)
{
	U8_T	i, u8_batt_group_num;   	

	for(i=0; i<2; i++)
	{
		v_batm_batt_manage_init(i);							//初始化上电状态
	}

	while (1) 
	{
		os_evt_set(BATT_FEED_DOG, g_tid_wdt);           //设置喂狗事件标志

		os_mut_wait(g_mut_share_data, 0xFFFF);								//等待互斥量
		u8_batt_group_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		os_mut_release(g_mut_share_data);			 			//释放互斥量

		for(i=0; i<u8_batt_group_num; i++)
		{
			v_batm_count_battery_capability(i);			//电容容量计算
			v_batm_estimate_charge_state(i);			//充电状态转换
			v_batm_relay_out(i);						//均浮充输出继电器控制
			v_batm_adjust_rect_limit(i);				//限流调节
		}

		os_dly_wait(20);								//空闲200ms		
	}
}
