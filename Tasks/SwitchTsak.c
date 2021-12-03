/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：switch.h
版    本：1.00
创建日期：2012-05-29
作    者：肖江
功能描述：开关量任务

函数列表：

修改记录：
	作者      日期        版本     修改内容
	肖江    2012-05-29    1.00     创建
**************************************************************/
#include "../Drivers/Switch.h"
#include "PublicData.h"
#include "ShareDataStruct.h"
#include "FaultId.h"
#include "string.h"


/********************************************
 *  函数名称：__task v_switch_switchtask(void)
 *  输出参数:   无
 *  返回结果:   无
 *  功能介绍: 对当前采样的数据进行分析
*********************************************/
__task void v_switch_switchtask()
{  
	U32_T data, reg, i;
	U8_T u8_alm_new[SWT_BRANCH_MAX];                         //开关告警量，1：告警，0：不告警
	U8_T u8_alm_old[SWT_BRANCH_MAX];                         //开关告警量，1：告警，0：不告警
	U8_T u8_swt_cnt, u8_state_cnt;
	SWT_CFG_T *pt_swt_cfg = &(g_t_share_data.t_sys_cfg.t_dc_panel.t_swt);

	if (g_u8_product_type == PRODUCT_TYPE_SC12)
		u8_swt_cnt = 4;
	else
		u8_swt_cnt = SWT_BRANCH_MAX;
		
	os_mut_wait(g_mut_share_data, 0xFFFF);
	if (pt_swt_cfg->u8_state_cnt > u8_swt_cnt)
		pt_swt_cfg->u8_state_cnt = u8_swt_cnt;
	u8_state_cnt = pt_swt_cfg->u8_state_cnt;
	os_mut_release(g_mut_share_data);

	while(1)
	{
		os_evt_set(SWT_SAMPLE_FEED_DOG, g_tid_wdt);             //设置喂狗事件标志

		data=switch_switch_data();
		os_dly_wait(2);		//开关延迟多长时间检测，在此设置
		reg=switch_switch_data();
		reg=reg&data;	  //传递信息

		os_mut_wait(g_mut_share_data, 0xFFFF);
		
		//根据接入类型进行处理
		for (i=0; i<u8_swt_cnt; i++)
		{
			if ((reg & (1<<i)) != 0)
				g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[i] = 1;
			else
				g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[i] = 0;

			if (pt_swt_cfg->t_swt_item[i].u8_join_way == 0)   //常开
			{
				if ((reg & (1<<i)) != 0)
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 1;
				else
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 0;
			}
			else                                              //常闭
			{
				if ((reg & (1<<i)) != 0)
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 0;
				else
					g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i] = 1;
			}
			
		}
		
		//处理状态量
		for (i=0; i<u8_state_cnt; i++)
		{
			g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_state[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i];
		}
		
		//处理告警量	
		for (i=0; i<u8_swt_cnt-u8_state_cnt; i++)
		{
			u8_alm_old[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_alm[i];
			g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_alm[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_swt_state[i+u8_state_cnt];
			u8_alm_new[i] = g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_alm[i];
		}
		
		os_mut_release(g_mut_share_data);									
				
		//告警量发送告警ID
		for (i=0; i<u8_swt_cnt-u8_state_cnt; i++)
		{
			if ((u8_alm_old[i] == 0) && (u8_alm_new[i] != 0))
				v_fauid_send_fault_id_occur((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+i);
			else if ((u8_alm_old[i] != 0) && (u8_alm_new[i] == 0))
				v_fauid_send_fault_id_resume((FAULT_SWT_GROUP<<FAULT_GROUP_OFFSET)+i);
		}
		
		os_dly_wait(10);		 
	}    		 
}


