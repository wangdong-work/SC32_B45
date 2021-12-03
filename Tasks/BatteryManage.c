/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����BatteryManage.c
��    ����1.00
�������ڣ�2012-05-25
��    �ߣ����ķ�
������������ع���ʵ���ļ���ʵ�ֵ�س��״̬����&���ģ�������ѹ���ơ�����������㡢��������

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	���ķ�    2012-05-25  1.00     ����
	������	  2012-10-25  1.01     ���ӵڶ��������������Լ��ڶ����صĳ�ŵ����
	������	  2012-10-31  1.02     ���״̬�ı�󣬶Ը����״̬�µ�ʱ���ʱ���������㣬�Խ����ʱ��׼ȷ�����⡣
	                               ���Ӻ���ָʾ�ɽ��������ơ�
								   �궨���Ϊ����ͷ�ļ��У�����ԭ����Сд��Ϊ��д���Է��Ϻ궨�Ĺ���
	������	  2012-12-08  1.03     �������ܴ�ʱ�����ݵ�ص�����������ͬ�ĵ��������ʼֵ��
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



static	BATM_CHARGE_DATA_T	m_t_batm_charge_data[2];		 	//��س���������
static	U16_T	m_u16_batm_copy_batt[2];					 	//���õ�س��״̬����
static	U16_T	m_u16_batm_relay_batt[2];						//���ڵ�ؾ����������ָʾ�����ֵ�ǰ���״̬
static	BATT_MGMT_MODE_E	m_e_batm_copy_mode[2];				//����ϵͳ����ģʽ

static	U32_T	m_u32_last_estimate_t[2] = {0};	   	        //�ϴγ��״ת����ʱ���
static	U16_T	m_u16_flo_lost_ac_minute[2] = {0};	        //����״̬�½���ͣ���ʱʱ�ۼ���
static	U32_T	m_u32_last_ajdust_limit_t[2] = {0};	        //�ϴ���������ʱ���
static	U32_T	m_u32_board_code_count[2] = {0};	        //������������������㲥�������
static	U8_T	m_u8_dis_setup_basic_limit[2] = {1};        //�ŵ�״̬���Ƿ����趨��Ԥ����ֵ��1��ʾû�У�0��ʾ��
static	U8_T	m_u8_chr_setup_basic_limit[2] = {1};        //���״̬���Ƿ����趨��Ԥ����ֵ��1��ʾû�У�0��ʾ��


static	U16_T	m_u16_flo_hour_acc[2] = {0};	   	   	    //�����ʱʱ�ۼ�����
static	U16_T	m_u16_equ_limit_minute[2] = {0};	   	    //�����άʱ���ʱ�ۼ���	  
static	U16_T	m_u16_chd_limit_minute[2] = {0};	   	    //����״̬ʱ���ʱ�ۼ��� 

static const F32_T m_f32_mdl_rated_cur[] = {5.0,7.0,10.0,20.0,30.0,35.0,40.0,50.0};	 //����ģ���������ֵ


#define KI_DOT_NUM	10								//���õķŵ���������
/* �ŵ�ʱ��*/
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

/* �ŵ������*/
F32_T	f32_batt_ki[KI_DOT_NUM] = {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};





