/************************************************************
Copyright (C) ����Ӣ����Ƽ��������޹�˾
�� �� ����key.c
��    ����1.00
�������ڣ�2012-05-29
��    �ߣ�Ф��
����������ATT7022E chip����

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	Ф��    2012-05-29    1.00     ����
	Ф��    2012-11-22	  2.00	   �޸Ľ������֣������жϱ�����һ���ģ������жϱ��������⡣
	Ф��    2012-11-24    3.00     �޸ĵ���Ϊһ·���ȣ����ҵ�һ·���ϣ��ɵڶ�·�������ϵͳ������������
**************************************************************/
#include "rtl.h"
#include "../Drivers/Att7022eu.h"
#include "../Drivers/delay.h"
#include "PublicData.h"
#include "ShareDataStruct.h"
#include "FetchFlash.h"
#include "FaultId.h"
#include "string.h"
#include "../Drivers/Relay.h"

#define  AC1ON()  (AC_CTR_CH1_ON)	//	�պϵ�һ·����
#define  AC2ON()  (AC_CTR_CH2_ON)	   //������Ͷ���صĿ���
#define  AC1OFF() (AC_CTR_CH1_OFF)	  //�Ͽ���һ·����
#define  AC2OFF() (AC_CTR_CH2_OFF)



#define  UP   (1.19*1.2)
#define  DOWN  (1.19*0.8)			 //У׼ϵ����������


//ADJUST_COEFF_T m_att_default_cal={1.19,1.19,1.19,1.19,1.19,1.19};	   //ϵͳĬ�ϵĽ���ϵ��

