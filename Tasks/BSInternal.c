/************************************************************
Copyright (C), ����Ӣ����Ƽ��������޹�˾, ����һ��Ȩ��
�� �� ����BSInteranl.c
��    ����1.00
�������ڣ�2013-03-01
��    �ߣ�������
�����������ڲ�MODBUS��Լʵ���ļ�

�����б�

�޸ļ�¼��
	����      ����        �汾     �޸�����
	������    2013-03-01  1.00     ����
***********************************************************/

#include <string.h>
#include <math.h>

#include "Type.h"
#include "PublicData.h"
#include "BSInternal.h"
#include "Backstage.h"
#include "Crc.h"
#include "Fault.h"
#include "FetchFlash.h"

#include "../Drivers/uart_device.h"
#include "../Drivers/Relay.h"
#include "../Drivers/Delay.h"


//#define DEBUG

#ifdef DEBUG
	#define DPRINT(fmt, args...) u32_usb_debug_print(fmt, ##__VA_ARGS__)
#else
	#define DPRINT(fmt, args...)
#endif


#define MODBUS_BYTE_SIZE           8
#define MODBUS_MAXREG_NUM          125    /* һ�����ɶ�ȡ�ļĴ������� */

#define BS_DC10_YC_DATA_CNT       20
#define BS_DC10_YC_AD_CNT         17
#define BS_DC10_YX_CNT            70
#define BS_DC10_YT_SET_CNT        43
#define BS_DC10_YT_ADJUST_CNT     33
#define BS_DC10_YC_DATA_OFFSET    0x0100
#define BS_DC10_YC_AD_OFFSET      0x0400
#define BS_DC10_YX_OFFSET         0x0500
#define BS_DC10_YT_SET_OFFSET     0x1000
#define BS_DC10_YT_ADJUST_OFFSET  0x1100
#define BS_DC10_ADDR_MASK         0xFF00
#define BS_DC10_INDEX_MASK        0x00FF

#define BS_DC10_AC_INPUT_MODE_INDEX 0x00
#define BS_DC10_AC_CTRL_MODE_INDEX  0x01
#define BS_DC10_V1_HIGH_VOLT_INDEX  0x05
#define BS_DC10_V1_LOW_VOLT_INDEX   0x06
#define BS_DC10_A2_OVER_CURR_INDEX  0x0C
#define BS_DC10_A3_OVER_CURR_INDEX  0x0D
#define BS_DC10_A1_SHUNT_VOLT_INDEX 0x0E
#define BS_DC10_A2_SHUNT_VOLT_INDEX 0x10
#define BS_DC10_A3_SHUNT_VOLT_INDEX 0x12
#define BS_DC10_DIODE_CHAIN_INDEX   0x21
#define BS_DC10_RELAY_START_INDEX   0x22


//ң����װ��Ŀ
typedef struct
{
	void   *pv_val;        //ָ��ң������ָ��
	U16_T  u16_type;       //pv_valָ�����������
	U16_T  u16_coeff;      //���
}DC10_YC_ASSEMBLE_T;

//ң����װ��Ŀ
typedef struct
{
	void   *pv_val;        //ָ��ң������ָ��
	U16_T  u16_type;        //pv_valָ�����������
	U16_T  u16_mask;       //������
}DC10_YX_ASSEMBLE_T;



static DC10_YC_ASSEMBLE_T m_t_yc_data_assemble[BS_DC10_YC_DATA_CNT] =
{
	//����1·UV��ѹ���Ĵ�����ַ0x100
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_uv),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//����1·VW��ѹ���Ĵ�����ַ0x101
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_vw),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//����1·WU��ѹ���Ĵ�����ַ0x102
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_first_path_volt_wu),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//����2·UV��ѹ���Ĵ�����ַ0x103
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_uv),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//����2·VW��ѹ���Ĵ�����ַ0x104
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_vw),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//����2·WU��ѹ���Ĵ�����ַ0x105
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.f32_second_path_volt_wu),  //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//�����¶ȣ��Ĵ�����ַ0x106
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_temperature),         //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V1��ѹֵ����ĸ��ѹ���Ĵ�����ַ0x107
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_cb_volt),             //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A1���������������ص������Ĵ�����ַ0x108
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_load_curr),           //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V2��ѹֵ����ص�ѹ���Ĵ�����ַ0x109
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_volt),           //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A2������������һ���ص������Ĵ�����ַ0x10A
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[0]),        //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V3��ѹֵ����ĸ��ѹ���Ĵ�����ַ0x10B
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_pb_volt),             //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A3�����������������ص������Ĵ�����ַ0x10C
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_batt_curr[1]),        //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//��������·S1����������޴����ݣ��Ĵ�����ַ0x10D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//��������·S2����������޴����ݣ��Ĵ�����ַ0x10E
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//��������·S3����������޴����ݣ��Ĵ�����ַ0x10F
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},

	//�����״̬(0:���䣬1:���䣬2:����)���Ĵ�����ַ0x110
	{
		&(g_t_share_data.t_rt_data.t_batt.e_state[0]),                         //pv_val
		BS_TYPE_U8,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//�����1�������Ĵ�����ַ0x111
	{
		&(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].f32_capacity),    //pv_val
		BS_TYPE_F32,                                                        //u16_type
		10,                                                                 //u16_coeff
	},

	//�����2�������Ĵ�����ַ0x112
	{
		&(g_t_share_data.t_rt_data.t_batt.t_batt_group[1].f32_capacity),    //pv_val
		BS_TYPE_F32,                                                        //u16_type
		10,                                                                 //u16_coeff
	},

	//ĸ�߸��Եص�ѹ���Ĵ�����ַ0x113
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.f32_dc_bus_neg_to_gnd_volt),    //pv_val
		BS_TYPE_F32,                                                        //u16_type
		10,                                                                 //u16_coeff
	},
};