/*************************************************************
��������: v_batm_save_record		           				
��������: �������¼�ת����¼	 		 					
�������: recond_id �¼�ID        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_batm_save_record(U16_T recond_id)
{
	RTC_TIME_T	pt_time;					
	LOG_DATA_T	log_data;

	v_rtc_rtc_get_time(&pt_time);		  	//ȡʱ��
	
   	log_data.u16_log_id = 	recond_id;		   
    log_data.u8_occur_year = (U8) (pt_time.year - 2000);
	log_data.u8_occur_mon = pt_time.month;
	log_data.u8_occur_day = pt_time.day;
	log_data.u8_occur_hour = pt_time.hour;
	log_data.u8_occur_min = pt_time.min;
	log_data.u8_occur_sec = pt_time.sec;

	v_log_save_record(&log_data);		 //�����¼���Ϣ
	v_log_set_log_state();				 //�����¼���¼��Ϣ�仯��־
}




/*************************************************************
��������: v_batm_reset_batm_t		           				
��������: ��λ���г���ʱ��Ϊ0	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/

void v_batm_reset_batm_t(U8_T no)
{
	m_u16_flo_hour_acc[no] = 0;	   	   	//�����ʱʱ�ۼ�����
	m_u16_equ_limit_minute[no] = 0;	   	//�����άʱ���ʱ�ۼ���	  
	m_u16_chd_limit_minute[no] = 0;	   	//����״̬ʱ���ʱ�ۼ��� 

	m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //����ת�������ʱ����0
	m_t_batm_charge_data[no].u16_persist_flo_t = 0;   		  //��ά����ʱ���ʱ����0
	m_t_batm_charge_data[no].u16_persist_equ_t = 0;			  //�����ʱ����0
	m_t_batm_charge_data[no].u16_equ_count_down = 0;		  //���䵹��ʱ��0	
	m_t_batm_charge_data[no].u16_persist_chd_t = 0;			  //���ݼ�ʱ����0  	
	m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //�彻��ͣ���ʱ��
	g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = 0;   //���״̬�ۼ�ʱ��
}



/*************************************************************
��������: v_batm_equ_manage		           				
��������: ��ؾ������������ʱ�䵽���趨�������ʱ�㣬����䵹��ʱʱ�䵽����תΪ����
		  ���ñ�����ǰ����Ҫ�Ⱦ����������������Ļ�����g_mut_share_data	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/

void v_batm_equ_manage(U8_T no)	
{
	if ( (m_t_batm_charge_data[no].u16_persist_equ_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_equ_time)
  	     || (m_t_batm_charge_data[no].u16_equ_count_down >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_curr_go_time) )
	{  
		m_e_batm_copy_mode[no] = AUTO_MODE;		 					 //���ݽ�����ת�Զ���ع���ʽ
		g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] = m_e_batm_copy_mode[no];
		
		m_t_batm_charge_data[no].u16_charge_mode = FLO;
		g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no] = FLO;
		v_fetch_save_cfg_data();                                   //�����������ݵ�dataflash 						
		
		v_batm_reset_batm_t(no); 
		v_batm_save_record(LOG_ID_BATT_AUTO_TO_FLO + LOG_ID_BATT_NUM * no);			  //���Զ�ת�����¼�  		
	}
}			



/*************************************************************
��������: v_batm_flo_manage		           				
��������: ��ظ������������ʱ�䵽���趨�����ھ���㣬���ص�������ת�������������Ҫ�ĳ���ʱ��
		  ����ͣ��ﵽ�趨��ת����ʱ�䳤�ȣ���תΪ����
		  ���ñ�����ǰ����Ҫ�Ⱦ����������������Ļ�����g_mut_share_data	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/	  
void v_batm_flo_manage(U8_T no)	
{
	if ((m_t_batm_charge_data[no].u16_persist_flo_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_equ_cycle)
  	     || (m_t_batm_charge_data[no].u16_oc_durable_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_to_equ_dur_time)
		 ||	(m_t_batm_charge_data[no].u16_lost_ac_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_ac_fail_time))
	{
		m_t_batm_charge_data[no].u16_charge_mode = EQU;
		v_batm_reset_batm_t(no);
		v_batm_save_record(LOG_ID_BATT_AUTO_TO_EQU + LOG_ID_BATT_NUM * no);			  //���Զ�ת�����¼� 
	}
}


/*************************************************************
��������: v_batm_chd_manage		           				
��������: ��غ��ݹ���������ʱ�䵽���趨�ķŵ�ʱ�䣬�������ѹ�����趨�ķŵ���ֹ��ѹ
		  ����һ�����ص�ѹ�е����趨�ĵ���ŵ��ѹ����תΪ����
		  ���ñ�����ǰ����Ҫ�Ⱦ����������������Ļ�����g_mut_share_data	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/	  
void v_batm_chd_manage(U8_T no)	
{
	U8_T  batt_bxx_num;	 //ÿ����Ѳ�쵥Ԫ����
	F32_T min_cell_volt;	 //ÿ������С�����ѹ

	min_cell_volt = g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_min_cell_volt;
	batt_bxx_num  = g_t_share_data.t_sys_cfg.t_batt.t_batt_group[no].u8_bms_num;

	if ( (m_t_batm_charge_data[no].u16_persist_chd_t >= g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_max_dis_time)
  	     || (g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_total_end_volt)
		 /*����е��Ѳ�첢��͵�ѹ�����趨�ŵ���Ŀ����ֵ�����޵��Ѳ�죬����������Ч*/
		 ||	((min_cell_volt < g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_cell_end_volt) ? batt_bxx_num : 0) )	   
	{ 			
		m_e_batm_copy_mode[no] = AUTO_MODE;		 					 //���ݽ�����ת�Զ���ع���ʽ
		g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] = m_e_batm_copy_mode[no];

		m_t_batm_charge_data[no].u16_charge_mode = EQU;
		g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no] = EQU;
		v_fetch_save_cfg_data();                                   //�����������ݵ�dataflash

		v_batm_reset_batm_t(no);
		v_batm_save_record(LOG_ID_BATT_AUTO_TO_EQU + LOG_ID_BATT_NUM * no);			  //���Զ�ת�����¼� 
	}
}