//ADJUST_COEFF_T *m_pt_att_cal;  //��ǰʹ�õ�ϵ��ָ��
//ADJUST_COEFF_T m_att_cal_new;  //У׼��ȫ��ϵ���ݴ�
/********************************************
 *  �������ƣ� void v_att_att_jz(U32_T flag)
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   ATTУ׼
*********************************************/
void v_att_att_jz(U32_T flag)
{  	
	F32_T reg,adjcoff;
  	U32_T t;
  	switch(flag)
  	{	
  		case AC_ADJUST_FIRST_PATH_UV:
			t = att_att_read(V_A1);
			t = t/(1<<13);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			reg = t*g_t_share_data.t_coeff_data.f32_ac1_uv_slope;	//�ٳ���У׼ϵ��			
			adjcoff = g_t_share_data.t_ac_adjust_data.f32_first_path_volt_uv*g_t_share_data.t_coeff_data.f32_ac1_uv_slope/reg;
			os_mut_release(g_mut_share_data);
			if((adjcoff > UP)||(adjcoff < DOWN))
			{
				os_evt_set (AC_ADJUST_FAIL, g_tid_display);
			}
			else
			{ 
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_ac1_uv_slope = adjcoff; 
				os_mut_release(g_mut_share_data);
				v_fetch_save_adjust_coeff();
//				m_pt_att_cal->f32_coeff_1 = adjcoff;
//			    v_fetch_save_ac_adjust_coeff(m_pt_att_cal);
				os_evt_set (AC_ADJUST_SCUESS, g_tid_display);
			}
			break;


		case AC_ADJUST_FIRST_PATH_VW:
			t = att_att_read(V_B1);
			t = t/(1<<13);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			reg = t*g_t_share_data.t_coeff_data.f32_ac1_vw_slope;	//�ٳ���У׼ϵ��
			adjcoff = g_t_share_data.t_ac_adjust_data.f32_first_path_volt_vw*g_t_share_data.t_coeff_data.f32_ac1_vw_slope/reg;
			os_mut_release(g_mut_share_data);
			if((adjcoff > UP)||(adjcoff < DOWN))
			{
				os_evt_set (AC_ADJUST_FAIL, g_tid_display);
			}
			else
			{ 
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_ac1_vw_slope = adjcoff; 
				os_mut_release(g_mut_share_data);
				v_fetch_save_adjust_coeff();
//				m_pt_att_cal->f32_coeff_2 = adjcoff;
//			    v_fetch_save_ac_adjust_coeff(m_pt_att_cal);
				os_evt_set (AC_ADJUST_SCUESS, g_tid_display);
			}
			break;
			
					
		case AC_ADJUST_FIRST_PATH_WU:
			t = att_att_read(V_C1);
			t = t/(1<<13);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			reg = t*g_t_share_data.t_coeff_data.f32_ac1_wu_slope;	//�ٳ���У׼ϵ��			
			adjcoff = g_t_share_data.t_ac_adjust_data.f32_first_path_volt_wu*g_t_share_data.t_coeff_data.f32_ac1_wu_slope/reg;
			os_mut_release(g_mut_share_data);
			if((adjcoff > UP)||(adjcoff < DOWN))
			{
				os_evt_set (AC_ADJUST_FAIL, g_tid_display);
			}
			else
			{ 
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_ac1_wu_slope = adjcoff; 
				os_mut_release(g_mut_share_data);
				v_fetch_save_adjust_coeff();
//				m_pt_att_cal->f32_coeff_3 = adjcoff;
//			    v_fetch_save_ac_adjust_coeff(m_pt_att_cal);
				os_evt_set (AC_ADJUST_SCUESS, g_tid_display);
			}
			break;
			
					
		case AC_ADJUST_SECOND_PATH_UV:
			t = att_att_read(V_A2);
			t = t/(1<<13);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			reg = t*g_t_share_data.t_coeff_data.f32_ac2_uv_slope;	//�ٳ���У׼ϵ��			
			adjcoff = g_t_share_data.t_ac_adjust_data.f32_second_path_volt_uv*g_t_share_data.t_coeff_data.f32_ac2_uv_slope/reg;
			os_mut_release(g_mut_share_data);
			if((adjcoff > UP)||(adjcoff < DOWN))
			{
				os_evt_set (AC_ADJUST_FAIL, g_tid_display);
			}
			else
			{ 
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_ac2_uv_slope = adjcoff; 
				os_mut_release(g_mut_share_data);
				v_fetch_save_adjust_coeff();
//				m_pt_att_cal->f32_coeff_4 = adjcoff;
//			    v_fetch_save_ac_adjust_coeff(m_pt_att_cal);
				os_evt_set (AC_ADJUST_SCUESS, g_tid_display);
			}
			break;
			
					
		case AC_ADJUST_SECOND_PATH_VW:
			t = att_att_read(V_B2);
			t = t/(1<<13);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			reg = t*g_t_share_data.t_coeff_data.f32_ac2_vw_slope;	//�ٳ���У׼ϵ��			
			adjcoff = g_t_share_data.t_ac_adjust_data.f32_second_path_volt_vw*g_t_share_data.t_coeff_data.f32_ac2_vw_slope/reg;
			os_mut_release(g_mut_share_data);
			if((adjcoff > UP)||(adjcoff < DOWN))
			{
				os_evt_set (AC_ADJUST_FAIL, g_tid_display);
			}
			else
			{ 
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_ac2_vw_slope = adjcoff; 
				os_mut_release(g_mut_share_data);
				v_fetch_save_adjust_coeff();
//				m_pt_att_cal->f32_coeff_5 = adjcoff;
//			    v_fetch_save_ac_adjust_coeff(m_pt_att_cal);
				os_evt_set (AC_ADJUST_SCUESS, g_tid_display);
			}
			break;
			
			
					
		case AC_ADJUST_SECOND_PATH_WU:
			t = att_att_read(V_C2);
			t = t/(1<<13);
			os_mut_wait(g_mut_share_data, 0xFFFF);
			reg = t*g_t_share_data.t_coeff_data.f32_ac2_wu_slope;	//�ٳ���У׼ϵ��		
			adjcoff = g_t_share_data.t_ac_adjust_data.f32_second_path_volt_wu*g_t_share_data.t_coeff_data.f32_ac2_wu_slope/reg;
			os_mut_release(g_mut_share_data);
			if((adjcoff > UP)||(adjcoff < DOWN))
			{
				os_evt_set (AC_ADJUST_FAIL, g_tid_display);
			}
			else
			{ 
				os_mut_wait(g_mut_share_data, 0xFFFF);
				g_t_share_data.t_coeff_data.f32_ac2_wu_slope = adjcoff; 
				os_mut_release(g_mut_share_data);
				v_fetch_save_adjust_coeff();
//				m_pt_att_cal->f32_coeff_6 = adjcoff;
//			    v_fetch_save_ac_adjust_coeff(m_pt_att_cal);
				os_evt_set (AC_ADJUST_SCUESS, g_tid_display);
			}
			break;
					
		default: 
	    	break;
  	}  
}