static DC10_YC_ASSEMBLE_T m_t_yc_ad_assemble[BS_DC10_YC_DATA_CNT] =
{
	//����1·UV����ADֵ���Ĵ�����ַ0x400
	{
		&(g_t_share_data.t_ad_data.u16_ac1_uv_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//����1·VW����ADֵ���Ĵ�����ַ0x401
	{
		&(g_t_share_data.t_ad_data.u16_ac1_vw_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//����1·WU����ADֵ���Ĵ�����ַ0x402
	{
		&(g_t_share_data.t_ad_data.u16_ac1_wu_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//����2·UV����ADֵ���Ĵ�����ַ0x403
	{
		&(g_t_share_data.t_ad_data.u16_ac2_uv_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//����2·VW����ADֵ���Ĵ�����ַ0x404
	{
		&(g_t_share_data.t_ad_data.u16_ac2_vw_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                  //u16_coeff
	},

	//����2·WU����ADֵ���Ĵ�����ַ0x405
	{
		&(g_t_share_data.t_ad_data.u16_ac2_wu_ad),                           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//�����¶Ȳ���ADֵ���Ĵ�����ַ0x406
	{
		&(g_t_share_data.t_ad_data.u16_temp_ad),                             //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V1����ADֵ���Ĵ�����ַ0x407
	{
		&(g_t_share_data.t_ad_data.u16_v1_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A1����ADֵ���Ĵ�����ַ0x408
	{
		&(g_t_share_data.t_ad_data.u16_a1_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V2����ADֵ���Ĵ�����ַ0x409
	{
		&(g_t_share_data.t_ad_data.u16_v2_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A2����ADֵ���Ĵ�����ַ0x40A
	{
		&(g_t_share_data.t_ad_data.u16_a2_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V3����ADֵ���Ĵ�����ַ0x40B
	{
		&(g_t_share_data.t_ad_data.u16_v3_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A3����ADֵ���Ĵ�����ַ0x40C
	{
		&(g_t_share_data.t_ad_data.u16_a3_ad),                               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S1����ADֵ������޴����ݣ��Ĵ�����ַ0x40D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S2����ADֵ������޴����ݣ��Ĵ�����ַ0x40E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S3����ADֵ������޴����ݣ��Ĵ�����ַ0x40F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//ĸ�߸��Եص�ѹ����ADֵ������޴����ݣ��Ĵ�����ַ0x410
	{
		&(g_t_share_data.t_ad_data.u16_neg_v_ad),                            //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
};												

static DC10_YX_ASSEMBLE_T m_t_yx_assemble[BS_DC10_YX_CNT] =
{
	//����һ·��ѹ���Ĵ�����ַ0x500
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0004,                                                              //u16_mask
	},
	
	//����һ·Ƿѹ���Ĵ�����ַ0x501
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0002,                                                              //u16_mask
	},
	
	//����һ·ȱ�࣬�Ĵ�����ַ0x502
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0008,                                                              //u16_mask
	},
	
	//����һ·ͣ�磬�Ĵ�����ַ0x503
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//������·��ѹ���Ĵ�����ַ0x504
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0040,                                                              //u16_mask
	},
	
	//������·Ƿѹ���Ĵ�����ַ0x505
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0020,                                                              //u16_mask
	},
	
	//������·ȱ�࣬�Ĵ�����ַ0x506
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0080,                                                              //u16_mask
	},
	
	//������·ͣ�磬�Ĵ�����ַ0x507
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_ac.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0010,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x508
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x509
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x50A
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x50B
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x50C
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x50D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x50E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x50F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//V1·��ѹ���ߣ���ĸ��ѹ����ĸ�߹�ѹ���Ĵ�����ַ0x510
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0014,                                                              //u16_mask
	},
	
	//V1·��ѹ���ͣ���ĸǷѹ����ĸ��Ƿѹ���Ĵ�����ַ0x511
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0028,                                                              //u16_mask
	},
	
	//A1·�������ߣ�����޴����ݣ��Ĵ�����ַ0x512
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//A1·�������쳣������޴����ݣ��Ĵ�����ַ0x513
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//V2·��ѹ���ߣ���ع�ѹ���Ĵ�����ַ0x514
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0040,                                                              //u16_mask
	},
	
	//V2·��ѹ���ͣ����Ƿѹ���Ĵ�����ַ0x515
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0080,                                                              //u16_mask
	},
	
	//A2·�������ߣ�һ���ع������Ĵ�����ַ0x516
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0200,                                                              //u16_mask
	},
	
	//A2·�������쳣������޴����ݣ��Ĵ�����ַ0x517
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//V3·��ѹ���ߣ���ĸ��ѹ���Ĵ�����ַ0x518
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//V3·��ѹ���ͣ���ĸǷѹ���Ĵ�����ַ0x519
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0002,                                                              //u16_mask
	},
	
	//A3·�������ߣ������ع������Ĵ�����ַ0x51A
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_dc.u16_state),               //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0400,                                                              //u16_mask
	},
	
	//A3·�������쳣������޴����ݣ��Ĵ�����ַ0x51B
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S1·�������ߣ�����޴����ݣ��Ĵ�����ַ0x51C
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S1·�������쳣������޴����ݣ��Ĵ�����ַ0x51D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S2·�������ߣ�����޴����ݣ��Ĵ�����ַ0x51E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S2·�������쳣������޴����ݣ��Ĵ�����ַ0x51F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S3·�������ߣ�����޴����ݣ��Ĵ�����ַ0x520
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//S3·�������쳣������޴����ݣ��Ĵ�����ַ0x521
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x522
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x523
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x524
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x525
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x526
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x527
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x528
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x529
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x52A
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x52B
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x52C
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x52D
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x52E
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//���ã��Ĵ�����ַ0x52F
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		0x0000,                                                              //u16_mask
	},
	
	//����01״̬���Ĵ�����ַ0x530
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[0]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����02״̬���Ĵ�����ַ0x531
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[1]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����03״̬���Ĵ�����ַ0x532
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[2]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����04״̬���Ĵ�����ַ0x533
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[3]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����05״̬���Ĵ�����ַ0x534
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[4]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����06״̬���Ĵ�����ַ0x535
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[5]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����07״̬���Ĵ�����ַ0x536
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[6]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����08״̬���Ĵ�����ַ0x537
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[7]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����09״̬���Ĵ�����ַ0x538
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[8]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����10״̬���Ĵ�����ַ0x539
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[9]),    //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����11״̬���Ĵ�����ַ0x53A
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[10]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����12״̬���Ĵ�����ַ0x53B
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[11]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����13״̬���Ĵ�����ַ0x53C
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[12]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����14״̬���Ĵ�����ַ0x53D
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[13]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����15״̬���Ĵ�����ַ0x53E
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[14]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����16״̬���Ĵ�����ַ0x53F
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[15]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},

	//����17״̬���Ĵ�����ַ0x540
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[16]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����18״̬���Ĵ�����ַ0x541
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[17]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����19״̬���Ĵ�����ַ0x542
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[18]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
	
	//����20״̬���Ĵ�����ַ0x543
	{
		&(g_t_share_data.t_rt_data.t_dc_panel.t_swt.u8_raw_swt_state[19]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},

	//һ����1#Ѳ��ģ��ͨ���жϣ��Ĵ�����ַ0x544
	{
		&(g_t_share_data.t_rt_data.t_batt.t_batt_group[0].u8_comm_state[0]),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},

	//1#����1#����ģ��ͨ���жϣ��Ĵ�����ַ0x545
	{
		&(g_t_share_data.t_rt_data.t_feeder_panel[0].t_feeder_module[0].u8_comm_state),   //pv_val
		BS_TYPE_U8,                                                          //u16_type
		0x0001,                                                              //u16_mask
	},
};

static DC10_YC_ASSEMBLE_T m_t_yt_set_assemble[BS_DC10_YT_SET_CNT] =
{
	//�������뷽ʽ�������⴦���Ĵ�����ַ0x1000
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����Ͷ��ģʽ�������⴦���Ĵ�����ַ0x1001
	{
		NULL,                                                                //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//������ѹ�������ޣ��Ĵ�����ַ0x1002
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_high_volt),           //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//������ѹ�������ޣ��Ĵ�����ַ0x1003
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_low_volt),            //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//����ȱ�����ޣ��Ĵ�����ַ0x1004
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.u16_lack_phase),          //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V1��ѹ�������ޣ���ĸ��ѹ����ĸ�߹�ѹ�������⴦�����Ĵ�����ַ0x1005
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt),        //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V1��ѹ�������ޣ���ĸǷѹ����ĸ��Ƿѹ�������⴦�����Ĵ�����ַ0x1006
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt),        //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V2��ѹ�������ޣ���ع�ѹ���Ĵ�����ַ0x1007
	{
		&(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_volt_limit),         //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V2��ѹ�������ޣ����Ƿѹ���Ĵ�����ַ0x1008
	{
		&(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_low_volt_limit),          //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V3��ѹ�������ޣ���ĸ��ѹ���Ĵ�����ַ0x1009
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_high_volt),        //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//V3��ѹ�������ޣ���ĸǷѹ���Ĵ�����ַ0x100A
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_pb_low_volt),         //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A1�������ޣ�����޴����ݣ��Ĵ�����ַ0x100B
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A2�������ޣ���ع�������ֵ�������⴦���Ĵ�����ַ0x100C
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A3�������ޣ���ع�������ֵ�������⴦���Ĵ�����ַ0x100D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//A1�������������ѹ�������⴦���Ĵ�����ַ0x100E
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A1����������̣��Ĵ�����ַ0x100F
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_load_shunt_range),    //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A2�������������ѹ�������⴦���Ĵ�����ַ0x1010
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A2����������̣��Ĵ�����ַ0x1011
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt1_shunt_range),    //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A3�������������ѹ�������⴦���Ĵ�����ַ0x1012
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//A3����������̣��Ĵ�����ַ0x1013
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_batt2_shunt_range),    //pv_val
		BS_TYPE_U16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S1�������ޣ�����޴����ݣ��Ĵ�����ַ0x1014
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//S2�������ޣ�����޴����ݣ��Ĵ�����ַ0x1015
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//S3�������ޣ�����޴����ݣ��Ĵ�����ַ0x1016
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//S1���������̣�����޴����ݣ��Ĵ�����ַ0x1017
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S2���������̣�����޴����ݣ��Ĵ�����ַ0x1018
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//S3���������̣�����޴����ݣ��Ĵ�����ַ0x1019
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//���ã�����޴����ݣ��Ĵ�����ַ0x101A
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//���ã�����޴����ݣ��Ĵ�����ַ0x101B
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//���ã�����޴����ݣ��Ĵ�����ַ0x101C
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//���ã�����޴����ݣ��Ĵ�����ַ0x101D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//���ã�����޴����ݣ��Ĵ�����ַ0x101E
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//���ã�����޴����ݣ��Ĵ�����ַ0x101F
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
	
	//�������������ѹ���Ĵ�����ַ0x1020
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_output_volt),      //pv_val
		BS_TYPE_U16,                                                         //u16_type
		10,                                                                  //u16_coeff
	},
	
	//������񣬼Ĵ�����ַ0x1021
	{
		&(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl),       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//ϵͳ���ϼ̵������ƣ��Ĵ�����ַ0x1022
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay),                      //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���01���ƣ��Ĵ�����ַ0x1023
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[0]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���02���ƣ��Ĵ�����ַ0x1024
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[1]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���03���ƣ��Ĵ�����ַ0x1025
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[2]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���04���ƣ��Ĵ�����ַ0x1026
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[3]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���05���ƣ��Ĵ�����ַ0x1027
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[4]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���06���ƣ��Ĵ�����ַ0x1028
	{
		&(g_t_share_data.t_sys_cfg.t_ctl.u8_relay[5]),                       //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���07���ƣ�����޴����ݣ��Ĵ�����ַ0x1029
	{
		NULL,                                                                //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
	
	//����̵���08���ƣ�����޴����ݣ��Ĵ�����ַ0x102A
	{
		NULL,                                                                //pv_val
		BS_TYPE_U8,                                                          //u16_type
		1,                                                                   //u16_coeff
	},
};

static DC10_YC_ASSEMBLE_T m_t_yt_adjust_assemble[BS_DC10_YT_ADJUST_CNT] =
{
	//����1·UVб�ʣ��Ĵ�����ַ0x1100
	{
		&(g_t_share_data.t_coeff_data.f32_ac1_uv_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//����1·VWб�ʣ��Ĵ�����ַ0x1101
	{
		&(g_t_share_data.t_coeff_data.f32_ac1_vw_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//����1·WUб�ʣ��Ĵ�����ַ0x1102
	{
		&(g_t_share_data.t_coeff_data.f32_ac1_wu_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//����2·UVб�ʣ��Ĵ�����ַ0x1103
	{
		&(g_t_share_data.t_coeff_data.f32_ac2_uv_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//����2·VWб�ʣ��Ĵ�����ַ0x1104
	{
		&(g_t_share_data.t_coeff_data.f32_ac2_vw_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//����2·WUб�ʣ��Ĵ�����ַ0x1105
	{
		&(g_t_share_data.t_coeff_data.f32_ac2_wu_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V1��ѹ����б�ʣ��Ĵ�����ַ0x1106
	{
		&(g_t_share_data.t_coeff_data.f32_v1_vol_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V1��ѹ������㣬�Ĵ�����ַ0x1107
	{
		&(g_t_share_data.t_coeff_data.s16_v1_vol_zero),                      //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V2��ѹ����б�ʣ��Ĵ�����ַ0x1108
	{
		&(g_t_share_data.t_coeff_data.f32_v2_vol_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V2��ѹ������㣬�Ĵ�����ַ0x1109
	{
		&(g_t_share_data.t_coeff_data.s16_v2_vol_zero),                      //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//V3��ѹ����б�ʣ��Ĵ�����ַ0x110A
	{
		&(g_t_share_data.t_coeff_data.f32_v3_vol_slope),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//V3��ѹ������㣬�Ĵ�����ַ0x110B
	{
		&(g_t_share_data.t_coeff_data.s16_v3_vol_zero),                      //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A1��·�̶���㣬�Ĵ�����ַ0x110C
	{
		&(g_t_share_data.t_coeff_data.s16_a1_fixed_zero),                    //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A2��·�̶���㣬�Ĵ�����ַ0x110D
	{
		&(g_t_share_data.t_coeff_data.s16_a2_fixed_zero),                    //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A3��·�̶���㣬�Ĵ�����ַ0x110E
	{
		&(g_t_share_data.t_coeff_data.s16_a3_fixed_zero),                    //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//A1��������б�ʣ��Ĵ�����ַ0x110F
	{
		&(g_t_share_data.t_coeff_data.f32_a1_curr_slope),                    //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//A1����������㣬�Ĵ�����ַ0x1110
	{
		&(g_t_share_data.t_coeff_data.f32_a1_curr_zero),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//A2��������б�ʣ��Ĵ�����ַ0x1111
	{
		&(g_t_share_data.t_coeff_data.f32_a2_curr_slope),                    //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//A2����������㣬�Ĵ�����ַ0x1112
	{
		&(g_t_share_data.t_coeff_data.f32_a2_curr_zero),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//A3��������б�ʣ��Ĵ�����ַ0x1113
	{
		&(g_t_share_data.t_coeff_data.f32_a3_curr_slope),                    //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//A3����������㣬�Ĵ�����ַ0x1114
	{
		&(g_t_share_data.t_coeff_data.f32_a3_curr_zero),                     //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//AD������׼��ѹ���Ĵ�����ַ0x1115
	{
		&(g_t_share_data.t_coeff_data.f32_ref_volt),                         //pv_val
		BS_TYPE_F32,                                                         //u16_type
		1000,                                                                //u16_coeff
	},

	//S1��·�̶���㣬����޴����ݣ��Ĵ�����ַ0x1116
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S2��·�̶���㣬����޴����ݣ��Ĵ�����ַ0x1117
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S3��·�̶���㣬����޴����ݣ��Ĵ�����ַ0x1118
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},

	//S1��������б�ʣ�����޴����ݣ��Ĵ�����ַ0x1119
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//S1����������㣬����޴����ݣ��Ĵ�����ַ0x111A
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//S2��������б�ʣ�����޴����ݣ��Ĵ�����ַ0x111B
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//S2����������㣬����޴����ݣ��Ĵ�����ַ0x111C
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//S3��������б�ʣ�����޴����ݣ��Ĵ�����ַ0x111D
	{
		NULL,                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//S3����������㣬����޴����ݣ��Ĵ�����ַ0x111E
	{
		NULL,                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		10,                                                                   //u16_coeff
	},

	//ĸ�߸��Եص�ѹ����б�ʣ��Ĵ�����ַ0x111F
	{
		&(g_t_share_data.t_coeff_data.f32_neg_vol_slope),                                                                //pv_val
		BS_TYPE_F32,                                                         //u16_type
		10000,                                                               //u16_coeff
	},

	//ĸ�߸��Եص�ѹ������㣬�Ĵ�����ַ0x1120
	{
		&(g_t_share_data.t_coeff_data.s16_neg_vol_zero),                                                                //pv_val
		BS_TYPE_S16,                                                         //u16_type
		1,                                                                   //u16_coeff
	},
};


/*************************************************************
��������: v_internal_dc10_handle		           				
��������: DC10Э�鴦����						
�������: u8_code         -- ������
          u16_reg         -- �Ĵ�����ַ
          u16_cnt_or_data -- �Ĵ�������������������     		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_internal_dc10_handle(U8_T u8_code, U16_T u16_reg, U16_T u16_cnt_or_data)
{
	U8_T u8_reg_index, u8_change_flag;
	U16_T u16_cnt, u16_len, u16_crc, i;
	S16_T s16_data;
		
	u8_reg_index = (u16_reg & BS_DC10_INDEX_MASK);
	
	os_mut_wait(g_mut_share_data, 0xFFFF);
		
	if (u8_code == MODBUS_FUNC_CODE_04)         //��ң����
	{
		DC10_YC_ASSEMBLE_T *pt_yc_assemble = NULL;
		U16_T u16_yc_cnt;

		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YC_DATA_OFFSET) && (u8_reg_index < BS_DC10_YC_DATA_CNT))
		{
			pt_yc_assemble = m_t_yc_data_assemble;
			u16_yc_cnt = BS_DC10_YC_DATA_CNT;
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YC_AD_OFFSET) && (u8_reg_index < BS_DC10_YC_AD_CNT))
		{
			pt_yc_assemble = m_t_yc_ad_assemble;
			u16_yc_cnt = BS_DC10_YC_AD_CNT;
		}
		else
		{
			goto OUT;
		}

		if (u16_yc_cnt - u8_reg_index < u16_cnt_or_data)
			u16_cnt = u16_yc_cnt - u8_reg_index;
		else
			u16_cnt = u16_cnt_or_data;
			
		memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
		g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
		g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_04;
		g_u8_bs_send_buf[2] = (U8_T)(u16_cnt*2);
		
		for (i=0; i<u16_cnt; i++)
		{
			if (pt_yc_assemble[u8_reg_index+i].pv_val == NULL)
				s16_data = 0;
			else if (pt_yc_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
				s16_data = (S16_T)((*(U8_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			else if (pt_yc_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
				s16_data = (S16_T)((*(U16_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			else if (pt_yc_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
				s16_data = (S16_T)((*(S16_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			else
				s16_data = (S16_T)((*(F32_T *)(pt_yc_assemble[u8_reg_index+i].pv_val)) * pt_yc_assemble[u8_reg_index+i].u16_coeff);
			
			g_u8_bs_send_buf[3+i*2] = (U8_T)(s16_data>>8);
			g_u8_bs_send_buf[4+i*2] = (U8_T)s16_data;
		}
	
		u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 3+u16_cnt*2);
    	g_u8_bs_send_buf[3+u16_cnt*2] = (U8_T)u16_crc;
		g_u8_bs_send_buf[4+u16_cnt*2] = (U8_T)(u16_crc >> 8);
		
		u16_len = 5+u16_cnt*2;
	}
	else if (u8_code == MODBUS_FUNC_CODE_02)    //��ң����
	{
		U16_T u16_yx_data[(BS_DC10_YX_CNT+15)/16];
		U16_T data;
		U32_T i;
		U8_T  word_index, bit_index, byte_cnt, rem_bit_cnt;
		
		if (((u16_reg & BS_DC10_ADDR_MASK) != BS_DC10_YX_OFFSET) ||
			(u8_reg_index >= BS_DC10_YX_CNT))
			goto OUT;
			
		memset(u16_yx_data, 0, sizeof(u16_yx_data));
		for (i=0; i<BS_DC10_YX_CNT; i++)
		{
			if (m_t_yx_assemble[i].pv_val == NULL)
				continue;

			if (m_t_yx_assemble[i].u16_type == BS_TYPE_U8)
			{
				if (((*(U8_T *)(m_t_yx_assemble[i].pv_val)) & (U8_T)(m_t_yx_assemble[i].u16_mask)) != 0)
					u16_yx_data[i/16] |= (1 << (i%16));
			}
			else if (m_t_yx_assemble[i].u16_type == BS_TYPE_U16)
			{
				if (((*(U16_T *)(m_t_yx_assemble[i].pv_val)) & m_t_yx_assemble[i].u16_mask) != 0)
					u16_yx_data[i/16] |= (1 << (i%16));
			}
		}

		if (BS_DC10_YX_CNT - u8_reg_index < u16_cnt_or_data)
			u16_cnt = BS_DC10_YX_CNT - u8_reg_index;
		else
			u16_cnt = u16_cnt_or_data;
			
		word_index = u8_reg_index / 16;
    	bit_index  = u8_reg_index % 16;
    	byte_cnt    = u16_cnt / 8;
		rem_bit_cnt = u16_cnt % 8;
		
    	if (rem_bit_cnt > 0)
			byte_cnt++;
		
		memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
		g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
		g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_02;
		g_u8_bs_send_buf[2] = byte_cnt;	
    	
		for (i=0; i<byte_cnt; i++)
		{
    	    data = u16_yx_data[word_index];
    	    g_u8_bs_send_buf[3+i] = (U8_T)(data>>bit_index);
    	
    	    if (bit_index > 8)  //����8λ���ݲ�����ȡ
    	    {
    	        data = u16_yx_data[++word_index];
    	        g_u8_bs_send_buf[3+i] |= (U8_T)(data<<(16-bit_index));
    	    }
    	    else if (bit_index==8)
    	        ++word_index;
    	
    	    bit_index = ((bit_index+8)%16);
		}
		
		if (rem_bit_cnt > 0)
			g_u8_bs_send_buf[2+byte_cnt] &= ((1<<rem_bit_cnt) - 1);
			
		u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, byte_cnt+3);
		g_u8_bs_send_buf[byte_cnt+3] = (U8_T)u16_crc;
		g_u8_bs_send_buf[byte_cnt+4] = (U8_T)(u16_crc >> 8);
		
		u16_len = byte_cnt+5;
	}
	else if (u8_code == MODBUS_FUNC_CODE_03)    //�����üĴ���
	{
		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_SET_OFFSET) && (u8_reg_index < BS_DC10_YT_SET_CNT))
		{
			if (BS_DC10_YT_SET_CNT - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_SET_CNT - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_03;
			g_u8_bs_send_buf[2] = (U8_T)(u16_cnt*2);
		
			for (i=0; i<u16_cnt; i++)
			{
				if ((u8_reg_index+i) == BS_DC10_AC_INPUT_MODE_INDEX)
				{
					s16_data = ((g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path << 8) |
								g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase);
				}
				else if ((u8_reg_index+i) == BS_DC10_AC_CTRL_MODE_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac == 1)
						s16_data = 2;
					else if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac == 2)
						s16_data = 1;
					else
						s16_data = g_t_share_data.t_sys_cfg.t_ctl.u16_ac;
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_HIGH_VOLT_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt * 10);
					else
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt * 10);
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_LOW_VOLT_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt * 10);
					else
						s16_data = (U16_T)(g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt * 10);
				}
				else if (((u8_reg_index+i) == BS_DC10_A2_OVER_CURR_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A3_OVER_CURR_INDEX))
				{
					s16_data = g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit * g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10;
				}
				else if (((u8_reg_index+i) == BS_DC10_A1_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A2_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A3_SHUNT_VOLT_INDEX))
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt == VOLT_75MV)
						s16_data = 75;
					else
						s16_data = 50;
				}
				else
				{
					if (m_t_yt_set_assemble[u8_reg_index+i].pv_val == NULL)
						s16_data = 0;
					else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
						s16_data = (S16_T)((*(U8_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
					else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
						s16_data = (S16_T)((*(U16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
					else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
						s16_data = (S16_T)((*(S16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
					else
						s16_data = (S16_T)((*(F32_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val)) * m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
				}
			
				g_u8_bs_send_buf[3+i*2] = (U8_T)(s16_data>>8);
				g_u8_bs_send_buf[4+i*2] = (U8_T)s16_data;
			}
	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 3+u16_cnt*2);
    		g_u8_bs_send_buf[3+u16_cnt*2] = (U8_T)u16_crc;
			g_u8_bs_send_buf[4+u16_cnt*2] = (U8_T)(u16_crc >> 8);
		
			u16_len = 5+u16_cnt*2;
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_ADJUST_OFFSET) && (u8_reg_index < BS_DC10_YT_ADJUST_CNT))
		{
			if (BS_DC10_YT_ADJUST_CNT - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_ADJUST_CNT - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_03;
			g_u8_bs_send_buf[2] = (U8_T)(u16_cnt*2);
		
			for (i=0; i<u16_cnt; i++)
			{
				if (m_t_yt_adjust_assemble[u8_reg_index+i].pv_val == NULL)
					s16_data = 0;
				else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
					s16_data = (S16_T)((*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
				else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
					s16_data = (S16_T)((*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
				else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
					s16_data = (S16_T)((*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
				else
					s16_data = (S16_T)((*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val)) * m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
			
				g_u8_bs_send_buf[3+i*2] = (U8_T)(s16_data>>8);
				g_u8_bs_send_buf[4+i*2] = (U8_T)s16_data;
			}
	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 3+u16_cnt*2);
    		g_u8_bs_send_buf[3+u16_cnt*2] = (U8_T)u16_crc;
			g_u8_bs_send_buf[4+u16_cnt*2] = (U8_T)(u16_crc >> 8);
		
			u16_len = 5+u16_cnt*2;
		}
		else
		{
			goto OUT;
		}
	}
	else if (u8_code == MODBUS_FUNC_CODE_06)    //���õ�������
	{
		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_SET_OFFSET) && (u8_reg_index < BS_DC10_YT_SET_CNT))
		{
			u8_change_flag = 0;
			
			if (u8_reg_index == BS_DC10_AC_INPUT_MODE_INDEX)
			{
				if (((u16_cnt_or_data>>8) <= 2) && ((u16_cnt_or_data&0x00FF) <= 1))
				{
					if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path != (AC_INPUT_NUM_E)(u16_cnt_or_data>>8)) ||
						(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase != (AC_INPUT_TYPE_E)u16_cnt_or_data))
					{
						u8_change_flag = 1;
					}

					g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = (AC_INPUT_NUM_E)(u16_cnt_or_data>>8);
					g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase = (AC_INPUT_TYPE_E)u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_AC_CTRL_MODE_INDEX)
			{
				if (u16_cnt_or_data == 1)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 2)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 2;
				}
				else if (u16_cnt_or_data == 2)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 1)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 1;
				}
				else if (u16_cnt_or_data <= 3)
				{
					if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != u16_cnt_or_data)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_ctl.u16_ac = u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_V1_HIGH_VOLT_INDEX)
			{
				if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt = u16_cnt_or_data / 10;
				}
				else
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt = u16_cnt_or_data / 10;
				}
			}
			else if (u8_reg_index == BS_DC10_V1_LOW_VOLT_INDEX)
			{	
				if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt = u16_cnt_or_data / 10;
				}
				else
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt != u16_cnt_or_data / 10)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt = u16_cnt_or_data / 10;
				}
			}
			else if ((u8_reg_index == BS_DC10_A2_OVER_CURR_INDEX) ||
					 (u8_reg_index == BS_DC10_A3_OVER_CURR_INDEX))
			{
				if (fabs(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit-((F32_T)((S16_T)u16_cnt_or_data)) / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10)) > 0.000001)
					u8_change_flag = 1;
				
				g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit = ((F32_T)((S16_T)u16_cnt_or_data)) / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10);
			}
			else if ((u8_reg_index == BS_DC10_A1_SHUNT_VOLT_INDEX) ||
					 (u8_reg_index == BS_DC10_A2_SHUNT_VOLT_INDEX) ||
					 (u8_reg_index == BS_DC10_A3_SHUNT_VOLT_INDEX))
			{
				if (u16_cnt_or_data == 75)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_75MV)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_75MV;
				}
				else if (u16_cnt_or_data == 50)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_50MV)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_50MV;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_DIODE_CHAIN_INDEX)
			{
				if (u16_cnt_or_data <= 4)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != (DIODE_CHAIN_E)u16_cnt_or_data)
						u8_change_flag = 1;
				
					g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl = (DIODE_CHAIN_E)u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if (u8_reg_index == BS_DC10_RELAY_START_INDEX)
			{
				if (u16_cnt_or_data == 0)
				{
					v_relay_relay_operation(SYS_FIAL_ON);
					g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = u16_cnt_or_data;
				}
				else if (u16_cnt_or_data == 1)
				{
					v_relay_relay_operation(SYS_FIAL_OFF);
					g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else if ((u8_reg_index > BS_DC10_RELAY_START_INDEX) &&
					 (u8_reg_index < (BS_DC10_YT_SET_CNT - 2)))
			{
				if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != NO_DIODE_CHAIN)
					&& (((u8_reg_index-BS_DC10_RELAY_START_INDEX) == 4) || 
						((u8_reg_index-BS_DC10_RELAY_START_INDEX) == 5) ||
						((u8_reg_index-BS_DC10_RELAY_START_INDEX) == 6)))
				{
					goto OUT;
				}
        	
				if (u16_cnt_or_data == 0)
				{
					RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_OFF, ALARM_RELAY2_OFF, ALARM_RELAY3_OFF, ALARM_RELAY4_OFF,
												ALARM_RELAY5_OFF, ALARM_RELAY6_OFF };
	    	                   
					v_relay_relay_operation(e_cmd[u8_reg_index-BS_DC10_RELAY_START_INDEX-1]);
					g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index-BS_DC10_RELAY_START_INDEX-1] = u16_cnt_or_data;
				}
				else if (u16_cnt_or_data == 1)
				{
					RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_ON, ALARM_RELAY2_ON, ALARM_RELAY3_ON, ALARM_RELAY4_ON,
												ALARM_RELAY5_ON, ALARM_RELAY6_ON };
	    	                   
					v_relay_relay_operation(e_cmd[u8_reg_index-BS_DC10_RELAY_START_INDEX-1]);
					g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index-BS_DC10_RELAY_START_INDEX-1] = u16_cnt_or_data;
				}
				else
				{
					goto OUT;
				}
			}
			else
			{
				if (m_t_yt_set_assemble[u8_reg_index].pv_val != NULL)
				{
					if (m_t_yt_set_assemble[u8_reg_index].u16_type == BS_TYPE_U8)
					{
						if (*(U8_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) != (U8_T)(u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff))
							u8_change_flag = 1;
						
						*(U8_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = (U8_T)(u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff);
					}
					else if (m_t_yt_set_assemble[u8_reg_index].u16_type == BS_TYPE_U16)
					{
						if (*(U16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) != u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff)
							u8_change_flag = 1;
						
						*(U16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff;
					}
					else if (m_t_yt_set_assemble[u8_reg_index].u16_type == BS_TYPE_S16)
					{
						if (*(S16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) != (S16_T)u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff)
							u8_change_flag = 1;
						
						*(S16_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = (S16_T)u16_cnt_or_data / m_t_yt_set_assemble[u8_reg_index].u16_coeff;
					}
					else
					{
						if (fabs(*(F32_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) - ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_set_assemble[u8_reg_index].u16_coeff) > 0.000001)
							u8_change_flag = 1;
						
						*(F32_T *)(m_t_yt_set_assemble[u8_reg_index].pv_val) = ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_set_assemble[u8_reg_index].u16_coeff;
					}
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_cfg_data();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_06;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8;
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_ADJUST_OFFSET) && (u8_reg_index < BS_DC10_YT_ADJUST_CNT))
		{
			if (m_t_yt_adjust_assemble[u8_reg_index].pv_val != NULL)
			{
				if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_U8)
				{
					if (*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != (U8_T)(u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff))
						u8_change_flag = 1;
					
					*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = (U8_T)(u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff);
				}
				else if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_U16)
				{
					if (*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff)
						u8_change_flag = 1;
					
					*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
				}
				else if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_S16)
				{
					if (*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != (S16_T)u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff)
						u8_change_flag = 1;
					
					*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = (S16_T)u16_cnt_or_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
				}
				else
				{
					if (fabs(*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) - ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff) > 0.000001)
						u8_change_flag = 1;
					
					*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = ((F32_T)((S16_T)u16_cnt_or_data)) / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_adjust_coeff();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_06;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8;
		}
		else
		{
			goto OUT;
		}
	}
	else if (u8_code == MODBUS_FUNC_CODE_16)    //���ö������
	{
		if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_SET_OFFSET) && (u8_reg_index < BS_DC10_YT_SET_CNT))
		{	
			if (BS_DC10_YT_SET_OFFSET - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_SET_OFFSET - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			u8_change_flag = 0;
			
			for (i=0; i<u16_cnt; i++)
			{
				s16_data = (S16_T)((g_u8_bs_recv_buf[7+i*2]<<8) + g_u8_bs_recv_buf[8+i*2]);
				
				if ((u8_reg_index+i) == BS_DC10_AC_INPUT_MODE_INDEX)
				{
					if (((s16_data>>8) <= 2) && ((s16_data&0x00FF) <= 1))
					{
						if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path != (AC_INPUT_NUM_E)(s16_data>>8)) ||
							(g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase != (AC_INPUT_TYPE_E)s16_data))
						{
							u8_change_flag = 1;
						}
							
						g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_path = (AC_INPUT_NUM_E)(s16_data>>8);
						g_t_share_data.t_sys_cfg.t_dc_panel.t_ac.e_phase = (AC_INPUT_TYPE_E)s16_data;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_AC_CTRL_MODE_INDEX)
				{
					if (s16_data == 1)
					{
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 2)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 2;
					}
					else if (s16_data == 2)
					{
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != 1)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_ctl.u16_ac = 1;
					}
					else if (s16_data <= 3)
					{
						if (g_t_share_data.t_sys_cfg.t_ctl.u16_ac != s16_data)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_ctl.u16_ac = s16_data;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_HIGH_VOLT_INDEX)
				{
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_high_volt = s16_data / 10;
					}
					else
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_high_volt = s16_data / 10;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_V1_LOW_VOLT_INDEX)
				{	
					if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_have_cb == HAVE)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_cb_low_volt = s16_data / 10;
					}
					else
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt != (s16_data / 10))
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.u16_bus_low_volt = s16_data / 10;
					}
				}
				else if (((u8_reg_index+i) == BS_DC10_A2_OVER_CURR_INDEX) ||
					 	 ((u8_reg_index+i) == BS_DC10_A3_OVER_CURR_INDEX))
				{
					if (fabs(g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit - (F32_T)s16_data / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10)) > 0.0001)
						u8_change_flag = 1;
					
					g_t_share_data.t_sys_cfg.t_batt_mgmt.f32_high_curr_limit = (F32_T)s16_data / (g_t_share_data.t_sys_cfg.t_batt_mgmt.u16_rate_c10 * 10);
				}
				else if (((u8_reg_index+i) == BS_DC10_A1_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A2_SHUNT_VOLT_INDEX) ||
						 ((u8_reg_index+i) == BS_DC10_A3_SHUNT_VOLT_INDEX))
				{
					if (s16_data == 75)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_75MV)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_75MV;
					}
					else if (s16_data == 50)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt != VOLT_50MV)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_shunt_rated_volt = VOLT_50MV;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_DIODE_CHAIN_INDEX)
				{
					if (s16_data <= 4)
					{
						if (g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != (DIODE_CHAIN_E)s16_data)
							u8_change_flag = 1;
						
						g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl = (DIODE_CHAIN_E)s16_data;
					}
				}
				else if ((u8_reg_index+i) == BS_DC10_RELAY_START_INDEX)
				{
					if (s16_data == 0)
					{
						v_relay_relay_operation(SYS_FIAL_ON);
						g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = s16_data;
					}
					else if (s16_data == 1)
					{
						v_relay_relay_operation(SYS_FIAL_OFF);
						g_t_share_data.t_sys_cfg.t_ctl.u8_sys_relay = s16_data;
					}
				}
				else if (((u8_reg_index+i) > BS_DC10_RELAY_START_INDEX) &&
						 ((u8_reg_index+i) < (BS_DC10_YT_SET_CNT - 2)))
				{
					if ((g_t_share_data.t_sys_cfg.t_dc_panel.t_dc.e_diode_chain_ctl != NO_DIODE_CHAIN)
					&& (((u8_reg_index+i-BS_DC10_RELAY_START_INDEX) == 4) || 
						((u8_reg_index+i-BS_DC10_RELAY_START_INDEX) == 5) ||
						((u8_reg_index+i-BS_DC10_RELAY_START_INDEX) == 6)))
					{
						continue;
					}
        	
					if (s16_data == 0)
					{
						RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_OFF, ALARM_RELAY2_OFF, ALARM_RELAY3_OFF, ALARM_RELAY4_OFF,
												ALARM_RELAY5_OFF, ALARM_RELAY6_OFF };
	    	                   
						v_relay_relay_operation(e_cmd[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1]);
						g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1] = s16_data;
					}
					else if (s16_data == 1)
					{
						RLY_CMD_E e_cmd[6] = { ALARM_RELAY1_ON, ALARM_RELAY2_ON, ALARM_RELAY3_ON, ALARM_RELAY4_ON,
												ALARM_RELAY5_ON, ALARM_RELAY6_ON };
	    	                   
						v_relay_relay_operation(e_cmd[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1]);
						g_t_share_data.t_sys_cfg.t_ctl.u8_relay[u8_reg_index+i-BS_DC10_RELAY_START_INDEX-1] = s16_data;
					}
				}
				else
				{
					if (m_t_yt_set_assemble[u8_reg_index+i].pv_val != NULL)
					{
						if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
						{
							if (*(U8_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) != (U8_T)(s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff))
								u8_change_flag = 1;
							
							*(U8_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) = (U8_T)(s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff);
						}
						else if (m_t_yt_set_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
						{
							if (*(U16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) != s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff)
								u8_change_flag = 1;
							
							*(U16_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) = s16_data / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff;
						}
						else if (m_t_yt_adjust_assemble[u8_reg_index].u16_type == BS_TYPE_S16)
						{
							if (*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) != s16_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff)
								u8_change_flag = 1;
					
							*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index].pv_val) = s16_data / m_t_yt_adjust_assemble[u8_reg_index].u16_coeff;
						}
						else
						{
							if (fabs(*(F32_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) - ((F32_T)s16_data) / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff) > 0.000001)
								u8_change_flag = 1;
							
							*(F32_T *)(m_t_yt_set_assemble[u8_reg_index+i].pv_val) = ((F32_T)s16_data) / m_t_yt_set_assemble[u8_reg_index+i].u16_coeff;
						}
					}
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_cfg_data();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_16;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8; 
		}
		else if (((u16_reg & BS_DC10_ADDR_MASK) == BS_DC10_YT_ADJUST_OFFSET) && (u8_reg_index < BS_DC10_YT_ADJUST_CNT))
		{
			if (BS_DC10_YT_ADJUST_CNT - u8_reg_index < u16_cnt_or_data)
				u16_cnt = BS_DC10_YT_ADJUST_CNT - u8_reg_index;
			else
				u16_cnt = u16_cnt_or_data;
			
			u8_change_flag = 0;
			
			for (i=0; i<u16_cnt; i++)
			{
				s16_data = (S16_T)((g_u8_bs_recv_buf[7+i*2]<<8) + g_u8_bs_recv_buf[8+i*2]);

				if (m_t_yt_adjust_assemble[u8_reg_index+i].pv_val != NULL)
				{
					if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U8)
					{
						if (*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) != (U8_T)(s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff))
							u8_change_flag = 1;
						
						*(U8_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = (U8_T)(s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff);
					}
					else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_U16)
					{
						if (*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) != s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff)
							u8_change_flag = 1;
						
						*(U16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff;
					}
					else if (m_t_yt_adjust_assemble[u8_reg_index+i].u16_type == BS_TYPE_S16)
					{
						if (*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) != s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff)
							u8_change_flag = 1;
						
						*(S16_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = s16_data / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff;
					}
					else
					{
						if (fabs(*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) - ((F32_T)s16_data) / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff) > 0.000001)
							u8_change_flag = 1;
						
						*(F32_T *)(m_t_yt_adjust_assemble[u8_reg_index+i].pv_val) = ((F32_T)s16_data) / m_t_yt_adjust_assemble[u8_reg_index+i].u16_coeff;
					}
				}
			}
			
			if (u8_change_flag == 1)
				v_fetch_save_adjust_coeff();
			
			memset(g_u8_bs_send_buf, 0, sizeof(g_u8_bs_send_buf));
	    	
			g_u8_bs_send_buf[0] = DC10_START_ADDR + g_u8_addr - 1;
			g_u8_bs_send_buf[1] = MODBUS_FUNC_CODE_16;
			g_u8_bs_send_buf[2] = (U8_T)(u16_reg>>8);
			g_u8_bs_send_buf[3] = (U8_T)u16_reg;
			g_u8_bs_send_buf[4] = (U8_T)(u16_cnt_or_data>>8);
			g_u8_bs_send_buf[5] = (U8_T)u16_cnt_or_data;
	    	
			u16_crc = u16_crc_calculate_crc(g_u8_bs_send_buf, 6);
   			g_u8_bs_send_buf[6] = (U8_T)u16_crc;
			g_u8_bs_send_buf[7] = (U8_T)(u16_crc >> 8);
			
			u16_len = 8; 
		}
		else
		{
			goto OUT;
		}
	}
	else
	{
		goto OUT;
	}
	
	os_mut_release(g_mut_share_data);
	
	v_bs_send_data(g_u8_bs_send_buf, u16_len);
	return;

OUT:
	os_mut_release(g_mut_share_data);	
}

/*************************************************************
��������: v_internal_run		           				
��������: �ڲ�modubsЭ�����к���						
�������: ��     		   				
�������: ��
����ֵ  ����														   				
**************************************************************/
void v_internal_run(void)
{
	U32_T i, h;
	S32_T cnt = 0;
	U16_T u16_reg_addr, u16_reg_cnt_or_data;
	U8_T  u8_func_code;

	cnt = Uart_recv(BS_COM_PORT_NUM, g_u8_bs_recv_buf+g_u8_bs_recv_len, BS_RECV_BUF_SIZE-g_u8_bs_recv_len);
	if (cnt == -1)
		return;
			
	g_u8_bs_recv_len += cnt;	
	h = 0;
	
	//����ͷβ��־�Ա�ȡ��һ���������ݰ�
	while (g_u8_bs_recv_len >= MODBUS_BYTE_SIZE)
	{
		/*��ַ�����*/
		if(g_u8_bs_recv_buf[h] != (DC10_START_ADDR + g_u8_addr - 1))		/*��ַ�������ͷָ��ǰ��һ��λ��*/
		{
			h++;
			g_u8_bs_recv_len--;
			continue;
		}
		
		//�����벻�����ͷָ��ǰ��һ��λ��
		if ((g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_02) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_04) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_03) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_06) &&
			(g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_16))
		{
			h++;
			g_u8_bs_recv_len--;
			continue;
		}		
	
		//�Ƚ�У����
		if (g_u8_bs_recv_buf[h+1] != MODBUS_FUNC_CODE_16)
		{
			if (u16_crc_calculate_crc(&g_u8_bs_recv_buf[h], MODBUS_BYTE_SIZE) != 0)
			{
				h++;
				g_u8_bs_recv_len--;
				continue;
			}
		}
		else
		{
			//ֻ��DC10��16�����룬16��������Ҫ��һЩ������������ַ��λΪ0x10���ֽ������ڼĴ�����������2���ֽ���С��86
			if (((g_u8_bs_recv_buf[h+2] != 0x10) && (g_u8_bs_recv_buf[h+2] != 0x11)) ||
				(((g_u8_bs_recv_buf[h+4]<<8)+g_u8_bs_recv_buf[h+5])*2 != g_u8_bs_recv_buf[h+6]) ||
				(g_u8_bs_recv_buf[h+6] > 86))
			{
				h++;
				g_u8_bs_recv_len--;
				continue;
			}
				
			if ((g_u8_bs_recv_buf[h+6]+9) > g_u8_bs_recv_len)
			{
				if (h > 0)
				{
					for (i=0; i<g_u8_bs_recv_len; i++)
						g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
				}
					
				break;
			}
			else if (u16_crc_calculate_crc(&g_u8_bs_recv_buf[h], g_u8_bs_recv_buf[h+6]+9) != 0)
			{
				h++;
				g_u8_bs_recv_len--;
				continue;
			}
		}
			
		if (h > 0)
		{
			for (i=0; i<g_u8_bs_recv_len; i++)
				g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
			
			h = 0;
		}

		u8_func_code = g_u8_bs_recv_buf[1];
		u16_reg_addr = (g_u8_bs_recv_buf[2]<<8) + g_u8_bs_recv_buf[3];
		u16_reg_cnt_or_data = (g_u8_bs_recv_buf[4]<<8) + g_u8_bs_recv_buf[5];
		DPRINT("modbus, u8_func_code=%x, u16_reg_addr=%x, cnt=%x\n", u8_func_code, u16_reg_addr, u16_reg_cnt);
		
		//��ΪУ׼Э��ʱ����	
		v_internal_dc10_handle(u8_func_code, u16_reg_addr, u16_reg_cnt_or_data);
					
		if (u8_func_code != MODBUS_FUNC_CODE_16)
		{
			h += MODBUS_BYTE_SIZE;
			g_u8_bs_recv_len -= MODBUS_BYTE_SIZE;
		}
		else
		{
			h += (g_u8_bs_recv_buf[6]+9);
			g_u8_bs_recv_len -= (g_u8_bs_recv_buf[6]+9);
		}
			
		if (h > 0)
		{
			for (i=0; i<g_u8_bs_recv_len; i++)
				g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
				
			h = 0;
		}
	}

	if (h > 0)
	{
		for (i=0; i<g_u8_bs_recv_len; i++)
			g_u8_bs_recv_buf[i] = g_u8_bs_recv_buf[h+i];
	}
}