/*************************************************************
��������: v_batm_setup_charg_panl_out_vol		           				
��������: ���ݳ��״̬�����ó��������ѹֵ,�������״̬�������������ѹΪ�ŵ���ֹ��ѹ��ȥ2V
		  �����ѹ�в����²��������������²�
		  ���ñ�����ǰ����Ҫ�Ⱦ����������������Ļ�����g_mut_share_data	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/	

void v_batm_setup_charg_panl_out_vol(U8_T no)
{ 	
	F32_T	  equalize_vol;			//�²�ֵ,��VΪ��λ

	/*�����²�ֵ����25��Ϊ���ģ����е��Ѳ��ʱ�����õ��Ѳ�����¶ȼ��㣬���ޣ����ü�������Դ����¶ȼ���*/
	if (g_t_share_data.t_sys_cfg.t_batt.t_batt_group[no].u8_bms_num != 0)
	{
		equalize_vol = (25.0-g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_temperature1) 
		               * g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt / 1000;		  //�²�ϵ������mVΪ��λ
	 
	}
	else
	{
		equalize_vol = (25.0-g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature) 
		               * g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_batt_temp_comp_volt / 1000;		 //�²�ϵ������mVΪ��λ
	}

	/*����²�ֵ��+3.0V~-3.0V*/
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
��������: v_batm_count_single_battery_capability		           				
��������: ����ʱ���͵����ţ����㵥��������	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_batm_count_single_battery_capability(F32_T dt, U8_T no)
{
	U8_T	i;
	F32_T	C10, Ki, Ib, Kc;

	/* һ�����������㿪ʼ */
    C10 = g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;   			// C10��ض����
    Ib  = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];  	// һ���ص��� 
    
	/* ��� */
	if ( Ib >= 0 ) 
	{
		m_t_batm_charge_data[no].f32_bat_capacity += dt * Ib * 0.9500000; 		//�̶����Ч��Ϊ0.95  		
	}
	/* �ŵ� */
    else
	{
		if (Ib > -0.5)    //���ŵ�����Ƚ�Сʱ��������������
			goto save_capability;

		if (Ib < -C10)    //����ŵ��������1C������1C�ķŵ��������������
			Ib = -C10;

		Ki = Ib / C10;
		Ki = -Ki;
	
		for( i = 1; i < KI_DOT_NUM; i++ )
		{
			if ( Ki <= f32_batt_ki[i] )
				break;
		}

		if (i >= KI_DOT_NUM)						   				//�±걣������ֹ�����쳣���ص�������C1ʱ�±����
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

	g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_capacity = m_t_batm_charge_data[no].f32_bat_capacity;		  //ˢ����ʾ

	/*��������¼��RTC�Ĵ�����ʵ�ֶϵ���´��ϵ�ɶ�ȡ��֮ǰ����Ľ��*/
	v_rtc_save_user_data((U32_T)(m_t_batm_charge_data[no].f32_bat_capacity*BATM_CAPACITY_COEFF), (RTC_GPREG_E)(GP_REG_1 + no));

	/* һ��������������� */
}

/*************************************************************
��������: v_batm_count_battery_capability		           				
��������: ����������㣬ÿ10Sʱ��������һ��
		  ���ʱ�̶����Ч��Ϊ0.95���ŵ�ʱ��������õķŵ����߽���ϵ��������	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_batm_count_battery_capability(U8_T no)
{
	static	U32_T	last_count_cap_t = 0;	   //�ϴμ���������ʱ���	
	U32_T	this_count_cap_t;	       		   //���μ���������ʱ���
	F32_T	dt;	

	/*���ݽ���������ָ��������*/
	if (os_evt_wait_or(BATT_CAPACITY_RESTORE, 0) == OS_R_EVT) 
	{ 	
		os_mut_wait(g_mut_share_data, 0xFFFF);					//�ȴ�������
		m_t_batm_charge_data[0].f32_bat_capacity = g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;
		m_t_batm_charge_data[1].f32_bat_capacity = g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;
		g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity = m_t_batm_charge_data[0].f32_bat_capacity;		  //ˢ����ʾ 
		g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity = m_t_batm_charge_data[1].f32_bat_capacity;
		os_mut_release(g_mut_share_data);			 			//�ͷŻ�����   		
	}


	this_count_cap_t = u32_delay_get_timer_val();

	if (u32_delay_time_elapse(last_count_cap_t, this_count_cap_t) < BATM_CUN_CAP_TIME_SLOT * OSC_SECOND)		 //ÿ10Sʱ�����ж�һ��
	{ 			
		return;
	}

	last_count_cap_t = (last_count_cap_t + BATM_CUN_CAP_TIME_SLOT * OSC_SECOND) % 0xFFFFFFFF;				  //����ʱ���¼��

	os_mut_wait(g_mut_share_data, 0xFFFF);								//�ȴ�������
	
	dt =  (float)BATM_CUN_CAP_TIME_SLOT	/ 3600.0;		   				//������ʱ��������ʱ��ʾ	
	
	v_batm_count_single_battery_capability(dt, no);                      //һ������������
	
	/* ����������֮�ͼ��� */	
//	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num <= 1)
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity;
//	else	
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity + m_t_batm_charge_data[1].f32_bat_capacity;			  

	os_mut_release(g_mut_share_data);			 			//�ͷŻ�����
}




