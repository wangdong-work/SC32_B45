/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：DCcollect.c
版    本：1.00
创建日期：2012-04-23
作    者：邹 伟 明
功能描述：直流数据采集

函数列表：

修改记录：
	作者      日期        版本     修改内容
	邹伟明    2012-04-23  1.00     创建
**************************************************************/


#include <rtl.h>
#include <math.h>
#include "DCcollect.h"
#include "PublicData.h"
#include "FetchFlash.h"
#include "ShareDataStruct.h"
#include "FaultId.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Delay.h"
#include "../Drivers/Usb.h"


#define VOLT_DEF_P 0.10647				 //电压缺省斜率
#define PE_V_DEF_P 0.0773				 //母线负对地电压缺省斜率
#define CURR_DEF_A 0.04739				 //电流缺省系数A
#define CURR_DEF_B -97.05		 		 //电流缺省系数B

#define MIN_DISP_REFRESH 0.002	         //最小显示刷新百分比0.2%
#define CUR_UNBAL_JUDGE_PCT 0.02 		 //分流器量程之和乘以此百分比后进行电流不平衡判断

static U8_T g_bat1_cur_times = 0;				//电流显示控制系数，当g_bat1_cur_times<=3时，电流显示为0,是为了解决上电一瞬间电池电流为一个很大的负值的问题
//static U8_T g_bat2_cur_times = 0;				//电流显示控制系数，当g_bat2_cur_times<=3时，电流显示为0,是为了解决上电一瞬间电池电流为一个很大的负值的问题

static F32_T g_cal_buff_p;				 //电压校准斜率缓存
static F32_T g_cal_buff_a;				 //电流校准系数a缓存
static F32_T g_cal_buff_b;				 //电流校准系数b缓存

static F32_T g_dip_buff;				 //显示数据缓存

static U32_T g_time1 = 0;						   //
static U32_T g_time2 = 0;						   //

//电流校准相关系数
static F32_T g_load_curr_cal_ad1 = 0;
static F32_T g_load_curr_cal_ad2 = 0;
static F32_T g_load_curr_cal_rel1 = 0;
static F32_T g_load_curr_cal_rel2 = 0;

static F32_T g_batt_curr_cal_ad1 = 0;
static F32_T g_batt_curr_cal_ad2 = 0;
static F32_T g_batt_curr_cal_rel1 = 0;
static F32_T g_batt_curr_cal_rel2 = 0;

static F32_T g_rect_curr_cal_ad1 = 0;
static F32_T g_rect_curr_cal_ad2 = 0;
static F32_T g_rect_curr_cal_rel1 = 0;
static F32_T g_rect_curr_cal_rel2 = 0;

static U8_T si_state = 0x00;

static U32_T m_u32_bus_err_time = 0;


static const F32_T TEP_TABLE[]={-20.0,-19.5,-19.0,-18.5,-18.0,-17.5,-17.0,-16.5,-16.0,-15.5,
								-15.1,-14.8,-14.5,-14.0,-13.5,-13.1,-12.8,-12.5,-12.1,-11.7,
								-11.3,-10.9,-10.5,-10.3, -10.1,-9.7, -9.2, -8.8, -8.4, -7.9,
								 -7.5, -7.1, -6.7, -6.3, -6.0, -5.6, -5.2, -4.8, -4.5, -4.2,
								 -3.9, -3.6, -3.3, -3.0, -2.6, -2.3, -2.0, -1.7, -1.3, -1.0,
								 -0.7, -0.3,  0.0,  0.4,  0.8,  1.2,  1.6,  2.0,  2.3,  2.7,
								  3.0,  3.3,  3.6,  3.9,  4.3,  4.6,  5.0,  5.3,  5.6,  5.9,
								  6.2,  6.5,  6.9,  7.3,  7.6,  7.9,  8.2,  8.5,  8.8,  9.1, 
								  9.5,  9.8, 10.1, 10.5, 10.9, 11.2, 11.5, 11.7, 12.0, 12.4,
								 12.7, 13.0, 13.4, 13.7, 14.0, 14.3, 14.6, 14.9, 15.2, 15.5,
								 15.8, 16.1, 16.4, 16.7, 17.0, 17.4, 17.7, 18.0, 18.4, 18.7,
								 19.0, 19.4, 19.7, 20.0, 20.3, 20.6, 20.9, 21.2, 21.5, 21.8,
								 22.2, 22.5, 22.8, 23.1, 23.5, 23.8, 24.1, 24.5, 24.8, 25.2,
								 25.5, 25.9, 26.2, 26.6, 27.0, 27.3, 27.6, 27.9, 28.3, 28.6,
								 29.0, 29.3, 29.6, 29.9, 30.1, 30.5, 30.8, 31.1, 31.4, 31.7, 
								 32.0, 32.4, 32.8, 33.1, 33.4, 33.7, 34.0, 34.3, 34.7, 35.0, 
								 35.4, 35.7, 36.1, 36.5, 36.8, 37.2, 37.6, 37.9, 38.2, 38.5, 
								 38.9, 39.3, 39.6, 40.0, 40.2, 40.6, 41.0, 41.5, 42.0, 42.3, 
								 42.5, 42.8, 43.2, 43.6, 44.0, 44.5, 45.0, 45.4, 45.8, 46.2,
								 46.6, 47.0, 47.4, 47.9, 48.2, 48.6, 49.0, 49.4, 49.8, 50.2, 
								 50.6, 51.0, 51.5, 52.0, 52.3, 52.7, 53.0, 53.3, 53.7, 54.2, 
								 54.7, 55.2, 55.7, 56.2, 56.7, 57.2, 57.7, 58.2, 58.7, 59.2, 
								 59.6, 60.0};



  /****************************************************************************
 *  函数名称：  v_dc_calbration(U16_T u16_t_cal_item) 
 *  输入参数:   u16_t_cal_item为要校准的项
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   直流校准函数，校准当前项目的斜率 
 ***************************************************************************/