/********************************************
 *  �������ƣ�void  v_att7_attreadac1(void)
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   ��ȡ��һ·��������
*********************************************/
void  v_att7_attreadac1(void)
{	
	F32_T reg;
	U32_T t;
	t = att_att_read(V_A1);
	t = t/(1<<13);
	os_mut_wait(g_mut_share_data, 0xFFFF);
	reg = t*g_t_share_data.t_coeff_data.f32_ac1_uv_slope;	//�ٳ���У׼ϵ��
	g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv = reg;
	os_mut_release(g_mut_share_data);

	
	t = att_att_read(V_B1);
	t = t/(1<<13);	
	os_mut_wait(g_mut_share_data, 0xFFFF);
	reg = t*g_t_share_data.t_coeff_data.f32_ac1_vw_slope;	//�ٳ���У׼ϵ��
	g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw = reg;
	os_mut_release(g_mut_share_data);

	
	t = att_att_read(V_C1);
	t = t/(1<<13);
	os_mut_wait(g_mut_share_data, 0xFFFF);
	reg = t*g_t_share_data.t_coeff_data.f32_ac1_wu_slope;	//�ٳ���У׼ϵ��
	g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu = reg;
	os_mut_release(g_mut_share_data);

}
/********************************************
 *  �������ƣ�void  v_att7_attreadac2(void)
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   ��ȡ�ڶ�·��������
*********************************************/
void  v_att7_attreadac2(void)
{	
	F32_T reg;
	U32_T t;
	t = att_att_read(V_A2);
	t = t/(1<<13);
	os_mut_wait(g_mut_share_data, 0xFFFF);
	reg = t*g_t_share_data.t_coeff_data.f32_ac2_uv_slope;	//�ٳ���У׼ϵ��
	g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv = reg;
	os_mut_release(g_mut_share_data);


	t = att_att_read(V_B2);
	t = t/(1<<13);
	os_mut_wait(g_mut_share_data, 0xFFFF);
	reg = t*g_t_share_data.t_coeff_data.f32_ac2_vw_slope;	//�ٳ���У׼ϵ��
	g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw = reg;
	os_mut_release(g_mut_share_data);

	
	t = att_att_read(V_C2);
	t = t/(1<<13);
	os_mut_wait(g_mut_share_data, 0xFFFF);
	reg = t*g_t_share_data.t_coeff_data.f32_ac2_wu_slope;	//�ٳ���У׼ϵ��
	g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu = reg;
	os_mut_release(g_mut_share_data);
		  
}
/********************************************
 *  �������ƣ�__task void v_att_atttask(void)
 *  �������:   ��
 *  �������:   ��
 *  ���ؽ��:   ��
 *  ȫ�ֱ���:	��
 *  ���ܽ���:   Att����ʵ��
*********************************************/
__task void v_att_atttask()
{	
	U32_T flag,cnterr[8] = {0};
	U16_T i,old_state,statenum =0;
	AC_CFG_T ac_cfg;
	AC_RT_DATA_T ac_rt_data_t;
	AC_INPUT_NUM_E AC;
	AC_INPUT_TYPE_E  ACT;
	RLY_CMD_E   s1,s2,s3,s4;
	U32_T t1,t2,time;
	U32_T	flag1,flag2;
	U16_T   ac_ctl;
	
	U32_T   u32_ac_ov_start1;			//������ѹ��ʱ�ж�
	U32_T   u32_ac_ov_start2;			//������ѹ��ʱ�ж�	
	U8_T    u8_ac_ov_delay;
	
//	m_pt_att_cal = &m_att_cal_new;
	v_att_spi_init(200);
	v_att_att_init();
	old_state = 0;
	t1 = 0;
	t2 = 0;
	time = 0;
	flag2 = 0;
	flag1 = 0;
	s1 = AC1ON();
	s2 = AC2ON();
	v_relay_relay_operation(s1); 
	v_relay_relay_operation(s2);
	u32_ac_ov_start1 = u32_delay_get_timer_val();
	u32_ac_ov_start2 = u32_delay_get_timer_val();
	
/*
	if(s32_fetch_read_ac_adjust_coeff(m_pt_att_cal) != 0)//ϵ������
	{  
		m_pt_att_cal = &m_att_default_cal;
		v_fetch_save_ac_adjust_coeff(m_pt_att_cal);	
	}  
	else
	{
		
		s32_fetch_read_ac_adjust_coeff(m_pt_att_cal);
		
	}
*/
	
	while(1) 
	{
		os_evt_set(AC_SAMPLE_FEED_DOG, g_tid_wdt);             //����ι���¼���־
		os_mut_wait(g_mut_share_data, 0xFFFF);
		u8_ac_ov_delay = g_t_share_data.t_sys_cfg.t_sys_param.u8_ats_offline_cnt;
		os_mut_release(g_mut_share_data);
		
	    if (os_evt_wait_or(0x003F, 0) == OS_R_EVT) 
		{ 
			flag = os_evt_get();
		
			if(flag!=0)		//=0,��ʾ��ִ��У׼��=1��ʾִ��У׼
			{
				v_att_att_jz(flag);
					  
			}
		}
		else
		{	os_mut_wait(g_mut_share_data, 0xFFFF);
		    AC = g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path;
			ACT =g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase;
			os_mut_release(g_mut_share_data);
			switch(AC)
			{ 
				case TWO_PATH: 
					v_att7_attreadac1();	 //��ȡ��һ·������ѹֵ������䵽ʵʱ������ʾ��
					v_att7_attreadac2();	 //��ȡ�ڶ�·������ѹֵ������䵽ʵʱ������ʾ��

					os_mut_wait(g_mut_share_data, 0xFFFF);	 //׼���ڹ���������ȡ����
					memcpy(&ac_cfg,&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac),sizeof(ac_cfg));	//����������copy����ǰ������ʹ��
					memcpy(&ac_rt_data_t,&(g_t_share_data.t_rt_data.t_dc_panel.t_ac),sizeof(ac_rt_data_t));	//��ʵʱ��ʾ��������䵽��ǰ�����ݽṹ�У�����ʹ��
					os_mut_release(g_mut_share_data);
					old_state = ac_rt_data_t.u16_state;
					if(ACT==AC_3_PHASE)
					{
						if((ac_rt_data_t.f32_first_path_volt_uv <= 30.0) 
						|| (ac_rt_data_t.f32_first_path_volt_vw <= 30.0)
						|| (ac_rt_data_t.f32_first_path_volt_wu <= 30.0))
				 		{
							//ac_rt_data_t.u16_state |=0x01;
							cnterr[0]++;
							goto ac2;
						}
						else
						{
							 cnterr[0] = 0;
						}
					}
					else
					{
					    if(ac_rt_data_t.f32_first_path_volt_uv <= 30.0) 
				 		{
							//ac_rt_data_t.u16_state |=0x01;
							cnterr[0]++;
							goto ac2;
						}
						else
						{
							 cnterr[0] = 0;
						}	
					}
					if(ACT==AC_3_PHASE)
					{
						if(((ac_rt_data_t.f32_first_path_volt_uv >30.0 ) && (ac_rt_data_t.f32_first_path_volt_uv < ac_cfg.u16_lack_phase)) 
						|| ((ac_rt_data_t.f32_first_path_volt_vw >30.0 ) && (ac_rt_data_t.f32_first_path_volt_vw < ac_cfg.u16_lack_phase))
						|| ((ac_rt_data_t.f32_first_path_volt_wu >30.0 ) && (ac_rt_data_t.f32_first_path_volt_wu < ac_cfg.u16_lack_phase)))
						
						{
							//ac_rt_data_t.u16_state |=0x08;
							cnterr[3] ++;
							goto ac2;
						}
						else
						{
							cnterr[3] = 0;
						}
				     }
					 if(ACT==AC_3_PHASE)
					 {
						 if((ac_rt_data_t.f32_first_path_volt_uv > ac_cfg.u16_high_volt) 
						 || (ac_rt_data_t.f32_first_path_volt_vw > ac_cfg.u16_high_volt)
						 || (ac_rt_data_t.f32_first_path_volt_wu > ac_cfg.u16_high_volt))
						{
							//ac_rt_data_t.u16_state |=0x04;
							cnterr[2] ++;
							goto ac2;
						}
						else
						{
							cnterr[2] =0;
						}
					}
					else
					{
						if(ac_rt_data_t.f32_first_path_volt_uv > ac_cfg.u16_high_volt) 
						{
							//ac_rt_data_t.u16_state |=0x04;
							cnterr[2] ++;
							goto ac2;
						}
						else
						{
							cnterr[2] =0;
						}
					}
				  if(ACT==AC_3_PHASE)
				  {	
					  if((ac_rt_data_t.f32_first_path_volt_uv < ac_cfg.u16_low_volt) 
					  || (ac_rt_data_t.f32_first_path_volt_vw < ac_cfg.u16_low_volt)
					  || (ac_rt_data_t.f32_first_path_volt_wu < ac_cfg.u16_low_volt)) 	
						{
							//ac_rt_data_t.u16_state |=0x02;
							cnterr[1] ++;
						}
					   else 
					   {
					   		cnterr[1] =0;
					   }
				  }
				  else
				  {
				  		if(ac_rt_data_t.f32_first_path_volt_uv < ac_cfg.u16_low_volt) 	
						{
							//ac_rt_data_t.u16_state |=0x02;
							cnterr[1] ++;
						}
					   else 
					   {
					   		cnterr[1] =0;
					   }
				  }

				  
				ac2:
					if(ACT==AC_3_PHASE)
					{
						if((ac_rt_data_t.f32_second_path_volt_uv <= 30.0) 
						|| (ac_rt_data_t.f32_second_path_volt_vw <= 30.0)						
						|| (ac_rt_data_t.f32_second_path_volt_wu <= 30.0))
						{
							//ac_rt_data_t.u16_state |=0x10;
							cnterr[4] ++;
							goto ac21;
						}
						else
						{
							cnterr[4] = 0;
						}
					}
					else
					{
						if(ac_rt_data_t.f32_second_path_volt_uv <= 30.0) 
						{
							//ac_rt_data_t.u16_state |=0x10;
							cnterr[4] ++;
							goto ac21;
						}
						else
						{
							cnterr[4] = 0;
						}
					}
					if(ACT==AC_3_PHASE)
					{
						if(((ac_rt_data_t.f32_second_path_volt_uv >30.0 ) && (ac_rt_data_t.f32_second_path_volt_uv < ac_cfg.u16_lack_phase)) 
						|| ((ac_rt_data_t.f32_second_path_volt_vw >30.0 ) && (ac_rt_data_t.f32_second_path_volt_vw < ac_cfg.u16_lack_phase))
						|| ((ac_rt_data_t.f32_second_path_volt_wu >30.0 ) && (ac_rt_data_t.f32_second_path_volt_wu < ac_cfg.u16_lack_phase)))						
						{
							//ac_rt_data_t.u16_state |=0x80;
							cnterr[7] ++;
							goto ac21;
						}
						else
						{
							cnterr[7] = 0;
						}
					 }
					if(ACT==AC_3_PHASE)
					{
					    if((ac_rt_data_t.f32_second_path_volt_uv > ac_cfg.u16_high_volt) 
						|| (ac_rt_data_t.f32_second_path_volt_vw > ac_cfg.u16_high_volt)
						|| (ac_rt_data_t.f32_second_path_volt_wu > ac_cfg.u16_high_volt))
						{
							//ac_rt_data_t.u16_state |=0x40;
							cnterr[6] ++;
							goto ac21;
						}
						else
						{
							cnterr[6] = 0;
						}
					}
					else
					{
						if(ac_rt_data_t.f32_second_path_volt_uv > ac_cfg.u16_high_volt) 
						{
							//ac_rt_data_t.u16_state |=0x40;
							cnterr[6] ++;
							goto ac21;
						}
						else
						{
							cnterr[6] = 0;
						}
					}
				  	if(ACT==AC_3_PHASE)
					{
						if((ac_rt_data_t.f32_second_path_volt_uv < ac_cfg.u16_low_volt) 
						|| (ac_rt_data_t.f32_second_path_volt_vw < ac_cfg.u16_low_volt)
						|| (ac_rt_data_t.f32_second_path_volt_wu < ac_cfg.u16_low_volt)) 	
						{
							//ac_rt_data_t.u16_state |=0x20;
							cnterr[5] ++;
	
						}
						else
						{
							 cnterr[5] =0;
						}
					}
					else
					{
						if(ac_rt_data_t.f32_second_path_volt_uv < ac_cfg.u16_low_volt)  	
						{
							//ac_rt_data_t.u16_state |=0x20;
							cnterr[5] ++;
	
						}
						else
						{
							 cnterr[5] =0;
						}
					}
					break;
	
				case NONE_PATH: 
					break; 
	
				case ONE_PATH: 	
				default:    
					v_att7_attreadac1();

					os_mut_wait(g_mut_share_data, 0xFFFF);	 //׼���ڹ���������ȡ����
					memcpy(&ac_cfg,&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac),sizeof(ac_cfg));	//����������copy����ǰ������ʹ��
					memcpy(&ac_rt_data_t,&(g_t_share_data.t_rt_data.t_dc_panel.t_ac),sizeof(ac_rt_data_t));	//��ʵʱ��ʾ��������䵽��ǰ�����ݽṹ�У�����ʹ��
					os_mut_release(g_mut_share_data);
					old_state = ac_rt_data_t.u16_state;

					if(ACT==AC_3_PHASE)
					{
						if((ac_rt_data_t.f32_first_path_volt_uv <= 30.0) 
						|| (ac_rt_data_t.f32_first_path_volt_vw <= 30.0)
						|| (ac_rt_data_t.f32_first_path_volt_wu <= 30.0))
				 		{
							//ac_rt_data_t.u16_state |=0x01;
							cnterr[0] ++;
							goto ac21;
						}
						else
						{
							cnterr[0] = 0;
						}
					}
					else
					{
						if(ac_rt_data_t.f32_first_path_volt_uv <= 30.0)
				 		{
							//ac_rt_data_t.u16_state |=0x01;
							cnterr[0] ++;
							goto ac21;
						}
						else
						{
							cnterr[0] = 0;
						}
					}
					if(ACT==AC_3_PHASE)
					{
						if(((ac_rt_data_t.f32_first_path_volt_uv >30.0 ) && (ac_rt_data_t.f32_first_path_volt_uv < ac_cfg.u16_lack_phase)) 
						|| ((ac_rt_data_t.f32_first_path_volt_vw >30.0 ) && (ac_rt_data_t.f32_first_path_volt_vw < ac_cfg.u16_lack_phase))
						|| ((ac_rt_data_t.f32_first_path_volt_wu >30.0 ) && (ac_rt_data_t.f32_first_path_volt_wu < ac_cfg.u16_lack_phase)))
						
						{
							//ac_rt_data_t.u16_state |=0x08;
							cnterr[3] ++;
							goto ac21;
	
						}
						 else
						 {
						 	cnterr[3] = 0;
						 }
				     }
					 if(ACT==AC_3_PHASE)
					 {
						 if((ac_rt_data_t.f32_first_path_volt_uv > ac_cfg.u16_high_volt) 
						 || (ac_rt_data_t.f32_first_path_volt_vw > ac_cfg.u16_high_volt)																						 
                         || (ac_rt_data_t.f32_first_path_volt_wu > ac_cfg.u16_high_volt))
						{
						//	ac_rt_data_t.u16_state |=0x04;
							cnterr[2] ++;
							goto ac21;
						}
						else
						{
							 cnterr[2] = 0;
						}
					}
					else
					{
						if(ac_rt_data_t.f32_first_path_volt_uv > ac_cfg.u16_high_volt) 
						{
						//	ac_rt_data_t.u16_state |=0x04;
							cnterr[2] ++;
							goto ac21;
						}
						else
						{
							 cnterr[2] = 0;
						}
					}
				  	 if(ACT==AC_3_PHASE)
					 {
						if((ac_rt_data_t.f32_first_path_volt_uv < ac_cfg.u16_low_volt) 
						 || (ac_rt_data_t.f32_first_path_volt_vw < ac_cfg.u16_low_volt)
						 || (ac_rt_data_t.f32_first_path_volt_wu < ac_cfg.u16_low_volt)) 	
						{
							//ac_rt_data_t.u16_state |=0x02;
							cnterr[1] ++;
						}
						else
						{
							cnterr[1] = 0;
						}
					 }
					 else
					 {
					 	if(ac_rt_data_t.f32_first_path_volt_uv < ac_cfg.u16_low_volt)  	
						{
							//ac_rt_data_t.u16_state |=0x02;
							cnterr[1] ++;
						}
						else
						{
							cnterr[1] = 0;
						}
					 }
					
					break;

			}	
			 	 
			
		}
	 ac21:
		
	 for(i=0;i<8;i++)
	 { 	

		if(i== 2)
		{
			if(cnterr[i]<=2)
			{
				u32_ac_ov_start1 = u32_delay_get_timer_val();
			}
			else
			{
				cnterr[i] = 6;
				if(!(ac_rt_data_t.u16_state&(1<<i)))  //û�й�ѹ
				{
					if(u32_delay_time_elapse(u32_ac_ov_start1,u32_delay_get_timer_val()) > u8_ac_ov_delay*1000000)  //��ѹʱ�䵽
					{
						ac_rt_data_t.u16_state |=(1<<i);   //��λ��ѹ
					}
				}
			}
		}
		else if( i == 6)
		{
			if(cnterr[i]<=2)
			{
				u32_ac_ov_start2 = u32_delay_get_timer_val();
			}
			else
			{
				cnterr[i] = 6;
				if(!(ac_rt_data_t.u16_state&(1<<i)))  //û�й�ѹ
				{
					if(u32_delay_time_elapse(u32_ac_ov_start2,u32_delay_get_timer_val()) > u8_ac_ov_delay*1000000)  //��ѹʱ�䵽
					{
						ac_rt_data_t.u16_state |=(1<<i);   //��λ��ѹ
					}
				}
			}
		}
		else
		{		 
			if(cnterr[i]>2)
			{
				ac_rt_data_t.u16_state |=(1<<i);
				cnterr[i] = 6;
			}
			else
			{
				ac_rt_data_t.u16_state &=(~(1<<i));
			}
		}
		
		if(ac_rt_data_t.u16_state&0x01)
		{
			ac_rt_data_t.u16_state &=0xf1;	
			u32_ac_ov_start1 = u32_delay_get_timer_val();			
		}
		else if((ac_rt_data_t.u16_state&0x08)&&(ACT==AC_3_PHASE))
		{
		 	ac_rt_data_t.u16_state &=0xf8;
			u32_ac_ov_start1 = u32_delay_get_timer_val();
		}
		else if(ac_rt_data_t.u16_state&0x04)
		{
			ac_rt_data_t.u16_state &=0xf4;
		}
		else if(ac_rt_data_t.u16_state&0x02)
		{
			ac_rt_data_t.u16_state &=0xf2;
			u32_ac_ov_start1 = u32_delay_get_timer_val();
		}


		if(ac_rt_data_t.u16_state&0x10)
		{
			ac_rt_data_t.u16_state &=0x1f;	 
			u32_ac_ov_start2 = u32_delay_get_timer_val();
		}
		else if((ac_rt_data_t.u16_state&0x80)&&(ACT==AC_3_PHASE))
		{
		 	ac_rt_data_t.u16_state &=0x8f;
			u32_ac_ov_start2 = u32_delay_get_timer_val();
		}
		else if(ac_rt_data_t.u16_state&0x40)
		{
			ac_rt_data_t.u16_state &=0x4f;
		}
		else if(ac_rt_data_t.u16_state&0x20)
		{
			ac_rt_data_t.u16_state &=0x2f;
			u32_ac_ov_start2 = u32_delay_get_timer_val();
		}	

	 }
	 os_mut_wait(g_mut_share_data, 0xFFFF);	 
	 g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state = ac_rt_data_t.u16_state;
	 os_mut_release(g_mut_share_data);

	 for(i=0;i<8;i++)
	 {
	    if(old_state & (1<<i))
		{
			if( !(ac_rt_data_t.u16_state&(1<<i)) )
			{
				v_fauid_send_fault_id_resume(i);
			}
	 	}	
	   	else
	   	{
	   		if( ac_rt_data_t.u16_state&(1<<i) )
			{
			   	v_fauid_send_fault_id_occur(i);
			}			
	   	}	
	 }
	 os_mut_wait(g_mut_share_data, 0xFFFF);
	 ac_ctl = g_t_share_data.t_sys_cfg.t_ctl.u16_ac;
	 os_mut_release(g_mut_share_data);

	 ac_ctl = ac_ctl & 0x3;
	 s3=s1;
	 s4=s2;