/*************************************************************
��������: v_batm_estimate_charge_state		           				
��������: ���״̬ת��,ÿ1������һ���ж�	 		 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_batm_estimate_charge_state(U8_T no)
{
	U32_T	this_estimate_t, time_diff_t;	    //���γ��״̬ת�����ڵ㣬��uSΪ��λ
	F32_T	refer_bat_cur;
    U32_T	u32_charge_state_time;              //���״̬������ʱ��


	this_estimate_t = u32_delay_get_timer_val();

	time_diff_t = u32_delay_time_elapse(m_u32_last_estimate_t[no],this_estimate_t);
	if (time_diff_t < BATM_TIME_SLOT * OSC_SECOND)		 //ÿ1Sʱ�����ж�һ��
	{ 			
		return;
	}

	m_u32_last_estimate_t[no] = (m_u32_last_estimate_t[no] + BATM_TIME_SLOT * OSC_SECOND) % 0xFFFFFFFF;				  //����ʱ���¼��

	os_mut_wait(g_mut_share_data, 0xFFFF);					//�ȴ�������
	 
	/*���ʱ��ͳ�ƣ������״̬�ֱ�ͳ��*/
   	switch (m_t_batm_charge_data[no].u16_charge_mode)
	{

		/*����ģʽ�µļ�ʱ������*/
		case	FLO:			
				/*����״̬�£���ά����ʱ��ͳ�ƣ���ʱΪ��λ*/
				if ((m_u16_flo_hour_acc[no] += BATM_TIME_SLOT) >= 3600) 
				{
					m_t_batm_charge_data[no].u16_persist_flo_t++;
					g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] =  m_t_batm_charge_data[no].u16_persist_flo_t; 		//ˢ�µ����ʾҳ�������ʱ��
					m_u16_flo_hour_acc[no] = 0;
				}

				/*����״̬�£�������άʧ��ʱ��ͳ�ƣ��Է�Ϊ��λ*/
				switch ( g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path )
				{
					case ONE_PATH: 
						if ((g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state & 0x1) == 0x1 )	   //1·����������һ·�Ƿ�ͣ��
						{
							if ((m_u16_flo_lost_ac_minute[no] += BATM_TIME_SLOT) >= 60) 
							{
								m_t_batm_charge_data[no].u16_lost_ac_t ++;
								m_u16_flo_lost_ac_minute[no] = 0;
							}							
						}
						else
						{										
							m_t_batm_charge_data[no].u16_lost_ac_t = 0;								     //����ĳ·��������������ͣ���ʱ��
							m_u16_flo_lost_ac_minute[no] = 0;
						}
						break;

					 case TWO_PATH: 
						if ((g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state & 0x11) == 0x11 )	   //2·����������2·�Ƿ�ͬʱͣ��
						{
							if ((m_u16_flo_lost_ac_minute[no] += BATM_TIME_SLOT) >= 60) 
							{
								m_t_batm_charge_data[no].u16_lost_ac_t++;
								m_u16_flo_lost_ac_minute[no] = 0;
							}							
						}
						else
						{
							m_t_batm_charge_data[no].u16_lost_ac_t = 0;									 //����ĳ·��������������ͣ���ʱ��
							m_u16_flo_lost_ac_minute[no] = 0;
						}
						break;

					default:
						m_t_batm_charge_data[no].u16_lost_ac_t = 0;
						m_u16_flo_lost_ac_minute[no] = 0;
						break;	
			   	}

				/*�����ص��������趨ת�������*/
				refer_bat_cur = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];

				if (refer_bat_cur >= (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_to_equ_curr))
				{
					m_t_batm_charge_data[no].u16_oc_durable_t += BATM_TIME_SLOT;
				}
				else
				{
					m_t_batm_charge_data[no].u16_oc_durable_t = 0;
				}					   				
				
			   	m_t_batm_charge_data[no].u16_persist_equ_t = 0;			   //�����ʱ����0
			   	m_t_batm_charge_data[no].u16_equ_count_down = 0;		   //���䵹��ʱ��0
				m_t_batm_charge_data[no].u16_persist_chd_t = 0;			   //���ݼ�ʱ����0
				break;

		/*����ģʽ�µļ�ʱ������*/
		case	EQU:
					/*�����άʱ���ʱ��*/
				if ((m_u16_equ_limit_minute[no] += BATM_TIME_SLOT) >= 60) 
				{
					m_t_batm_charge_data[no].u16_persist_equ_t++;
					g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] =  m_t_batm_charge_data[no].u16_persist_equ_t; 		//ˢ�µ����ʾҳ�������ʱ��
					m_u16_equ_limit_minute[no] = 0;
				

					/*�����ص���С���趨ת�������*/
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

				m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //����ת�������ʱ����0
				m_t_batm_charge_data[no].u16_persist_flo_t = 0;   		  //��ά����ʱ���ʱ����0
				m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //�彻��ͣ���ʱ��
				m_t_batm_charge_data[no].u16_persist_chd_t = 0;			  //���ݼ�ʱ����0 				
				break;

		/*����ģʽ�¼�ʱ������*/
		case	DIS:
			   	if ((m_u16_chd_limit_minute[no] += BATM_TIME_SLOT) >= 60) 
				{
					m_t_batm_charge_data[no].u16_persist_chd_t++;
					g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] =  m_t_batm_charge_data[no].u16_persist_chd_t; 		//ˢ�µ����ʾҳ�������ʱ��
					m_u16_chd_limit_minute[no] = 0;
				}
				m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //����ת�������ʱ����0
				m_t_batm_charge_data[no].u16_persist_flo_t = 0;   		  //��ά����ʱ���ʱ����0
				m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //�彻��ͣ���ʱ��
				m_t_batm_charge_data[no].u16_persist_equ_t = 0;			  //�����ʱ����0
			   	m_t_batm_charge_data[no].u16_equ_count_down = 0;		  //���䵹��ʱ��0 				
				break;

		default:
				break;
	}
	

	/* ����/�Զ�ת���¼� */
	g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[1] = g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[0];//�ڶ������Զ����ƹ��õ�һ��ֵ
	if (m_e_batm_copy_mode[no] != g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no])	
	{
		U16_T	e_mode_id;
		m_e_batm_copy_mode[no] = g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no];		//������״̬
		if (m_e_batm_copy_mode[no] == AUTO_MODE)
		{
			e_mode_id = LOG_ID_BATT_TO_AUTO;		   //������תΪ�Զ�
		}
		else
		{
			e_mode_id = LOG_ID_BATT_TO_MANUAL;		   //������תΪ�ֶ�	
		}

		v_batm_save_record(e_mode_id + LOG_ID_BATT_NUM * no);			  
	}


	/*�Զ�������*/
	if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] == AUTO_MODE)		 
	{		
		switch (m_t_batm_charge_data[no].u16_charge_mode)
		{
			case	EQU:						//�������
				v_batm_equ_manage(no);
				break;

			case	DIS:					   //���ݹ���
				v_batm_chd_manage(no);
				break;

			case	FLO:					  //�������
			default:
				v_batm_flo_manage(no);
				break;

		}

		/*���ݵ�ǰ���״̬���Ա�����תΪ�ֶ�״̬ʱ�����������ֶ�����״̬��һ��*/
		m_u16_batm_copy_batt[no] = m_t_batm_charge_data[no].u16_charge_mode;		
	}  


	/*�ֶ�������	 */
	else
	{
		if (m_u16_batm_copy_batt[no] != g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no])		  //����趨��״̬�б䣬�򽫵��״̬תΪ�趨����״̬
		{ 	
			U16_T	charge_mode_id;
			m_u16_batm_copy_batt[no] = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];
			m_t_batm_charge_data[no].u16_charge_mode = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];
			v_batm_reset_batm_t(no);													  //���м��������¿�ʼ	
			
			switch (m_t_batm_charge_data[no].u16_charge_mode)	
			{
				case	EQU:
					charge_mode_id = LOG_ID_BATT_MANUAL_TO_EQU;						   //����ֶ�ת����
					break;

				case	DIS:
					charge_mode_id = LOG_ID_BATT_MANUAL_TO_DIS;						   //����ֶ�ת����
					break;

				case	FLO:
				default:
					charge_mode_id = LOG_ID_BATT_MANUAL_TO_FLO;						   //����ֶ�ת����
					break;	 
			}	
			
			v_batm_save_record(charge_mode_id + LOG_ID_BATT_NUM * no);								  

		}

		switch (m_t_batm_charge_data[no].u16_charge_mode)
		{
			case	EQU:						//�������
				v_batm_equ_manage(no);
				break;

			case	DIS:					   //���ݹ���
				v_batm_chd_manage(no);
				break;

			case	FLO:					  //��������ֶ�����£����ڸ���״̬��������й���
			default:
				break;

		}

	}

	

	/*���ݳ��״̬�����ó��������ѹֵ*/
	v_batm_setup_charg_panl_out_vol(no); 	 

	g_t_share_data.t_rt_data.t_batt.e_state[no] = (BATT_STATE_E)m_t_batm_charge_data[no].u16_charge_mode;			  //ˢ�½�����ʾ

   	u32_charge_state_time = ((U32_T)m_t_batm_charge_data[no].u16_charge_mode) << 16;	//��ص�ǰ���״̬
	u32_charge_state_time += g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];
	v_rtc_save_user_data(u32_charge_state_time, (RTC_GPREG_E)(GP_REG_3 + no));	 		            //����鵱ǰ���״̬

	os_mut_release(g_mut_share_data);			 //�ͷŻ�����



}