void v_dc_calbration(U16_T u16_t_cal_item)
{
	U32_T ad;
	U16_T shunt_rated_volt;
	DC_CFG_T *p_dc_cfg = &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc);
	os_mut_wait(g_mut_share_data, 0xFFFF);
	if(p_dc_cfg->e_shunt_rated_volt)
	{
		shunt_rated_volt = 50;	
	}
	else
	{
		shunt_rated_volt = 75;
	}
	os_mut_release(g_mut_share_data);				   
	switch(u16_t_cal_item)
	{
		case DC_ADJUST_PB_VOLT:						  //合母电压斜率校准
			ad = ad_convert(PB_VOLT);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_cal_buff_p = (g_t_share_data.t_dc_adjust_data.f32_pb_volt/ad);
			os_mut_release(g_mut_share_data);
			if((g_cal_buff_p<VOLT_DEF_P*0.8)||(g_cal_buff_p>VOLT_DEF_P*1.2))
			{
			 	os_evt_set(DC_ADJUST_FAIL, g_tid_display);
			}
			else
			{
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_v1_vol_slope = g_cal_buff_p;
				os_mut_release(g_mut_share_data);
				os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
			}
			break;

		case DC_ADJUST_CB_VOLT:						  //控母电压斜率校准
			ad = ad_convert(CB_VOLT);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_cal_buff_p = (g_t_share_data.t_dc_adjust_data.f32_cb_volt/ad);
			os_mut_release(g_mut_share_data);
			if((g_cal_buff_p<VOLT_DEF_P*0.8)||(g_cal_buff_p>VOLT_DEF_P*1.2))
			{
			 	os_evt_set(DC_ADJUST_FAIL, g_tid_display);
			}
			else
			{
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_v2_vol_slope = g_cal_buff_p;
				os_mut_release(g_mut_share_data);
				os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
			}
			break;

		case DC_ADJUST_BUS_NEG_TO_END_VOLT:						  //母线负对地电压斜率校准
			ad = ad_convert(S_PE);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_cal_buff_p = (g_t_share_data.t_dc_adjust_data.f32_bus_neg_to_gnd_volt/ad);
			os_mut_release(g_mut_share_data);
			if((g_cal_buff_p<PE_V_DEF_P*0.8)||(g_cal_buff_p>PE_V_DEF_P*1.2))
			{
			 	os_evt_set(DC_ADJUST_FAIL, g_tid_display);
			}
			else
			{
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_neg_vol_slope = g_cal_buff_p;
				os_mut_release(g_mut_share_data);
				os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
			}
			break;

		case DC_ADJUST_BATT_VOLT:						  //电池电压斜率校准
			ad = ad_convert(BATT_VOLT);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_cal_buff_p = (g_t_share_data.t_dc_adjust_data.f32_batt_volt/ad);
			os_mut_release(g_mut_share_data);
			if((g_cal_buff_p<VOLT_DEF_P*0.8)||(g_cal_buff_p>VOLT_DEF_P*1.2))
			{
			 	os_evt_set(DC_ADJUST_FAIL, g_tid_display);
			}
			else
			{
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_v3_vol_slope = g_cal_buff_p;
				os_mut_release(g_mut_share_data);
				os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
			}
			break;

		case DC_ADJUST_LOAD_CURR_1:						  //负载电流校准
			g_load_curr_cal_ad1 = ad_convert(LOAD_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_load_curr_cal_rel1 = g_t_share_data.t_dc_adjust_data.f32_load_curr_1*((F32_T)shunt_rated_volt/p_dc_cfg->u16_load_shunt_range);
			os_mut_release(g_mut_share_data);
			if(!g_load_curr_cal_ad2)
			{
				os_evt_set(DC_ADJUST_FIRST_CURR_COMPLETE, g_tid_display);
			}
			if(g_load_curr_cal_ad1 && g_load_curr_cal_ad2)
			{
				g_cal_buff_a = (g_load_curr_cal_rel1 - g_load_curr_cal_rel2)/(g_load_curr_cal_ad1 - g_load_curr_cal_ad2);
				g_cal_buff_b = g_load_curr_cal_rel1 - g_cal_buff_a*g_load_curr_cal_ad1;
				g_load_curr_cal_rel1 = 	0;
				g_load_curr_cal_rel2 = 	0;
				g_load_curr_cal_ad1 = 0;
				g_load_curr_cal_ad2 = 0;
				if(((g_cal_buff_a<CURR_DEF_A*0.5)||(g_cal_buff_a>CURR_DEF_A*1.5)) &&
				   ((g_cal_buff_b<CURR_DEF_B*0.5)||(g_cal_buff_b>CURR_DEF_B*1.5)))
				{
					os_evt_set(DC_ADJUST_FAIL, g_tid_display);
				}
				else
				{
					os_mut_wait(g_mut_share_data, 0xFFFF);
					g_t_share_data.t_coeff_data.f32_a1_curr_slope = g_cal_buff_a;
					g_t_share_data.t_coeff_data.f32_a1_curr_zero = g_cal_buff_b;
					os_mut_release(g_mut_share_data);
					os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
				}
			}
			break;

		case DC_ADJUST_LOAD_CURR_2:						  //负载电流校准
			g_load_curr_cal_ad2 = ad_convert(LOAD_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_load_curr_cal_rel2 = g_t_share_data.t_dc_adjust_data.f32_load_curr_2*((F32_T)shunt_rated_volt/p_dc_cfg->u16_load_shunt_range);
			os_mut_release(g_mut_share_data);
			if(!g_load_curr_cal_ad1)
			{
				os_evt_set(DC_ADJUST_FIRST_CURR_COMPLETE, g_tid_display);
			}
			if(g_load_curr_cal_ad1 && g_load_curr_cal_ad2)
			{
				g_cal_buff_a = (g_load_curr_cal_rel1 - g_load_curr_cal_rel2)/(g_load_curr_cal_ad1 - g_load_curr_cal_ad2);
				g_cal_buff_b = g_load_curr_cal_rel1 - g_cal_buff_a*g_load_curr_cal_ad1;
				g_load_curr_cal_rel1 = 0;
				g_load_curr_cal_rel2 = 0;
				g_load_curr_cal_ad1 = 0;
				g_load_curr_cal_ad2 = 0;
				if(((g_cal_buff_a<CURR_DEF_A*0.5)||(g_cal_buff_a>CURR_DEF_A*1.5)) &&
				   ((g_cal_buff_b<CURR_DEF_B*0.5)||(g_cal_buff_b>CURR_DEF_B*1.5)))
				{
					os_evt_set(DC_ADJUST_FAIL, g_tid_display);
				}
				else
				{
					os_mut_wait(g_mut_share_data, 0xFFFF);
					g_t_share_data.t_coeff_data.f32_a1_curr_slope = g_cal_buff_a;
					g_t_share_data.t_coeff_data.f32_a1_curr_zero = g_cal_buff_b;
					os_mut_release(g_mut_share_data);
					os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
				}
			}
			break;

		case DC_ADJUST_BATT1_CURR_1:						  //1组电池电流校准
			g_batt_curr_cal_ad1 = ad_convert(BATT1_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_batt_curr_cal_rel1 = g_t_share_data.t_dc_adjust_data.f32_batt1_curr_1*((F32_T)shunt_rated_volt/p_dc_cfg->u16_batt1_shunt_range);
			os_mut_release(g_mut_share_data);
			if(!g_batt_curr_cal_ad2)
			{
				os_evt_set(DC_ADJUST_FIRST_CURR_COMPLETE, g_tid_display);
			}
			if(g_batt_curr_cal_ad1&&g_batt_curr_cal_ad2)
			{
				g_cal_buff_a = (g_batt_curr_cal_rel1 - g_batt_curr_cal_rel2)/(g_batt_curr_cal_ad1 - g_batt_curr_cal_ad2);
				g_cal_buff_b = g_batt_curr_cal_rel1 - g_cal_buff_a*g_batt_curr_cal_ad1;
				g_batt_curr_cal_rel1 = 	0;
				g_batt_curr_cal_rel2 = 	0;
				g_batt_curr_cal_ad1 = 0;
				g_batt_curr_cal_ad2 = 0;
				if(((g_cal_buff_a<CURR_DEF_A*0.5)||(g_cal_buff_a>CURR_DEF_A*1.5))&&((g_cal_buff_b<CURR_DEF_B*0.5)||(g_cal_buff_b>CURR_DEF_B*1.5)))
				{
					os_evt_set(DC_ADJUST_FAIL, g_tid_display);
				}
				else
				{
					os_mut_wait(g_mut_share_data, 0xFFFF);
					g_t_share_data.t_coeff_data.f32_a2_curr_slope = g_cal_buff_a;
					g_t_share_data.t_coeff_data.f32_a2_curr_zero = g_cal_buff_b;
					os_mut_release(g_mut_share_data);
					os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
				}
			}
			break;

		case DC_ADJUST_BATT1_CURR_2:						  //1组电池电流校准
			g_batt_curr_cal_ad2 = ad_convert(BATT1_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_batt_curr_cal_rel2 = g_t_share_data.t_dc_adjust_data.f32_batt1_curr_2*((F32_T)shunt_rated_volt/p_dc_cfg->u16_batt1_shunt_range);
			os_mut_release(g_mut_share_data);
			if(!g_batt_curr_cal_ad1)
			{
				os_evt_set(DC_ADJUST_FIRST_CURR_COMPLETE, g_tid_display);
			}
			if(g_batt_curr_cal_ad1&&g_batt_curr_cal_ad2)
			{
				g_cal_buff_a = (g_batt_curr_cal_rel1 - g_batt_curr_cal_rel2)/(g_batt_curr_cal_ad1 - g_batt_curr_cal_ad2);
				g_cal_buff_b = g_batt_curr_cal_rel1 - g_cal_buff_a*g_batt_curr_cal_ad1;
				g_batt_curr_cal_rel1 = 	0;
				g_batt_curr_cal_rel2 = 	0;
				g_batt_curr_cal_ad1 = 0;
				g_batt_curr_cal_ad2 = 0;
				if(((g_cal_buff_a<CURR_DEF_A*0.5)||(g_cal_buff_a>CURR_DEF_A*1.5))&&((g_cal_buff_b<CURR_DEF_B*0.5)||(g_cal_buff_b>CURR_DEF_B*1.5)))
				{
					os_evt_set(DC_ADJUST_FAIL, g_tid_display);
				}
				else
				{
					os_mut_wait(g_mut_share_data, 0xFFFF);
					g_t_share_data.t_coeff_data.f32_a2_curr_slope = g_cal_buff_a;
					g_t_share_data.t_coeff_data.f32_a2_curr_zero = g_cal_buff_b;
					os_mut_release(g_mut_share_data);
					os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
				}
			}
			break;

		case DC_ADJUST_BATT2_CURR_1:						  //2组电池电流校准
			g_rect_curr_cal_ad1 = ad_convert(BATT2_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_rect_curr_cal_rel1 = g_t_share_data.t_dc_adjust_data.f32_batt2_curr_1*((F32_T)shunt_rated_volt/p_dc_cfg->u16_batt2_shunt_range);
			os_mut_release(g_mut_share_data);
			if(!g_rect_curr_cal_ad2)
			{
				os_evt_set(DC_ADJUST_FIRST_CURR_COMPLETE, g_tid_display);
			}
			if(g_rect_curr_cal_ad1&&g_rect_curr_cal_ad2)
			{
				g_cal_buff_a = (g_rect_curr_cal_rel1 - g_rect_curr_cal_rel2)/(g_rect_curr_cal_ad1 - g_rect_curr_cal_ad2);
				g_cal_buff_b = g_rect_curr_cal_rel1 - g_cal_buff_a*g_rect_curr_cal_ad1;
				g_rect_curr_cal_rel1 = 	0;
				g_rect_curr_cal_rel2 = 	0;
				g_rect_curr_cal_ad1 = 0;
				g_rect_curr_cal_ad2 = 0;
				if(((g_cal_buff_a<CURR_DEF_A*0.5)||(g_cal_buff_a>CURR_DEF_A*1.5))&&((g_cal_buff_b<CURR_DEF_B*0.5)||(g_cal_buff_b>CURR_DEF_B*1.5)))
				{
					os_evt_set(DC_ADJUST_FAIL, g_tid_display);
				}
				else
				{
					os_mut_wait(g_mut_share_data, 0xFFFF);
					g_t_share_data.t_coeff_data.f32_a3_curr_slope = g_cal_buff_a;
					g_t_share_data.t_coeff_data.f32_a3_curr_zero = g_cal_buff_b;
					os_mut_release(g_mut_share_data);
					os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
				}
			}
			break;

		case DC_ADJUST_BATT2_CURR_2:						      //2组电池电流校准
			g_rect_curr_cal_ad2 = ad_convert(BATT2_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_rect_curr_cal_rel2 = g_t_share_data.t_dc_adjust_data.f32_batt2_curr_2*((F32_T)shunt_rated_volt/p_dc_cfg->u16_batt2_shunt_range);
			os_mut_release(g_mut_share_data);
			if(!g_rect_curr_cal_ad1)
			{
				os_evt_set(DC_ADJUST_FIRST_CURR_COMPLETE, g_tid_display);
			}
			if(g_rect_curr_cal_ad1&&g_rect_curr_cal_ad2)
			{
				g_cal_buff_a = (g_rect_curr_cal_rel1 - g_rect_curr_cal_rel2)/(g_rect_curr_cal_ad1 - g_rect_curr_cal_ad2);
				g_cal_buff_b = g_rect_curr_cal_rel1 - g_cal_buff_a*g_rect_curr_cal_ad1;
				g_rect_curr_cal_rel1 = 	0;
				g_rect_curr_cal_rel2 = 	0;
				g_rect_curr_cal_ad1 = 0;
				g_rect_curr_cal_ad2 = 0;
				if(((g_cal_buff_a<CURR_DEF_A*0.5)||(g_cal_buff_a>CURR_DEF_A*1.5))&&((g_cal_buff_b<CURR_DEF_B*0.5)||(g_cal_buff_b>CURR_DEF_B*1.5)))
				{
					os_evt_set(DC_ADJUST_FAIL, g_tid_display);
				}
				else
				{
					os_mut_wait(g_mut_share_data, 0xFFFF);
					g_t_share_data.t_coeff_data.f32_a3_curr_slope = g_cal_buff_a;
					g_t_share_data.t_coeff_data.f32_a3_curr_zero = g_cal_buff_b;
					os_mut_release(g_mut_share_data);
					os_evt_set(DC_ADJUST_SCUESS, g_tid_display);
				}
			}
			break;
	}
		 
}

 /****************************************************************************
 *  函数名称：  v_dc_collect(U8_T u8_t_col_item) 
 *  输入参数:   u8_t_col_item为要采集的项目
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   采集当前项目直流数据转换为实际要显示的值送共享数据区 
 ***************************************************************************/
void v_dc_collect(U8_T u8_t_col_item)
{
	U16_T shunt_rated_volt;
	U32_T ad,i,temp;
	F32_T value,f32_shunt_vlot;
	DC_RT_DATA_T *p_dc_rt_data = &(g_t_share_data.t_rt_data.t_dc_panel.t_dc);
	DC_CFG_T *p_dc_cfg = &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc);
	U8_T  u8_d21_num;

	os_mut_wait(g_mut_share_data, 0xFFFF);
	u8_d21_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num;
	if(p_dc_cfg->e_shunt_rated_volt)
	{
		shunt_rated_volt = 50;	
	}
	else
	{
		shunt_rated_volt = 75;
	}
	os_mut_release(g_mut_share_data);
	switch(u8_t_col_item)
	{
		case PB_VOLT:						  //合母电压采集
			if(u8_d21_num == 0)
			{
				ad = ad_convert(PB_VOLT);
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_ad_data.u16_v1_ad = ad;
				g_dip_buff = (ad * g_t_share_data.t_coeff_data.f32_v1_vol_slope);
				value = g_dip_buff - (p_dc_rt_data->f32_pb_volt);
				if((value>=0) && (value>(p_dc_rt_data->f32_pb_volt)*MIN_DISP_REFRESH))
				{	
					p_dc_rt_data->f32_pb_volt = g_dip_buff;	
				}
				if((value<0) && (value<(p_dc_rt_data->f32_pb_volt)*(-MIN_DISP_REFRESH)))
				{	
					p_dc_rt_data->f32_pb_volt = g_dip_buff;	
				}
				os_mut_release(g_mut_share_data);
			}
			break;

		case CB_VOLT:						  //控母电压采集
			if(u8_d21_num == 0)
			{
				ad = ad_convert(CB_VOLT);
				os_mut_wait(g_mut_share_data, 0xFFFF);
				//显示数据更新处理
				g_t_share_data.t_ad_data.u16_v2_ad = ad;
				g_dip_buff = (ad * g_t_share_data.t_coeff_data.f32_v2_vol_slope);
				value = g_dip_buff - (p_dc_rt_data->f32_cb_volt);
				if((value>=0) && (value>(p_dc_rt_data->f32_cb_volt)*MIN_DISP_REFRESH))
				{	
					p_dc_rt_data->f32_cb_volt = g_dip_buff;	
				}
				if((value<0) && (value<(p_dc_rt_data->f32_cb_volt)*(-MIN_DISP_REFRESH)))
				{	
					p_dc_rt_data->f32_cb_volt = g_dip_buff;	
				}	
	
				if(p_dc_cfg->e_diode_chain_ctl)						//如果有配置硅链调压，则启动硅链调压
				{
					g_time2 = u32_delay_get_timer_val();
					if(u32_delay_time_elapse(g_time1,g_time2)>=2000000)//如果距离上次硅链调压超过2s，则启动硅链调压
					{
						v_dc_si_adj_v(g_dip_buff);					   
						g_time1 = u32_delay_get_timer_val();		   //记录下此次调压时间
					}	
				}
				os_mut_release(g_mut_share_data);
			}
			break;

		case BATT_VOLT:						  //电池电压采集
			ad = ad_convert(BATT_VOLT);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_ad_data.u16_v3_ad = ad;
			g_dip_buff = (ad * g_t_share_data.t_coeff_data.f32_v3_vol_slope);
			value = g_dip_buff - (p_dc_rt_data->f32_batt_volt);
			if((value>=0) && (value>(p_dc_rt_data->f32_batt_volt)*MIN_DISP_REFRESH))
			{	
				p_dc_rt_data->f32_batt_volt = g_dip_buff;	
			}
			if((value<0) && (value<(p_dc_rt_data->f32_batt_volt)*(-MIN_DISP_REFRESH)))
			{	
				p_dc_rt_data->f32_batt_volt = g_dip_buff;	
			}
			os_mut_release(g_mut_share_data);
			break;

		case LOAD_CURR:						  //负载电流采集
			if(u8_d21_num == 0)
			{
				ad = ad_convert(LOAD_CURR);
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_ad_data.u16_a1_ad = ad;
				f32_shunt_vlot = ad * g_t_share_data.t_coeff_data.f32_a1_curr_slope + g_t_share_data.t_coeff_data.f32_a1_curr_zero;			 
				g_dip_buff = f32_shunt_vlot * ((F32_T)p_dc_cfg->u16_load_shunt_range / shunt_rated_volt);
				if(g_t_share_data.t_sys_cfg.t_sys_param.e_load_sign == POSITIVE)
				{
					g_dip_buff = g_dip_buff + g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero; 
				}
				else
				{
					g_dip_buff = g_dip_buff - g_t_share_data.t_sys_cfg.t_sys_param.f32_load_zero; 
				}
				value = g_dip_buff - (p_dc_rt_data->f32_load_curr);
				if((value>=0) && (value>(p_dc_rt_data->f32_load_curr)*MIN_DISP_REFRESH))
				{	
					p_dc_rt_data->f32_load_curr = g_dip_buff;	
				}
				if((value<0) && (value<(p_dc_rt_data->f32_load_curr)*(-MIN_DISP_REFRESH)))
				{	
					p_dc_rt_data->f32_load_curr = g_dip_buff;	
				}
				//如果计算出来的负载电流显示值小于0，则让它等于0
				if(p_dc_rt_data->f32_load_curr<0)
				{
					 p_dc_rt_data->f32_load_curr=0;
				}
				os_mut_release(g_mut_share_data);
			}
			break;

		case BATT1_CURR:						  //1组电池电流采集
			ad = ad_convert(BATT1_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_ad_data.u16_a2_ad = ad;
			f32_shunt_vlot = ad * g_t_share_data.t_coeff_data.f32_a2_curr_slope + g_t_share_data.t_coeff_data.f32_a2_curr_zero;
			g_dip_buff = f32_shunt_vlot * ((F32_T)p_dc_cfg->u16_batt1_shunt_range / shunt_rated_volt);
			if(g_t_share_data.t_sys_cfg.t_sys_param.e_bat1_sign == POSITIVE)
			{
				g_dip_buff = g_dip_buff + g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero; 
			}
			else
			{
				g_dip_buff = g_dip_buff - g_t_share_data.t_sys_cfg.t_sys_param.f32_bat1_zero; 
			}
			value = g_dip_buff - (p_dc_rt_data->f32_batt_curr[0]);
#if (!BATT_CURR_FROM_SENSOR)
			if((value>0)&& (p_dc_rt_data->f32_batt_curr[0]>=0) && (value>(p_dc_rt_data->f32_batt_curr[0])*MIN_DISP_REFRESH))
			{	
				p_dc_rt_data->f32_batt_curr[0] = g_dip_buff;	
			}
			if((value<0) && (p_dc_rt_data->f32_batt_curr[0]<0) && (value<(p_dc_rt_data->f32_batt_curr[0])*(MIN_DISP_REFRESH)))
			{	
				p_dc_rt_data->f32_batt_curr[0] = g_dip_buff;	
			}
			if((value>0)&& (p_dc_rt_data->f32_batt_curr[0]<0) && (value>(p_dc_rt_data->f32_batt_curr[0])*(-MIN_DISP_REFRESH)))
			{	
				p_dc_rt_data->f32_batt_curr[0] = g_dip_buff;	
			}
			if((value<0) && (p_dc_rt_data->f32_batt_curr[0]>=0) && (value<(p_dc_rt_data->f32_batt_curr[0])*(-MIN_DISP_REFRESH)))
			{	
				p_dc_rt_data->f32_batt_curr[0] = g_dip_buff;	
			}
#endif
			if(g_bat1_cur_times<=3)
			{
				g_bat1_cur_times++;
				p_dc_rt_data->f32_batt_curr[0] = 0;	
			}
			os_mut_release(g_mut_share_data);
			break;


		case BATT2_CURR:						  //2组电池电流采集
			ad = ad_convert(BATT2_CURR);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			g_t_share_data.t_ad_data.u16_a3_ad = ad;
			f32_shunt_vlot = ad * g_t_share_data.t_coeff_data.f32_a3_curr_slope + g_t_share_data.t_coeff_data.f32_a3_curr_zero;
			g_dip_buff = f32_shunt_vlot * ((F32_T)p_dc_cfg->u16_batt2_shunt_range / shunt_rated_volt);
			if(g_t_share_data.t_sys_cfg.t_sys_param.e_bat2_sign == POSITIVE)
			{
				g_dip_buff = g_dip_buff + g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero; 
			}
			else
			{
				g_dip_buff = g_dip_buff - g_t_share_data.t_sys_cfg.t_sys_param.f32_bat2_zero; 
			}
			value = g_dip_buff - (p_dc_rt_data->f32_batt_curr[1]);
//现二组电池电流改为由DC10采样，所以屏蔽以下代码
//#if (!BATT_CURR_FROM_SENSOR)
//			if((value>0)&& (p_dc_rt_data->f32_batt_curr[1]>=0) && (value>(p_dc_rt_data->f32_batt_curr[1])*MIN_DISP_REFRESH))
//			{	
//				p_dc_rt_data->f32_batt_curr[1] = g_dip_buff;	
//			}
//			if((value<0) && (p_dc_rt_data->f32_batt_curr[1]<0) && (value<(p_dc_rt_data->f32_batt_curr[1])*(MIN_DISP_REFRESH)))
//			{	
//				p_dc_rt_data->f32_batt_curr[1] = g_dip_buff;	
//			}
//			if((value>0)&& (p_dc_rt_data->f32_batt_curr[1]<0) && (value>(p_dc_rt_data->f32_batt_curr[1])*(-MIN_DISP_REFRESH)))
//			{	
//				p_dc_rt_data->f32_batt_curr[1] = g_dip_buff;	
//			}
//			if((value<0) && (p_dc_rt_data->f32_batt_curr[1]>=0) && (value<(p_dc_rt_data->f32_batt_curr[1])*(-MIN_DISP_REFRESH)))
//			{	
//				p_dc_rt_data->f32_batt_curr[1] = g_dip_buff;	
//			}
//#endif
//			if(g_bat2_cur_times<=3)
//			{
//				g_bat2_cur_times++;
//				p_dc_rt_data->f32_batt_curr[1] = 0;	
//			}
			os_mut_release(g_mut_share_data);				
			break;

		case TEMPERATURE:						  //温度采集
			ad = ad_convert(TEMPERATURE);
			g_t_share_data.t_ad_data.u16_temp_ad = ad;
			temp = (ad/10);
			if(temp>334)
			{
				temp = 334;
			}
			else if(temp<113)
			{
				temp = 113;
			} 
			i = (334-temp);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			p_dc_rt_data->f32_temperature = TEP_TABLE[i];
			os_mut_release(g_mut_share_data);
			break;

		case S_PE:								 //PE电压及合、控母正对地电压采集
			if(u8_d21_num == 0)
			{
				ad = ad_convert(S_PE);
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_ad_data.u16_neg_v_ad = ad;
				g_dip_buff = (ad * g_t_share_data.t_coeff_data.f32_neg_vol_slope);
				value = g_dip_buff - (p_dc_rt_data->f32_dc_bus_neg_to_gnd_volt);
				if((value>=0) && (value>(p_dc_rt_data->f32_dc_bus_neg_to_gnd_volt)*MIN_DISP_REFRESH))
				{	
					p_dc_rt_data->f32_dc_bus_neg_to_gnd_volt = g_dip_buff;	
				}
				if((value<0) && (value<(p_dc_rt_data->f32_dc_bus_neg_to_gnd_volt)*(-MIN_DISP_REFRESH)))
				{	
					p_dc_rt_data->f32_dc_bus_neg_to_gnd_volt = g_dip_buff;	
				}
				p_dc_rt_data->f32_dc_pb_pos_to_gnd_volt = p_dc_rt_data->f32_pb_volt - p_dc_rt_data->f32_dc_bus_neg_to_gnd_volt;
				p_dc_rt_data->f32_dc_cb_pos_to_gnd_volt = p_dc_rt_data->f32_cb_volt - p_dc_rt_data->f32_dc_bus_neg_to_gnd_volt;
				if(p_dc_rt_data->f32_dc_pb_pos_to_gnd_volt < 0)
				{
					p_dc_rt_data->f32_dc_pb_pos_to_gnd_volt = 0;
				}
				if(p_dc_rt_data->f32_dc_cb_pos_to_gnd_volt < 0)
				{
					p_dc_rt_data->f32_dc_cb_pos_to_gnd_volt = 0;
				}
				os_mut_release(g_mut_share_data);
			}
			break;				
			
	}
	os_dly_wait(1);	 	
}
/****************************************************************************
 *  函数名称：  v_dc_data_judge(void) 
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   判断采集上来的直流数据是否异常，异常则至相应标志位 
 ***************************************************************************/
 void v_dc_data_judge(void)
 {
 	U16_T id;
	U8_T i,cnt;
	U8_T count=0;
 	DC_RT_DATA_T *dc_p_rt_data=&(g_t_share_data.t_rt_data.t_dc_panel.t_dc);
	DC_CFG_T *dc_p_cfg=&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc);
	BATT_MGMT_CFG_T *batt_p_mgmt_cfg=&(g_t_share_data.t_sys_cfg.t_batt_mgmt);
	os_mut_wait(g_mut_share_data, 0xFFFF);
	if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb)
	{
			//合母过压判断
		if((dc_p_rt_data->f32_pb_volt)>=(dc_p_cfg->u16_pb_high_volt))
		{
			if(!((dc_p_rt_data->u16_state)&0x0001))
			{
				dc_p_rt_data->u16_state |= 0x0001;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state)&0x0001)
			{
				dc_p_rt_data->u16_state &= (~0x0001);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//合母欠压判断
		if((dc_p_rt_data->f32_pb_volt)<=(dc_p_cfg->u16_pb_low_volt))
		{
			if(!((dc_p_rt_data->u16_state)&0x0002))
			{
				dc_p_rt_data->u16_state |= 0x0002;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state)&0x0002)
			{
				dc_p_rt_data->u16_state &= (~0x0002);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_PB_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}

		//控母过压判断
		if((dc_p_rt_data->f32_cb_volt)>=(dc_p_cfg->u16_cb_high_volt))
		{
			if(!((dc_p_rt_data->u16_state)&0x0004))
			{
				dc_p_rt_data->u16_state |= 0x0004;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state)&0x0004)
			{
				dc_p_rt_data->u16_state &= (~0x0004);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//控母欠压判断
		if((dc_p_rt_data->f32_cb_volt)<=(dc_p_cfg->u16_cb_low_volt))
		{
			if(!((dc_p_rt_data->u16_state)&0x0008))
			{
				dc_p_rt_data->u16_state |= 0x0008;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state)&0x0008)
			{
				dc_p_rt_data->u16_state &= (~0x0008);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_CB_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}

	}
	else
	{
		//母线过压判断
		if((dc_p_rt_data->f32_cb_volt)>=(dc_p_cfg->u16_bus_high_volt))
		{
			if(!((dc_p_rt_data->u16_state)&0x0010))
			{
				dc_p_rt_data->u16_state |= 0x0010;
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS_OVER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state)&0x0010)
			{
				dc_p_rt_data->u16_state &= (~0x0010);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS_OVER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
		//母线欠压判断
		if((dc_p_rt_data->f32_cb_volt)<=(dc_p_cfg->u16_bus_low_volt))
		{
			if(!((dc_p_rt_data->u16_state)&0x0020))
			{
				dc_p_rt_data->u16_state |= 0x0020;
				id=(FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS_UNDER_VOLT;
				v_fauid_send_fault_id_occur(id);
			}
		}
		else
		{
			if((dc_p_rt_data->u16_state)&0x0020)
			{
				dc_p_rt_data->u16_state &= (~0x0020);
				id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_DC_BUS_UNDER_VOLT;
				v_fauid_send_fault_id_resume(id);
			}
		}
	}
	
	//电池过压判断
	if((dc_p_rt_data->f32_batt_volt)>=(batt_p_mgmt_cfg->f32_high_volt_limit))
	{
		if(!((dc_p_rt_data->u16_state)&0x0040))
		{
			dc_p_rt_data->u16_state |= 0x0040;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT_OVER_VOLT;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else 
	{
		if((dc_p_rt_data->u16_state)&0x0040)
		{
			dc_p_rt_data->u16_state &= (~0x0040);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT_OVER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	//电池欠压判断
	if((dc_p_rt_data->f32_batt_volt)<=(batt_p_mgmt_cfg->f32_low_volt_limit))
	{
		if(!((dc_p_rt_data->u16_state)&0x0080))
		{
			dc_p_rt_data->u16_state |= 0x0080;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT_UNDER_VOLT;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else
	{
		if((dc_p_rt_data->u16_state)&0x0080)
		{
			dc_p_rt_data->u16_state &= (~0x0080);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT_UNDER_VOLT;
			v_fauid_send_fault_id_resume(id);
		}
	}
	//1组电池过流判断
	if((dc_p_rt_data->f32_batt_curr[0])>=(batt_p_mgmt_cfg->f32_high_curr_limit*g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10))
	{
		if(!((dc_p_rt_data->u16_state)&0x0200))
		{
			dc_p_rt_data->u16_state |= 0x0200;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT1_OVER_CURR;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else
	{
		if((dc_p_rt_data->u16_state)&0x0200)
		{
			dc_p_rt_data->u16_state &= (~0x0200);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT1_OVER_CURR;
			v_fauid_send_fault_id_resume(id);
		}
	}

	//2组电池过流判断
	if((dc_p_rt_data->f32_batt_curr[1])>=(batt_p_mgmt_cfg->f32_high_curr_limit*g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10))
	{
		if(!((dc_p_rt_data->u16_state)&0x0400))
		{
			dc_p_rt_data->u16_state |= 0x0400;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_OVER_CURR;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else
	{
		if((dc_p_rt_data->u16_state)&0x0400)
		{
			dc_p_rt_data->u16_state &= (~0x0400);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_BATT2_OVER_CURR;
			v_fauid_send_fault_id_resume(id);
		}
	}

	//母线绝缘继电器告警
	if(g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num==0)
	{
		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num == 0)
		{
			v_bus_ins_alm();
		}
	}
	else
	{
		cnt=g_t_share_data.t_sys_cfg.t_sys_param.u8_dc_feeder_panel_num;
		for(i=0;i<cnt;i++)
		{
			if(g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num)
				count+=g_t_share_data.t_sys_cfg.t_feeder_panel[i].u8_feeder_module_num;
		}
		if(count==0)
		{
			if(g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_d21_num == 0)
			{
				v_bus_ins_alm();
			}
		} 
	}
	
	os_mut_release(g_mut_share_data);		
 }

/****************************************************************************
 *  函数名称：  v_bus_ins_alm 
 *  输入参数:   无
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   母线绝缘继电器告警（调用前先获取共享区互斥锁） 
 ***************************************************************************/
#define INSU_MAX_RES               999.9    //正常显示电阻值
#define INSU_SAMPLE_RES_POS        336.0	//母线正对地采样电阻，实测值
#define INSU_SAMPLE_RES_NEG        336.0	//母线负对地采样电阻，实测值，400//2040

void v_bus_ins_alm(void)
{
	U16_T id, u16_ref_volt, u16_ref_res;
	F32_T f32_scale, f32_bus_volt_diff, f32_res;
	U32_T u32_now_time;
	DC_RT_DATA_T *dc_p_rt_data=&(g_t_share_data.t_rt_data.t_dc_panel.t_dc);

	u16_ref_volt = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_volt_imbalance;
	u16_ref_res  = g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_insu_res_thr;
		
	f32_bus_volt_diff = fabs((dc_p_rt_data->f32_dc_bus_neg_to_gnd_volt) - (dc_p_rt_data->f32_dc_cb_pos_to_gnd_volt));
	if(f32_bus_volt_diff >= (F32_T)u16_ref_volt)
	{
		u32_now_time = u32_delay_get_timer_val();
		if(u32_delay_time_elapse(m_u32_bus_err_time, u32_now_time) >= (U32_T)(3 * OSC_SECOND))
		{
			if(dc_p_rt_data->f32_dc_bus_neg_to_gnd_volt < (dc_p_rt_data->f32_dc_cb_pos_to_gnd_volt))
			{	//母线负接地
				if(dc_p_rt_data->f32_dc_cb_pos_to_gnd_volt == 0)
				{
					dc_p_rt_data->f32_dc_cb_pos_to_gnd_volt = 0.001;
				}
				f32_scale = (dc_p_rt_data->f32_dc_bus_neg_to_gnd_volt) / (dc_p_rt_data->f32_dc_cb_pos_to_gnd_volt);
				f32_res = f32_scale * INSU_SAMPLE_RES_POS;
				if(INSU_SAMPLE_RES_NEG != f32_res)
				{
					f32_res = (f32_res * INSU_SAMPLE_RES_NEG) / (INSU_SAMPLE_RES_NEG - f32_res);
				}
			}
			else
			{   //母线正接地
				if(dc_p_rt_data->f32_dc_bus_neg_to_gnd_volt == 0)
				{
					dc_p_rt_data->f32_dc_bus_neg_to_gnd_volt = 0.001;
				}
				f32_scale = (dc_p_rt_data->f32_dc_cb_pos_to_gnd_volt) / (dc_p_rt_data->f32_dc_bus_neg_to_gnd_volt);
				f32_res = f32_scale * INSU_SAMPLE_RES_NEG;
				if(INSU_SAMPLE_RES_POS != f32_res)
				{
					f32_res = (f32_res * INSU_SAMPLE_RES_POS) / (INSU_SAMPLE_RES_POS - f32_res);
				}
			}		
			
	
			if(dc_p_rt_data->f32_dc_bus_neg_to_gnd_volt < (dc_p_rt_data->f32_dc_cb_pos_to_gnd_volt))
			{	//母线负接地
				dc_p_rt_data->f32_bus_pos_to_gnd_res = INSU_MAX_RES;
				dc_p_rt_data->f32_bus_neg_to_gnd_res = f32_res;
			}
			else
			{	//母线正接地
				dc_p_rt_data->f32_bus_pos_to_gnd_res = f32_res;
				dc_p_rt_data->f32_bus_neg_to_gnd_res = INSU_MAX_RES;
			}
		}
	}
	else
	{
		m_u32_bus_err_time = u32_delay_get_timer_val();

		f32_res = INSU_MAX_RES;
		dc_p_rt_data->f32_bus_pos_to_gnd_res = f32_res;
		dc_p_rt_data->f32_bus_neg_to_gnd_res = f32_res;		
	}

	if(f32_res < (F32_T)u16_ref_res)
	{
		if(!((dc_p_rt_data->u16_state)&0x4000))
		{
			dc_p_rt_data->u16_state |= 0x4000;
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_INSU_RELAY_FAULT;
			v_fauid_send_fault_id_occur(id);
		}
	}
	else
	{
		if((dc_p_rt_data->u16_state)&0x4000)
		{
			dc_p_rt_data->u16_state &= (~0x4000);
			id = (FAULT_DC_BUS_GROUP<<FAULT_GROUP_OFFSET)|FAULT_INSU_RELAY_FAULT;
			v_fauid_send_fault_id_resume(id);
		}
	}
}



/****************************************************************************
 *  函数名称：  v_dc_si_adj_v(F32_T f32_t_col_data) 
 *  输入参数:   f32_t_col_data为当前采集到的控母电压值
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   硅链调压(调用之前需获取互斥锁) 
 ***************************************************************************/
 void v_dc_si_adj_v(F32_T f32_t_col_data)
 {
 	F32_T d_value1;			//采集到的当前控母电压与配置的控母电压之间的差值
	F32_T d_value2;			//加、减一次硅链后的理论电压与配置的控母电压之间的差值
	F32_T si_v;
 	DC_CFG_T *dc_p_cfg = &(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc);
	si_state = u8_relay_get_diode_chain_status();
	//f32_t_col_data为当前实际输出电压， dc_p_cfg->u16_cb_output_volt目标控母输出值
	if((dc_p_cfg->u16_cb_output_volt) >= f32_t_col_data)
	{	//实际值比目标值小，需上调输出
		d_value1 = (dc_p_cfg->u16_cb_output_volt) - f32_t_col_data;
				
		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_5_4V)
		{
			si_v = 4;
			switch(si_state)
			{
			 	case 0x00:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x01;
					}
					break;

				case 0x01:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						si_state = 0x02;
					}
					break;

				case 0x02:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x03;
					}
					break;

				case 0x03:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY6_ON);
						si_state = 0x06;
					}
					break;

				case 0x06:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x07;
					}
					break;

				case 0x07:
					break;
			}	
		}		
				
		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_5_7V)
		{
			si_v = 7;
			switch(si_state)
			{
			 	case 0x00:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x01;
					}
					break;

				case 0x01:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						si_state = 0x02;
					}
					break;

				case 0x02:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x03;
					}
					break;

				case 0x03:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY6_ON);
						si_state = 0x06;
					}
					break;

				case 0x06:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x07;
					}
					break;

				case 0x07:
					break;
			}	
		}		

		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_7_3V)
		{
			si_v = 3;
			switch(si_state)
			{
			 	case 0x00:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x01;
					}
					break;

				case 0x01:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						si_state = 0x02;
					}
					break;

				case 0x02:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x03;
					}
					break;

				case 0x03:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						v_relay_relay_operation(ALARM_RELAY6_ON);
						si_state = 0x04;
					}
					break;

				case 0x04:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x05;
					}
					break;

				case 0x05:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						si_state = 0x06;
					}
					break;

				case 0x06:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x04;
					}
					break;

				case 0x07:
					break;
			}	
		}		

		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_7_5V)
		{
			si_v = 5;
			switch(si_state)
			{
			 	case 0x00:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x01;
					}
					break;

				case 0x01:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						si_state = 0x02;
					}
					break;

				case 0x02:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x03;
					}
					break;

				case 0x03:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						v_relay_relay_operation(ALARM_RELAY6_ON);
						si_state = 0x04;
					}
					break;

				case 0x04:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x05;
					}
					break;

				case 0x05:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						si_state = 0x06;
					}
					break;

				case 0x06:
					if((f32_t_col_data + si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data + si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data + si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						si_state = 0x04;
					}
					break;

				case 0x07:
					break;
			}	
		}										
	}
	else
	{	//实际值比目标值大，需下调输出
		d_value1 = f32_t_col_data - (dc_p_cfg->u16_cb_output_volt);

		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_5_4V)
		{
			si_v = 4;
			switch(si_state)
			{
			 	case 0x00:
					break;

				case 0x01:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x00;
					}
					break;

				case 0x02:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						si_state = 0x01;
					}
					break;

				case 0x03:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x02;
					}
					break;

				case 0x06:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY6_OFF);
						si_state = 0x03;
					}
					break;

				case 0x07:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{	//调节后的差值变少，才真正调节
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x06;
					}
					break;
			}	
		}		
				
		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_5_7V)
		{
			si_v = 7;
			switch(si_state)
			{
			 	case 0x00:
					break;

				case 0x01:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x00;
					}
					break;

				case 0x02:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						si_state = 0x01;
					}
					break;

				case 0x03:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x02;
					}
					break;

				case 0x06:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY6_OFF);
						si_state = 0x03;
					}
					break;

				case 0x07:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x06;
					}
					break;
			}	
		}		

		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_7_3V)
		{
			si_v = 3;
			switch(si_state)
			{
			 	case 0x00:
					break;

				case 0x01:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x00;
					}
					break;

				case 0x02:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						si_state = 0x01;
					}
					break;

				case 0x03:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x02;
					}
					break;

				case 0x04:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						v_relay_relay_operation(ALARM_RELAY6_OFF);
						si_state = 0x03;
					}
					break;

				case 0x05:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x04;
					}
					break;

				case 0x06:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						si_state = 0x05;
					}
					break;

				case 0x07:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x06;
					}
					break;
			}	
		}		

		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl==STEP_7_5V)
		{
			si_v = 5;
			switch(si_state)
			{
			 	case 0x00:
					break;

				case 0x01:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x00;
					}
					break;

				case 0x02:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						si_state = 0x01;
					}
					break;

				case 0x03:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x02;
					}
					break;

				case 0x04:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_ON);
						v_relay_relay_operation(ALARM_RELAY6_OFF);
						si_state = 0x03;
					}
					break;

				case 0x05:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x04;
					}
					break;

				case 0x06:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_ON);
						v_relay_relay_operation(ALARM_RELAY5_OFF);
						si_state = 0x05;
					}
					break;

				case 0x07:
					if((f32_t_col_data - si_v) >=(dc_p_cfg->u16_cb_output_volt))
					{
					 	d_value2 = (f32_t_col_data - si_v) - (dc_p_cfg->u16_cb_output_volt);	
					}
					else
					{
						d_value2 = 	(dc_p_cfg->u16_cb_output_volt) - (f32_t_col_data - si_v);
					}

					if(d_value1 > d_value2)
					{
						v_relay_relay_operation(ALARM_RELAY4_OFF);
						si_state = 0x06;
					}
					break;
			}	
		}	
	}
 }

 /****************************************************************************
 *  函数名称：  v_dc_col_task (void) __task 
 *  输入参数:   
 *  输出参数:   无
 *  返回结果:	无	
 *  功能介绍:   直流采集任务 
 ***************************************************************************/