//ac_ctl = 0��1,2,3�ֱ�����һ·���ȣ��̶���һ·���ڶ�·���ȣ��̶��ڶ�·
	statenum++;				//��ѭ��ִ�д������˴������߼��ӳ٣�����������ȷ��֮������Ͷ��
	if(statenum > 4)	    //��ʾ�����ȶ���
	{  
		 
		 statenum = 4;
		 switch(AC)
		 {	 
		 
		    case ONE_PATH:
				s2 = AC2OFF();  
				if(ac_ctl==1)
				{
					if(s1 != AC1ON())
					{
						s1 = AC1ON();
					}
					flag1 = 0;
					flag2 = 0;												
				}
				else if(ac_ctl == 0)
				{
					if(ac_rt_data_t.u16_state&0x0F)
					{
						s1 = AC1OFF();
						flag1 = 0;
						flag2 = 0;								
					}
					else
					{	
						if(s1 == AC1ON())
						{
							//û���ϱպ��˾Ͳ����ٱպ�
						}
						else//��һ·���ȣ�û�պϾ���ʱ���ٱպ�
						{	
							if(flag1 ==0)
							{
								t1 = u32_delay_get_timer_val();
								flag1 = 1;
							}
							if(flag2 ==0)
							{
								t2 = u32_delay_get_timer_val();
								time = u32_delay_time_elapse(t1,t2);
								if(time/1000000>=10)
								{
									s1 = AC1ON(); 
									flag2 = 1;
								}
							}			
						}
					}
				}
				break;
						
			case TWO_PATH:
				if(ac_ctl==1)
				{   
					s1 = AC1ON();
					s2 = AC2OFF();
					flag1 = 0;
					flag2 = 0;
				}
				else if(ac_ctl ==3)
					{
						s1 = AC1OFF();
						s2 = AC2ON();
						flag1 = 0;
						flag2 = 0;
					}
					else if(ac_ctl ==0)
					{
						if((ac_rt_data_t.u16_state & 0x0F)==0)
						{	
							s2 = AC2OFF();
							if(s1==	AC1ON())
							{
								flag1=0;
								flag2=0;
							}
							else
							{
								if(flag1 ==0)
								{
							   	 	t1 = u32_delay_get_timer_val();
									flag1 = 1;
								}
							    if(flag2 ==0)
								{
									t2 = u32_delay_get_timer_val();
									time = u32_delay_time_elapse(t1,t2);
				
									if(time/1000000>=10)
									{
										s1 = AC1ON(); 
										
										flag2 = 1;
									}
								}
							}						 
	
						}
						else if((ac_rt_data_t.u16_state & 0xF0)==0)
						{
							s1 = AC1OFF();
							if(s2 == AC2ON())
							{
								flag1 = 0;
								flag2 = 0;
							}
							else
							{
								if(flag1 ==0)
								{
								  	t1 = u32_delay_get_timer_val();
									flag1 = 1;
								}
								if(flag2 ==0)
								{
	
									t2 = u32_delay_get_timer_val();
									time = u32_delay_time_elapse(t1,t2);
									if(time/1000000>=10)
									{
									 	s2 = AC2ON();
										flag2 = 1;	
									}
								}
							}
						}
						else
						{
							s1 = AC1OFF();
							s2 = AC2OFF();
							flag1 = 0;
							flag2 = 0;
						}
	
					}
					else 
					{
						if((ac_rt_data_t.u16_state&0xF0)==0)
						{
							s1 = AC1OFF();
							if(s2 == AC2ON())
							{							
								flag1 = 0;
								flag2 = 0;
							}
							else
							{
								
								if(flag1 ==0)
								{
								    t1 = u32_delay_get_timer_val();
									flag1 = 1;
								}
								if(flag2 ==0)
								{
									t2 = u32_delay_get_timer_val();
									time = u32_delay_time_elapse(t1,t2);
			
									if(time/1000000>=10)
									{
									 	s2 = AC2ON();
										flag2 = 1;
									}
								}
							}
							 
						}
						else if((ac_rt_data_t.u16_state&0x0F)==0)
						{
							s2 = AC2OFF();
							if(s1 ==AC1ON())
							{
								flag1 =0;
								flag2 =0;
							}
							else
							{
								if(flag1 ==0)
								{
									t1 = u32_delay_get_timer_val();
									flag1 = 1;
								}
								if(flag2 ==0)
								{
									t2 = u32_delay_get_timer_val();
									time = u32_delay_time_elapse(t1,t2);
			
									if(time/1000000>=10)
									{
										s1 = AC1ON(); 
										flag2 = 1;	
									}
								}
							}
						}
						else
						{
							  s1 = AC1OFF();
			    			  s2 = AC2OFF();
							  flag1 = 0;
							  flag2 = 0;
						}
	
					}
					break;
	
				default:
						s1 = AC1OFF();
					    s2 = AC2OFF();
						flag1 = 0;
						flag2 = 0;
						break;
					
		 }

	 }



	if(s1!=s3)
	{
		v_relay_relay_operation(s1);
	}
	if(s2!=s4)
	{
		v_relay_relay_operation(s2);
	}
	 
	 
	 os_dly_wait(50);	  
		  
	}
}