/*************************************************************
��������: v_batm_adjust_rect_limit		           				
��������: ������������ڣ����ݵ�ǰ�ĵ�ص��������ص���ֵ���趨�ĺ����������������� 	
		  Ϊ�˱�֤���ڵ��ȶ��ԣ�ÿ5�����һ��,�ڵ�طŵ缰����50%������ʱ����һ��Ԥ����ֵ
		  ��Ԥ����ֵ��һ�����ڹ����У�ֻ�ɸ�һ�Σ� �Է�ֹ���ڲ������쳣�����������ڹ���	 					
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_batm_adjust_rect_limit(U8_T no)
{
 	U32_T	this_ajdust_limit_t = 0;	   			//������������ʱ���
	F32_T	adjust_limit_value_abs;					//����Ӧ���ڵĵ������Ծ���ֵ��ʾ�����ǰٷֱ�
	F32_T	setup_bat_limit_cur;				//ϵͳ�趨�ĳ�����ֵ���Ծ���ֵl��ʾ
	F32_T   refer_bat_cur;						//�����������ĳ�����
	U32_T	u32_elapse_time;
	U8_T    u8_rect_num;
	RATED_CURR_E e_rated_curr;


	//�ֶ�������ʽ��ֱ���ý������õ��������·������ý��бջ�����
	os_mut_wait(g_mut_share_data, 0xFFFF);
	if (g_t_share_data.t_sys_cfg.t_sys_param.e_limit_curr == MANUAL_LIMIT_CURR)
	{
		g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[no] = g_t_share_data.t_sys_cfg.t_sys_param.f32_manual_limit_curr;	   //����������
		
		os_mut_release(g_mut_share_data);			 //�ͷŻ�����
		return;
	}
	os_mut_release(g_mut_share_data);			 //�ͷŻ�����

	//�������Զ�������ʽ�Ĵ�����Ҫ���бջ��ȴ�
	/*������������ִ��������������� */
	if (os_evt_wait_or((RECT_SET_CURR_SCUESS1 + 2 * no), 0) != OS_R_EVT) 
	{ 	
		return; 		
	}
	m_u32_board_code_count[no]++;

	//�̶�ʱ��������������
	this_ajdust_limit_t = u32_delay_get_timer_val();
	u32_elapse_time = u32_delay_time_elapse(m_u32_last_ajdust_limit_t[no],this_ajdust_limit_t);
	if ((u32_elapse_time < 6 * OSC_SECOND) && (m_u32_board_code_count[no] < 10))		 //ÿ5Sʱ��������һ��
	{ 			
		return;
	}
	m_u32_board_code_count[no] = 0;		//�ָ���������0
	m_u32_last_ajdust_limit_t[no] =  this_ajdust_limit_t;   


	os_mut_wait(g_mut_share_data, 0xFFFF);					//�ȴ�������

	setup_bat_limit_cur = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_limit_curr * g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10;	 //�趨����ֵ

	/*������Ǵ��ڷŵ�״̬*/
	refer_bat_cur = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];

	if (refer_bat_cur < 0) 
	{
		/*֮ǰû�и�������ֵ�����������ֵΪ�������ֵ+���ص���*/
		if (m_u8_dis_setup_basic_limit[no] && (m_t_batm_charge_data[no].f32_charge_limit_percent < BATM_MDL_MAX_LIMIT_PERCENT))		  
		{
			m_u8_dis_setup_basic_limit[no] = 0;
			if (no == 0)
				adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr;			//��ǰ�ܵ�����ֵΪ�������ֵ+���ص���
			else
				adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr;		//��ǰ�ܵ�����ֵΪ�������ֵ+���ص���

			/*���������ֵת��Ϊ�ٷֱ�*/
			e_rated_curr = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr;
			if( g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
				u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num / 2;
			else
				u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
			m_t_batm_charge_data[no].f32_charge_limit_percent = (float)(adjust_limit_value_abs * 100 / u8_rect_num
														                / m_f32_mdl_rated_cur[e_rated_curr]); 			 
		}
	    /*֮ǰ�и�������ֵ�����������ֵ��5%�����ſ�*/
		else
		{
			m_t_batm_charge_data[no].f32_charge_limit_percent += BATM_MDL_MIN_LIMIT_PERCENT;
		}
		 
	}
	/*������Ǵ��ڳ��״̬*/
	else
	{
		F32_T	allow_limit_error;				//�����������趨ֵ

		m_u8_dis_setup_basic_limit[no] = 1;			 //���´ηŵ����Ԥ����ֵ����
		/*���������ֵ��30A���ڣ��������ֵ���������0.3A*/
		if (setup_bat_limit_cur <= 30.0)
		{
			allow_limit_error = 0.3;
		}

		/*�趨�ĳ�����ֵ��30A���ϣ��������1%�趨����ֵ*/
		else
		{
			allow_limit_error =  setup_bat_limit_cur * 0.01;
		}

		/*�жϳ������Ƿ����趨����ֵ��Χ��*/
		refer_bat_cur = g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[no];

		if (fabs (refer_bat_cur - setup_bat_limit_cur) <= allow_limit_error )
		{
		}
		/*���ڷ�Χ��*/
		else 
		{	
			F32_T	limit_error;		//���������
			limit_error = refer_bat_cur - setup_bat_limit_cur;

			/*��������������趨����ֵ1.5������û���ڸ�����Ԥ����ֵ�����������趨Ϊ������+���ص������Աܵ�س�ʱ�����*/
			if ((limit_error > setup_bat_limit_cur * 0.5) && m_u8_chr_setup_basic_limit[no] ) 
			{
				m_u8_chr_setup_basic_limit[no] = 0;
				if (no == 0)
					adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr;			//��ǰ�ܵ�����ֵΪ�������ֵ+���ص���
				else
					adjust_limit_value_abs = setup_bat_limit_cur + g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load2_curr;		//��ǰ�ܵ�����ֵΪ�������ֵ+���ص���

				/*���������ֵת��Ϊ�ٷֱ�*/
				e_rated_curr = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr;
				if( g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num / 2;
				else
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
				m_t_batm_charge_data[no].f32_charge_limit_percent = (float)(adjust_limit_value_abs * 100 / u8_rect_num
															                / m_f32_mdl_rated_cur[e_rated_curr]); 			 
			}

		    /*���򽫵�ǰ�������ȥ���ٷֱ�*/
			else
			{
				e_rated_curr = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.e_rated_curr;
				if( g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num >= 2)
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num / 2;
				else
					u8_rect_num = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.u8_rect_num;
				m_t_batm_charge_data[no].f32_charge_limit_percent -= (float)(limit_error * 100 / u8_rect_num
														                     / m_f32_mdl_rated_cur[e_rated_curr]); 

				if (limit_error < 0)				   //���С���趨�����㣬�����´ι�������Ԥ����ֵ����
				{
					m_u8_chr_setup_basic_limit[no] = 1;
				} 
			}

		} 	
	}

	/*�������ֵ�޶�Ϊ105%~5%��Χ��*/
	if (m_t_batm_charge_data[no].f32_charge_limit_percent > BATM_MDL_MAX_LIMIT_PERCENT)
	{
		m_t_batm_charge_data[no].f32_charge_limit_percent = BATM_MDL_MAX_LIMIT_PERCENT;
	} 
	else if (m_t_batm_charge_data[no].f32_charge_limit_percent < BATM_MDL_MIN_LIMIT_PERCENT)
	{
		m_t_batm_charge_data[no].f32_charge_limit_percent = BATM_MDL_MIN_LIMIT_PERCENT;	
	}

	g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_curr_percent[no] = m_t_batm_charge_data[no].f32_charge_limit_percent;	   //����������

	os_mut_release(g_mut_share_data);			 //�ͷŻ�����

}

  	

/*************************************************************
��������: v_batm_relay_out		           				
��������: ������״̬����̵�������,ֱ�������̵����������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/

void v_batm_relay_out(U8_T no)
{
	U8_T	u8_setdata;	

	os_mut_wait(g_mut_share_data, 0xFFFF);					//�ȴ�������   

	if (m_u16_batm_relay_batt[no] != m_t_batm_charge_data[no].u16_charge_mode)		  //��״̬�Ƿ����ı�
	{  		 
		switch (m_t_batm_charge_data[no].u16_charge_mode)
		{
			/*��ǰ״̬Ϊ����*/
			case   EQU:																 
				u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output;
				v_fault_occur_fault_output(u8_setdata);	  						//���������


				if (m_u16_batm_relay_batt[no] == DIS)										//����ǰΪ����״̬�����ͷź���״̬�̵���
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output; 
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}
				break;

			/*��ǰ״̬Ϊ����*/
			case   FLO:					 
				if (m_u16_batm_relay_batt[no] == EQU)										//����ǰΪ����״̬�����ͷž���״̬�̵���
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output;
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}

				if (m_u16_batm_relay_batt[no] == DIS)										//����ǰΪ����״̬�����ͷź���״̬�̵���
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output; 
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}
				break;

			case	DIS:
			default:
				if (m_u16_batm_relay_batt[no] == EQU)										//����ǰΪ����״̬�����ͷž���״̬�̵���
				{
			    	u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output; 
					v_fault_resume_fault_output(u8_setdata);	  				
					
				}

				u8_setdata = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output;
				v_fault_occur_fault_output(u8_setdata);	  						//������ݽ��
				break;
		}

		m_u16_batm_relay_batt[no] = m_t_batm_charge_data[no].u16_charge_mode;		//ˢ�µ�ǰ����
	}

	os_mut_release(g_mut_share_data);			 //�ͷŻ�����
	
}