__task void v_dc_col_task(void) 
 {
 	U16_T flags;
	U16_T cb_have;													//1表示分合、控母；0表示不分合、控母
	U8_T  u8_batt_group_num;

	m_u32_bus_err_time = u32_delay_get_timer_val();

 	while(1)
	{
		os_evt_set(DC_SAMPLE_FEED_DOG, g_tid_wdt);                    //设置喂狗事件标志
		os_mut_wait(g_mut_share_data, 0xFFFF);
		u8_batt_group_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		if(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb)
		{
			cb_have = 1;	
		}
		else
		{
			cb_have = 0;
		}
		os_mut_release(g_mut_share_data);

		if((os_evt_wait_or(DC_ADJUST_EVT_FLAGS, 10) == OS_R_EVT))   //如果直流校准标志有效，进入校准
		{
			flags = os_evt_get();
			if(cb_have)
			{
				if (flags & DC_ADJUST_PB_VOLT)							//满足条件则进入合母电压校准
				{
					v_dc_calbration(DC_ADJUST_PB_VOLT);
				}		
			}
			else
			{
				if(flags & DC_ADJUST_PB_VOLT)
				{
					os_evt_set(DC_ADJUST_FAIL, g_tid_display);
				}
			}

			if (flags & DC_ADJUST_CB_VOLT)						//满足条件则进入控母电压校准
			{
				v_dc_calbration(DC_ADJUST_CB_VOLT);
			}
			if (flags & DC_ADJUST_BUS_NEG_TO_END_VOLT)						//满足条件则进入母线负对地电压校准
			{
				v_dc_calbration(DC_ADJUST_BUS_NEG_TO_END_VOLT);
			}
			if (flags & DC_ADJUST_BATT_VOLT)						//满足条件则进入电池电压校准
			{
				v_dc_calbration(DC_ADJUST_BATT_VOLT);
			}
			if (flags & DC_ADJUST_LOAD_CURR_1)							//满足条件则进入负载电流校准
			{
				v_dc_calbration(DC_ADJUST_LOAD_CURR_1);
			}
			if (flags & DC_ADJUST_LOAD_CURR_2)						   //满足条件则进入负载电流校准
			{
				v_dc_calbration(DC_ADJUST_LOAD_CURR_2);
			}
			if (flags & DC_ADJUST_BATT1_CURR_1)						  //满足条件则进入电池电流校准
			{
				v_dc_calbration(DC_ADJUST_BATT1_CURR_1);
			}
			if (flags & DC_ADJUST_BATT1_CURR_2)						   //满足条件则进入电池电流校准
			{
				v_dc_calbration(DC_ADJUST_BATT1_CURR_2);
			}
			if (flags & DC_ADJUST_BATT2_CURR_1)					   //满足条件则进入充电机电流校准
			{
				v_dc_calbration(DC_ADJUST_BATT2_CURR_1);
			}
			if (flags & DC_ADJUST_BATT2_CURR_2)					   //满足条件则进入充电机电流校准
			{
				v_dc_calbration(DC_ADJUST_BATT2_CURR_2);
			}
			v_fetch_save_adjust_coeff();		  //将校准后的斜率存至dataflash
		}
		if(cb_have)
		{
		 	v_dc_collect(PB_VOLT);		  // 合母电压采集
			v_dc_collect(CB_VOLT);		  // 控母电压采集
		}
		else								   
		{
			v_dc_collect(CB_VOLT);	  // 母线电压采集
		}
 		v_dc_collect(LOAD_CURR);	  // 负载电流采集
		v_dc_collect(BATT_VOLT);	  // 电池电压采集

		os_evt_set(DC_SAMPLE_FEED_DOG, g_tid_wdt);                    //设置喂狗事件标志

		v_dc_collect(S_PE);
		if (u8_batt_group_num >= 1)
			v_dc_collect(BATT1_CURR);	 //1组电池电流采集
		if (u8_batt_group_num >= 2)
			v_dc_collect(BATT2_CURR);	 //2组电池电流采集

		os_mut_wait(g_mut_share_data, 0xFFFF);
		if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num == 0)
		{
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0] = 0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] = 0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_total_curr = 0;
		}
		else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num==1)
		{
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1] = 0;
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_total_curr = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0];
		}
		else if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num==2)
		{
			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_total_curr = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]+ 
																			g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1];
		}
		os_mut_release(g_mut_share_data);

		v_dc_collect(TEMPERATURE);	 //	 环境温度采集
		v_dc_data_judge();
	}
 }