/*************************************************************
��������: v_batm_batt_manage_init		           				
��������: ��ع������ݳ�ʼ������ʼ�����״̬����������㣬���ʱ���ʱ��						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/

void v_batm_batt_manage_init(U8_T no)	
{
	U8_T	u8_setdata;
	U32_T   u32_charge_state_time;

	os_mut_wait(g_mut_share_data, 0xFFFF);


	/*��ʼ�����״̬*/
	m_t_batm_charge_data[no].f32_bat_capacity = (F32_T)u32_rtc_read_user_data((RTC_GPREG_E)(GP_REG_1 + no)) / BATM_CAPACITY_COEFF;	//��RTC�Ĵ����ж�ȡ����ǰ�������
	g_t_share_data.t_rt_data.t_batt.t_batt_group[no].f32_capacity = m_t_batm_charge_data[no].f32_bat_capacity;		  //ˢ����ʾ

	/* ����������֮�ͼ��� */	
//	if (g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num <= 1)
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity;
//	else	
//		g_t_share_data.t_rt_data.t_batt.f32_total_capacity = m_t_batm_charge_data[no].f32_bat_capacity + m_t_batm_charge_data[1].f32_bat_capacity;

	u32_charge_state_time  =  u32_rtc_read_user_data((RTC_GPREG_E)(GP_REG_3 + no));
	m_t_batm_charge_data[no].u16_charge_mode = (U16_T)(u32_charge_state_time >> 16);	//��ص�ǰ���״̬
	g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = (U16_T)u32_charge_state_time;
	if(m_t_batm_charge_data[no].u16_charge_mode >= 3)  //�Ƿ����״̬
	{
		g_t_share_data.t_rt_data.t_batt.e_state[no] = FLO;        //����鵱ǰ���״̬
		g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = 0;	 //����鵱ǰ���״̬����ʱ�� 
	}
	else
	{
		g_t_share_data.t_rt_data.t_batt.e_state[no] = (BATT_STATE_E)(m_t_batm_charge_data[no].u16_charge_mode);        //����鵱ǰ���״̬
		g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no] = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];	 //����鵱ǰ���״̬����ʱ�� 
	}

	/*if (g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no] == AUTO_MODE)		 			//�����Զ�״̬�����ʼ��Ϊ����
	{
		m_t_batm_charge_data[no].u16_charge_mode = FLO;
	}   
	else
	{
		m_t_batm_charge_data[no].u16_charge_mode = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];	   //�����ֶ���״̬�����ʼ��Ϊ�ֶ����õ�״̬		
	}*/
	m_t_batm_charge_data[no].u16_charge_mode = g_t_share_data.t_rt_data.t_batt.e_state[no];	    //���Ƴ��״̬
	m_u16_batm_copy_batt[no] = g_t_share_data.t_sys_cfg.t_ctl.u16_batt[no];					//�����ֶ����״̬
	m_e_batm_copy_mode[no] = g_t_share_data.t_sys_cfg.t_batt_mgmt.e_mode[no];				//����ϵͳ����ģʽ	

   

	/*��ʼ������ѹ*/
	v_batm_setup_charg_panl_out_vol(no); 	


	/*��ʼ�������㣬�������ֵΪ��ǰ���õ��ѻ�������*/
    m_t_batm_charge_data[no].f32_charge_limit_percent = g_t_share_data.t_sys_cfg.t_dc_panel.t_rect.f32_offline_curr_percent;


	/*��ʼ������ʱ��Ϊ0*/
	m_u16_flo_hour_acc[no]     = 0;	   	//�����ʱʱ�ۼ�����
	m_u16_equ_limit_minute[no] = 0;	   	//�����άʱ���ʱ�ۼ���	  
	m_u16_chd_limit_minute[no] = 0;	   	//����״̬ʱ���ʱ�ۼ��� 

	m_t_batm_charge_data[no].u16_oc_durable_t = 0;			  //����ת�������ʱ����0
	m_t_batm_charge_data[no].u16_equ_count_down = 0;		  //���䵹��ʱ��0
	m_t_batm_charge_data[no].u16_lost_ac_t = 0;				  //�彻��ͣ���ʱ��


	/*��ʼ������������״̬*/
	if (m_t_batm_charge_data[no].u16_charge_mode == FLO)
	{ 		
		m_t_batm_charge_data[no].u16_persist_flo_t = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];

		/*u8_setdata =  g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_flo_output; 
		v_fault_occur_fault_output(u8_setdata);	  						//���������*/
	}
	else if (m_t_batm_charge_data[no].u16_charge_mode == EQU)
	{
	    m_t_batm_charge_data[no].u16_persist_equ_t = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];

		u8_setdata =  g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_equ_output; 
		v_fault_occur_fault_output(u8_setdata);	  						//��������� 
	}
	else if (m_t_batm_charge_data[no].u16_charge_mode == DIS)
	{
		m_t_batm_charge_data[no].u16_persist_chd_t = g_t_share_data.t_rt_data.t_batt.u16_time_from_change_state[no];

		u8_setdata =  g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_dis_output; 
		v_fault_occur_fault_output(u8_setdata);
	}
	m_u16_batm_relay_batt[no] = m_t_batm_charge_data[no].u16_charge_mode;					//��ʼ����������Ƽ̵����뵱ǰ���״̬һ��	
	

	os_mut_release(g_mut_share_data);  					   //�ͷŻ�����

}





/*************************************************************
��������: v_batm_battery_manage_task		           				
��������: ��ع�������ʵ�ֵ�ؾ�����ת��&��ѹ���ڡ������������ڡ�
		  �����������㡢������״̬���������������ͨ��RTX����os_tsk_create������						
�������: ��        		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
__task void v_batm_battery_manage_task(void)
{
	U8_T	i, u8_batt_group_num;   	

	for(i=0; i<2; i++)
	{
		v_batm_batt_manage_init(i);							//��ʼ���ϵ�״̬
	}

	while (1) 
	{
		os_evt_set(BATT_FEED_DOG, g_tid_wdt);           //����ι���¼���־

		os_mut_wait(g_mut_share_data, 0xFFFF);								//�ȴ�������
		u8_batt_group_num = g_t_share_data.t_sys_cfg.t_sys_param.u8_batt_group_num;
		os_mut_release(g_mut_share_data);			 			//�ͷŻ�����

		for(i=0; i<u8_batt_group_num; i++)
		{
			v_batm_count_battery_capability(i);			//������������
			v_batm_estimate_charge_state(i);			//���״̬ת��
			v_batm_relay_out(i);						//����������̵�������
			v_batm_adjust_rect_limit(i);				//��������
		}

		os_dly_wait(20);								//����200ms		
	}
}
